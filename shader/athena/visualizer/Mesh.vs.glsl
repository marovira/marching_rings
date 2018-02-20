#version 450 core

#include "athena/globa/LayoutLocations.glsl"

layout (location = VERTICES_LAYOUT_LOCATION) in vec3 position;
layout (location = NORMALS_LAYOUT_LOCATION) in vec3 normal;

out VertexData
{
    vec3 position;
    vec3 normal;
} outData;

const vec3 Light = vec3(0, 5, 0);

void main()
{
    outData.position = position;
    outData.normal = normal;
}
