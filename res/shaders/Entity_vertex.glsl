#version 450 core

layout(location = 0) in vec4 position;

layout(location = 0) out vec3 f_position;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * position;
    f_position = vec3(gl_Position.x,gl_Position.y,gl_Position.z);
}