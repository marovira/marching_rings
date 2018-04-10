#include "athena/polygonizer/BranchingManager.hpp"

#include <atlas/core/Assert.hpp>
#include <atlas/core/Constants.hpp>

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
                        multiBranch(top, bottom);
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

        void BranchingManager::multiBranch(Slice const& top, Slice const& bottom)
        {
            // The idea is the following: we basically connect each contour
            // with the one that is closest (geometrically) on the following
            // slice. Now admittedly a first implementation will run in O(n^2),
            // but I'm sure there's a faster way of doing this.
            
            std::vector<std::size_t> doneContours;
            for (auto& topContour : top)
            {
                auto topPt = mMesh.vertices()[topContour[0]];
                std::size_t index = 0;
                float distance = atlas::core::infinity();
                for (std::size_t i = 0; i < bottom.size(); ++i)
                {
                    auto bottomPt = mMesh.vertices()[bottom[i][0]];
                    auto d = glm::distance(topPt, bottomPt);
                    if (d < distance && 
                        std::find(doneContours.begin(), doneContours.end(), i) == doneContours.end())
                    {
                        distance = d;
                        index = i;
                    }
                }

                // At this point we now have the index of the minimal distance,
                // so we create two dummy slices and send them to be linked.
                Slice t{ topContour };
                Slice b{ bottom[index] };
                singleBranch(t, b);
                doneContours.push_back(index);
            }
        }
    }
}
