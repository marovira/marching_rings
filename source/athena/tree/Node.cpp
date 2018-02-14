#include "athena/tree/Node.hpp"

namespace athena
{
    namespace tree
    {
        Node::Node()
        { }

        Node::Node(fields::ImplicitFieldPtr const& field) :
            mField(field),
            mBox(mField->getBBox())
        { }

        void Node::setField(fields::ImplicitFieldPtr const& field)
        {
            mField = field;
            mBox = field->getBBox();
        }

        atlas::utils::BBox Node::getBBox() const
        {
            return mBox;
        }

        void Node::addChild(NodePtr const& child)
        {
            using atlas::utils::join;

            mBox = join(mBox, child->getBBox());
            mChildren.push_back(child);
        }

        void Node::addChildren(std::vector<NodePtr> const& children)
        {
            for (auto& child : children)
            {
                addChild(child);
            }
        }

        std::vector<NodePtr> Node::getChildren() const
        {
            return mChildren;
        }

        void Node::setParent(NodePtr const& parent)
        {
            mParent = parent;
        }

        NodePtr Node::getParent() const
        {
            return mParent;
        }

        std::vector<fields::ImplicitFieldPtr> Node::visit(
            atlas::utils::BBox const& cell) const
        {
            // First check against our box. If the cell isn't inside,
            // return immediately.
            if (!mBox.overlaps(cell))
            {
                return {};
            }

            // We know the cell is inside our box, so now we check if we have
            // children. If we don't the we are a leaf and we return the field
            // we hold.
            if (mChildren.empty())
            {
                return { mField };
            }

            // We are not a leaf node, so we need to figure out which of our
            // children contains the point.
            std::vector<std::size_t> overlaps;
            std::size_t i = 0;
            for (auto& child : mChildren)
            {
                if (child->getBBox().overlaps(cell))
                {
                    overlaps.push_back(i);
                }

                ++i;
            }

            std::vector<fields::ImplicitFieldPtr> ret;
            if (mField->getBBox().overlaps(cell))
            {
                ret.push_back(mField);
            }

            for (auto& idx : overlaps)
            {
                auto v = mChildren[idx]->visit(cell);
                ret.insert(ret.end(), v.begin(), v.end());
            }

            return ret;
        }

        std::vector<atlas::math::Point> Node::getSeeds(
            atlas::math::Normal const& u) const
        {
            return mField->getSeeds(u);
        }
    }
}