#include "Scene.h"
#include <fstream>

using namespace toy::core::scene;

void toy::core::scene::saveSceneFile(const std::vector<SceneObject>& scene,
	const std::string& filePath)
{
	const auto totalObjects = static_cast<uint32_t>(scene.size());
	const auto header = SceneHeader{ totalObjects };

	auto perObjectHeader = std::vector<RuntimeMeshHeader>{};
	perObjectHeader.resize(totalObjects);
	for (uint32_t i{}; i < totalObjects; i++)
	{
		perObjectHeader[i] = scene[i].mesh.header;
	}
	auto file = std::ofstream{ filePath, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc };

	file.write(reinterpret_cast<const char*>(&header), sizeof(SceneHeader));
	file.write(reinterpret_cast<const char*>(perObjectHeader.data()), sizeof(RuntimeMeshHeader) * perObjectHeader.size());

	for (uint32_t i{}; i < totalObjects; i++)
	{
		file.write(reinterpret_cast<const char*>(&scene[i].transform), sizeof(Transform));
	}

	for (uint32_t i{}; i < totalObjects; i++)
	{
		auto perLodHeader = std::vector<LodHeader>{};
		perLodHeader.resize(scene[i].mesh.header.lods);

		for (uint32_t j{}; j < perLodHeader.size(); j++)
		{
			perLodHeader[j] = scene[i].mesh.lods[j].header;
		}

		file.write(reinterpret_cast<const char*>(perLodHeader.data()), sizeof(LodMesh) * perLodHeader.size());

		for (uint32_t j{}; j < perLodHeader.size(); j++)
		{
			file.write(reinterpret_cast<const char*>(scene[i].mesh.lods[j].cullData.data()), sizeof(MeshClusterCullData) * perLodHeader[j].meshletsCount);
			file.write(reinterpret_cast<const char*>(scene[i].mesh.lods[j].meshlets.data()), sizeof(Meshlet) * perLodHeader[j].meshletsCount);
		}

	}


	for (uint32_t i{}; i < totalObjects; i++)
	{
		file.write(reinterpret_cast<const char*>(scene[i].mesh.triangles.data()), sizeof(unsigned char) * scene[i].mesh.triangles.size());
		file.write(reinterpret_cast<const char*>(scene[i].mesh.positionVertexStream.data()), sizeof(Position) * scene[i].mesh.positionVertexStream.size());
	}
}

std::vector<SceneObject> toy::core::scene::loadSceneFile(
	const std::string& filePath)
{
	auto scene = std::vector<SceneObject>{};
	auto header = SceneHeader{};
	auto file = std::ifstream{ filePath, std::ios_base::in | std::ios_base::binary };

	file.read(reinterpret_cast<char*>(&header), sizeof(SceneHeader));

	const auto totalObjects = static_cast<uint32_t>(header.objectCount);

	scene.resize(totalObjects);

	auto perObjectHeader = std::vector<RuntimeMeshHeader>{};
	perObjectHeader.resize(totalObjects);

	file.read(reinterpret_cast<char*>(perObjectHeader.data()), sizeof(RuntimeMeshHeader) * totalObjects);


	auto perObjectTransform = std::vector<Transform>{};
	perObjectTransform.resize(totalObjects);

	for (uint32_t i{}; i < totalObjects; i++)
	{
		file.read(reinterpret_cast<char*>(perObjectTransform.data()), sizeof(Transform));
	}


	for (uint32_t i{}; i < totalObjects; i++)
	{
		scene[i].mesh.header = perObjectHeader[i];
		scene[i].transform = perObjectTransform[i];
		scene[i].mesh.lods.resize(perObjectHeader[i].lods);
		scene[i].mesh.triangles.resize(perObjectHeader[i].totalTriangles);
		scene[i].mesh.positionVertexStream.resize(perObjectHeader[i].totalVertexCount);
	}

	for (uint32_t i{}; i < totalObjects; i++)
	{
		auto perLodHeader = std::vector<LodHeader>{};
		perLodHeader.resize(perObjectHeader[i].lods);
		file.read(reinterpret_cast<char*>(perLodHeader.data()), sizeof(LodMesh) * perLodHeader.size());

		for (uint32_t j{}; j < perLodHeader.size(); j++)
		{
			scene[i].mesh.lods[j].header = perLodHeader[j];
			scene[i].mesh.lods[j].cullData.resize(perLodHeader[j].meshletsCount);
			scene[i].mesh.lods[j].meshlets.resize(perLodHeader[j].meshletsCount);

			file.read(reinterpret_cast<char*>(scene[i].mesh.lods[j].cullData.data()), sizeof(MeshClusterCullData) * perLodHeader[j].meshletsCount);
			file.read(reinterpret_cast<char*>(scene[i].mesh.lods[j].meshlets.data()), sizeof(Meshlet) * perLodHeader[j].meshletsCount);
		}
	}


	for (uint32_t i{}; i < totalObjects; i++)
	{

		file.read(reinterpret_cast<char*>(scene[i].mesh.triangles.data()), sizeof(unsigned char) * scene[i].mesh.triangles.size());
		file.read(reinterpret_cast<char*>(scene[i].mesh.positionVertexStream.data()), sizeof(Position) * scene[i].mesh.positionVertexStream.size());
	}

	return scene;
}
