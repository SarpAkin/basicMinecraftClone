#version 450 core

layout(location = 0) out vec4 color;

layout(location = 0) in vec2 v_TexCord;
layout(location = 1) in float text_id;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

void main()
{
    const vec2 atlas_size = vec2(16,1);

    color = texture(
            u_Texture,
            ((v_TexCord - floor(v_TexCord)) / atlas_size) 
            + vec2(text_id / atlas_size.x , 0)
        );
}