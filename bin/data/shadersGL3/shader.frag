#version 150

uniform vec3 thrustColor;
out vec4 outputColor;

void main()
{
    outputColor = vec4(thrustColor, 1.0);
}
