#version 450 core

layout(location = 0) in vec3 f_position;

layout(location = 0) out vec4 color;


float posToColor(float axis)
{
    return (sin(axis) + 1) / 2;
}

void main()
{
    color = vec4(posToColor(f_position.x),posToColor(f_position.y),posToColor(f_position.z),1);
}