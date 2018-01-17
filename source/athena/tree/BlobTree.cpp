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
            mSeeds.push_back(field->getSeed());
        }

        void BlobTree::inserFields(
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
            mTree = mNodes[tree.size() - 1];
            mNodes.clear();
        }

        std::vector<fields::ImplicitFieldPtr> BlobTree::getOverlappingFields(
            core::BBox const& box) const
        {
            return mTree->visit(box);
        }

        core::BBox BlobTree::getTreeBox() const
        {
            return mTree->getBBox();
        }

        std::vector<core::Point> BlobTree::getSeeds() const
        {
            return mSeeds;
        }
    }
}