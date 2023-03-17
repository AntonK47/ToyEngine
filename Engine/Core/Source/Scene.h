#pragma once
#define ZPP_BITS_AUTODETECT_MEMBERS_MODE 0
#include <array>
#include <string>
#include <vector>

#include "CommonTypes.h"

//#pragma warning(disable:2500)
#include <zpp_bits.h>
//#pragma warning(default:2500)

namespace toy::core::scene
{
	using Index = unsigned int;


	struct Normal
	{
		using serialize = zpp::bits::members<3>;
		float x;
		float y;
		float z;
	};

	struct Position
	{
		using serialize = zpp::bits::members<3>;
		float x;
		float y;
		float z;
	};

	struct TextureCoordinate
	{
		using serialize = zpp::bits::members<2>;
		float u;
		float v;
	};

	struct Vector
	{
		using serialize = zpp::bits::members<3>;
		float x;
		float y;
		float z;
	};

	struct TangentFrame
	{
		using serialize = zpp::bits::members<3>;
		Vector normal;
		Vector tangent;
		Vector bitangent;
	};

	struct Mesh
	{
		using serialize = zpp::bits::members<4>;
		std::vector<Index> indices;
		std::vector<Position> positionsVertexStream;
		std::vector<TextureCoordinate> uvVertexStream;
		std::vector<TangentFrame> tangentFrameVertexStream;
	};


	struct Header
	{
		using serialize = zpp::bits::members<2>;
		u32 verticesCount;
		uint32_t indicesCount;
	};


	struct ConeAxis
	{
		using serialize = zpp::bits::members<2>;
		u8 axis[3];
		u8 opening;
	};

	struct MeshClusterCullData
	{
		using serialize = zpp::bits::members<4>;
		float midpoint[3];
		float halfSize[3];
		uint32_t coneCenter;
		ConeAxis coneAxisConeOpening;
	};

	struct AABB
	{
		using serialize = zpp::bits::members<2>;
		float min[3];
		float max[3];
	};


	struct LodHeader
	{
		using serialize = zpp::bits::members<4>;
		u32 lodLevel;
		AABB aabb;
		u32 totalTriangles;
		u32 meshletsCount;
	};


	using Triangle = std::array<u32, 3>;

	struct Meshlet
	{
		using serialize = zpp::bits::members<4>;
		u32 triangleOffset;
		u32 triangleCount;

		u32 positionStreamOffset;
		u32 positionStreamCount;
	};

	struct LodMesh
	{
		using serialize = zpp::bits::members<3>;
		LodHeader header;
		std::vector<MeshClusterCullData> cullData;
		std::vector<Meshlet> meshlets;
	};

	struct RuntimeMeshHeader
	{
		using serialize = zpp::bits::members<3>;
		u32 lods{ 0 };
		u32 totalTriangles{ 0 };
		u32 totalVertexCount{ 0 };
	};

	

	struct RuntimeMesh
	{
		using serialize = zpp::bits::members<6>;
		RuntimeMeshHeader header;
		std::vector<LodMesh> lods;
		std::vector<u8> triangles;

		std::vector<Position> positionVertexStream;
		std::vector<TextureCoordinate> uvVertexStream;
		std::vector<TangentFrame> tangentFrameVertexStream;
	};

	using Transform = std::array<float, 16>;
	struct SceneObject
	{
		using serialize = zpp::bits::members<2>;
		RuntimeMesh mesh;
		Transform transform{};
	};

	struct SceneHeader
	{
		using serialize = zpp::bits::members<1>;
		uint32_t objectCount;
	};


	void saveSceneFile(const std::vector<SceneObject>& scene, const std::string& filePath);

	std::vector<SceneObject> loadSceneFile(const std::string& filePath);
}

