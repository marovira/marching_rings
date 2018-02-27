#ifndef ATHENA_INCLUDE_ATHENA_TREE_NODE_HPP
#define ATHENA_INCLUDE_ATHENA_TREE_NODE_HPP

#pragma once

#include "Tree.hpp"
#include "athena/fields/ImplicitField.hpp"

#include <vector>

namespace athena
{
    namespace tree
    {
        class Node
        {
        public:
            Node();
            Node(fields::ImplicitFieldPtr const& field);
            ~Node() = default;

            void setField(fields::ImplicitFieldPtr const& field);
            atlas::utils::BBox getBBox() const;

            void addChild(NodePtr const& child);
            void addChildren(std::vector<NodePtr> const& children);
            std::vector<NodePtr> getChildren() const;

            void setParent(NodePtr const& parent);
            NodePtr getParent() const;

            fields::ImplicitFieldPtr subTree(
                atlas::utils::BBox const& cell) const;

        private:
            fields::ImplicitFieldPtr mField;
            NodePtr mParent;
            std::vector<NodePtr> mChildren;
            atlas::utils::BBox mBox;
        };
    }
}

#endif