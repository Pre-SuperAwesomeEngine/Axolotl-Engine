#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shading_language_include : require

#include "/Common/Functions/pbr_functions.glsl"

#include "/Common/Functions/srgba_functions.glsl"

#include "/Common/Structs/effect.glsl"

#include "/Common/Structs/tiling.glsl"

struct Material {
    vec4 diffuse_color;         //0 //16
    int has_diffuse_map;        //16 //4
    int has_normal_map;         //20 //4
    int has_metallic_map;       //24 //4
    int has_emissive_map;       //28 //4
    float smoothness;           //32 //4
    float metalness;            //36 //4
    float normal_strength;      //40 //4
    sampler2D diffuse_map;      //48 //8
    sampler2D normal_map;       //56 //8
    sampler2D metallic_map;     //64 //8
    sampler2D emissive_map;     //72 //8 --> 80
};

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffuse;
layout (location = 3) out vec4 gSpecular;
layout (location = 4) out vec4 gEmissive;

readonly layout(std430, binding = 11) buffer Materials {
    Material materials[];
};

readonly layout(std430, binding = 12) buffer Tilings {
    Tiling tilings[];
};

readonly layout(std430, binding = 13) buffer Effects {
    Effect effects[];
};

in vec3 FragTangent;
in vec3 Normal;
in vec3 FragPos;
in vec3 ViewPos;
in vec2 TexCoord;

in flat int InstanceIndex;

void main()
{    
    Material material = materials[InstanceIndex];
    Effect effect = effects[InstanceIndex];

    if (effect.discardFrag == 1)
    {
        discard;
        return;
    }

    Tiling tiling = tilings[InstanceIndex];

    vec2 newTexCoord = TexCoord*tiling.percentage*tiling.tiling+tiling.offset;

    gPosition = FragPos;
    gNormal = Normal;

    vec4 metallicColor = texture(material.metallic_map, newTexCoord);

    float metalnessMask = material.has_metallic_map * metallicColor.r + (1 - material.has_metallic_map) * 
     material.metalness;

    //Normals
    if (material.has_normal_map == 1)
	{
        mat3 space = CreateTangentSpace(gNormal, FragTangent);
        gNormal = texture(material.normal_map, newTexCoord).rgb;
        gNormal = gNormal * 2.0 - 1.0;
        gNormal.xy *= material.normal_strength;
        gNormal = normalize(gNormal);
        gNormal = space * gNormal;
	}
    gNormal = normalize(gNormal);

    //Diffuse
    gDiffuse = vec4(material.diffuse_color.rgb, material.diffuse_color.a);
    if (material.has_diffuse_map == 1)
    {
        gDiffuse = SRGBA(texture(material.diffuse_map, newTexCoord));
    }
    gDiffuse.rgb += effect.color;

    //Metallic and Smoothness
    gSpecular.a = material.smoothness;
    gSpecular.rgb = mix(vec3(0.04), gDiffuse.rgb, metalnessMask);
    if (material.has_metallic_map == 1)
	{
        gSpecular.a = metallicColor.a;
    }

    gDiffuse.rgb = vec3(gDiffuse*(1-metalnessMask));

    //Emissive
    gEmissive= vec4(0);
    if (material.has_emissive_map == 1) 
    {
        gEmissive = texture(material.emissive_map, newTexCoord);
    }
}