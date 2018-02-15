#include "athena/polygonizer/Bsoid.hpp"

#include <atlas/core/Timer.hpp>
#include <atlas/core/Macros.hpp>

#include <numeric>
#include <functional>
#include <unordered_set>
#include <fstream>

#if defined ATLAS_DEBUG
#define ATHENA_DEBUG_CONTOURS 0

#define ATHENA_DEBUG_CONTOUR_NUM 1
#define ATHENA_DEBUG_CONTOUR_START 0
#define ATHENA_DEBUG_CONTOUR_END 3

#define ATHENA_DEBUG_CONTOUR_UNTIL(i) \
if (i >= ATHENA_DEBUG_CONTOUR_NUM) { ++i; continue; }
#define ATHENA_DEBUG_SPECIFIC_CONTOUR(i) \
if (i != ATHENA_DEBUG_CONTOUR_NUM) { ++i; continue; }
#define ATHENA_DEBUG_CONTOUR_RANGE(i) \
if (i < ATHENA_DEBUG_CONTOUR_START || i > ATHENA_DEBUG_CONTOUR_END) { ++i; continue; }
#endif


namespace athena
{
    namespace polygonizer
    {
        Bsoid::Bsoid() :
            mName("model")
        { }

        Bsoid::Bsoid(tree::BlobTree const& model, std::string const& name) :
            mTree(std::make_unique<tree::BlobTree>(model)),
            mName(name)
        { }

        Bsoid::Bsoid(Bsoid&& b) :
            mLattice(std::move(b.mLattice)),
            mTree(std::move(b.mTree)),
            mCrossSectionDelta(b.mCrossSectionDelta),
            mCrossSections(std::move(b.mCrossSections)),
            mLog(std::move(b.mLog)),
            mMesh(std::move(b.mMesh)),
            mName(b.mName)
        { }

        void Bsoid::setModel(tree::BlobTree const& model)
        {
            mTree = std::make_unique<tree::BlobTree>(model);
        }

        void Bsoid::setCrossSectionDelta(float delta, SlicingAxes const& axis)
        {
            // Grab the box of the model.
            auto box = mTree->getTreeBox();
            auto slices = (box.pMax - box.pMin) / delta;
            std::size_t numSlices = 0;

            switch (axis)
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

        void Bsoid::setNumCrossSections(std::size_t num, SlicingAxes const& axis)
        {
            auto box = mTree->getTreeBox();
            auto deltas = (box.pMax - box.pMin) / static_cast<float>(num - 1);
            switch (axis)
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
            return 0;
        }

        void Bsoid::makeCrossSections(SlicingAxes const& axis,
            std::uint32_t gridSize, std::uint32_t svSize)
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
            switch (axis)
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
                    axis, min, max, gridSize, svSize, mTree.get());

                switch (axis)
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
            switch (axis)
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
                std::make_unique<CrossSection>(axis, min, max, gridSize, svSize,
                    mTree.get());
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
                t.start();
                section->constructLattice();
                auto duration = t.elapsed();
                mLog << "Generated lattice " << i << " in: " << duration
                    << " seconds.\n";
                ++i;
            }

            auto time = global.elapsed();
            mLog << "Total lattice generation time: " << time << " seconds\n";

            std::vector<Voxel> voxels;
            i = 0;
            for (auto& section : mCrossSections)
            {
                voxels.insert(voxels.end(), section->getVoxels().begin(),
                    section->getVoxels().end());
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
            std::vector<std::vector<FieldPoint>> contours;
            i = 0;
            for (auto& section : mCrossSections)
            {
                contours.insert(contours.end(), section->getContour().begin(),
                    section->getContour().end());
                ++i;
            }

            mContour.makeContour(contours);
        }

        void Bsoid::constructMesh()
        {
            atlas::core::Timer<float> global;
            atlas::core::Timer<float> t;
            int i = 0;

            // First we need to subdivide the contours into the largest
            // size that we have.
            std::size_t size = 0;
            for (auto& section : mCrossSections)
            {
                if (size < section->getLargestContourSize())
                {
                    size = section->getLargestContourSize();
                }
            }

            // Now that we have it, lets resize all of the contours. Again
            // this can be done in parallel.
            for (auto& section : mCrossSections)
            {
                section->resizeContours(size);
                ++i;
            }


            // The contours all have the same sizes, so its just a matter of
            // connecting everything together. 
            connectContours();

            std::vector<std::vector<FieldPoint>> contours;
            i = 0;
            for (auto& section : mCrossSections)
            {
                contours.insert(contours.end(), section->getContour().begin(),
                    section->getContour().end());
                ++i;
            }

            mContour.makeContour(contours);
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
                auto contour = section->getContour();
                // Remove when we solve branching.
                //assert(contour.size() == 1);

                for (auto& ring : contour)
                {
                    for (auto& pt : ring)
                    {
                        mMesh.vertices().push_back(pt.value.xyz());
                        mMesh.normals().push_back(pt.g);
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

                // They are not caps, so we need to create the quads.
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
