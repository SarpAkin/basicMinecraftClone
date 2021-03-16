#version 330 core

layout(location = 0) out vec4 color;

in vec3 v_TexCord;

float posToColor(float axis)
{
    return (sin(axis) + 1) / 2;
}

void main()
{
    color = vec4(posToColor(v_TexCord.x),posToColor(v_TexCord.y),posToColor(v_TexCord.z),1);
}