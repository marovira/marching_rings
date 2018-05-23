#include "athena/polygonizer/BranchingManager.hpp"

#include <atlas/core/Assert.hpp>
#include <atlas/core/Constants.hpp>

namespace athena
{
    namespace polygonizer
    {
        BranchingManager::BranchingManager()
        { }

        std::vector<BranchingManager::BranchData> 
            BranchingManager::findBranches(std::vector<std::size_t> const& slices)
        {
            std::vector<BranchData> result;
            for (std::size_t i = 0; i < slices.size() - 1; ++i)
            {
                std::size_t top    = slices[i + 0];
                std::size_t bottom = slices[i + 1];

                // Ignore empty slices.
                if (top == 0 && bottom == 0)
                {
                    continue;
                }

                // Single or multi-branches are easy to handle, so ignore them.
                if (top == bottom)
                {
                    continue;
                }

                // If one of the two is 0 and the other one isn't, then we have
                // a cap.
                if ((top == 0 && bottom != 0) || (top != 0 && bottom == 0))
                {
                    result.emplace_back(i, i + 1, BranchType::Cap);
                    continue;
                }

                if (top != 0 && bottom != 0 && top != bottom)
                {
                    if (top < bottom)
                    {
                        result.emplace_back(i, i + 1, BranchType::Branch);
                    }
                    else
                    {
                        result.emplace_back(i + 1, i, BranchType::Branch);
                    }
                }
            }

            return result;
        }

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
                        manyToManyBranch(top, bottom);
                    }
                }
                else
                {
                    //multiBranch(top, bottom);
                }
            }

            return mMesh;
        }

        void BranchingManager::singleBranch(Slice const& top, Slice const& bottom)
        {
            auto topRing = top[0];
            auto bottomRing = bottom[0];
            auto size = topRing.size();

            // This "fixes" the twisting that was occuring in the quads.
            // The idea is the following: since the contours can't vary too much
            // between layers, then they will be roughly offset by a couple of
            // voxels tops (in theory). So in reality what needs to be done is 
            // just to find the point in the lower layer that has the shortest
            // distance with the first vertex of the upper layer. This will
            // ensure that all of the quads are aligned correctly. Now a somewhat
            // better way of doing this would be to check a relatively small 
            // neighbourhood around the first vertex in the lower layer, since
            // that will save up in search time.
            auto seed = mMesh.vertices()[topRing[0]];
            float distance = atlas::core::infinity();
            std::size_t start = 0;
            for (std::size_t i = 0; i < size; ++i)
            {
                if (glm::distance(mMesh.vertices()[bottomRing[i]], seed) < distance)
                {
                    distance = glm::distance(mMesh.vertices()[bottomRing[i]], seed);
                    start = i;
                }
            }

            for (std::size_t i = 0; i < size; ++i)
            {
                mMesh.indices().push_back(topRing[i]);
                mMesh.indices().push_back(bottomRing[(start + i) % size]);
                mMesh.indices().push_back(bottomRing[(start + i + 1) % size]);

                mMesh.indices().push_back(bottomRing[(start + i + 1) % size]);
                mMesh.indices().push_back(topRing[(i + 1) % size]);
                mMesh.indices().push_back(topRing[i]);
            }
        }

        void BranchingManager::manyToManyBranch(Slice const& top, Slice const& bottom)
        {
            // The idea is the following: we basically connect each contour
            // with the one that is closest (geometrically) on the following
            // slice. Now admittedly a first implementation will run in O(n^2),
            // but I'm sure there's a faster way of doing this.

            // Now the problem with the above approach is that it relies on us
            // being able to compute the intersection and union of the two mesh 
            // strips which could get expensive (and difficult). That said though,
            // it tries to emulate the case of CT data, which doesn't have a whole
            // lot of information on the underlying surface. So an alternative
            // formulation could be this (and yes, I'm well aware that this may
            // not work for more convoluted branches, but bear with me): 
            // compute the centre of gravity of each of the contours in the 
            // branch and then compute their intersection (in the case of just 
            // two contours take the midpoint). Then project that point up into
            // the other slice. If I'm right (I know I could be wrong but whatever),
            // then the point should be inside the surface and have a value less
            // than 0. So with that in mind we can perform linear interpolation
            // to obtain an approximation for the "saddle" point (or at least a
            // point in the vicinity of the saddle). Once we have that, take
            // the "height" and create a new slice there. The trick is to ensure
            // that the "saddle" is included in the contour(s) that are generated
            // and then just connect them (if the slice generates disjoint
            // contours then we can just connect them using the point). The 
            // unsolved part of all this is how the new slice would get joined
            // with the branching slice, since the number of vertices won't
            // match, but that's another story.
            
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

        void BranchingManager::multiBranch(Slice const& t, Slice const& b)
        {
            // First figure out which slice is the one with the least
            Slice top = (t.size() < b.size()) ? t : b;
            Slice bottom = (t.size() < b.size()) ? b : t;

            // Now, for each contour in the top slice, connect it with every
            // contour in the bottom slice.
            for (auto& contour : top)
            {
                for (auto& ring : bottom)
                {
                    // Dummy code to see the intersection. We may need to create
                    // a separate mesh to process it.
                    Slice s1{ contour };
                    Slice s2{ ring };
                    singleBranch(s1, s2);
                }
            }
        }
    }
}
