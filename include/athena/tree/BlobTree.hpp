#ifndef ATHENA_INCLUDE_ATHENA_TREE_BLOB_TREE_HPP
#define ATHENA_INCLUDE_ATHENA_TREE_BLOB_TREE_HPP

#pragma once

#include "Tree.hpp"
#include "Node.hpp"
#include "athena/fields/ImplicitField.hpp"

#include <vector>

namespace athena
{
    namespace tree
    {
        class BlobTree
        {
        public:
            BlobTree();
            ~BlobTree() = default;

            void insertField(fields::ImplicitFieldPtr const& field);
            void inserFields(
                std::vector<fields::ImplicitFieldPtr> const& fields);
            void insertNodeTree(std::vector<std::vector<int>> const& tree);

            std::vector<fields::ImplicitFieldPtr>
                getOverlappingFields(core::BBox const& box) const;

            core::BBox getTreeBox() const;
            std::vector<core::Point> getSeeds() const;

        private:
            std::vector<core::Point> mSeeds;
            std::vector<NodePtr> mNodes;
            NodePtr mTree;
        };
    }
}

#endif