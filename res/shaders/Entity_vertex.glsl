#version 330 core

layout(location = 0) in vec4 position;

out vec3 v_TexCord;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * position;
    v_TexCord = vec3(gl_Position.x,gl_Position.y,gl_Position.z);
}