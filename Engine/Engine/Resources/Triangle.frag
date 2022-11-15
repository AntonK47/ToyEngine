#version 460

layout(location = 0) out vec4 outputColor;
layout(location = 0) in flat uint clusterId;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
uint hash(uint a)
{
   a = (a+0x7ed55d16) + (a<<12);
   a = (a^0xc761c23c) ^ (a>>19);
   a = (a+0x165667b1) + (a<<5);
   a = (a+0xd3a2646c) ^ (a<<9);
   a = (a+0xfd7046c5) + (a<<3);
   a = (a^0xb55a4f09) ^ (a>>16);
   return a;
}
void main()
{
    uint hash = hash(clusterId);
    uint aa = hash % 256;
    uint bb = (hash >> 8) % 256;
    uint cc = (hash >> 16) % 256;

    vec3 id = vec3(aa/256.0f,bb/256.0f,cc/256.0f);

    vec3 lightDirection = vec3(1.0,1.0,1.0);

    float a = max(0, dot(lightDirection, normalize(normal)));

    outputColor = vec4(vec3(uv, 0.0f), 1.0);
}