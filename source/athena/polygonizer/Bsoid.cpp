#include "athena/polygonizer/Bsoid.hpp"

#include <atlas/core/Timer.hpp>

#include <numeric>
#include <functional>
#include <unordered_set>
#include <fstream>


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
            switch (axis)
            {
            case SlicingAxes::XAxis:
                max = Point(box.pMin.x, box.pMax.yz());
                break;

            case SlicingAxes::YAxis:
                max = Point(box.pMax.x, box.pMin.y, box.pMax.z);
                break;

            case SlicingAxes::ZAxis:
                max = Point(box.pMax.xy(), box.pMin.z);
                break;
            }

            for (std::size_t i = 0; i < mCrossSections.size(); ++i)
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
            for (auto& section : mCrossSections)
            {
                voxels.insert(voxels.end(), section->getVoxels().begin(),
                    section->getVoxels().end());
            }

            mLattice.makeLattice(voxels);
        }

        Lattice const& Bsoid::getLattice() const
        {
            return mLattice;
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

        void Bsoid::saveLattice() const
        {
            std::string filename = mName + "_lattice.obj";
            std::fstream file(filename, std::fstream::out);

            file.close();
        }
    }
}
