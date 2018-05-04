#version 450 core

in VertexData
{
    float value;
    vec3 gradient;
    vec3 naturalGrad;
} inData;

out vec4 fragColour;

uniform int renderMode;

vec4 colourField(float value)
{
    vec4 field = vec4(0, 0, 0, 1);
    if (value > 0.495 && value < 0.505)
    {
        field.r = 1;
    }
    else if (value < 0.9 || value > 0.1)
    {
        field.r = value;
    }

    return field;
}

vec4 colourGradient(vec3 gradient)
{
    vec4 grad = vec4(0, 0, 0, 1);
    float l = clamp(length(gradient), 0, 1);
    if (l < 0.1)
    {
        grad.g = 1;
    }
    else
    {
        grad.b = l;
    }

    return grad;
}

void main()
{
    if (renderMode == 0)
    {
        fragColour = colourField(inData.value);
    }
    else if (renderMode == 1)
    {
        fragColour = colourGradient(inData.gradient);
    }
    else
    {
        fragColour = colourGradient(inData.naturalGrad);
    }
}
