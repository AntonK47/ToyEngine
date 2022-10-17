#pragma once
#include <array>
#include <string>
#include <vector>

#include "CommonTypes.h"


namespace toy::core::scene
{
	using Index = unsigned int;


	struct Normal
	{
		float x;
		float y;
		float z;
	};

	struct Position
	{
		float x;
		float y;
		float z;
	};

	struct Mesh
	{
		std::vector<Position> vertices;
		std::vector<Normal> normals;
		std::vector<Index> indices;
	};


	struct Header
	{
		u32 verticesCount;
		uint32_t indicesCount;
	};


	struct MeshClusterCullData
	{
		float midpoint[3];
		float halfSize[3];
		uint32_t coneCenter;

		struct
		{
			u8 axis[3];
			u8 opening;
		} coneAxisConeOpening;
	};

	struct AABB
	{
		float min[3];
		float max[3];
	};


	struct LodHeader
	{
		u32 lodLevel;
		AABB aabb;
		u32 totalTriangles;
		u32 meshletsCount;
	};


	using Triangle = std::array<u32, 3>;

	struct Meshlet
	{
		u32 triangleOffset;
		u32 triangleCount;

		u32 positionStreamOffset;
		u32 positionStreamCount;
	};

	struct LodMesh
	{
		LodHeader header;
		std::vector<MeshClusterCullData> cullData;
		std::vector<Meshlet> meshlets;
	};

	struct RuntimeMeshHeader
	{
		u32 lods{ 0 };
		u32 totalTriangles{ 0 };
		u32 totalVertexCount{ 0 };
	};

	struct RuntimeMesh
	{
		RuntimeMeshHeader header;
		std::vector<LodMesh> lods;
		std::vector<unsigned char> triangles;

		std::vector<Position> positionVertexStream;
	};

	using Transform = std::array<float, sizeof(float) * 16>;
	struct SceneObject
	{
		RuntimeMesh mesh;
		Transform transform{};
	};

	struct SceneHeader
	{
		uint32_t objectCount;
	};


	void saveSceneFile(const std::vector<SceneObject>& scene, const std::string& filePath);

	std::vector<SceneObject> loadSceneFile(const std::string& filePath);
}

