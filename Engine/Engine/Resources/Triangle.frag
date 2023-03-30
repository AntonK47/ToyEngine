#version 460

layout(location = 0) out vec4 outputColor;
layout(location = 0) in flat uint clusterId;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 positionWorldSpace;

#define PI 3.14159265358979f

int hash(int a)
{
   a = (a+0x7ed55d16) + (a<<12);
   a = (a^0xc761c23c) ^ (a>>19);
   a = (a+0x165667b1) + (a<<5);
   a = (a+0xd3a2646c) ^ (a<<9);
   a = (a+0xfd7046c5) + (a<<3);
   a = (a^0xb55a4f09) ^ (a>>16);
   return a;
}

int simpleHash(int a)
{
    return a;
}

int h(float s, vec3 p)
{
    int hashZ = hash(int(floor(p.z/s)));
    int hashY = hash(int(floor(p.y/s)) + hashZ);
    int hashX = hash(int(floor(p.x/s)) + hashY);
    return hashX;
}

int h_s(float s, vec3 p)
{
    return h(s-1.0f, p) 
    + 1*(int(floor(p.x/s))%2)
    + 2*(int(floor(p.y/s))%2)
    + 4*(int(floor(p.z/s))%2);
}

int h_ss(float s, vec3 p, float alpha)
{
    const float pi = 3.14159265358979f;
    float baseFrequency = 1.0f/s;
    float fx = sin(2.0f*pi*p.x*3.0f*baseFrequency);
    float fy = sin(2.0f*pi*p.y*5.0f*baseFrequency);
    float fz = sin(2.0f*pi*p.z*7.0f*baseFrequency);

    vec3 offset = vec3(fx, fy, fz);

    return h(s, p + alpha * offset);
}

vec3 hashToColor(int hash)
{
    uint aa = hash % 256;
    uint bb = (hash >> 8) % 256;
    uint cc = (hash >> 16) % 256;

    return vec3(aa/256.0f,bb/256.0f,cc/256.0f);
}

void main()
{
    int hash = hash(hash(int(clusterId)));

    vec3 id = hashToColor(hash);

    vec3 lightDirection = vec3(1.0,1.0,1.0);

    //float a = max(0, dot(lightDirection, normalize(normal)));

    //outputColor = vec4(vec3(uv, 0.0f), 1.0);

    float s = 0.1f;
    int spatialHash = h_ss(s, positionWorldSpace, s*0.1f);
    outputColor = vec4(normal,/*hashToColor(hash),*/ 1.0);
    //outputColor = vec4(vec3(a), 1.0);
}