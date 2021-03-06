#include "athena/polygonizer/CrossSection.hpp"
#include "athena/polygonizer/Hash.hpp"
#include "athena/polygonizer/Tables.hpp"
#include "athena/Athena.hpp"

#include <atlas/core/Float.hpp>
#include <atlas/core/Log.hpp>
#include <atlas/core/Assert.hpp>

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#if defined ATLAS_DEBUG
#define ATHENA_DEBUG_CONTOURS 0 
#endif

namespace athena
{
    namespace polygonizer
    {
        CrossSection::CrossSection(SlicingAxes const& axis, 
            atlas::math::Point const& min, atlas::math::Point const& max, 
            std::uint32_t gridSize, std::uint32_t svSize, float isoValue,
            tree::BlobTree* tree) :
            mMin(min),
            mMax(max),
            mGridSize(gridSize),
            mSvSize(svSize),
            mMagic(isoValue),
            mShadowMagic(0.1f),
            mTree(tree),
            mAxis(axis),
            mLargestContourSize(0)
        {
            using atlas::math::Normal;

            ATLAS_ASSERT(gridSize % svSize == 0,
                "Grid size must be a multiple of super-voxel grid size.");

            auto const& start = min;
            auto const& end = max;

            mGridDelta = (end - start) / static_cast<float>(mGridSize);
            mSvDelta = (end - start) / static_cast<float>(mSvSize);

            // Set our normal.
            switch (axis)
            {
            case SlicingAxes::XAxis:
                mNormal = Normal(start.x, 0, 0);
                mAxisId = glm::uvec2(1, 2);
                break;

            case SlicingAxes::YAxis:
                mNormal = Normal(0, start.y, 0);
                mAxisId = glm::uvec2(0, 2);
                break;

            case SlicingAxes::ZAxis:
                mNormal = Normal(0, 0, start.z);
                mAxisId = glm::uvec2(0, 1);
                break;
            }

            mUnitNormal = glm::normalize(mNormal);
        }


        void CrossSection::constructLattice()
        {
            using atlas::math::Point;
            using atlas::utils::BBox;

            // Can should be done in parallel by having each super-voxel be a
            // separate task.
            for (std::uint32_t x = 0; x < mSvSize; ++x)
            {
                for (std::uint32_t y = 0; y < mSvSize; ++y)
                {
                    auto pt = createCellPoint(x, y, mSvDelta);

                    // Construct the cell that corresponds to the super-voxel.
                    BBox cell(pt, pt + mSvDelta);

                    SuperVoxel sv;
                    sv.field = mTree->getSubTree(cell);
                    sv.id = { x, y };

                    if (sv.field)
                    {
                        auto idx = BsoidHash32::hash(x, y);
                        mSuperVoxels[idx] = sv;
                    }
                }
            }

            // This can also be done in parallel (provided the number of seeds
            // is sufficiently large (could be based on the number of cores).
            auto seedPoints = mTree->getSeeds(mNormal);
            std::vector<Voxel> seedVoxels;
            for (auto& pt : seedPoints)
            {
                auto v = (pt - mMin) / mGridDelta;
                PointId id;
                id.x = static_cast<std::uint32_t>(v[mAxisId.x]);
                id.y = static_cast<std::uint32_t>(v[mAxisId.y]);
                seedVoxels.emplace_back(id);
            }

            marchVoxelOnSurface(seedVoxels);

#if defined ATLAS_DEBUG
            validateVoxels();
#endif
        }

        void CrossSection::constructContour()
        {
            auto segments = generateLineSegments(mVoxels);
            auto contours = convertToContour(segments);

            mContours.insert(mContours.end(),
                contours.begin(), contours.end());

           // Grab the size of the largest contour.
           for (auto& contour : mContours)
           {
               if (mLargestContourSize < contour.size())
               {
                   mLargestContourSize = contour.size();
               }
           }

#if defined ATLAS_DEBUG
             validateContour();
#endif
        }

        void CrossSection::resizeContours(std::size_t size)
        {
            // If we only have a single vertex, then return.
            if (mLargestContourSize == 1)
            {
                return;
            }

            // Now let's loop through our contours and see which ones
            // need to be resized.
            int i = 0;
            for (auto& contour : mContours)
            {
                if (contour.size() <= size)
                {
                    subdivideContour(i, size);
                }
                ++i;
            }
        }

        std::vector<FieldPoint> CrossSection::findShadowPoints()
        {
            // First we need to march the voxels inside the surface to find
            // the ones that contain shadow points.
            auto shadowVoxels = findShadowVoxels();

            // Once we have the shadow voxels we need to convert them into their
            // corresponding line segments.
            auto shadowSegments = generateShadowSegments(shadowVoxels);

            // Finally, convert into contours and return.
            auto shadowContours = convertToContour(shadowSegments);

#if defined(ATHENA_USE_GUI)
            // Only update our contours and lattices if there is a GUI running.
            mVoxels.insert(mVoxels.end(), shadowVoxels.begin(),
                shadowVoxels.end());
            mContours.insert(mContours.end(), shadowContours.begin(),
                shadowContours.end());
#endif

            // TODO: Change this later on.
            return shadowContours[0];
        }

        std::vector<Voxel> const& CrossSection::getVoxels() const
        {
            return mVoxels;
        }

        std::vector<std::vector<FieldPoint>> const& 
            CrossSection::getContour() const
        {
            return mContours;
        }

        std::size_t CrossSection::getLargestContourSize() const
        {
            return mLargestContourSize;
        }

        atlas::math::Normal CrossSection::getNormal() const
        {
            return mNormal;
        }

        std::pair<std::uint32_t, std::uint32_t> CrossSection::getResolutions() const
        {
            return { mGridSize, mSvSize };
        }

        atlas::math::Point CrossSection::createCellPoint(std::uint32_t x,
            std::uint32_t y, atlas::math::Point const& delta)
        {
            atlas::math::Point pt;
            auto const& start = mMin;

            pt = start;
            // Figure out if there is a way to convert this to vector code.
            pt[mAxisId.x] = start[mAxisId.x] + x * delta[mAxisId.x];
            pt[mAxisId.y] = start[mAxisId.y] + y * delta[mAxisId.y];

            return pt;
        }

        atlas::math::Point CrossSection::createCellPoint(glm::u32vec2 const& p,
            atlas::math::Point const& delta)
        {
            return createCellPoint(p.x, p.y, delta);
        }

        FieldPoint CrossSection::findVoxelPoint(PointId const& id)
        {
            using atlas::math::Point4;
            using atlas::math::Point;

            // First check if we have seen this point before.
            auto entry = mSeenVoxelPoints.find(BsoidHash32::hash(id.x, id.y));
            if (entry != mSeenVoxelPoints.end())
            {
                // We have seen it, return the point.
                return (*entry).second;
            }
            else
            {
                // We haven't so first convert the id to an actual point.
                auto pt = createCellPoint(id, mGridDelta);

                // Now let's figure out which super-voxel contained the point.
                PointId svId;
                {
                    auto v = (pt - mMin) / mSvDelta;
                    svId.x = static_cast<std::uint32_t>(v[mAxisId.x]);
                    svId.y = static_cast<std::uint32_t>(v[mAxisId.y]);

                    // Check if either of the coordinates of the id are beyond
                    // the edge of the grid.
                    svId.x = (svId.x < mSvSize) ? svId.x : svId.x - 1;
                    svId.y = (svId.y < mSvSize) ? svId.y : svId.y - 1;
                }

                // Now that we have the id, let's evaluate the point.
                FieldPoint fp;
                {
                    auto svHash = BsoidHash32::hash(svId.x, svId.y);
                    SuperVoxel sv = mSuperVoxels[svHash];
                    auto val = sv.eval(pt);
                    auto g = sv.grad(pt);
                    fp = { pt, val, g, svHash };
                }

                // Now that we have the point, let's add it to our list and
                // return it.
                auto hash = BsoidHash32::hash(id.x, id.y);
                mSeenVoxelPoints.insert(
                    std::pair<std::uint32_t, FieldPoint>(hash, fp));
                return fp;
            }
        }

        void CrossSection::fillVoxel(Voxel& v)
        {
            int d = 0;
            for (auto& decal : VoxelDecals)
            {
                auto decalId = v.id + decal;
                v.points[d] = findVoxelPoint(decalId);
                ++d;
            }
        }

        bool CrossSection::seenVoxel(VoxelId const& id)
        {
            if (mSeenVoxels.find(BsoidHash32::hash(id.x, id.y)) !=
                mSeenVoxels.end())
            {
                return true;
            }
            else
            {
                mSeenVoxels.insert(
                    std::pair<std::uint32_t, VoxelId>(
                        BsoidHash32::hash(id.x, id.y), id));
                return false;
            }
        }

        void CrossSection::marchVoxelOnSurface(std::vector<Voxel> const& seeds)
        {
            using atlas::math::Point4;
            using atlas::math::Point;

            std::queue<PointId> frontier;

            auto getEdges = [this](Voxel const& v)
            {
                FieldPoint start, end;
                int edgeId = 0;
                std::vector<int> edges;

                for (std::size_t i = 0; i < v.points.size(); ++i)
                {
                    start = v.points[i];
                    end = v.points[(i + 1) % v.points.size()];
                    float val1 = start.value.w - mMagic;
                    float val2 = end.value.w - mMagic;

                    // All that we care about is the change in sign. If there
                    // is a change, we know the surface crosses this edge.
                    if (glm::sign(val1) != glm::sign(val2))
                    {
                        edges.push_back(edgeId);
                    }
                    edgeId++;
                }

                return edges;
            };

            if (seeds.empty())
            {
                DEBUG_LOG_V("Cross-section (%f, %f, %f) exiting on empty seeds.",
                    mNormal.x, mNormal.y, mNormal.z);
                return;
            }

            // Before anything else happens, we need to ensure that the seed 
            // voxels are actually on the surface itself.
            {
                using atlas::math::Point2;
                auto containsSurface = [this, getEdges](Voxel const& v)
                {
                    // First fill in the voxel points.
                    Voxel voxel = v;
                    fillVoxel(voxel);

                    // Now that we have them, let's check to see if the voxel
                    // is indeed in the surface.
                    auto edges = getEdges(voxel);
                    return !edges.empty();
                };

                auto findSurface = [this, containsSurface](Voxel const& v)
                {
                    bool found = false;
                    Voxel last, current;
                    current = v;

                    while (!found)
                    {
                        auto cPos = (2u * v.id) + glm::u32vec2(1, 1);
                        Point origin = createCellPoint(cPos, mGridDelta / 2.0f);
                        float originVal = mTree->eval(origin);
                        auto norm = mTree->grad(origin);
                        auto projNorm = norm - glm::proj(norm, glm::normalize(mNormal));
                        projNorm = glm::normalize(projNorm);
                        projNorm = (originVal > mMagic) ? -projNorm : projNorm;

                        // Now find the voxel that we are pointing to.
                        auto absNorm = glm::abs(projNorm);
                        Point2 next;
                        if (absNorm[mAxisId.x] > absNorm[mAxisId.y])
                        {
                            // We need to move along the x axis.
                            next = (projNorm[mAxisId.x] > 0.0f) ? Point2(1, 0) :
                                Point2(-1, 0);
                        }
                        else if (absNorm[mAxisId.y] > absNorm[mAxisId.x])
                        {
                            // We need to move along the y axis.
                            next = (projNorm[mAxisId.y] > 0.0f) ? Point2(0, 1) :
                                Point2(0, -1);
                        }
                        else
                        {
                            // They are both equal, so move along the diagonal
                            // depending on the value of x and y.
                            next.x = (projNorm[mAxisId.x] > 0.0f) ? 1 : -1;
                            next.y = (projNorm[mAxisId.y] > 0.0f) ? 1 : -1;
                        }

                        current.id.x += static_cast<std::uint32_t>(next.x);
                        current.id.y += static_cast<std::uint32_t>(next.y);

                        // Check if the voxel hasn't run off the edge of the grid.
                        if (!validVoxel(current))
                        {
                            break;
                        }

                        // Check if the new voxel contains the surface.
                        if (containsSurface(current))
                        {
                            break;
                        }
                    }
                    return current;
                };

                for (auto& seed : seeds)
                {
                    auto v = seed;
                    if (!containsSurface(seed))
                    {
                        v = findSurface(v);
                        if (!validVoxel(v))
                        {
                            continue;
                        }
                    }
                    frontier.push(v.id);
                }
            }

            if (frontier.empty())
            {
                DEBUG_LOG("Exiting on empty queue.");
                return;
            }

            while (!frontier.empty())
            {
                // Grab a voxel from the queue.
                auto top = frontier.front();
                frontier.pop();

                // Check if we have seen this voxel before.
                if (seenVoxel(top))
                {
                    continue;
                }

                // Now fill its values.
                Voxel v(top);
                fillVoxel(v);

                // Check how many edges cross the surface.
                auto edges = getEdges(v);
                if (edges.empty())
                {
                    // There are no crossings, so continue;
                    continue;
                }

                // For each edge that crosses the surface, we need to add the 
                // corresponding voxel to our queue.
                for (auto& edge : edges)
                {
                    // Grab the decal for the corresponding neighbour.
                    auto decal = EdgeDecals.at(edge);

                    // Now get the corresponding voxel id.
                    auto neighbourDecal = v.id;
                    neighbourDecal.x += decal.x;
                    neighbourDecal.y += decal.y;

                    // Make sure that we don't run off the edge of the grid.
                    if (!validVoxel(Voxel(neighbourDecal)))
                    {
                        continue;
                    }

                    frontier.push(neighbourDecal);
                }

                mVoxels.push_back(v);
            }
        }

        std::vector<LineSegment> CrossSection::generateLineSegments( 
            std::vector<Voxel> const& voxels)
        {
            using atlas::math::Point;
            using atlas::math::Normal;

            // We may need to move this to the scope of the segments.
            std::map<std::uint64_t, LinePoint> computedPoints;
            std::vector<LineSegment> segments;

            auto interpolate = [this](FieldPoint const& p1, FieldPoint const& p2)
            {
                using atlas::math::Point2;

                auto pt = glm::mix(p1.value.xyz(), p2.value.xyz(),
                    (mMagic - p1.value.w) / (p2.value.w - p1.value.w));
                // Note that for now we assume that this is irrelevant. It may
                // so happen that there is a case when this is no longer true.
                auto hash = p1.svHash;
                auto sv = mSuperVoxels[hash];
                auto val = sv.eval(pt);
                auto grad = sv.grad(pt);
                return FieldPoint(pt, val, grad, hash);
            };

            auto generatePoint =
                [&computedPoints, interpolate, this](PointId const& p1,
                    PointId const& p2, FieldPoint const& fp1,
                    FieldPoint const& fp2)
            {
                auto h1 = BsoidHash32::hash(p1.x, p1.y);
                auto h2 = BsoidHash32::hash(p2.x, p2.y);
                
                // We need to keep in mind that the hashes are order sensitive,
                // which means that a single edge has two possible hashes: 
                // (h1, h2) and (h2, h1), so we have to check both.
                auto edgeHash1 = BsoidHash64::hash(h1, h2);
                auto edgeHash2 = BsoidHash64::hash(h2, h1);

                // Now look up both hashes.
                auto entry1 = computedPoints.find(edgeHash1);
                auto entry2 = computedPoints.find(edgeHash2);
                
                if (entry1 != computedPoints.end() || 
                    entry2 != computedPoints.end())
                {
                    if (entry1 != computedPoints.end())
                    {
                        return (*entry1).second;
                    }

                    return (*entry2).second;
                }
                else
                {
                    auto pt = interpolate(fp1, fp2);
                    auto edgeHash = edgeHash1;

                    LinePoint p(pt, edgeHash);
                    computedPoints.insert(
                        std::pair<std::uint64_t, LinePoint>(edgeHash1, p));
                    return p;
                }

            };

            // Iterate over the set of voxels.
            int k = 0;
            std::vector<std::uint32_t> coeffs = { 1, 2, 4, 8 };
            for (auto& voxel : voxels)
            {
                // First compute the cell index for our voxel.
                std::uint32_t voxelIndex = 0;
                for (std::size_t i = 0; i < 4; ++i)
                {
                    if (atlas::core::leq(voxel.points[i].value.w, mMagic))
                    {
                        voxelIndex |= coeffs[i];
                    }
                }

                // If a voxel has at least one tangent point and the rest are
                // inside the surface, then just skip it.
                if (voxelIndex == 15)
                {
                    continue;
                }

                // Safety check.
                ATLAS_ASSERT(EdgeTable[voxelIndex] != 0,
                    "Voxel should not be empty by this point.");

                std::vector<LinePoint> vertList(4);
                if (EdgeTable[voxelIndex] & 1)
                {
                    // Edge 0.
                    vertList[0] = generatePoint(
                        voxel.id + VoxelDecals[0],
                        voxel.id + VoxelDecals[1],
                        voxel.points[0],
                        voxel.points[1]);
                }

                if (EdgeTable[voxelIndex] & 2)
                {
                    // Edge 1.
                    vertList[1] = generatePoint(
                        voxel.id + VoxelDecals[1],
                        voxel.id + VoxelDecals[2],
                        voxel.points[1],
                        voxel.points[2]);
                }


                if (EdgeTable[voxelIndex] & 4)
                {
                    // Edge 2.
                    vertList[2] = generatePoint(
                        voxel.id + VoxelDecals[2],
                        voxel.id + VoxelDecals[3],
                        voxel.points[2],
                        voxel.points[3]);
                }

                if (EdgeTable[voxelIndex] & 8)
                {
                    // Edge 3.
                    vertList[3] = generatePoint(
                        voxel.id + VoxelDecals[3],
                        voxel.id + VoxelDecals[0],
                        voxel.points[3],
                        voxel.points[0]);
                }

                for (int i = 0; LineTable[voxelIndex][i] != -1; i += 2)
                {
                    auto start = vertList[LineTable[voxelIndex][i + 0]];
                    auto end   = vertList[LineTable[voxelIndex][i + 1]];
                    segments.emplace_back(start, end);
                }
                ++k;
            }

            return segments;
       }

       std::vector<std::vector<FieldPoint>> CrossSection::convertToContour(
           std::vector<LineSegment> const& segments)
       {
           if (segments.empty())
           {
               return {};
           }

           std::unordered_map<std::uint64_t, FieldPoint> vertices;
           std::map<std::uint64_t, 
               std::pair<std::size_t, std::uint64_t>> contourMap;
           std::vector<std::vector<FieldPoint>> resultContours;

           for (std::size_t i = 0; i < segments.size(); ++i)
           {
               auto start = segments[i].start;
               auto end = segments[i].end;

               vertices.insert({ start.edge, start.point });
               vertices.insert({ end.edge, end.point });

               contourMap.insert({ start.edge, {i, end.edge} });
           }

           auto currentPt = contourMap.begin()->first;
           auto currentIdx = contourMap.begin()->second;
           std::vector<bool> used(segments.size(), false);
           auto startPt = currentPt;
           std::vector<FieldPoint> contour;
           for (std::size_t i = 0; i < segments.size(); ++i)
           {
               if (!used[currentIdx.first])
               {
                   contour.push_back(vertices[currentPt]);
                   used[currentIdx.first] = true;
               }

               // Grab the next entry.
               auto range = contourMap.equal_range(currentIdx.second);
               for (auto& it = range.first; it != range.second; ++it)
               {
                   // Check if the current end point is equal to the point
                   // we are in. If it is, then we have a degenerate segment.
                   // Alternatively, we can also check for distance.
                   auto currentVertex = vertices[currentPt];
                   auto nextVertex = vertices[it->first];
                   auto dist = glm::distance2(currentVertex.value, 
                       nextVertex.value);
                   if (currentPt == it->first || atlas::core::isZero(dist))
                   {
                       used[it->second.first] = true;
                       continue;
                   }

                   // Check if there is more than one segment.
                   if (it->first == startPt && i != (segments.size() - 1))
                   {
                       // We have encountered a branch. So first save the 
                       // current contour and then clear it.
                       resultContours.push_back(contour);
                       contour.clear();

                       // Now we need to select the next point. We can do this
                       // by running through the map until we find the next 
                       // free vertex.
                       // HACK: Is there a better way of doing this?
                       for (auto& vertex : contourMap)
                       {
                           if (!used[vertex.second.first])
                           {
                               // This vertex hasn't been used yet, so set it
                               // to be our current vertex.
                               currentPt = vertex.first;
                               currentIdx = vertex.second;
                               startPt = vertex.first;
                               break;
                           }
                       }
                   }

                   if (!used[it->second.first])
                   {
                       // We haven't used this point yet, then select it to be
                       // the next one we visit.
                       currentPt = it->first;
                       currentIdx = it->second;
                       break;
                   }
               }
           }

           resultContours.push_back(contour);
           return resultContours;

       }

       void CrossSection::subdivideContour(int idx, std::size_t size)
       {
           using atlas::math::Point;
           using atlas::math::Vector;
           using atlas::math::Normal;
           using atlas::core::areEqual;

           auto& contour = mContours[idx];

#if !(ATHENA_DEBUG_CONTOURS)
           auto pushToSurface = 
               [this, contour](Point const& p, std::size_t i, float delta)
           {
               // First check if we are already on the surface.
               SuperVoxel sv = mSuperVoxels[contour[i].svHash];
               if (areEqual(mMagic, sv.eval(p)))
               {
                   float val = sv.eval(p);
                   Normal g = sv.grad(p);
                   return FieldPoint(p, val, g);
               }

               Point in = p;
               float inVal = sv.eval(in);
               auto norm = sv.grad(in);
               auto projNorm = norm - glm::proj(norm, glm::normalize(mNormal));
               projNorm = (inVal > mMagic) ? -projNorm : projNorm;
               projNorm = glm::normalize(projNorm);

               float d = delta;
               Point out = in + (delta * projNorm);
               float outVal = sv.eval(out);
               while (glm::sign(outVal - mMagic) == glm::sign(inVal - mMagic))
               {
                   d += delta;
                   out = in + (d * projNorm);
                   outVal = sv.eval(out);
               }

               Point newPt = glm::mix(in, out, (mMagic - inVal) / (outVal - inVal));
               float val = sv.eval(newPt);
               Normal g = sv.grad(newPt);
               return FieldPoint(newPt, val, g);
           };
#elif defined(ATLAS_DEBUG) && (ATHENA_DEBUG_CONTOURS)
           auto pushToSurface = [](Point const& p, std::size_t i, float delta)
           {
               return FieldPoint(p, 0.0f);
           };
#endif

           // Compute the arc length using the contour.
           float delta = 0.0f;
           {
               float arcLength = 0.0f;
               std::size_t cSize = contour.size();
               for (std::size_t i = 0; i < cSize; ++i)
               {
                   Point start = contour[i + 0].value.xyz();
                   Point end = contour[(i + 1) % cSize].value.xyz();

                   arcLength += glm::length(end - start);
               }

               delta = arcLength / static_cast<float>(size);
           }

           std::vector<FieldPoint> subDivContour;
           std::size_t currentSegment = 0;
           Point previousSegmentPoint = contour[0].value.xyz();
           subDivContour.push_back(pushToSurface(previousSegmentPoint, 
               currentSegment, delta));
           float dist = delta;
           for (std::size_t i = 1; i < size; ++i)
           {
               // Find where the next point is going to be.
               bool done = false;
               while (!done)
               {
                   // Construct the current segment.
                   Point A = 
                       contour[(currentSegment + 0) % contour.size()].value.xyz();
                   Point B =
                       contour[(currentSegment + 1) % contour.size()].value.xyz();
                   Vector eps = dist * ((B - A) / glm::length(B - A));
                   Point newPt = previousSegmentPoint + eps;

                   if (glm::length(newPt - A) > glm::length(B - A)) {
                       currentSegment++;
                       float travelledDistance =
                           glm::distance(B, previousSegmentPoint);
                       dist -= travelledDistance;
                       previousSegmentPoint = B;
                       continue;
                   }

                   // The next point is in this segment.
                   auto s = pushToSurface(newPt, currentSegment, delta);
                   previousSegmentPoint = newPt;
                   subDivContour.emplace_back(s);

                   dist = delta;
                   done = true;
               }
           }

           contour = subDivContour;
       }

       float CrossSection::shadowField(FieldPoint const& p)
       {
           return glm::length(p.g - glm::proj(p.g, getNormal()));
       }

       std::vector<Voxel> CrossSection::findShadowVoxels()
       {
           using atlas::math::Point4;
           using atlas::math::Point;

           std::queue<PointId> frontier;

           auto getEdges = [this](Voxel const& v)
           {
               FieldPoint start, end;
               int edgeId = 0;
               std::vector<int> edges;

               for (std::size_t i = 0; i < v.points.size(); ++i)
               {
                   start = v.points[i];
                   end = v.points[(i + 1) % v.points.size()];
                   float val1 = start.value.w - mMagic;
                   float val2 = end.value.w - mMagic;

                   if (glm::sign(val1) != glm::sign(val2) ||
                       (glm::sign(val1) == 1 && glm::sign(val2) == 1))
                   {
                       edges.push_back(edgeId);
                   }
                   edgeId++;
               }

               return edges;
           };

           auto containsShadow = [this](Voxel const& v)
           {
               std::array<FieldPoint, 4> shadowPoints;
               {
                   int i = 0;
                   for (auto& pt : v.points)
                   {
                       float shadow = shadowField(pt);
                       FieldPoint shadowPoint = { pt.value.xyz(), shadow };
                       shadowPoints[i] = shadowPoint;
                       ++i;
                   }
               }

               FieldPoint start, end;
               for (std::size_t i = 0; i < shadowPoints.size(); ++i)
               {
                   start = shadowPoints[i];
                   end = shadowPoints[(i + 1) % shadowPoints.size()];

                   float val1 = start.value.w - mShadowMagic;
                   float val2 = end.value.w - mShadowMagic;
                   if (glm::sign(val1) != glm::sign(val2))
                   {
                       return true;
                   }
               }

               return false;
           };

           frontier.push(mVoxels[0].id);
           std::vector<Voxel> shadowVoxels;
           std::map<std::uint32_t, VoxelId> seenVoxels;

           while (!frontier.empty())
           {
               auto top = frontier.front();
               frontier.pop();

               if (seenVoxels.find(BsoidHash32::hash(top.x, top.y)) !=
                   seenVoxels.end())
               {
                   continue;
               }
               else
               {
                   seenVoxels.insert(
                       std::pair<std::uint32_t, VoxelId>(
                           BsoidHash32::hash(top.x, top.y), top));
               }

               Voxel v(top);
               fillVoxel(v);

               auto edges = getEdges(v);
               if (edges.empty())
               {
                   continue;
               }

               for (auto& edge : edges)
               {
                   auto decal = EdgeDecals.at(edge);

                   auto neighbourDecal = v.id;
                   neighbourDecal.x += decal.x;
                   neighbourDecal.y += decal.y;

                   if (!validVoxel(Voxel(neighbourDecal)))
                   {
                       continue;
                   }

                   frontier.push(neighbourDecal);
               }

               if (containsShadow(v))
               {
                   // Now we need to change the points from regular field values
                   // to the shadow field.
                   Voxel shadowVoxel;
                   int i = 0;
                   for (auto& pt : v.points)
                   {
                       FieldPoint shadowPoint = pt;
                       shadowPoint.value.w = shadowField(pt);
                       shadowVoxel.points[i] = shadowPoint;
                       ++i;
                   }
                   
                   shadowVoxels.push_back(shadowVoxel);
               }
           }

           return shadowVoxels;
       }

       std::vector<LineSegment> CrossSection::generateShadowSegments(
           std::vector<Voxel> const& shadowVoxels)
       {
           using atlas::math::Point;
           using atlas::math::Normal;

           std::map<std::uint64_t, LinePoint> computedPoints;
           std::vector<LineSegment> shadowSegments;

           auto interpolate = [this](FieldPoint const& p1, FieldPoint const& p2)
           {
               using atlas::math::Point2;

               auto pt = glm::mix(p1.value.xyz(), p2.value.xyz(),
                   (mShadowMagic - p1.value.w) / (p2.value.w - p1.value.w));
               FieldPoint p;
               p.value.xyz = pt;
               return p;
           };

           auto generatePoint = 
               [&computedPoints, interpolate, this](PointId const& p1,
                   PointId const& p2, FieldPoint const& fp1,
                   FieldPoint const& fp2)
           {
               auto h1 = BsoidHash32::hash(p1.x, p1.y);
               auto h2 = BsoidHash32::hash(p2.x, p2.y);

               auto edgeHash1 = BsoidHash64::hash(h1, h2);
               auto edgeHash2 = BsoidHash64::hash(h2, h1);

               auto entry1 = computedPoints.find(edgeHash1);
               auto entry2 = computedPoints.find(edgeHash2);

               if (entry1 != computedPoints.end() ||
                   entry2 != computedPoints.end())
               {
                   if (entry1 != computedPoints.end())
                   {
                       return (*entry1).second;
                   }

                   return (*entry2).second;
               }
               else
               {
                   auto pt = interpolate(fp1, fp2);
                   auto edgeHash = edgeHash1;

                   LinePoint p(pt, edgeHash);
                   computedPoints.insert(
                       std::pair<std::uint64_t, LinePoint>(edgeHash1, p));
                   return p;
               }
           };

            // Iterate over the set of voxels.
            int k = 0;
            std::vector<std::uint32_t> coeffs = { 1, 2, 4, 8 };
            for (auto& voxel : shadowVoxels)
            {
                // First compute the cell index for our voxel.
                std::uint32_t voxelIndex = 0;
                for (std::size_t i = 0; i < 4; ++i)
                {
                    if (atlas::core::leq(voxel.points[i].value.w, mShadowMagic))
                    {
                        voxelIndex |= coeffs[i];
                    }
                }

                // If a voxel has at least one tangent point and the rest are
                // inside the surface, then just skip it.
                if (voxelIndex == 15)
                {
                    continue;
                }

                // Safety check.
                ATLAS_ASSERT(EdgeTable[voxelIndex] != 0,
                    "Voxel should not be empty by this point.");

                std::vector<LinePoint> vertList(4);
                if (EdgeTable[voxelIndex] & 1)
                {
                    // Edge 0.
                    vertList[0] = generatePoint(
                        voxel.id + VoxelDecals[0],
                        voxel.id + VoxelDecals[1],
                        voxel.points[0],
                        voxel.points[1]);
                }

                if (EdgeTable[voxelIndex] & 2)
                {
                    // Edge 1.
                    vertList[1] = generatePoint(
                        voxel.id + VoxelDecals[1],
                        voxel.id + VoxelDecals[2],
                        voxel.points[1],
                        voxel.points[2]);
                }


                if (EdgeTable[voxelIndex] & 4)
                {
                    // Edge 2.
                    vertList[2] = generatePoint(
                        voxel.id + VoxelDecals[2],
                        voxel.id + VoxelDecals[3],
                        voxel.points[2],
                        voxel.points[3]);
                }

                if (EdgeTable[voxelIndex] & 8)
                {
                    // Edge 3.
                    vertList[3] = generatePoint(
                        voxel.id + VoxelDecals[3],
                        voxel.id + VoxelDecals[0],
                        voxel.points[3],
                        voxel.points[0]);
                }

                for (int i = 0; LineTable[voxelIndex][i] != -1; i += 2)
                {
                    auto start = vertList[LineTable[voxelIndex][i + 0]];
                    auto end   = vertList[LineTable[voxelIndex][i + 1]];
                    shadowSegments.emplace_back(start, end);
                }
                ++k;
            }


           return shadowSegments;
       }

       bool CrossSection::validVoxel(Voxel const& v) const
       {
           return (
               v.isValid() &&
               v.id.x < mGridSize &&
               v.id.y < mGridSize);
       }

        void CrossSection::validateVoxels() const
        {
            std::map<std::uint32_t, Voxel> seenMap;
            for (auto& voxel : mVoxels)
            {
                auto hash = BsoidHash32::hash(voxel.id.x, voxel.id.y);
                seenMap.insert(std::pair<std::uint32_t, Voxel>(hash, voxel));
            }

            ATLAS_ASSERT(seenMap.size() == mVoxels.size(),
                "There should be no repeated voxels in the lattice.");
            DEBUG_LOG("Voxel validation succeeded.");
        }

        void CrossSection::validateContour() const
        {
            struct PointCompare
            {
                bool operator()(FieldPoint const& lhs,
                    FieldPoint const& rhs) const
                {
                    return lhs.value == rhs.value;
                }
            };

            struct PointHasher
            {
                std::size_t operator()(FieldPoint const& cp) const
                {
                    return std::hash<float>()(cp.value.x + cp.value.y +
                        cp.value.z);
                }
            };

            std::unordered_set<FieldPoint, PointHasher, PointCompare> uniques;
            std::size_t size = 0;
            for (auto& contour : mContours)
            {
                for (auto& pt : contour)
                {
                    uniques.insert(pt);
                    ++size;
                }
            }

            ATLAS_ASSERT(uniques.size() == size,
                "There should be no repeated vertices in the contour.");
            DEBUG_LOG("Contour validation succeeded.");
        }
    }
}