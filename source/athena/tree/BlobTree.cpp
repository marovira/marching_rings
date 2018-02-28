#include "athena/tree/BlobTree.hpp"

namespace athena
{
    namespace tree
    {
        BlobTree::BlobTree()
        { }

        void BlobTree::insertField(fields::ImplicitFieldPtr const& field)
        {
            mNodes.push_back(std::make_shared<Node>(field));
        }

        void BlobTree::insertFields(
            std::vector<fields::ImplicitFieldPtr> const& fields)
        {
            for (auto& field : fields)
            {
                insertField(field);
            }
        }

        void BlobTree::insertNodeTree(std::vector<std::vector<int>> const& tree)
        {
            // The idea is the following:
            // We are given a list of all of the children that each node (that
            // is, each field that was inserted) has. So all that we have to
            // do is construct the volume tree using the indices that
            // we are given.
            for (std::size_t i = 0; i < tree.size(); ++i)
            {
                auto node = mNodes[i];
                for (auto& nodeIdx : tree[i])
                {
                    if (nodeIdx == -1)
                    {
                        // This node has no children, so do nothing.
                        continue;
                    }

                    node->addChild(mNodes[nodeIdx]);
                }
            }

            // The final index is the parent, so just assign that and clear
            // the copies of the node.
            mVolumeTree = mNodes[tree.size() - 1];
        }

        void BlobTree::insertFieldTree(fields::ImplicitFieldPtr const& tree)
        {
            mFieldTree = tree;
        }

        float BlobTree::eval(atlas::math::Point const& p) const
        {
            // Theoretically, we should be able to do this:
            //using atlas::utils::BBox;
            //auto subTree = getSubTree(BBox(p, p));
            //return subTree->eval(p);
            return mFieldTree->eval(p);
        }

        atlas::math::Normal BlobTree::grad(atlas::math::Point const& p) const
        {
            return mFieldTree->grad(p);
        }

        fields::ImplicitFieldPtr BlobTree::getSubTree(
            atlas::utils::BBox const& box) const
        {
            return mVolumeTree->subTree(box);
        }

        atlas::utils::BBox BlobTree::getTreeBox() const
        {
            return mVolumeTree->getBBox();
        }

        std::vector<atlas::math::Point> BlobTree::getSeeds(
            atlas::math::Normal const& u, float offset) const
        {
            return mFieldTree->getSeeds(u, offset);
        }
    }
}