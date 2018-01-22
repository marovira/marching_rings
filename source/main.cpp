#include "athena/Athena.hpp"

#include "athena/fields/Sphere.hpp"
#include "athena/tree/BlobTree.hpp"
//#include "athena/blob/Bsoid.hpp"

#include <atlas/core/Log.hpp>

#if (ATHENA_USE_GUI)

int main()
{
    return 0;
}

#else

int main()
{
    using athena::fields::ImplicitFieldPtr;
    using athena::fields::Sphere;
    using athena::tree::BlobTree;
    //using athena::blob::Bsoid;

    INFO_LOG_V("Welcome to Athena %s", ATHENA_VERSION_STRING);
    INFO_LOG("Building model...");

    //ImplicitFieldPtr sphere = std::make_shared<Sphere>();
    //BlobTree tree;
    //tree.insertField(sphere);
    //tree.insertNodeTree({ {-1} });

    //INFO_LOG("Model done!");
    //INFO_LOG("Starting polygonization...");
    //Bsoid soid;
    //soid.setModel(tree);
    //soid.setName("sphere");
    //soid.polygonize(4, 8);
    //soid.saveCubicLattice();
    //INFO_LOG("Polygonization done!");
    return 0;
}

#endif