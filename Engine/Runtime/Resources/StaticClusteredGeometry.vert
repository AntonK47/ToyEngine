#version 460
#extension GL_EXT_shader_8bit_storage: require
#extension GL_ARB_shader_draw_parameters: require
#extension GL_EXT_scalar_block_layout: require


struct Position
{
    float x;
    float y; 
    float z;
};

struct Meshlet
{
    int triangleOffset;
    int triangleCount;
    int positionStreamOffset;
    int positionStreamCount;
};

struct TangentFrame
{
    vec3 normal;
	vec3 tangent;
	vec3 bitangent;
};

layout(set = 0, binding = 0, scalar) buffer PositionStreamBlock
{
    Position positionStream[];
};

layout(set = 0, binding = 1, scalar) buffer UvStreamBlock
{
    vec2 uvStream[];
};

layout(set = 0, binding = 2, scalar) buffer TangentFrameStreamBlock
{
    TangentFrame tangentFrameStream[];
};

layout(set = 0, binding = 3, scalar) buffer TriangleBlock
{
    uint8_t triangles[];
};

layout(set = 0, binding = 4, scalar) buffer MeshletsBlock
{
    Meshlet meshlets[];
};

struct View
{
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
};

layout(set = 1, binding = 0) buffer perFrame
{
    View view;
};

struct InstanceData
{
    mat4 model;
    uint clusterOffset;
    uint triangleOffset;
    uint positionStreamOffset;
//    uint sharedMaterialOffset;
//    uint instncedMaterialOffset;
};

layout(set = 2, binding = 0) buffer perObject
{
    InstanceData instances[];
};

//layout(set = 3, binding = 0) buffer sharedMaterial

layout(push_constant) uniform constants
{
	uint drawId;
};

layout(location = 0) out flat uint clusterId;
layout(location = 1) out vec2 uv;
layout(location = 2) out vec3 positionWorldSpace;
layout(location = 3) out mat3 ntb;

vec4 evaluateMaterialLocalSpaceCustomPoint(vec4 positionLS)
{
    return positionLS;
}

vec4 evaluateMaterialWorldSpaceCustomPoint(vec4 positionWS)
{
    return positionWS;
}

void main()
{
    InstanceData instance = instances[drawId];

    int meshletId = int(instance.clusterOffset)+gl_VertexIndex/(64*3);
    int triangleId = gl_VertexIndex % (64*3);
    clusterId = uint(meshletId);
    Meshlet meshlet = meshlets[meshletId];

	if(triangleId >= meshlet.triangleCount*3) return;
    int index = int(instance.positionStreamOffset) + meshlet.positionStreamOffset + int(uint(triangles[int(instance.triangleOffset) + meshlet.triangleOffset + triangleId]));
    
    

    vec3 normal = tangentFrameStream[index].normal;
    vec3 tangent = tangentFrameStream[index].tangent;
    vec3 bitangent = tangentFrameStream[index].bitangent;
    
    ntb = mat3(normal, tangent, bitangent);
    
    uv = uvStream[index];

    Position p = positionStream[index];

    vec4 positionLS = vec4(p.x, p.y, p.z, 1.0);

    positionLS = evaluateMaterialLocalSpaceCustomPoint(positionLS);
    vec4 positionWS = instance.model * positionLS;

    positionWS = evaluateMaterialWorldSpaceCustomPoint(positionWS);

	gl_Position =  view.viewProjection * positionWS;
    positionWorldSpace  = positionWS.xyz/positionWS.w;
}