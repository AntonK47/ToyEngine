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

layout(set = 0, binding = 0, scalar) buffer PositionStreamBlock
{
    Position positionStream[];
};

layout(set = 0, binding = 1, scalar) buffer TriangleBlock
{
    uint8_t triangles[];
};

layout(set = 0, binding = 2, scalar) buffer MeshletsBlock
{
    Meshlet meshlets[];
};

layout(set = 1, binding = 0) uniform perFrame
{
    vec2 iResolution;
    vec2 iMouse;
    float iTime;
};

layout(location = 0) out uint clusterId;


void main()
{
    
    int meshletId = gl_VertexIndex/(64*3);
    int triangleId = gl_VertexIndex % (64*3);
    clusterId = uint(meshletId);
    Meshlet meshlet = meshlets[meshletId];

    
	if(triangleId >= meshlet.triangleCount*3) return;
    int index = meshlet.positionStreamOffset + int(uint(triangles[meshlet.triangleOffset + triangleId]));
    Position p = positionStream[index];
    float s = 1.0f;
	gl_Position = vec4(p.x*s,p.y*s,p.z*s+0.3f,1.0);
}