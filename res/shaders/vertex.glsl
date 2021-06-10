#version 450 core

layout(location = 0) in uint vertex_data;

layout(location = 0) out vec2 v_TexCord;
layout(location = 1) out float text_id;

uniform mat4 u_MVP;

void main()
{
    vec4 position;

    position.x = float((vertex_data >> 10) & 31);  //0b11111
    position.y = float((vertex_data >> 05) & 31);  //0b11111
    position.z = float((vertex_data >> 00) & 31);  //0b11111
    position.w = 1.0;

    uint corner = (vertex_data >> 15) & 7; // 0b111

    position.x += float((corner >> 2) & 1);
    position.y += float((corner >> 1) & 1);
    position.z += float((corner >> 0) & 1);


    const uint first_comp [3] = {1,0,0};
    const uint second_comp[3] = {2,2,1};

    uint plane_bits = (vertex_data >> 18) & 3; //0b11

    v_TexCord = vec2(position[first_comp[plane_bits]],position[second_comp[plane_bits]]);
    //a block face ranges in -0.5 to 0.5
    position += vec4(-0.5,-0.5,-0.5,0.0);

    text_id = float((vertex_data >> 20) & 31);

    gl_Position = u_MVP * position;
}