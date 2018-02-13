#include "athena/polygonizer/CrossSection.hpp"
#include "athena/polygonizer/Hash.hpp"
#include "athena/polygonizer/Tables.hpp"

#include <atlas/core/Float.hpp>
#include <atlas/core/Log.hpp>

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <queue>

namespace athena
{
    namespace polygonizer
    {
        CrossSection::CrossSection(SlicingAxes const& axis, 
            atlas::math::Point const& min, atlas::math::Point const& max, 
            std::uint32_t gridSize, std::uint32_t svSize, 
            tree::BlobTree* tree) :
            mAxis(axis),
            mMin(min),
            mMax(max),
            mGridSize(gridSize),
            mSvSize(svSize),
            mTree(tree)
        {
            using atlas::math::Normal;

            assert(gridSize % svSize == 0);

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
        }

        void CrossSection::constructLattice()
        {
            using atlas::math::Point;
            using atlas::utils::BBox;

            // Can be done in parallel.
            for (std::uint32_t x = 0; x < mSvSize; ++x)
            {
                for (std::uint32_t y = 0; y < mSvSize; ++y)
                {
                    auto pt = createCellPoint(x, y, mSvDelta);

                    // Construct the cell that corresponds to the super-voxel.
                    BBox cell(pt, pt + mSvDelta);

                    SuperVoxel sv;
                    sv.fields = mTree->getOverlappingFields(cell);
                    sv.id = { x, y };

                    if (!sv.fields.empty())
                    {
                        auto idx = BsoidHash32::hash(x, y);
                        mSuperVoxels[idx] = sv;
                    }
                }
            }

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
            auto segments = generateLineSegments();
            convertToContour(segments);
            DEBUG_LOG_V("Generated %d vertices for contour.",
                mContours.size());
#if defined ATLAS_DEBUG
            // validateContour();
#endif
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

        void CrossSection::marchVoxelOnSurface(std::vector<Voxel> const& seeds)
        {
            using atlas::math::Point4;
            using atlas::math::Point;

            std::map<std::uint32_t, FieldPoint> seenPoints;
            std::map<std::uint32_t, VoxelId> seenVoxels;
            std::queue<PointId> frontier;

            auto evalPoint = [this](Point const& p)
            {
                // We need to figure out which super-voxel this point belongs to.
                auto v = (p - mMin) / mSvDelta;
                PointId id;
                id.x = static_cast<std::uint32_t>(v[mAxisId.x]);
                id.y = static_cast<std::uint32_t>(v[mAxisId.y]);

                auto svHash = BsoidHash32::hash(id.x, id.y);
                SuperVoxel sv = mSuperVoxels[svHash];
                auto val = sv.eval(p);
                auto g = sv.grad(p);
                return FieldPoint(p, val, g, svHash);
            };

            auto generateVoxel = [this, &seenPoints, evalPoint](Voxel& v)
            {
                int d = 0;
                for (auto& decal : VoxelDecals)
                {
                    auto decalId = v.id + decal;
                    auto entry = seenPoints.find(
                        BsoidHash32::hash(decalId.x, decalId.y));
                    if (entry != seenPoints.end())
                    {
                        // We have seen this point already, so grab it from our list.
                        v.points[d] = (*entry).second;
                    }
                    else
                    {
                        // We haven't seen this point yet, so we need to add it
                        // to our list.
                        auto pt = createCellPoint(decalId, mGridDelta);
                        auto vp = evalPoint(pt);
                        auto hash = BsoidHash32::hash(decalId.x, decalId.y);
                        seenPoints.insert(
                            std::pair<std::uint32_t, FieldPoint>(hash, vp));
                        v.points[d] = vp;
                    }
                    ++d;
                }
            };

            auto getEdges = [this](Voxel const& v)
            {
                FieldPoint start, end;
                int edgeId = 0;
                std::vector<int> edges;

                for (std::size_t i = 0; i < v.points.size(); ++i)
                {
                    start = v.points[i];
                    end = v.points[(i + 1) % v.points.size()];
                    float val1 = start.value.w;
                    float val2 = end.value.w;

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

            // First fill in the values of each of the seeds and insert them.
            {
                for (auto& seed : seeds)
                {
                    int i = 0;
                    for (auto& decal : VoxelDecals)
                    {
                        auto decalId = seed.id + decal;
                        auto hash = BsoidHash32::hash(decalId.x, decalId.y);
                        auto pt = createCellPoint(decalId, mGridDelta);
                        auto vp = evalPoint(pt);
                        seenPoints.insert(
                            std::pair<std::uint32_t, FieldPoint>(hash, vp));
                    }
                    i++;
                    frontier.push(seed.id);
                }
            }


            while (!frontier.empty())
            {
                // Grab a voxel from the queue.
                auto top = frontier.front();
                frontier.pop();

                // Check if we have seen this voxel before.
                if (seenVoxels.find(BsoidHash32::hash(top.x, top.y)) !=
                    seenVoxels.end())
                {
                    // We have, so do nothing.
                    continue;
                }
                else
                {
                    // We haven't seen it before, so add it to our list and
                    // proceed.
                    seenVoxels.insert(
                        std::pair<std::uint32_t, VoxelId>(
                            BsoidHash32::hash(top.x, top.y), top));
                }

                // Now fill its values.
                Voxel v(top);
                generateVoxel(v);

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
                    if (!Voxel(neighbourDecal).isValid())
                    {
                        continue;
                    }

                    frontier.push(neighbourDecal);
                }

                mVoxels.push_back(v);
            }
        }

        std::vector<LineSegment> CrossSection::generateLineSegments()
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
                    (0.0f - p1.value.w) / (p2.value.w - p1.value.w));
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
                    // We haven't seent this point, so it's important that
                    // we enter the point twice!
                    auto pt = interpolate(fp1, fp2);
                    LinePoint p(pt, edgeHash1);
                    computedPoints.insert(
                        std::pair<std::uint64_t, LinePoint>(edgeHash1, p));
                    return LinePoint(pt, edgeHash1);
                }

            };

            // Iterate over the set of voxels.
            int k = 0;
            for (auto& voxel : mVoxels)
            {
                // First compute the cell index for our voxel.
                std::uint32_t voxelIndex = 0;
                std::vector<std::uint32_t> coeffs = { 1, 2, 4, 8 };
                for (std::size_t i = 0; i < 4; ++i)
                {
                    if (atlas::core::leq(voxel.points[i].value.w, 0.0f))
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
                assert(EdgeTable[voxelIndex] != 0);

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
                    //DEBUG_LOG_V("(%.2f, %.2f, %.2f, %llu) -> (%.2f, %.2f, %.2f, %llu)",
                    //    start.point.value.x,
                    //    start.point.value.y,
                    //    start.point.value.z,
                    //    start.edge,
                    //    end.point.value.x,
                    //    end.point.value.y,
                    //    end.point.value.z,
                    //    end.edge);
                }
                ++k;
            }

            return segments;
       }

       void CrossSection::convertToContour(
           std::vector<LineSegment> const& segments)
       {
           if (segments.empty())
           {
               return;
           }

           std::unordered_map<std::uint64_t, FieldPoint> vertices;
           std::unordered_map<std::uint64_t, 
               std::pair<std::size_t, std::uint64_t>> contourMap;

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
                       // Something happens here.
                       DEBUG_LOG("Found end of contour.");
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

               mContours.push_back(contour);
           }
       }

        void CrossSection::validateVoxels() const
        {
            std::map<std::uint32_t, Voxel> seenMap;
            for (auto& voxel : mVoxels)
            {
                auto hash = BsoidHash32::hash(voxel.id.x, voxel.id.y);
                seenMap.insert(std::pair<std::uint32_t, Voxel>(hash, voxel));
            }

            assert(seenMap.size() == mVoxels.size());
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

            assert(uniques.size() == size);
        }
    }
}