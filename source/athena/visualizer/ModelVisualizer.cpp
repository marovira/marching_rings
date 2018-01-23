#include "athena/visualizer/ModelVisualizer.hpp"

#include <atlas/gl/GL.hpp>
#include <atlas/utils/GUI.hpp>

#include <cstdarg>
#include <algorithm>

namespace gl = atlas::gl;
namespace math = atlas::math;
namespace utils = atlas::utils;
namespace tools = atlas::tools;

namespace athena
{
    namespace visualizer
    {
        ModelVisualizer::ModelVisualizer() :
            mCurrentView(0),
            ModellingScene()
        {
            //for (auto&& models : models)
            //{
            //    mViews.emplace_back(std::move(model));
            //}
        }

        void ModelVisualizer::renderScene()
        {
            utils::Gui::getInstance().newFrame();
            const float grey = 92.0f / 255.0f;
            glClearColor(grey, grey, grey, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            mProjection = glm::perspective(
                glm::radians(mCamera.getCameraFOV()),
                (float)mWidth / mHeight, 1.0f, 1000000.0f);

            mUniformMatrixBuffer.bindBuffer();
            mUniformMatrixBuffer.bufferSubData(0,
                gl::size<math::Matrix4>(1), &mProjection[0][0]);
            mUniformMatrixBuffer.unBindBuffer();

            mView = mCamera.getCameraMatrix();
            mUniformMatrixBuffer.bindBuffer();
            mUniformMatrixBuffer.bufferSubData(gl::size<math::Matrix4>(1),
                gl::size<math::Matrix4>(1), &mView[0][0]);

            if (mShowGrid)
            {
                mGrid.renderGeometry(mProjection, mView);
            }

            // Global HUD.
            ImGui::SetNextWindowSize(ImVec2(350, 140), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Global HUD");
            ImGui::Checkbox("Show grid", &mShowGrid);
            if (ImGui::Button("Reset Camera"))
            {
                mCamera.resetCamera();
            }

            // Model select.
            std::vector<std::string> strNames;
            for (auto& view : mViews)
            {
                strNames.push_back(view.getModelName());
            }

            std::vector<const char*> names;
            std::transform(strNames.begin(), strNames.end(),
                std::back_inserter(names),
                [](std::string const& str) { return str.c_str(); });

            ImGui::Combo("Select model", &mCurrentView, names.data(),
                ((int)names.size()));

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();

            // Render the GUI for the current view.
            //mViews[mCurrentView].drawGui();

            ImGui::Render();

            // Now render the view.
            //mViews[mCurrentView].renderGeometry();
        }
    }
}