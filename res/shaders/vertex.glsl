#version 450 core

layout(location = 0) in uint vertex_data;

layout(location = 0) out vec2 v_TexCord;
//layout(location = 1) out uint text_id;

uniform mat4 u_MVP;

void main()
{
    vec4 position;

    position.x = float((vertex_data >> 10) & 31) - 0.5;  //0b11111
    position.y = float((vertex_data >> 05) & 31) - 0.5;  //0b11111
    position.z = float((vertex_data >> 00) & 31) - 0.5;  //0b11111
    position.w = 1.0;


    gl_Position = u_MVP * position;
    v_TexCord = vec2(0,0);
}