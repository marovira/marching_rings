#include "athena/tree/Node.hpp"
#include "athena/operators/ImplicitOperator.hpp"

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

        fields::ImplicitFieldPtr Node::subTree(
            atlas::utils::BBox const& cell) const
        {
            using operators::ImplicitOperatorPtr;
            using operators::ImplicitOperator;
            // Check if we have an overlap. If we don't return null.
            if (!mBox.overlaps(cell))
            {
                return nullptr;
            }

            // We know the cell is in our box. If we don't have any children,
            // we are a leaf, so just return the field as is.
            if (mChildren.empty())
            {
                return mField;
            }

            // So we know that the point is in our box, and we have children.
            // This means (and it should mean) that we have have an operator
            // as our field. So first lets cast the pointer.
            ImplicitOperatorPtr op = 
                std::dynamic_pointer_cast<ImplicitOperator>(mField);
            assert(op);

            // We have successfully converted the pointer, so let's make a new
            // empty copy of the pointer.
            auto result = op->makeEmpty();

            for (auto& child : mChildren)
            {
                auto childField = child->subTree(cell);
                if (childField)
                {
                    result->insertField(childField);
                }
            }

            return result;
        }
    }
}