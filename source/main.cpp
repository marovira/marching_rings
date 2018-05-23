#include "athena/Athena.hpp"


#include "athena/visualizer/ModelView.hpp"
#include "athena/visualizer/ModelVisualizer.hpp"
#include "athena/models/Models.hpp"

#include <atlas/core/Log.hpp>
#include <atlas/utils/Application.hpp>
#include <atlas/utils/WindowSettings.hpp>
#include <atlas/gl/ErrorCheck.hpp>

#include <fstream>

std::vector<athena::models::ModelFn> getModels()
{
    using namespace athena::models;
    std::vector<athena::models::ModelFn> result;
    //result.push_back(makeSphere);
    result.push_back(makePeanut);
    //result.push_back(makeCylinder);
    //result.push_back(makeCone);
    //result.push_back(makeTorus);

    return result;
}

std::vector<athena::models::MCModelFn> getMCModels()
{
    using namespace athena::models;
    std::vector<MCModelFn> result;
    //result.push_back(makeMCSphere);
    //result.push_back(makeMCPeanut);
    //result.push_back(makeMCCylinder);
    //result.push_back(makeMCCone);
    //result.push_back(makeMCTorus);

    return result;
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
    std::vector<athena::polygonizer::MarchingCubes> mcModels;
    auto modelFns = getModels();
    auto mcModelFns = getMCModels();
    for (auto& modelFn : modelFns)
    {
        models.emplace_back(modelFn());
    }

    for (auto& mcFn : mcModelFns)
    {
        mcModels.emplace_back(mcFn());
    }

    atlas::gl::setGLErrorSeverity(ATLAS_GL_ERROR_SEVERITY_HIGH |
        ATLAS_GL_ERROR_SEVERITY_MEDIUM);

    WindowSettings settings;
    settings.contextVersion = ContextVersion(4, 5);
    settings.isForwardCompat = TRUE;
    settings.isMaximized = true;
    settings.title = "Athena " + std::string(ATHENA_VERSION_STRING);

    Application::getInstance().createWindow(settings);
    Application::getInstance().addScene(ScenePointer(
        new athena::visualizer::ModelVisualizer(models, mcModels)));
    Application::getInstance().runApplication();

    return 0;
}

#else

int main()
{
    INFO_LOG_V("Welcome to Athena %s", ATHENA_VERSION_STRING);

    auto modelFns = getModels();
    auto mcModelFns = getMCModels();

    std::fstream file("summary.txt", std::fstream::out);

    for (std::size_t i = 0; i < modelFns.size(); ++i)
    {
        INFO_LOG_V("Polygonizing model %d", i + 1);
        auto soid = modelFns[i]();
        soid.polygonize();
        std::string log = soid.getLog();
        file << log;
        soid.saveMesh();

        auto mc = mcModelFns[i]();
        mc.polygonize();
        log = mc.getLog();
        file << "\n";
        file << log;
        mc.saveMesh();
    }

    file.close();

    return 0;
}

#endif