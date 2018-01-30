#version 430 core

out vec4 fragColour;

void main()
{
    double rgb = 127.0 / 255;
    fragColour = vec4(rgb, rgb, rgb, 1.0);
}
