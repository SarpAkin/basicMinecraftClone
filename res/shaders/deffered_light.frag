#version 450 core

layout(location = 0) in vec2 tex_pos;

layout(location = 0) out vec4 frag_color;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;


struct DirectionalLight
{
    vec3 dir;
    vec3 col;
};

struct PointLight {
    vec3 pos;
    vec3 col;
    float range;
};

uniform vec3 view_pos;


#ifndef MAX_POINT_LIGHT_NUM
#define MAX_POINT_LIGHT_NUM 32
#endif

uniform PointLight p_lights[MAX_POINT_LIGHT_NUM];

uniform DirectionalLight dir_light;


const float ambiant_light = 0.15;



void main()
{
    vec3 frag_pos = texture(gPosition, tex_pos).rgb;
    vec3 normal = texture(gNormal, tex_pos).rgb;
    vec3 albedo = texture(gAlbedoSpec, tex_pos).rgb;
    float specular = texture(gAlbedoSpec, tex_pos).a;

    vec3 lighting = albedo * ambiant_light;
    vec3 viewDir = normalize(view_pos - frag_pos);
    for(int i = 0; i < MAX_POINT_LIGHT_NUM; i++)
    {
        // diffuse
        vec3 light_dir = normalize(p_lights[i].pos - frag_pos);
        vec3 diffuse = max(dot(normal, light_dir), 0.0) * albedo * p_lights[i].col;
        lighting += diffuse * max((p_lights[i].range - length(p_lights[i].pos - frag_pos)) / p_lights[i].range,0.0);
    }

    {    
        vec3 diffuse = max(dot(normal, dir_light.dir), 0.0) * albedo * dir_light.col;
        lighting += diffuse;
    }
    
    frag_color = vec4(lighting, 1.0);
}