#version 450 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

layout(location = 0) in vec2 v_TexCord;
layout(location = 1) in float text_id;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 position;

uniform sampler2D u_Texture;

const vec2 atlas_size = vec2(16,1);



void main()
{
    gPosition = position;
    gNormal = normal;

    vec4 color = texture(
        u_Texture,
        ((v_TexCord - floor(v_TexCord)) / atlas_size) 
        + vec2(text_id / atlas_size.x , 0)
    );

    if(color.a == 0.0)
    {
        discard;
    }

    gAlbedoSpec = vec4(color.xyz,0.5);
}