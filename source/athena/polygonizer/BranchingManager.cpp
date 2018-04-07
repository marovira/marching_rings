#include "athena/polygonizer/BranchingManager.hpp"

#include <atlas/core/Assert.hpp>

namespace athena
{
    namespace polygonizer
    {
        BranchingManager::BranchingManager()
        { }

        void BranchingManager::insertContours(
            std::vector<std::vector<FieldPoint>> const& slice)
        {
            Slice s;
            for (auto& ring : slice)
            {
                std::vector<std::uint32_t> contour;
                for (auto& pt : ring)
                {
                    mMesh.vertices().push_back(pt.value.xyz());
                    mMesh.normals().push_back(-pt.g);
                    contour.push_back(mMesh.vertices().size() - 1);
                }
                s.push_back(contour);
            }
            mSlices.push_back(s);
        }

        atlas::utils::Mesh BranchingManager::connectContours()
        {
            // Iterate over each pair of slices.
            for (std::size_t i = 0; i < mSlices.size() - 1; i++)
            {
                Slice top = mSlices[i + 0];
                Slice bottom = mSlices[i + 1];

                // Check if either of the slices is 0.
                if (top.empty() || bottom.empty())
                {
                    continue;
                }

                if (top.size() == bottom.size())
                {
                    // We either have a single branch or the same number of
                    // branches.
                    if (top.size() == 1)
                    {
                        singleBranch(top, bottom);
                    }
                    else
                    {
                        ATLAS_ASSERT(false, "Multiple branches isn't solved yet.");
                    }
                }
                else
                {
                    ATLAS_ASSERT(false, "Branching is not allowed yet.");
                }
            }

            return mMesh;
        }

        void BranchingManager::singleBranch(Slice const& top, Slice const& bottom)
        {
            auto topRing = top[0];
            auto bottomRing = bottom[0];
            auto size = topRing.size();

            for (std::size_t i = 0; i < size; ++i)
            {
                mMesh.indices().push_back(topRing[i]);
                mMesh.indices().push_back(bottomRing[i]);
                mMesh.indices().push_back(bottomRing[(i + 1) % size]);

                mMesh.indices().push_back(bottomRing[(i + 1) % size]);
                mMesh.indices().push_back(topRing[(i + 1) % size]);
                mMesh.indices().push_back(topRing[i]);
            }
        }
    }
}
