#version 450 core

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 v_TexCord;
layout(location = 1) in float text_id;
layout(location = 2) in vec3 normal;

uniform sampler2D u_Texture;

const float global_illumination = 0.3;
const vec2 atlas_size = vec2(16,1);

const vec3 dir_light_dir = normalize(vec3(.9,0.4,.1));
const float dir_light_intensity = 0.8;

void main()
{


    float diff = max(dot(normal, dir_light_dir), 0.0);

    vec4 color = texture(
        u_Texture,
        ((v_TexCord - floor(v_TexCord)) / atlas_size) 
        + vec2(text_id / atlas_size.x , 0)
    );

    if(color.a == 0.0)
    {
        discard;
    }

    out_color = vec4(color.xyz * (global_illumination + diff),color.w);
}