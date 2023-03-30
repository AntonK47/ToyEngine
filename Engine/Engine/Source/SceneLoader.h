#pragma once
#include <string>
#include <vector>
#include <RenderInterface.h>
#include <RenderInterfaceTypes.h>

struct RuntimeMesh
{
	toy::core::u32 clusterOffset;
	toy::core::u32 triangleOffset;
	toy::core::u32 positionStreamOffset;
	toy::core::u32 vertexCount;
};

struct DrawInstance
{
	glm::mat4 model;
	toy::core::u32 meshIndex;
};


class Scene
{
public:
	[[nodiscard]] static Scene loadSceneFromFile(toy::graphics::rhi::RenderInterface& rhi, const std::string& path);

	void buildAccelerationStructure();

private:
	Scene(const std::vector<RuntimeMesh>& meshes,
		const std::vector<DrawInstance>& drawInstances,
		const toy::graphics::rhi::Handle<toy::graphics::rhi::Buffer>& positionStream,
		const toy::graphics::rhi::Handle<toy::graphics::rhi::Buffer>& tangentFrameStream,
		const toy::graphics::rhi::Handle<toy::graphics::rhi::Buffer>& uvStream,
		const toy::graphics::rhi::Handle<toy::graphics::rhi::Buffer>& triangle,
		const toy::graphics::rhi::Handle<toy::graphics::rhi::Buffer>& clusters)
		: meshes_(meshes),
		  drawInstances_(drawInstances),
		  positionStream_(positionStream),
		  tangentFrameStream_(tangentFrameStream),
		  uvStream_(uvStream),
		  triangle_(triangle),
		  clusters_(clusters)
	{
	}

public:
	std::vector<RuntimeMesh> meshes_;
	std::vector<DrawInstance> drawInstances_;

	toy::graphics::rhi::Handle<toy::graphics::rhi::Buffer> positionStream_;
	toy::graphics::rhi::Handle<toy::graphics::rhi::Buffer> tangentFrameStream_;
	toy::graphics::rhi::Handle<toy::graphics::rhi::Buffer> uvStream_;
	toy::graphics::rhi::Handle<toy::graphics::rhi::Buffer> triangle_;
	toy::graphics::rhi::Handle<toy::graphics::rhi::Buffer> clusters_;
	toy::graphics::rhi::Handle<toy::graphics::rhi::Buffer> index_;
};
