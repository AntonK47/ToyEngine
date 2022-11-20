#pragma once
#include <string>
#include <vector>
#include <RenderInterface.h>

#include "VulkanRenderInterface.h"

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

using RenderInterface = toy::renderer::RenderInterface<toy::renderer::api::vulkan::VulkanRenderInterface>;

class Scene
{
public:
	[[nodiscard]] static Scene loadSceneFromFile(::RenderInterface& renderer, const std::string& path);

	void buildAccelerationStructure();

private:
	Scene(const std::vector<RuntimeMesh>& meshes,
		const std::vector<DrawInstance>& drawInstances,
		const toy::renderer::Buffer& positionStream,
		const toy::renderer::Buffer& tangentFrameStream,
		const toy::renderer::Buffer& uvStream,
		const toy::renderer::Buffer& triangle,
		const toy::renderer::Buffer& clusters)
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

	toy::renderer::Buffer positionStream_;
	toy::renderer::Buffer tangentFrameStream_;
	toy::renderer::Buffer uvStream_;
	toy::renderer::Buffer triangle_;
	toy::renderer::Buffer clusters_;

	toy::renderer::Buffer index_;
};
