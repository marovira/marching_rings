#include "athena/visualizer/ModelView.hpp"
#include "athena/ShaderPaths.hpp"
#include "athena/global/LayoutLocations.glsl"

#include <atlas/utils/GUI.hpp>
#include <atlas/core/Enum.hpp>

enum class ShaderNames : int
{
    Lattice = 0
};

namespace gl = atlas::gl;
namespace math = atlas::math;

namespace athena
{
    namespace visualizer
    {
        ModelView::ModelView() :
            mLatticeData(GL_ARRAY_BUFFER),
            mLatticeIndices(GL_ELEMENT_ARRAY_BUFFER),
            mLatticeNumIndices(0),
            mShowLattices(false),
            mRenderMode(0)
        {
            using atlas::core::enumToUnderlyingType;

            // Load the lattice shaders first.
            std::vector<gl::ShaderUnit> latticeShaders
            {
                { std::string(ShaderDirectory) +
                "athena/visualizer/Lattice.vs.glsl", GL_VERTEX_SHADER },
                { std::string(ShaderDirectory) +
                "athena/visualizer/Lattice.fs.glsl", GL_FRAGMENT_SHADER }
            };

            mShaders.push_back(gl::Shader(latticeShaders));

            for (auto& shader : mShaders)
            {
                shader.setShaderIncludeDir(ShaderDirectory);
                shader.compileShaders();
                shader.linkShaders();
            }

            // Grab the lattice uniforms.
            auto latticeIndex = enumToUnderlyingType(ShaderNames::Lattice);
            auto var = mShaders[latticeIndex].getUniformVariable("model");
            mUniforms.insert(UniformKey("lattice_model", var));

            for (auto& shader : mShaders)
            {
                shader.disableShaders();
            }
        }

        std::string ModelView::getModelName() const
        {
            //return mCSModel.getName();
            return "";
        }

        void ModelView::renderGeometry()
        {
            using atlas::core::enumToUnderlyingType;
            for (auto& shader : mShaders)
            {
                shader.hotReloadShaders();
                if (!shader.shaderProgramValid())
                {
                    return;
                }
            }

            if (mShowLattices)
            {
                auto latticeIndex = enumToUnderlyingType(ShaderNames::Lattice);
                mShaders[latticeIndex].enableShaders();

                glUniformMatrix4fv(mUniforms["lattice_model"], 1, GL_FALSE,
                    &mModel[0][0]);

                mLatticeVao.bindVertexArray();
                mLatticeIndices.bindBuffer();
                glDrawElements(GL_LINES, (GLsizei)mLatticeNumIndices,
                    GL_UNSIGNED_INT, gl::bufferOffset<GLuint>(0));
                mLatticeIndices.unBindBuffer();
                mLatticeVao.unBindVertexArray();

                mShaders[latticeIndex].disableShaders();
            }
        }

        void ModelView::drawGui()
        {
            // Polygonizer controls window.
            ImGui::SetNextWindowSize(ImVec2(470, 400), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Polygonizer Controls");
            ImGui::Text("Generation controls");
            ImGui::Separator();
            if (ImGui::Button("Construct Cross-sections"))
            {
                constructLattices();
            }

            if (ImGui::Button("Save model"))
            {

            }

            ImGui::Dummy(ImVec2(0, 10));
            ImGui::Text("Visualization Options");
            ImGui::Separator();
            ImGui::Checkbox("Show lattices", &mShowLattices);

            ImGui::Dummy(ImVec2(0, 10));
            ImGui::Text("Log");
            ImGui::Separator();
            ImGui::BeginChild("Log", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()),
                false, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
            //ImGui::TextWrapped(mCSModel.getLog().c_str());
            ImGui::EndChild();
            ImGui::End();

            // Render controls.
            ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Render Controls");

            std::vector<const char*> renderNames = { "Vertices", "Contour LInes",
            "Wireframe", "Shaded", "Normals" };
            ImGui::Combo("Render mode", &mRenderMode, renderNames.data(),
                ((int)renderNames.size()));
            ImGui::End();
        }

        void ModelView::constructLattices()
        {
            // TODO: Fill me in!
        }
    }
}