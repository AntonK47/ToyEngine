#include "MeshletBuilder.h"

#include "meshoptimizer.h"
#include "Mesh.h"
#include <vector>
#include <array>
#include <iostream>


using namespace toy::core::scene;

void MeshletBuilder::process(const Mesh& mesh, RuntimeMesh& processedMesh)
{
	/*
	 *	foreach mesh in scene
	 *		generate LODs
	 *		reconstruct mesh attributes (normals, uv's) for each LOD > 0
	 *		foreach LOD in LODs
	 *			generate meshlets
	 *				foreach meshlet in meshlets
	 *				compressMeshletData
	 *				generateCullData
	 *	pack meshes/LODs/meshlets/meshletCullData/meshletCompressData
	 */

	//TODO: max lod level should be computed
	for(uint32_t lodLevel = 0; lodLevel < 15; lodLevel++)
	{
		constexpr auto maxTriangles = 64u;
		constexpr auto maxVertices = maxTriangles * 3u;

		const auto maxMeshlets = meshopt_buildMeshletsBound(mesh.indices.size(), maxVertices, maxTriangles);

		std::vector<meshopt_Meshlet> meshlets;
		meshlets.resize(maxMeshlets);

		auto meshletVertices = std::vector<unsigned int>{};
		meshletVertices.resize(maxMeshlets * maxVertices);

		auto meshletTriangles = std::vector<unsigned char>{};
		meshletTriangles.resize(maxMeshlets * maxTriangles * 3);

		constexpr auto coneWeight = 0.2f;

		const auto meshletCount = meshopt_buildMeshlets(
			meshlets.data(),
			meshletVertices.data(),
			meshletTriangles.data(),
			mesh.indices.data(),
			mesh.indices.size(),
			&mesh.positionsVertexStream[0].x,
			mesh.positionsVertexStream.size(),
			sizeof(Position),
			maxVertices,
			maxTriangles,
			coneWeight);


		auto meshletAABBs = std::vector<AABB>{};
		meshletAABBs.resize(meshletCount);

		for (uint32_t i{}; i < meshletCount; i++)
		{
			const auto meshlet = meshlets[i];


			std::array<float, 3> min = 
			{
				std::numeric_limits<float>::max(),
				std::numeric_limits<float>::max(),
				std::numeric_limits<float>::max()
			};
			std::array<float, 3> max = 
			{
				std::numeric_limits<float>::min(),
				std::numeric_limits<float>::min(),
				std::numeric_limits<float>::min()
			};

			for (auto i = meshlet.vertex_offset; i < meshlet.vertex_offset + meshlet.vertex_count; i++)
			{
				min[0] = std::min(min[0], mesh.positionsVertexStream[meshletVertices[i]].x);
				min[1] = std::min(min[1], mesh.positionsVertexStream[meshletVertices[i]].y);
				min[2] = std::min(min[2], mesh.positionsVertexStream[meshletVertices[i]].z);

				max[0] = std::max(max[0], mesh.positionsVertexStream[meshletVertices[i]].x);
				max[1] = std::max(max[1], mesh.positionsVertexStream[meshletVertices[i]].y);
				max[2] = std::max(max[2], mesh.positionsVertexStream[meshletVertices[i]].z);
			}

			meshletAABBs[i].min[0] = min[0];
			meshletAABBs[i].min[1] = min[1];
			meshletAABBs[i].min[2] = min[2];

			meshletAABBs[i].max[0] = max[0];
			meshletAABBs[i].max[1] = max[1];
			meshletAABBs[i].max[2] = max[2];
		}

		auto meshletBounds = std::vector<meshopt_Bounds>{};
		meshletBounds.resize(meshletCount);


		auto meshletInfos = std::vector<Meshlet>{};
		meshletInfos.resize(meshletCount);

		auto totalTriangles = 0;
		auto totalVertexCount = 0;

		for (uint32_t i{}; i < meshletCount; i++)
		{


			const auto meshlet = meshlets[i];
			meshletBounds[i] = meshopt_computeMeshletBounds(
				&meshletVertices[meshlet.vertex_offset],
				&meshletTriangles[meshlet.triangle_offset],
				meshlet.triangle_count, &mesh.positionsVertexStream[0].x, mesh.positionsVertexStream.size(), sizeof(Position));


			meshletInfos[i] = Meshlet{ meshlet.triangle_offset, meshlet.triangle_count,  meshlet.vertex_offset, meshlet.vertex_count };
			totalTriangles += meshlet.triangle_count;
			totalVertexCount += meshlet.vertex_count;

		}
		

		auto meshletCullData = std::vector<MeshClusterCullData>{};
		meshletCullData.resize(meshletCount);

		for (uint32_t i{}; i < meshletCount; i++)
		{
			meshletCullData[i].midpoint[0] = (meshletAABBs[i].min[0] + meshletAABBs[i].max[0]) * 0.5f;
			meshletCullData[i].midpoint[1] = (meshletAABBs[i].min[1] + meshletAABBs[i].max[1]) * 0.5f;
			meshletCullData[i].midpoint[2] = (meshletAABBs[i].min[2] + meshletAABBs[i].max[2]) * 0.5f;

			meshletCullData[i].halfSize[0] = std::abs(meshletAABBs[i].max[0] - meshletAABBs[i].min[0]) * 0.5f;
			meshletCullData[i].halfSize[1] = std::abs(meshletAABBs[i].max[1] - meshletAABBs[i].min[1]) * 0.5f;
			meshletCullData[i].halfSize[2] = std::abs(meshletAABBs[i].max[2] - meshletAABBs[i].min[2]) * 0.5f;

			auto midpointOffset = std::array<float, 3>{};
			midpointOffset[0] = (meshletBounds[i].cone_apex[0] - meshletCullData[i].midpoint[0]) / meshletCullData[i].halfSize[0];
			midpointOffset[1] = (meshletBounds[i].cone_apex[1] - meshletCullData[i].midpoint[1]) / meshletCullData[i].halfSize[1];
			midpointOffset[2] = (meshletBounds[i].cone_apex[2] - meshletCullData[i].midpoint[2]) / meshletCullData[i].halfSize[2];

			meshletCullData[i].coneCenter = static_cast<uint8_t>(meshopt_quantizeSnorm(midpointOffset[0], 10)) << 20;
			meshletCullData[i].coneCenter |= static_cast<uint8_t>(meshopt_quantizeSnorm(midpointOffset[1], 10)) << 10;
			meshletCullData[i].coneCenter |= static_cast<uint8_t>(meshopt_quantizeSnorm(midpointOffset[2], 10));

			meshletCullData[i].coneAxisConeOpening.axis[0] = meshletBounds[i].cone_axis_s8[0];
			meshletCullData[i].coneAxisConeOpening.axis[1] = meshletBounds[i].cone_axis_s8[1];
			meshletCullData[i].coneAxisConeOpening.axis[2] = meshletBounds[i].cone_axis_s8[2];
			meshletCullData[i].coneAxisConeOpening.opening = meshletBounds[i].cone_cutoff_s8;
		}
		const meshopt_Meshlet& last = meshlets[meshletCount - 1];

		meshletVertices.resize(last.vertex_offset + last.vertex_count);
		meshletTriangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));
		meshlets.resize(meshletCount);
		auto positionStream = std::vector<Position>{};
		positionStream.resize(meshletVertices.size());

		auto uvVertexStream = std::vector<TextureCoordinate>{};
		uvVertexStream.resize(meshletVertices.size());

		auto tangentFrameVertexStream = std::vector<TangentFrame>{};
		tangentFrameVertexStream.resize(meshletVertices.size());

		for(uint32_t i = 0; i < positionStream.size();i++)
		{
			positionStream[i] = mesh.positionsVertexStream[meshletVertices[i]];
			uvVertexStream[i] = mesh.uvVertexStream[meshletVertices[i]];
			tangentFrameVertexStream[i] = mesh.tangentFrameVertexStream[meshletVertices[i]];
		}


		////TODO: store triangles in other way
		processedMesh.triangles.insert(processedMesh.triangles.end(), meshletTriangles.begin(), meshletTriangles.end());
		processedMesh.positionVertexStream.insert(processedMesh.positionVertexStream.end(), positionStream.begin(), positionStream.end());

		processedMesh.uvVertexStream.insert(processedMesh.uvVertexStream.end(), uvVertexStream.begin(), uvVertexStream.end());

		processedMesh.tangentFrameVertexStream.insert(processedMesh.tangentFrameVertexStream.end(), tangentFrameVertexStream.begin(), tangentFrameVertexStream.end());

		auto lodMesh = LodMesh
		{
			.header = LodHeader{ lodLevel, AABB{}, (uint32_t)totalTriangles, (uint32_t)meshletCount },
			.cullData = meshletCullData,
			.meshlets = meshletInfos
		};

		processedMesh.lods.push_back(lodMesh);
		processedMesh.header.totalTriangles = processedMesh.triangles.size()/3;
		processedMesh.header.totalVertexCount = processedMesh.positionVertexStream.size();

		//TODO: generate only lod0 for now
		break;
	}
	processedMesh.header.lods = 1;

}
