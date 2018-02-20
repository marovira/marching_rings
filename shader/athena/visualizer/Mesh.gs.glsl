#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VertexData
{
    vec3 position;
    vec3 normal;
} inData[];

out GeomData
{
    vec3 position;
    vec3 normal;
    vec3 eyeDirection;
    vec3 lightDirection;
    vec3 lightPosition;
    vec3 barycentric;
} outData;

#include "athena/global/UniformMatrices.glsl"

const vec3 Light = vec3(0, 5, 0);

void main()
{
    int i;
    for (i = 0; i < gl_in.length(); ++i)
    {
        gl_Position = projection * view * model * vec4(inData[i].position, 1.0);

        outData.position = vec3(model * vec4(inData.position, 1.0));
        vec3 vertexPos = (view * model * vec4(inData.position, 1.0)).xyz;
        outData.eyeDirection = vec3(0, 0, 0) - vertexPos;
        outData.lightDirection = outData.eyeDirection;
        outData.normal = (view * model * vec4(normal, 0)).xyz;
        outData.lightPosition = Light;
        if (i == 0)
        {
            outData.barycentric = vec3(1, 0, 0);
        }
        else if (i == 1)
        {
            outData.barycentric = vec3(0, 1, 0);
        }
        else
        {
            outData.barycentric = vec3(0, 0, 1);
        }

        EmitVertex();
    }
    EndPrimitive();
}
