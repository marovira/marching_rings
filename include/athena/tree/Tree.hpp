#ifndef ATHENA_INCLUDE_ATHENA_TREE_TREE_HPP
#define ATHENA_INCLUDE_ATHENA_TREE_TREE_HPP

#pragma once

#include <memory>

namespace athena
{
    namespace tree
    {
        class Node;
        class BlobTree;

        using NodePtr = std::shared_ptr<Node>;
    }
}

#endif