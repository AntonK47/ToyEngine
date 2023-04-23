#include "SceneLoader.h"

#include <fstream>

#include "Scene.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>


using namespace toy::core;
using namespace toy::graphics::rhi;
namespace
{
    /*inline std::string loadShaderFile(const std::string& filePath)
    {

        auto fileStream = std::ifstream{ filePath, std::ios::ate };
        assert(fileStream.is_open());

        const auto length = static_cast<uint32_t>(fileStream.tellg());
        fileStream.seekg(0);

        auto code = std::vector<char>{};
        code.resize(length);

        fileStream.read(code.data(), length);
        return std::string{ code.data(), length };
    }*/

    
    void uploadDataToBuffer(RenderInterface& rhi, const void* uploadData, const size_t dataSize, const Handle<Buffer>& buffer, const u32 byteOffset)
    {
        void* data;
        rhi.map(buffer, &data);

        std::memcpy(static_cast<u8*>(data) + byteOffset, uploadData, dataSize);
        rhi.unmap(buffer);
    }

    void copyDataToBuffer(void* dstData, const void* srcData, const size_t dataSize, const Handle<Buffer>& buffer, const u32 byteOffset)
    {
        std::memcpy(static_cast<u8*>(dstData) + byteOffset, srcData, dataSize);
    }
}

Scene Scene::loadSceneFromFile(RenderInterface& rhi,
    const std::string& path)
{
    //TODO: check if exists!
    auto scene = toy::core::scene::loadSceneFile(path);

    auto totalVertexCount = u32{};
    auto totalTriangleCount = u32{};
    auto totalClusterCount = u32{};

    for(auto i = u32{}; i < scene.size(); i++)
    {
        const auto& object = scene[i];

        totalVertexCount += object.mesh.header.totalVertexCount;
        totalTriangleCount += object.mesh.triangles.size();
        totalClusterCount += object.mesh.lods[0].header.meshletsCount;
    }

    const auto positionStreamBufferSize = static_cast<u32>(totalVertexCount * sizeof(scene::Position));
    auto usage = Flags<BufferAccessUsage>{ BufferAccessUsage::vertex };
    usage |= BufferAccessUsage::storage;
    const auto positionStreamBuffer = rhi.createBuffer(BufferDescriptor
        {
            .size = positionStreamBufferSize,
            .accessUsage = usage,
            .memoryUsage = MemoryUsage::cpuOnly,
        }, 
        DebugLabel{ "PositionStreamBuffer" });

    const auto uvStreamBufferSize = static_cast<u32>(totalVertexCount * sizeof(scene::TextureCoordinate));
    const auto uvStreamBuffer = rhi.createBuffer(BufferDescriptor
        {
            .size = uvStreamBufferSize,
            .accessUsage = BufferAccessUsage::storage,
            .memoryUsage = MemoryUsage::cpuOnly,
        },
        DebugLabel{ "UvStreamBuffer" });

    const auto tangentFrameStreamBufferSize = static_cast<u32>(totalVertexCount * sizeof(scene::TangentFrame));
    const auto tangentFrameStreamBuffer = rhi.createBuffer(BufferDescriptor
        {
            .size = tangentFrameStreamBufferSize,
            .accessUsage = BufferAccessUsage::storage,
            .memoryUsage = MemoryUsage::cpuOnly,
        }, 
        DebugLabel{ "TangentFrameStreamBuffer" });


    const auto triangleBufferSize = static_cast<u32>(totalTriangleCount * sizeof(u8));
    const auto triangleBuffer = rhi.createBuffer(BufferDescriptor
        {
            .size = triangleBufferSize,
            .accessUsage = BufferAccessUsage::storage,
            .memoryUsage = MemoryUsage::cpuOnly,
        }, 
        DebugLabel{ "TrianglesBuffer"});

    const auto meshletBufferSize = static_cast<u32>(totalClusterCount * sizeof(scene::Meshlet));
    const auto meshletBuffer = rhi.createBuffer(BufferDescriptor
        {
            .size = meshletBufferSize,
            .accessUsage = BufferAccessUsage::storage,
            .memoryUsage = MemoryUsage::cpuOnly,
        }, 
        DebugLabel{ "ClustersBuffer" });

    auto meshes = std::vector<RuntimeMesh>{};
    meshes.resize(scene.size());
    
    auto vertexBufferOffset = u32{};
    auto triangleBufferOffset = u32{};
    auto meshletBufferOffset = u32{};


    void* positionStreamBufferData;
    rhi.map(positionStreamBuffer, &positionStreamBufferData);
    void* uvStreamBufferData;
    rhi.map(uvStreamBuffer, &uvStreamBufferData);
    void* tangentFrameStreamData;
    rhi.map(tangentFrameStreamBuffer, &tangentFrameStreamData);
    void* triangleBufferData;
    rhi.map(triangleBuffer, &triangleBufferData);
    void* meshletBufferData;
    rhi.map(meshletBuffer, &meshletBufferData);

    for (auto i = u32{}; i < scene.size(); i++)
    {
        const auto& object = scene[i];

        const auto objectVertexBufferSize = object.mesh.positionVertexStream.size();
        const auto objectTriangleBufferSize = object.mesh.triangles.size();
        const auto objectMeshletBufferSize = object.mesh.lods[0].meshlets.size();


        meshes[i] = RuntimeMesh
        {
            .clusterOffset = meshletBufferOffset,
            .triangleOffset = triangleBufferOffset,
            .positionStreamOffset = vertexBufferOffset,
            .vertexCount = object.mesh.header.totalTriangles * 3
        };


        copyDataToBuffer(positionStreamBufferData, (void*)object.mesh.positionVertexStream.data(), object.mesh.positionVertexStream.size() * sizeof(scene::Position), positionStreamBuffer, vertexBufferOffset * sizeof(scene::Position));

        copyDataToBuffer(uvStreamBufferData, (void*)object.mesh.uvVertexStream.data(), object.mesh.uvVertexStream.size() * sizeof(scene::TextureCoordinate), uvStreamBuffer, vertexBufferOffset * sizeof(scene::TextureCoordinate));

        copyDataToBuffer(tangentFrameStreamData, (void*)object.mesh.tangentFrameVertexStream.data(), object.mesh.tangentFrameVertexStream.size() * sizeof(scene::TangentFrame), tangentFrameStreamBuffer, vertexBufferOffset * sizeof(scene::TangentFrame));
        
        copyDataToBuffer(triangleBufferData, (void*)object.mesh.triangles.data(), object.mesh.triangles.size() * sizeof(u8), triangleBuffer, triangleBufferOffset * sizeof(u8));
        

        copyDataToBuffer(meshletBufferData, (void*)object.mesh.lods[0].meshlets.data(), object.mesh.lods[0].meshlets.size() * sizeof(scene::Meshlet), meshletBuffer, meshletBufferOffset * sizeof(scene::Meshlet));
        


        vertexBufferOffset += objectVertexBufferSize;
        triangleBufferOffset += objectTriangleBufferSize;
        meshletBufferOffset += objectMeshletBufferSize;
    }
    rhi.unmap(positionStreamBuffer);
    rhi.unmap(uvStreamBuffer);
    rhi.unmap(tangentFrameStreamBuffer);
    rhi.unmap(triangleBuffer);
    rhi.unmap(meshletBuffer);

    auto drawInstances = std::vector<DrawInstance>{};
    drawInstances.resize(scene.size());
    for (auto i = u32{}; i < scene.size(); i++)
    {
        const auto& object = scene[i];
        const auto& t = object.transform;
        drawInstances[i] = DrawInstance
        {
            .model = glm::mat4(
                t[0],t[4],t[8],t[12],
                t[1],t[5],t[9],t[13],
                t[2],t[6],t[10],t[14],
                t[3],t[7],t[11],1.0f
                ),
            .meshIndex = i
        };
    }


    return Scene{
    	meshes,
    	drawInstances,
    	positionStreamBuffer,
        tangentFrameStreamBuffer,
        uvStreamBuffer,
    	triangleBuffer,
    	meshletBuffer };
}


void Scene::buildAccelerationStructure()
{
    std::vector<scene::SceneObject> scene;
    const auto triangleCount = scene[0].mesh.lods[0].header.totalTriangles;
    const auto indexCount = triangleCount * 3;
    auto indexData = std::vector<u32>{};
    indexData.resize(indexCount);
    const auto& meshlets = scene[0].mesh.lods[0].meshlets;
    for (auto i = u32{}; i < meshlets.size(); i++)
    {
        for (auto j = meshlets[i].triangleOffset; j < meshlets[i].triangleOffset + meshlets[i].triangleCount * 3; j++)
        {
            indexData[j] = scene[0].mesh.triangles[j] + meshlets[i].positionStreamOffset;
        }
    }
}
