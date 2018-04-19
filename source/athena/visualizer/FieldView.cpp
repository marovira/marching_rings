#include "athena/visualizer/FieldView.hpp"
#include "athena/polygonizer/Voxel.hpp"
#include "athena/ShaderPaths.hpp"
#include "athena/global/LayoutLocations.glsl"

#include <atlas/utils/GUI.hpp>

#include <algorithm>

namespace gl = atlas::gl;
namespace math = atlas::math;

namespace athena
{
    namespace visualizer
    {
        FieldView::FieldView(tree::BlobTree* tree) : 
            mSliceData(GL_ARRAY_BUFFER),
            mSliceIndices(GL_ELEMENT_ARRAY_BUFFER),
            mSliceNumIndices(0),
            mNumVertices(0),
            mTree(tree),
            mNumSlices(0),
            mSelectedSlice(0),
            mRenderMode(0),
            mShowField(false)
        {
            initShaders();
        }

        void FieldView::constructSlices(std::size_t numSlices, float sliceDelta,
            polygonizer::SlicingAxes const& axis)
        {
            using athena::polygonizer::FieldPoint;
            using athena::polygonizer::SlicingAxes;
            using atlas::math::Point;
            using atlas::math::Point2;

            const std::size_t gridSize = 128;
            const std::size_t totalGridSize = gridSize * gridSize;
            mNumSlices = numSlices;
            auto box = mTree->getTreeBox();
            Point min = box.pMin;
            Point max = box.pMax;

            Point gridDelta = (max - min) / static_cast<float>(gridSize);
            Point delta(0.0f);
            glm::u32vec2 axisMask;
            Point plane(0.0f);
            switch (axis)
            {
            case SlicingAxes::XAxis:
                delta.x = sliceDelta;
                gridDelta.x = 0.0f;
                axisMask = { 1, 2 };
                plane.x = min.x;
                break;

            case SlicingAxes::YAxis:
                delta.y = sliceDelta;
                gridDelta.y = 0.0f;
                axisMask = { 0, 2 };
                plane.y = min.y;
                break;

            case SlicingAxes::ZAxis:
                delta.z = sliceDelta;
                gridDelta.z = 0.0f;
                axisMask = { 0, 1 };
                plane.z = min.z;
                break;
            }

            Point height = min;
            std::vector<float> data;
            for (std::size_t i = 0; i < numSlices; ++i)
            {
                Point start = height;
                for (std::size_t x = 0; x < gridSize; ++x)
                {
                    for (std::size_t y = 0; y < gridSize; ++y)
                    {
                        Point pt = start;
                        pt[axisMask.x] = 
                            start[axisMask.x] + x * gridDelta[axisMask.x];
                        pt[axisMask.y] = 
                            start[axisMask.y] + y * gridDelta[axisMask.y];

                        float f = mTree->eval(pt);
                        auto naturalGrad = mTree->naturalGradient(pt);
                        auto gradient = mTree->grad(pt);

                        // Project the gradient.
                        auto projGrad = naturalGrad - 
                            glm::proj(naturalGrad, glm::normalize(plane));
                        auto projNaturalGrad = gradient - glm::proj(gradient,
                            glm::normalize(plane));


                        data.push_back(pt.x);
                        data.push_back(pt.y);
                        data.push_back(pt.z);
                        data.push_back(f);

                        data.push_back(projGrad.x);
                        data.push_back(projGrad.y);
                        data.push_back(projGrad.z);

                        data.push_back(projNaturalGrad.x);
                        data.push_back(projNaturalGrad.y);
                        data.push_back(projNaturalGrad.z);
                        mNumVertices++;
                    }
                }
                height += delta;
                plane += delta;
            }

            std::vector<GLuint> indices;
            std::size_t startIndex = 0;
            std::size_t sliceOffset = 0;
            std::size_t count = 0;
            std::size_t sliceStart = 0;
            for (std::size_t i = 0; i < numSlices; ++i)
            {
                count = 0;
                for (std::size_t x = 0; x < gridSize - 1; ++x)
                {
                    startIndex = sliceOffset + (gridSize * x);
                    for (std::size_t y = 0; y < gridSize - 1; ++y)
                    {
                        indices.push_back(startIndex + y);
                        indices.push_back(startIndex + gridSize + y);
                        indices.push_back(startIndex + gridSize + y + 1);

                        indices.push_back(startIndex + gridSize + y + 1);
                        indices.push_back(startIndex + y + 1);
                        indices.push_back(startIndex + y);
                        count += 6;
                    }
                }
                mOffsets.emplace_back(sliceStart, count);
                sliceOffset += totalGridSize;
                sliceStart += count;
            }

            mSliceNumIndices = indices.size();

            mVao.bindVertexArray();
            mSliceData.bindBuffer();
            mSliceData.bufferData(gl::size<float>(data.size()), data.data(),
                GL_STATIC_DRAW);
            mSliceData.vertexAttribPointer(VERTICES_LAYOUT_LOCATION, 4,
                GL_FLOAT, GL_FALSE, gl::stride<float>(10),
                gl::bufferOffset<float>(0));
            mSliceData.vertexAttribPointer(NORMALS_LAYOUT_LOCATION, 3,
                GL_FLOAT, GL_FALSE, gl::stride<float>(10),
                gl::bufferOffset<float>(4));
            mSliceData.vertexAttribPointer(GRADIENT_LAYOUT_LOCATION, 3,
                GL_FLOAT, GL_FALSE, gl::stride<float>(10),
                gl::bufferOffset<float>(7));

            mVao.enableVertexAttribArray(VERTICES_LAYOUT_LOCATION);
            mVao.enableVertexAttribArray(NORMALS_LAYOUT_LOCATION);
            mVao.enableVertexAttribArray(GRADIENT_LAYOUT_LOCATION);

            mSliceIndices.bindBuffer();
            mSliceIndices.bufferData(
                gl::size<GLuint>(indices.size()), indices.data(), GL_STATIC_DRAW);

            mSliceIndices.unBindBuffer();
            mSliceData.unBindBuffer();
            mVao.unBindVertexArray();
        }

        void FieldView::renderGeometry()
        {
            mShaders[0].hotReloadShaders();
            if (!mShaders[0].shaderProgramValid())
            {
                return;
            }

            if (mShowField)
            {
                mShaders[0].enableShaders();
                glUniformMatrix4fv(mUniforms["model"], 1, GL_FALSE, &mModel[0][0]);
                glUniform1i(mUniforms["renderMode"], mRenderMode);

                mVao.bindVertexArray();
                mSliceIndices.bindBuffer();
                auto currentSlice = mOffsets[mSelectedSlice];
                glDrawElements(GL_TRIANGLES, (GLsizei)currentSlice.second,
                    GL_UNSIGNED_INT, gl::bufferOffset<GLuint>(currentSlice.first));

                mVao.unBindVertexArray();
                mShaders[0].disableShaders();
            }
        }

        void FieldView::drawGui()
        {
            ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Field View Controls");
            ImGui::Separator();
            ImGui::Checkbox("Show field data", &mShowField);
            std::vector<const char*> sliceNames;
            std::vector<std::string> names;
            for (std::size_t i = 0; i < mNumSlices; ++i)
            {
                std::string s = "Cross-section " + std::to_string(i);
                names.push_back(s);
            }
            sliceNames.resize(names.size());
            std::transform(names.begin(), names.end(), sliceNames.begin(),
                std::mem_fun_ref(&std::string::c_str));
            ImGui::Combo("Cross-section", &mSelectedSlice, sliceNames.data(),
                ((int)mNumSlices));

            std::vector<const char*> renderNames = { "Field", "Gradient",
                "Scaled gradient" };
            ImGui::Combo("Render mode", &mRenderMode, renderNames.data(),
                ((int)renderNames.size()));
            ImGui::End();
        }

        void FieldView::initShaders()
        {

            std::vector<gl::ShaderUnit> shaders
            {
                { std::string(ShaderDirectory) +
                "athena/visualizer/Field.vs.glsl", GL_VERTEX_SHADER },
                { std::string(ShaderDirectory) +
                "athena/visualizer/Field.fs.glsl", GL_FRAGMENT_SHADER }
            };

            mShaders.push_back(gl::Shader(shaders));
            mShaders[0].setShaderIncludeDir(ShaderDirectory);
            mShaders[0].compileShaders();
            mShaders[0].linkShaders();

            auto var = mShaders[0].getUniformVariable("renderMode");
            mUniforms.insert(UniformKey("renderMode", var));

            var = mShaders[0].getUniformVariable("model");
            mUniforms.insert(UniformKey("model", var));

            mShaders[0].disableShaders();
        }

    }
}