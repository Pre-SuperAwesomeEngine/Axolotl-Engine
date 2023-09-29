#version 460

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shading_language_include : require

#include "/Common/Functions/pbr_functions.glsl"

#include "/Common/Functions/srgba_functions.glsl"

#include "/Common/Structs/lights.glsl"

#include "/Common/Structs/materials.glsl"

#include "/Common/Structs/effect.glsl"

#include "/Common/Structs/tiling.glsl"

layout(std140, binding=1) uniform Directional
{
	vec3 directionalDir;  	//12	//0
	vec4 directionalColor;	//16	//16     // note: alpha parameter of colour is the intensity 
};

readonly layout(std430, binding=2) buffer PointLights
{
	uint num_point;			//4		//0
	PointLight points[]; 	//32	//16
};

readonly layout(std430, binding=3) buffer SpotLights
{
	uint num_spot;
	SpotLight spots[];
};

readonly layout(std430, binding=4) buffer AreaLightsSphere
{
	uint num_spheres;
	AreaLightSphere areaSphere[];
};

readonly layout(std430, binding=5) buffer AreaLightsTube
{
	uint num_tubes;
	AreaLightTube areaTube[];
};

readonly layout(std430, binding = 11) buffer Materials {
    MetallicMaterial materials[];
};

readonly layout(std430, binding = 12) buffer Tilings {
    Tiling tilings[];
};

readonly layout(std430, binding = 13) buffer Effects {
    Effect effects[];
};

// IBL
layout(binding = 8) uniform samplerCube diffuse_IBL;
layout(binding = 9) uniform samplerCube prefiltered_IBL;
layout(binding = 10) uniform sampler2D environmentBRDF;

uniform int numLevels_IBL;
uniform float cubemap_intensity;

in vec3 FragTangent;
in vec3 Normal;
in vec3 FragPos;
in vec3 ViewPos;
in vec2 TexCoord;

in flat int InstanceIndex;

out vec4 outColor;

vec3 calculateDirectionalLight(vec3 N, vec3 V, vec3 Cd, vec3 f0, float roughness)
{
    vec3 L = normalize(-directionalDir);
    vec3 H = (L+V)/length(L+V);

    float dotNL = max(dot(N,L), EPSILON);

    vec3 FS = fresnelSchlick(f0, max(dot(L,H), EPSILON));
    float SV = smithVisibility(dotNL, max(dot(N,V), EPSILON), roughness);
    float GGXND = GGXNormalDistribution(max(dot(N,H), EPSILON), roughness);

    return (Cd*(1-f0)+0.25*FS*SV*GGXND)*directionalColor.rgb*directionalColor.a*dotNL;
}

vec3 calculatePointLights(vec3 N, vec3 V, vec3 Cd, vec3 f0, float roughness)
{
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < num_point; ++i)
    {
        vec3 pos = points[i].position.xyz;
        vec3 color = points[i].color.rgb;
        float radius = points[i].position.w;
        float intensity = points[i].color.a;

        vec3 L = normalize(pos-FragPos);
        vec3 H = (L+V)/length(L+V);
        float dotNL = max(dot(N,L), EPSILON);

        vec3 FS = fresnelSchlick(f0, max(dot(L,H), EPSILON));
        float SV = smithVisibility(dotNL, max(dot(N,V), EPSILON), roughness);
        float GGXND = GGXNormalDistribution(max(dot(N,H), EPSILON), roughness);

        // Attenuation
        float distance = length(FragPos-pos);
        float maxValue = pow(max(1-pow(distance/radius,4), 0),2);
        float attenuation = maxValue/(pow(distance,2) + 1);

        vec3 Li = color*intensity*attenuation;

        Lo += (Cd*(1-f0)+0.25*FS*SV*GGXND)*Li*dotNL;
    }

    return Lo;
}

vec3 calculateSpotLights(vec3 N, vec3 V, vec3 Cd, vec3 f0, float roughness)
{
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < num_spot; ++i)
    {
        vec3 pos = spots[i].position.xyz;
        vec3 aim = spots[i].aim;
        vec3 color = spots[i].color.rgb;
        float radius = spots[i].position.w;
        float intensity = spots[i].color.a;
        float innerAngle = spots[i].innerAngle;
        float outerAngle = spots[i].outerAngle;

        float cosInner = cos(innerAngle);
        float cosOuter = cos(outerAngle);

        vec3 L = normalize(pos-FragPos);
        vec3 H = (L+V)/length(L+V);
        float dotNL = max(dot(N,L), EPSILON);

        vec3 FS = fresnelSchlick(f0, max(dot(L,H), EPSILON));
        float SV = smithVisibility(dotNL, max(dot(N,V), EPSILON), roughness);
        float GGXND = GGXNormalDistribution(max(dot(N,H), EPSILON), roughness);

        // Attenuation
        float distance = dot(FragPos - pos, aim);
        float maxValue = pow(max(1 - pow(distance/radius,4), 0),2);
        float attenuation = maxValue/(pow(distance,2) + 1);

        float C = dot(-L, aim);
        float Catt = 0.0;

        if (C > cosInner)
        {
            Catt = 1.0;
        }
        else if (cosInner > C && C > cosOuter)
        {
            Catt = (C-cosOuter)/(cosInner-cosOuter);
        }
    
        vec3 Li = color*intensity*attenuation*Catt;
            
        Lo += (Cd*(1-f0)+0.25*FS*SV*GGXND)*Li*dotNL;
    }

    return Lo;
}

vec3 calculateAreaLightSpheres(vec3 N, vec3 V, vec3 Cd, vec3 f0, float roughness)
{
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < num_spheres; ++i)
    {
        vec3 sP = areaSphere[i].position.xyz;
        vec3 color = areaSphere[i].color.rgb;
        float sR = areaSphere[i].position.w;
        float intensity = areaSphere[i].color.a;
        float lightRadius = areaSphere[i].lightRadius;

        vec3 R = normalize(reflect(-V, N));
        
        // Attenuation from the closest point view
        float closeDistance = max(length(FragPos-sP)-sR, 0.0);
        float maxValue = pow(max(1-pow(closeDistance/lightRadius,4), 0),2);
        float attenuation = maxValue/(pow(closeDistance,2) + 1);

        // calculate closest point light specular
        vec3 closest = ClosestRayToSphere(sR, sP - FragPos, R);

        vec3 L = normalize(closest);
        vec3 H = normalize(L + V);
        float specularDotNL = max(dot(N,L), EPSILON);
        
        float alpha = max(roughness * roughness, EPSILON);
        float alphaPrime = clamp(sR/(closeDistance*2.0)+alpha, 0.0f, 1.0f);
        float D = GGXNDAreaLight(max(dot(N,H), EPSILON), roughness, alpha, alphaPrime);
        vec3 F = fresnelSchlick(f0, max(dot(L,H), EPSILON));
        float G = smithVisibility(specularDotNL, max(dot(N,V), EPSILON), roughness);
        
        // calculate closest point light diffuse
        closest = sP;

        L = normalize(closest-FragPos);
        float diffuseDotNL = max(dot(N,L), EPSILON);

        vec3 Li = color * intensity * attenuation;
        vec3 LoSpecular = 0.25 * D * F * G * Li * specularDotNL;
        vec3 LoDiffuse = Cd * (1 - f0) * Li * diffuseDotNL;
        Lo += LoDiffuse + LoSpecular;
    }

    return Lo;
}

vec3 calculateAreaLightTubes(vec3 N, vec3 V, vec3 Cd, vec3 f0, float roughness)
{
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < num_tubes; ++i)
    {
        vec3 posA = areaTube[i].positionA.xyz;
        vec3 posB = areaTube[i].positionB.xyz;
        float tubeRadius = areaTube[i].positionA.w;
        vec3 color = areaTube[i].color.rgb;
        float intensity = areaTube[i].color.a;
        float lightRadius = areaTube[i].lightRadius;

        // Attenuation from the closest point view
        vec3 pointAtt = ClosestPointToLine(FragPos, posA, posB, tubeRadius);
        float closeDistance = length(pointAtt - FragPos);
        float maxValue = pow(max(1-pow(closeDistance/lightRadius,4), 0),2);
        float attenuation = maxValue/(pow(closeDistance,2) + 1);

        vec3 R = normalize(reflect(-V, N));
        
        // calculate closest point light specular
        vec3 closest = ClosestRayToLine(FragPos, posA, posB, R);
   	    closest = ClosestRayToSphere(tubeRadius, closest, R);

        vec3 L = normalize(closest);
        vec3 H = normalize(L + V);
        float specularDotNL = max(dot(N,L), EPSILON);

        float alpha = max(roughness * roughness, EPSILON);
        float alphaPrime = clamp(tubeRadius/(closeDistance*2.0)+alpha, 0.0f, 1.0f);
        float D = GGXNDAreaLight(max(dot(N,H), EPSILON), roughness, alpha, alphaPrime);
        vec3 F = fresnelSchlick(f0, max(dot(L,H), EPSILON));
        float G = smithVisibility(specularDotNL, max(dot(N,V), EPSILON), roughness);

        // calculate closest point light diffuse
        float a = length(posA-FragPos);
        float b = length(posB-FragPos);
        float x = (a)/(b + a);
        closest = BisectionIntersection(FragPos, posA, posB);

        L = normalize(closest-FragPos);
        float diffuseDotNL = max(dot(N,L), EPSILON);

        vec3 Li = color * intensity * attenuation;
        vec3 LoSpecular = 0.25 * D * F * G * Li * specularDotNL;
        vec3 LoDiffuse = Cd * (1 - f0) * Li * diffuseDotNL;
        Lo += LoDiffuse + LoSpecular;
    }

    return Lo;
}
  
void main()
{
    MetallicMaterial material = materials[InstanceIndex];
    Effect effect = effects[InstanceIndex];

    if (effect.discardFrag == 1)
    {
        discard;
        return;
    }
    Tiling tiling = tilings[InstanceIndex];

    vec2 newTexCoord = TexCoord*tiling.percentage*tiling.tiling+tiling.offset;

	vec3 norm = Normal;
    vec3 tangent = FragTangent;
    vec3 viewDir = normalize(ViewPos - FragPos);

    // Diffuse
	vec4 textureMat = material.diffuse_color;
    if (material.has_diffuse_map == 1)
    {
        textureMat = texture(material.diffuse_map, newTexCoord);
    }
    textureMat = SRGBA(textureMat);
    textureMat.rgb += effect.color;
    
    if(textureMat.a < 0.01)
    {
        discard;
    }

    //Transparency
    textureMat.a = material.has_diffuse_map * textureMat.a + 
        (1.0f-material.has_diffuse_map) * material.diffuse_color.a;
    
    // Normals
	if (material.has_normal_map == 1)
	{
        mat3 space = CreateTangentSpace(norm, tangent);
        norm = texture(material.normal_map, newTexCoord).rgb;
        norm = norm * 2.0 - 1.0;
        norm.xy *= material.normal_strength;
        norm = normalize(norm);
        norm = space * norm;
	}
    norm = normalize(norm);
  
    // Metallic
    vec4 colorMetallic = texture(material.metallic_map, newTexCoord);
    float metalnessMask = material.has_metallic_map * colorMetallic.r + (1 - material.has_metallic_map) * 
        material.metalness;

    vec3 Cd = textureMat.rgb*(1.0-metalnessMask);
    vec3 f0 = mix(vec3(0.04), textureMat.rgb, metalnessMask);

    // smoothness and roughness
    float roughness = pow(1-material.smoothness,2) + EPSILON;
    if (material.has_metallic_map == 1)
	{
        roughness = pow(1.0 * colorMetallic.a,2) + EPSILON;
    }

    // Lights
    vec3 Lo = calculateDirectionalLight(norm, viewDir, Cd, f0, roughness);

    if (num_point > 0)
    {
        Lo += calculatePointLights(norm, viewDir, Cd, f0, roughness);
    }

    if (num_spot > 0)
    {
        Lo += calculateSpotLights(norm, viewDir, Cd, f0, roughness);
    }

    if (num_spheres > 0)
    {
        Lo += calculateAreaLightSpheres(norm, viewDir, Cd, f0, roughness);
    }

    if (num_tubes > 0)
    {
        Lo += calculateAreaLightTubes(norm, viewDir, Cd, f0, roughness);
    }

    vec3 R = reflect(-viewDir, norm);
    float NdotV = max(dot(norm, viewDir), EPSILON);
    vec3 ambient = GetAmbientLight(norm, R, NdotV, roughness, Cd, f0, diffuse_IBL, prefiltered_IBL, environmentBRDF, 
        numLevels_IBL, vec4(0)) * cubemap_intensity;
    vec3 color = ambient + Lo;
    
    //Emissive
    if (material.has_emissive_map == 1) 
    {
        color += vec3(texture(material.emissive_map, newTexCoord));
    }
       
    outColor = vec4(color, textureMat.a);
}