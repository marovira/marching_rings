#include "athena/Athena.hpp"

#include "athena/visualizer/ModelView.hpp"
#include "athena/visualizer/ModelVisualizer.hpp"

#include "athena/fields/Sphere.hpp"
#include "athena/operators/Blend.hpp"
#include "athena/tree/BlobTree.hpp"
#include "athena/polygonizer/Bsoid.hpp"

#include <atlas/core/Log.hpp>
#include <atlas/utils/Application.hpp>
#include <atlas/utils/WindowSettings.hpp>
#include <atlas/gl/ErrorCheck.hpp>

athena::polygonizer::Bsoid makeSphere()
{
    // Polygonizer using.
    using athena::fields::ImplicitFieldPtr;
    using athena::fields::Sphere;
    using athena::tree::BlobTree;
    using athena::polygonizer::Bsoid;
    using athena::polygonizer::SlicingAxes;

    ImplicitFieldPtr sphere = std::make_shared<Sphere>();
    BlobTree tree;
    tree.insertField(sphere);
    tree.insertNodeTree({ { -1 } });
    tree.insertFieldTree(sphere);

    Bsoid soid(tree, "sphere");
    soid.setSlicingAxis(SlicingAxes::YAxis);
    soid.setNumCrossSections(16);
    soid.makeCrossSections(8, 4);
    return soid;
}

athena::polygonizer::Bsoid makePeanut()
{
    // Polygonizer using.
    using atlas::math::Point;
    using athena::fields::ImplicitFieldPtr;
    using athena::operators::ImplicitOperatorPtr;
    using athena::fields::Sphere;
    using athena::operators::Blend;
    using athena::tree::BlobTree;
    using athena::polygonizer::Bsoid;
    using athena::polygonizer::SlicingAxes;

    ImplicitFieldPtr sphere1 = std::make_shared<Sphere>(1.0f, Point(1.0f, 0, 0));
    ImplicitFieldPtr sphere2 = std::make_shared<Sphere>(1.0f, Point(-1.0f, 0, 0));
    ImplicitOperatorPtr blend = std::make_shared<Blend>();
    blend->insertFields({ sphere1, sphere2 });

    BlobTree tree;
    tree.insertFields({ sphere1, sphere2, blend });
    tree.insertNodeTree({ { -1 }, { -1 }, { 0, 1 } });
    tree.insertFieldTree(blend);

    Bsoid soid(tree, "peanut");
    soid.setSlicingAxis(SlicingAxes::XAxis);
    soid.setNumCrossSections(32);
    soid.makeCrossSections(16, 4);
    return soid;
}



#if (ATHENA_USE_GUI)

int main()
{
    // Atlas using.
    using atlas::utils::WindowSettings;
    using atlas::utils::ContextVersion;
    using atlas::utils::Application;
    using atlas::utils::ScenePointer;

    std::vector<athena::polygonizer::Bsoid> models;
    models.emplace_back(makeSphere());
    //models.emplace_back(makePeanut());

    atlas::gl::setGLErrorSeverity(ATLAS_GL_ERROR_SEVERITY_HIGH |
        ATLAS_GL_ERROR_SEVERITY_MEDIUM);

    WindowSettings settings;
    settings.contextVersion = ContextVersion(4, 5);
    settings.isForwardCompat = TRUE;
    settings.isMaximized = true;
    settings.title = "Athena " + std::string(ATHENA_VERSION_STRING);

    Application::getInstance().createWindow(settings);
    Application::getInstance().addScene(ScenePointer(
        new athena::visualizer::ModelVisualizer(models)));
    Application::getInstance().runApplication();

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