#include "athena/polygonizer/Bsoid.hpp"
#include "athena/polygonizer/BranchingManager.hpp"

#include <atlas/core/Timer.hpp>
#include <atlas/core/Macros.hpp>
#include <atlas/core/Assert.hpp>
#include <atlas/core/Log.hpp>

#include <numeric>
#include <functional>
#include <unordered_set>
#include <fstream>

#if defined ATLAS_DEBUG
#define ATHENA_DEBUG_CONTOURS 0 

#define ATHENA_DEBUG_CONTOUR_START 8
#define ATHENA_DEBUG_CONTOUR_END 8

#define ATHENA_DEBUG_CONTOUR_RANGE(i, start, end) \
if (i < start || i > end) \
{\
    ++i;\
    continue;\
}
#endif


namespace athena
{
    namespace polygonizer
    {
        Bsoid::Bsoid() :
            mName("model")
        { }

        Bsoid::Bsoid(tree::BlobTree const& model, std::string const& name,
            float isoValue) :
            mTree(std::make_unique<tree::BlobTree>(model)),
            mMagic(isoValue),
            mName(name)
        { }

        Bsoid::Bsoid(Bsoid&& b) :
            mLattice(std::move(b.mLattice)),
            mContour(std::move(b.mContour)),
            mTree(std::move(b.mTree)),
            mCrossSectionDelta(b.mCrossSectionDelta),
            mAxis(b.mAxis),
            mCrossSections(std::move(b.mCrossSections)),
            mMesh(std::move(b.mMesh)),
            mMagic(b.mMagic),
            mLog(std::move(b.mLog)),
            mName(b.mName)
        { }

        void Bsoid::setModel(tree::BlobTree const& model)
        {
            mTree = std::make_unique<tree::BlobTree>(model);
        }

        void Bsoid::setIsoValue(float isoValue)
        {
            mMagic = isoValue;
        }

        void Bsoid::setSlicingAxis(SlicingAxes const& axis)
        {
            mAxis = axis;
        }

        void Bsoid::setCrossSectionDelta(float delta)
        {
            // Grab the box of the model.
            auto box = mTree->getTreeBox();
            auto slices = (box.pMax - box.pMin) / delta;
            std::size_t numSlices = 0;

            switch (mAxis)
            {
            case SlicingAxes::XAxis:
                numSlices = static_cast<std::size_t>(slices.x);
                break;

            case SlicingAxes::YAxis:
                numSlices = static_cast<std::size_t>(slices.y);
                break;

            case SlicingAxes::ZAxis:
                numSlices = static_cast<std::size_t>(slices.z);
                break;
            }

            // Resize the vector here.
            mCrossSectionDelta = delta;
            mCrossSections.resize(numSlices);
        }

        void Bsoid::setNumCrossSections(std::size_t num)
        {
            auto box = mTree->getTreeBox();
            auto deltas = (box.pMax - box.pMin) / static_cast<float>(num - 1);
            switch (mAxis)
            {
            case SlicingAxes::XAxis:
                mCrossSectionDelta = deltas.x;
                break;

            case SlicingAxes::YAxis:
                mCrossSectionDelta = deltas.y;
                break;

            case SlicingAxes::ZAxis:
                mCrossSectionDelta = deltas.z;
                break;
            }

            mCrossSections.resize(num);
        }

        std::size_t Bsoid::numCrossSections() const
        {
            return mCrossSections.size();
        }

        float Bsoid::crossSectionDelta() const
        {
            return mCrossSectionDelta;
        }

        SlicingAxes Bsoid::axis() const
        {
            return mAxis;
        }

        tree::BlobTree* Bsoid::tree() const
        {
            return mTree.get();
        }

        void Bsoid::makeCrossSections(std::uint32_t gridSize, 
            std::uint32_t svSize)
        {
            using atlas::math::Point;

            if (mCrossSections.empty())
            {
                return;
            }

            auto box = mTree->getTreeBox();

            // Initialize the points that frame the first plane.
            Point min, max;
            min = box.pMin;
            float maxAxis = 0.0f;
            switch (mAxis)
            {
            case SlicingAxes::XAxis:
                max = Point(box.pMin.x, box.pMax.yz());
                maxAxis = box.pMax.x;
                break;

            case SlicingAxes::YAxis:
                max = Point(box.pMax.x, box.pMin.y, box.pMax.z);
                maxAxis = box.pMax.y;
                break;

            case SlicingAxes::ZAxis:
                max = Point(box.pMax.xy(), box.pMin.z);
                maxAxis = box.pMax.z;
                break;
            }

            for (std::size_t i = 0; i < mCrossSections.size() - 1; ++i)
            {
                mCrossSections[i] = std::make_unique<CrossSection>(
                    mAxis, min, max, gridSize, svSize, mMagic, mTree.get());

                switch (mAxis)
                {
                case SlicingAxes::XAxis:
                    min.x += mCrossSectionDelta;
                    max.x += mCrossSectionDelta;
                    break;

                case SlicingAxes::YAxis:
                    min.y += mCrossSectionDelta;
                    max.y += mCrossSectionDelta;
                    break;

                case SlicingAxes::ZAxis:
                    min.z += mCrossSectionDelta;
                    max.z += mCrossSectionDelta;
                }
            }

            // Make sure that the final plane is always at the end of the box.
            switch (mAxis)
            {
            case SlicingAxes::XAxis:
                min.x = maxAxis;
                max.x = maxAxis;
                break;

            case SlicingAxes::YAxis:
                min.y = maxAxis;
                max.y = maxAxis;
                break;

            case SlicingAxes::ZAxis:
                min.z = maxAxis;
                max.z = maxAxis;
                break;
            }

            mCrossSections[mCrossSections.size() - 1] =
                std::make_unique<CrossSection>(mAxis, min, max, gridSize, svSize,
                    mMagic, mTree.get());
        }

        void Bsoid::constructLattices()
        {
            atlas::core::Timer<float> global;
            atlas::core::Timer<float> t;
            int i = 0;
            global.start();
            // This can be done in parallel.
            for (auto& section : mCrossSections)
            {
#if defined (ATLAS_DEBUG) && (ATHENA_DEBUG_CONTOURS)
                ATHENA_DEBUG_CONTOUR_RANGE(i, ATHENA_DEBUG_CONTOUR_START,
                    ATHENA_DEBUG_CONTOUR_END);
#endif
                t.start();
                section->constructLattice();
                auto duration = t.elapsed();
                mLog << "Generated lattice " << i << " in: " << duration
                    << " seconds.\n";
                ++i;
            }

            auto time = global.elapsed();
            mLog << "Total lattice generation time: " << time << " seconds\n";

            std::vector<std::vector<Voxel>> voxels;
            i = 0;
            for (auto& section : mCrossSections)
            {
#if defined (ATLAS_DEBUG) && (ATHENA_DEBUG_CONTOURS)
                ATHENA_DEBUG_CONTOUR_RANGE(i, ATHENA_DEBUG_CONTOUR_START,
                    ATHENA_DEBUG_CONTOUR_END);
#endif
                voxels.push_back(section->getVoxels());
                ++i;
            }

            mLattice.makeLattice(voxels);
        }
        
        void Bsoid::constructContours()
        {
            atlas::core::Timer<float> global;
            atlas::core::Timer<float> t;
            int i = 0;
            global.start();

            // This can be done in parallel. I think...
            for (auto& section : mCrossSections)
            {
#if defined (ATLAS_DEBUG) && (ATHENA_DEBUG_CONTOURS)
                ATHENA_DEBUG_CONTOUR_RANGE(i, ATHENA_DEBUG_CONTOUR_START,
                    ATHENA_DEBUG_CONTOUR_END);
#endif
                t.start();
                 section->constructContour();
                auto duration = t.elapsed();
                mLog << "Generated contour " << i << " in: " << duration
                    << " seconds.\n";
                ++i;
            }

            auto time = global.elapsed();
            mLog << "Total contour generation time: " << time << " seconds\n";

            // Something here to create the contour thing.
            std::vector<std::vector<std::vector<FieldPoint>>> contours;
            i = 0;
            for (auto& section : mCrossSections)
            {
#if defined (ATLAS_DEBUG) && (ATHENA_DEBUG_CONTOURS)
                ATHENA_DEBUG_CONTOUR_RANGE(i, ATHENA_DEBUG_CONTOUR_START,
                    ATHENA_DEBUG_CONTOUR_END);
#endif

                contours.push_back(section->getContour());
                ++i;
            }

            mContour.makeContour(contours);
        }

        void Bsoid::constructMesh()
        {
            using atlas::math::Point;

            atlas::core::Timer<float> global;
            atlas::core::Timer<float> t;

            // First we need to subdivide the contours into the largest
            // size that we have. Simulatenously grab the number of conours
            // per branch.
            std::size_t maxContourSize = 0;
            std::vector<std::size_t> branches;
            for (auto& section : mCrossSections)
            {
                branches.push_back(section->getContour().size());

                if (maxContourSize < section->getLargestContourSize())
                {
                    maxContourSize = section->getLargestContourSize();
                }
            }

            // Now we have the branching manager tell us which layers need to 
            // be processed for branching.
            BranchingManager manager;
            auto branchData = manager.findBranches(branches);

            for (auto& branchCase : branchData)
            {
                // Regardless of anything we need to grab the shadow points,
                // so grab them first.
                // TODO: Figure out how to have this distinguish if we have
                // multiple inflexion points.
                auto shadowPoints = 
                    mCrossSections[branchCase.top]->findShadowPoints();

                if (branchCase.type == BranchingManager::BranchType::Branch)
                {
                    // Compute the centre of gravity.
                    Point centre;
                    for (auto& shadowPoint : shadowPoints)
                    {
                        centre += shadowPoint.value.xyz();
                    }
                    centre /= static_cast<float>(shadowPoints.size());

                    // Once we have the centre of gravity, project it down to
                    // the lower layer.
                    auto lowNormal =
                        mCrossSections[branchCase.bottom]->getNormal();

                    // Project the point.
                    auto projCentre = glm::proj(centre, lowNormal);

                    // Now that we have the projected version, we need to
                    // sample our field at both ends to interpolate and find 
                    // the inflexion point.
                    float val1 = mTree->eval(centre);
                    float val2 = mTree->eval(projCentre);

                    auto inflexionPt = glm::mix(centre, projCentre,
                        (mMagic - val1) / (val2 - val1));

                    // Now that we have the point, create a new cross-section
                    // with this specific height.
                    auto resolution =
                        mCrossSections[branchCase.top]->getResolutions();
                    atlas::math::Point min, max;
                    auto box = mTree->getTreeBox();

                    switch (mAxis)
                    {
                    case SlicingAxes::XAxis:
                        max = Point(inflexionPt.x, box.pMax.yz());
                        min = Point(inflexionPt.x, box.pMin.yz());
                        break;

                    case SlicingAxes::YAxis:
                        max = Point(box.pMax.x, inflexionPt.y, box.pMax.z);
                        min = Point(box.pMin.x, inflexionPt.y, box.pMin.z);
                        break;

                    case SlicingAxes::ZAxis:
                        max = Point(box.pMax.xy(), inflexionPt.z);
                        min = Point(box.pMin.xy(), inflexionPt.z);
                        break;
                    }

                    mCrossSections.push_back(
                        std::make_unique<CrossSection>(mAxis, min, max,
                            resolution.first, resolution.second, mMagic, 
                            mTree.get()));

                    // With the cross-section in place, now create the lattice
                    // and the contours for it.
                    std::size_t newSlice = mCrossSections.size() - 1;
                    mCrossSections[newSlice]->constructLattice();
                    mCrossSections[newSlice]->constructContour();
                    auto newSize = mCrossSections[newSlice]->getLargestContourSize();

                    // Update the max size of the contours if need be.
                    maxContourSize = (maxContourSize < newSize) ? 
                        newSize : maxContourSize;
                }
                else if (branchCase.type == BranchingManager::BranchType::Cap)
                {
                    // Handle caps here.
                }
            }


            // Now that we have it, lets resize all of the contours. Again
            // this can be done in parallel.
            int i = 0;
            for (auto& section : mCrossSections)
            {
#if defined (ATLAS_DEBUG) && (ATHENA_DEBUG_CONTOURS)
                ATHENA_DEBUG_CONTOUR_RANGE(i, ATHENA_DEBUG_CONTOUR_START,
                    ATHENA_DEBUG_CONTOUR_END);
#endif
                section->resizeContours(maxContourSize);
                ++i;
            }

            // Once we have all of the contour data, send it down to the manager
            // for linking.
            for (auto& section : mCrossSections)
            {
                manager.insertContours(section->getContour());
            }

            mMesh = manager.connectContours();

            std::vector<std::vector<Voxel>> voxels;
            i = 0;
            for (auto& section : mCrossSections)
            {
#if defined(ATLAS_DEBUG) && (ATHENA_DEBUG_CONTOURS)
                ATHENA_DEBUG_CONTOUR_RANGE(i, ATHENA_DEBUG_CONTOUR_START,
                    ATHENA_DEBUG_CONTOUR_END);
#endif
                voxels.push_back(section->getVoxels());
                ++i;
            }

            mLattice.makeLattice(voxels);

            std::vector<std::vector<std::vector<FieldPoint>>> contours;
            i = 0;
            for (auto& section : mCrossSections)
            {
#if defined (ATLAS_DEBUG) && (ATHENA_DEBUG_CONTOURS)
                ATHENA_DEBUG_CONTOUR_RANGE(i, ATHENA_DEBUG_CONTOUR_START,
                    ATHENA_DEBUG_CONTOUR_END);
#endif
                contours.push_back(section->getContour());
                ++i;
            }

            mContour.makeContour(contours);
        }

        void Bsoid::polygonize()
        {
            using atlas::core::Timer;

            Timer<float> global;

            global.start();
            mLog << "Lattice generation.\n";
            mLog << "#===========================#\n";
            // Generate lattices.
            {
                Timer<float> step;
                Timer<float> part;
                step.start();
                int i = 0;
                for (auto& section : mCrossSections)
                {
                    part.start();
                    section->constructLattice();
                    auto duration = part.elapsed();
                    mLog << "Generated lattice " << i << " in: " << duration <<
                        " seconds.\n";
                    ++i;
                }

                auto stepElapsed = step.elapsed();
                mLog << "Total lattice generation time: " << stepElapsed 
                    << " seconds.\n";
            }

            mLog << "\nContour generation.\n";
            mLog << "#===========================#\n";

            // Generate contours.
            {
                Timer<float> step;
                Timer<float> part;
                step.start();
                int i = 0;
                for (auto& section : mCrossSections)
                {
                    part.start();
                    section->constructContour();
                    auto numContours = section->getContour().size();
                    auto duration = part.elapsed();
                    mLog << "Generated  " << numContours << " for cross-section " 
                        << i << " in: " << duration <<
                        " seconds.\n";
                    ++i;
                }

                auto stepElapsed = step.elapsed();
                mLog << "Total contour generation time: " << stepElapsed 
                    << " seconds.\n";
            }

            mLog << "\nMesh generation.\n";
            mLog << "#===========================#\n";
            {
                Timer<float> step;
                Timer<float> part;

                step.start();

                std::size_t size = 0;
                for (auto& section : mCrossSections)
                {
                    if (size < section->getLargestContourSize())
                    {
                        size = section->getLargestContourSize();
                    }
                }

                int i = 0;
                for (auto& section : mCrossSections)
                {
                    part.start();
                    section->resizeContours(size);
                    
                    auto elapsed = part.elapsed();
                    mLog << "Resized cross-section " << i << " in " << elapsed <<
                        " seconds.\n";
                    i++;
                }

                // Once we have all of the contour data, send it down to the manager
                // for linking.
                BranchingManager manager;
                for (auto& section : mCrossSections)
                {
                    manager.insertContours(section->getContour());
                }

                mMesh = manager.connectContours();

                auto stepElapsed = step.elapsed();
                mLog << "Mesh generated in " << stepElapsed << " seconds.\n";
            }

            mLog << "\nSummary:\n";
            mLog << "#===========================#\n";
            mLog << "Total runtime: " << global.elapsed() << " seconds\n";
            mLog << "Total vertices generated: " << mMesh.vertices().size() << "\n";
        }

        std::size_t Bsoid::getNumSlices() const
        {
            return mCrossSections.size();
        }

        Lattice const& Bsoid::getLattice() const
        {
            return mLattice;
        }

        Contour const& Bsoid::getContour() const
        {
            return mContour;
        }

        atlas::utils::Mesh& Bsoid::getMesh()
        {
            return mMesh;
        }

        void Bsoid::setName(std::string const& name)
        {
            mName = name;
        }

        std::string Bsoid::getName() const
        {
            return mName;
        }

        std::string Bsoid::getLog() const
        { 
            return mLog.str();
        }

        void Bsoid::clearLog()
        {
            mLog.str(std::string());
        }

        void Bsoid::saveMesh()
        {
            mMesh.saveToFile(mName + ".obj");
        }

        void Bsoid::connectContours()
        {
            // We are going to process each pair of contours to generate the 
            // indices. First, let's insert all of the vertices from all the 
            // contours into the mesh.
            // Note: for now, this assumes no branching.
            std::vector<std::pair<std::size_t, std::size_t>> contourOffsets;
            std::size_t start = 0;
            std::size_t num = 0;
            int i = 0;
            for (auto& section : mCrossSections)
            {
#if defined (ATLAS_DEBUG) && (ATHENA_DEBUG_CONTOURS)
                ATHENA_DEBUG_CONTOUR_RANGE(i, ATHENA_DEBUG_CONTOUR_START,
                    ATHENA_DEBUG_CONTOUR_END);
#endif
                auto contour = section->getContour();
                ATLAS_ASSERT(contour.size() == 0 || contour.size() == 1,
                    "Branching is currently not allowed.");

                for (auto& ring : contour)
                {
                    for (auto& pt : ring)
                    {
                        mMesh.vertices().push_back(pt.value.xyz());
                        mMesh.normals().push_back(-pt.g);
                        ++num;
                    }
                }

                contourOffsets.push_back({ start, num });
                start += num;
                num = 0;
                ++i;
            }

            // Now grab the first two offsets, as these are the first two 
            // contours.
            for (std::size_t i = 0; i < contourOffsets.size() - 1; i++)
            {
                auto c1 = contourOffsets[i + 0];
                auto c2 = contourOffsets[i + 1];
                if (c1.second == 0 || c2.second == 0)
                {
                    continue;
                }

                if (c1.second == 1 || c2.second == 1)
                {
                    // We have a cap contour. So we simply need to make a 
                    // triangle fan.
                    auto top = (c1.second == 1) ? c1.first : c2.first;
                    auto bottom = (c1.second == 1) ? c2.first : c1.first;
                    auto bSize = (c1.second == 1) ? c2.second : c1.second;

                    for (std::size_t i = 0; i < bSize; ++i)
                    {
                        mMesh.indices().push_back(top);
                        mMesh.indices().push_back(bottom + i);
                        mMesh.indices().push_back(bottom + ((i + 1) % bSize));
                    }

                    continue;
                }

                // Safety check.
                ATLAS_ASSERT(c1.second == c2.second,
                    "The sizes between contours should be identical.");

                auto top = c1.first;
                auto bottom = c2.first;
                auto size = c1.second;

                for (std::size_t i = 0; i < size; ++i)
                {
                    mMesh.indices().push_back(top + i);
                    mMesh.indices().push_back(bottom + i);
                    mMesh.indices().push_back(bottom + ((i + 1) % size));

                    mMesh.indices().push_back(bottom + ((i + 1) % size));
                    mMesh.indices().push_back(top + ((i + 1) % size));
                    mMesh.indices().push_back(top + i);
                }
            }
        }
    }
}
