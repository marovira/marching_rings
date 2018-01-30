#include "athena/Athena.hpp"

#include "athena/visualizer/ModelView.hpp"
#include "athena/visualizer/ModelVisualizer.hpp"

#include "athena/fields/Sphere.hpp"
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
    tree.insertNodeTree({ {-1} });

    return Bsoid(tree, "sphere");
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