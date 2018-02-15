#version 450 core

#include "athena/global/LayoutLocations.glsl"

layout (location = VERTICES_LAYOUT_LOCATION) in vec3 position;
layout (location = NORMALS_LAYOUT_LOCATION) in vec3 normal;

#include "athena/global/UniformMatrices.glsl"

out VertexData
{
    vec3 position;
    vec3 normal;
    vec3 eyeDirection;
    vec3 lightDirection;
    vec3 lightPosition;
} outData;

const vec3 Light = vec3(0, 5, 0);

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);

    // Vertex position in world-space.
    outData.position = vec3(model * vec4(position, 1.0));

    vec3 vertexPos = (view * model * vec4(position, 1.0)).xyz;
    outData.eyeDirection = vec3(0, 0, 0) - vertexPos;

    vec3 lightPos = (view * vec4(Light, 1.0)).xyz;
    outData.lightDirection = outData.eyeDirection;

    outData.normal = (view * model * vec4(normal, 0)).xyz;
    outData.lightDirection = Light;
}
