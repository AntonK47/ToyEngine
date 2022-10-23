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
    vec4 position = vec4(p.x,p.y,p.z,1.0);
    float s = 0.5f;
	gl_Position =  view.viewProjection * position;
}