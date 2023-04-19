#version 460
#extension GL_EXT_nonuniform_qualifier : require
layout(location = 0) out vec4 outputColor;
layout(location = 0) in flat uint clusterId;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 positionWS;
layout(location = 3) in mat3 ntb;

layout(set = 3, binding = 0) uniform sampler textureSampler;
layout(set = 3, binding = 1) uniform texture2D textures2D[];
//layout(set = 4, binding = 0) uniform texture3D textures3D[];
//nonuniformEXT

#define PI 3.14159265358979f

struct Attributes
{
    vec3 positionWS;
    mat3 ntb;
    vec2 uv;
};

//vec3 evaluateMaterialLighting(Attributes attributes)
//{
//    vec3 diffuse = texture(sampler2D(textures2D[6], textureSampler), vec2(uv)).xyz;
//    vec3 normalTS = texture(sampler2D(textures2D[7], textureSampler), vec2(uv)).xyz;
//
//    vec3 normalWS = ntb * normalTS;
//    vec3 lightDirection = normalize(vec3(1.0, 1.0, 1.0));
//    float radiance = max(0, dot(lightDirection, normalize(normalWS)));
//
//    vec3 shadowColor = vec3(130.0/255.0, 163.0/255.0, 255.0/255.0) * 0.15;
//    vec3 diffuseColor = vec3(0.9, 0.4, 0.1);
//
//    vec3 color = mix(shadowColor, diffuse, radiance);
//    return color;
//}

<[generated_fragment_code]>

void main()
{
    Attributes attributes;
    attributes.positionWS = positionWS;
    attributes.ntb = ntb;
    attributes.uv = uv;

    vec3 color = <[evaluate_material_fragment]>(attributes);

    outputColor = vec4(color, 1.0);
}