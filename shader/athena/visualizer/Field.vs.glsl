#version 450 core

#include "athena/global/LayoutLocations.glsl"

layout(location = VERTICES_LAYOUT_LOCATION) in vec4 position;
layout(location = NORMALS_LAYOUT_LOCATION) in vec3 gradient;

#include "athena/global/UniformMatrices.glsl"

out VertexData
{
    float value;
    vec3 gradient;
} outData;

void main()
{
    gl_Position = projection * view * model * vec4(position.xyz, 1.0);
    outData.gradient = gradient;
    outData.value = position.w;
}
