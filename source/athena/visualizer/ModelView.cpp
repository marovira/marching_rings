#include "athena/visualizer/ModelView.hpp"
#include "athena/ShaderPaths.hpp"
#include "athena/global/LayoutLocations.glsl"

#include <atlas/utils/GUI.hpp>
#include <atlas/core/Enum.hpp>

enum class ShaderNames : int
{
    Lattice = 0,
    Contour,
    Mesh
};

namespace gl = atlas::gl;
namespace math = atlas::math;

namespace athena
{
    namespace visualizer
    {
        ModelView::ModelView(polygonizer::Bsoid&& soid) :
            mSoid(std::move(soid)),
            mLatticeData(GL_ARRAY_BUFFER),
            mLatticeIndices(GL_ELEMENT_ARRAY_BUFFER),
            mLatticeNumIndices(0),
            mContourData(GL_ARRAY_BUFFER),
            mContourIndices(GL_ELEMENT_ARRAY_BUFFER),
            mContourNumIndices(0),
            mContourNumVertices(0),
            mMeshData(GL_ARRAY_BUFFER),
            mMeshIndices(GL_ELEMENT_ARRAY_BUFFER),
            mMeshNumIndices(0),
            mShowLattices(false),
            mShowContours(false),
            mShowMesh(false),
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

            // Next the contour shaders.
            std::vector<gl::ShaderUnit> contourShaders
            {
                { std::string(ShaderDirectory) +
                "athena/visualizer/Contour.vs.glsl", GL_VERTEX_SHADER },
                { std::string(ShaderDirectory) +
                "athena/visualizer/Contour.fs.glsl", GL_FRAGMENT_SHADER}
            };

            // Finally the mesh shaders.
            std::vector<gl::ShaderUnit> meshShaders
            {
                { std::string(ShaderDirectory) +
                "athena/visualizer/Mesh.vs.glsl", GL_VERTEX_SHADER },
                { std::string(ShaderDirectory) +
                "athena/visualizer/Mesh.fs.glsl", GL_FRAGMENT_SHADER} 
            };

            mShaders.push_back(gl::Shader(latticeShaders));
            mShaders.push_back(gl::Shader(contourShaders));
            mShaders.push_back(gl::Shader(meshShaders));

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

            // Now the contour uniforms.
            auto contourIndex = enumToUnderlyingType(ShaderNames::Contour);
            var = mShaders[contourIndex].getUniformVariable("model");
            mUniforms.insert(UniformKey("contour_model", var));

            var = mShaders[contourIndex].getUniformVariable("renderMode");
            mUniforms.insert(UniformKey("contour_renderMode", var));

            // Finally the mesh uniforms.
            auto meshIndex = enumToUnderlyingType(ShaderNames::Mesh);
            var = mShaders[meshIndex].getUniformVariable("model");
            mUniforms.insert(UniformKey("mesh_model", var));

            var = mShaders[meshIndex].getUniformVariable("renderMode");
            mUniforms.insert(UniformKey("mesh_renderMode", var));

            for (auto& shader : mShaders)
            {
                shader.disableShaders();
            }
        }

        std::string ModelView::getModelName() const
        {
            return mSoid.getName();
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

            if (mShowContours)
            {
                auto contourIndex = enumToUnderlyingType(ShaderNames::Contour);
                mShaders[contourIndex].enableShaders();
                auto var = mUniforms["contour_renderMode"];

                glUniformMatrix4fv(mUniforms["contour_model"], 1, GL_FALSE,
                    &mModel[0][0]);

                mContourVao.bindVertexArray();
                mContourIndices.bindBuffer();

                // First draw the vertices.
                glUniform1i(var, 0);
                glDrawArrays(GL_POINTS, 0, mContourNumVertices);

                // Now draw the actual contours.
                glUniform1i(var, 1);
                glDrawElements(GL_LINES, (GLsizei)mContourNumIndices,
                    GL_UNSIGNED_INT, gl::bufferOffset<GLuint>(0));
                mContourIndices.unBindBuffer();
                mContourVao.unBindVertexArray();
            }

            if (mShowMesh)
            {
                auto meshIndex = enumToUnderlyingType(ShaderNames::Mesh);
                mShaders[meshIndex].enableShaders();
                auto var = mUniforms["mesh_renderMode"];

                glUniformMatrix4fv(mUniforms["mesh_model"], 1, GL_FALSE,
                    &mModel[0][0]);
                mMeshVao.bindVertexArray();
                mMeshIndices.bindBuffer();

                if (mRenderMode == 0)
                {
                    // First draw the vertices.
                    glUniform1i(var, 0);
                    glDrawArrays(GL_POINTS, 0, mMeshNumVertices);

                    // Next enable wireframe mode.
                    glUniform1i(var, 1);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    glDrawElements(GL_TRIANGLES, (GLsizei)mMeshNumIndices,
                        GL_UNSIGNED_INT, gl::bufferOffset<GLuint>(0));
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
                else
                {
                    glUniform1i(var, mRenderMode + 1);
                    mMeshVao.bindVertexArray();
                    mMeshIndices.bindBuffer();
                    glDrawElements(GL_TRIANGLES, (GLsizei)mMeshNumIndices,
                        GL_UNSIGNED_INT, gl::bufferOffset<GLuint>(0));
                }

                mMeshIndices.unBindBuffer();
                mMeshVao.unBindVertexArray();
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

            if (ImGui::Button("Construct contours"))
            {
                constructContours();
            }

            if (ImGui::Button("Construct mesh"))
            {
                constructMesh();
            }

            if (ImGui::Button("Save mesh"))
            {
                mSoid.saveMesh();
            }

            ImGui::Dummy(ImVec2(0, 10));
            ImGui::Text("Visualization Options");
            ImGui::Separator();
            ImGui::Checkbox("Show lattices", &mShowLattices);
            ImGui::Checkbox("Show contours", &mShowContours);
            ImGui::Checkbox("Show mesh", &mShowMesh);

            ImGui::Dummy(ImVec2(0, 10));
            ImGui::Text("Log");
            ImGui::Separator();
            ImGui::BeginChild("Log", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
                false, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
            ImGui::TextWrapped(mSoid.getLog().c_str());
            ImGui::EndChild();
            ImGui::End();

            // Render controls.
            ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Render Controls");

            std::vector<const char*> renderNames = { "Wireframe", "Shaded", 
                "Normals" };
            ImGui::Combo("Render mode", &mRenderMode, renderNames.data(),
                ((int)renderNames.size()));
            ImGui::End();
        }

        void ModelView::constructLattices()
        {
            if (!mSoid.getLattice().vertices.empty())
            {
                return;
            }

            namespace gl = atlas::gl;
            namespace math = atlas::math;

            mSoid.constructLattices();

            auto verts = mSoid.getLattice().vertices;
            auto idx = mSoid.getLattice().indices;
            mLatticeNumIndices = idx.size();

            if (mLatticeNumIndices == 0)
            {
                return;
            }

            mLatticeVao.bindVertexArray();
            mLatticeData.bindBuffer();
            mLatticeData.bufferData(
                gl::size<math::Point>(verts.size()), verts.data(),
                GL_STATIC_DRAW);
            mLatticeData.vertexAttribPointer(VERTICES_LAYOUT_LOCATION, 3,
                GL_FLOAT, GL_FALSE, 0, gl::bufferOffset<float>(0));
            mLatticeVao.enableVertexAttribArray(VERTICES_LAYOUT_LOCATION);

            mLatticeIndices.bindBuffer();
            mLatticeIndices.bufferData(
                gl::size<GLuint>(idx.size()), idx.data(), GL_STATIC_DRAW);

            mLatticeIndices.unBindBuffer();
            mLatticeData.unBindBuffer();
            mLatticeVao.unBindVertexArray();
        }

        void ModelView::constructContours()
        {
            if (!mSoid.getContour().vertices.empty())
            {
                return;
            }

            namespace gl = atlas::gl;
            namespace math = atlas::math;

            mSoid.constructContours();

            auto verts = mSoid.getContour().vertices;
            auto idx = mSoid.getContour().indices;
            mContourNumIndices = idx.size();
            mContourNumVertices = verts.size();

            if (mContourNumIndices == 0)
            {
             return;
            }

            mContourVao.bindVertexArray();
            mContourData.bindBuffer();
            mContourData.bufferData(
             gl::size<math::Point>(verts.size()), verts.data(),
             GL_STATIC_DRAW);
            mContourData.vertexAttribPointer(VERTICES_LAYOUT_LOCATION, 3,
             GL_FLOAT, GL_FALSE, 0, gl::bufferOffset<float>(0));
            mContourVao.enableVertexAttribArray(VERTICES_LAYOUT_LOCATION);

            mContourIndices.bindBuffer();
            mContourIndices.bufferData(
             gl::size<GLuint>(idx.size()), idx.data(), GL_STATIC_DRAW);

            mContourIndices.unBindBuffer();
            mContourData.unBindBuffer();
            mContourVao.unBindVertexArray();
        }

        void ModelView::constructMesh()
        {
            // Some condition here.
            namespace gl = atlas::gl;
            namespace math = atlas::math;

            mSoid.constructMesh();

            // Update the contour buffers.
            auto verts = mSoid.getContour().vertices;
            auto idx = mSoid.getContour().indices;
            mContourNumIndices = idx.size();
            mContourNumVertices = verts.size();

            if (mContourNumIndices == 0)
            {
             return;
            }

            mContourVao.bindVertexArray();
            mContourData.bindBuffer();
            mContourData.bufferData(
             gl::size<math::Point>(verts.size()), verts.data(),
             GL_STATIC_DRAW);

            mContourIndices.bindBuffer();
            mContourIndices.bufferData(
             gl::size<GLuint>(idx.size()), idx.data(), GL_STATIC_DRAW);

            mContourIndices.unBindBuffer();
            mContourData.unBindBuffer();
            mContourVao.unBindVertexArray();

            // Now grab the mesh data.
            verts = mSoid.getMesh().vertices();
            auto normals = mSoid.getMesh().normals();
            idx = mSoid.getMesh().indices();

            mMeshNumVertices = verts.size();
            mMeshNumIndices = idx.size();

            std::vector<float> data;
            for (std::size_t i = 0; i < verts.size(); ++i)
            {
                data.push_back(verts[i].x);
                data.push_back(verts[i].y);
                data.push_back(verts[i].z);

                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }

            mMeshVao.bindVertexArray();
            mMeshData.bindBuffer();
            mMeshData.bufferData(gl::size<float>(data.size()), data.data(),
                GL_STATIC_DRAW);
            mMeshData.vertexAttribPointer(VERTICES_LAYOUT_LOCATION, 3,
                GL_FLOAT, GL_FALSE, gl::stride<float>(6), 
                gl::bufferOffset<float>(0));
            mMeshData.vertexAttribPointer(NORMALS_LAYOUT_LOCATION, 3,
                GL_FLOAT, GL_FALSE, gl::stride<float>(6), 
                gl::bufferOffset<float>(3));
            mMeshVao.enableVertexAttribArray(VERTICES_LAYOUT_LOCATION);
            mMeshVao.enableVertexAttribArray(NORMALS_LAYOUT_LOCATION);

            mMeshIndices.bindBuffer();
            mMeshIndices.bufferData(
                gl::size<GLuint>(idx.size()), idx.data(), GL_STATIC_DRAW);

            mMeshIndices.unBindBuffer();
            mMeshData.unBindBuffer();
        }
    }
}