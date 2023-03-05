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

struct DrawData
{
    uint instanceIndex;
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

layout(set = 1, binding = 0) uniform perFrame
{
    View view;
};

struct InstanceData
{
    mat4 model;
    uint clusterOffset;
    uint triangleOffset;
    uint positionStreamOffset;
};

layout(set = 2, binding = 0) buffer perObject
{
    InstanceData instances[];
};

layout(push_constant) uniform constants
{
	uint drawId;
};

layout(location = 0) out uint clusterId;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 uv;
layout(location = 3) out vec3 positionWorldSpace;


void main()
{
    InstanceData instance = instances[drawId];


    int meshletId = int(instance.clusterOffset)+gl_VertexIndex/(64*3);
    int triangleId = gl_VertexIndex % (64*3);
    clusterId = uint(meshletId);
    Meshlet meshlet = meshlets[meshletId];

    
	if(triangleId >= meshlet.triangleCount*3) return;
    int index = int(instance.positionStreamOffset) + meshlet.positionStreamOffset + int(uint(triangles[int(instance.triangleOffset) + meshlet.triangleOffset + triangleId]));
    Position p = positionStream[index];

    normal = tangentFrameStream[index].normal;
    uv = uvStream[index];

    vec4 position = vec4(p.x,p.y,p.z,1.0);
    float s = 0.005f;
    mat4 scale = mat4(  s, 0.0,0.0,0.0,
                        0.0, s, 0.0,0.0,
                        0.0,0.0, s, 0.0,
                        0.0,0.0,0.0,1.0);
    vec4 positionWS = scale * instance.model * position;
	gl_Position =  view.viewProjection * positionWS;
    positionWorldSpace  = positionWS.xyz/position.w;
}