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
                getOverlappingFields(atlas::utils::BBox const& box) const;

            atlas::utils::BBox getTreeBox() const;
            std::vector<atlas::math::Point> getSeeds(
                atlas::math::Normal const& u, float offset) const;

        private:
            std::vector<NodePtr> mNodes;
            NodePtr mTree;
        };
    }
}

#endif