#pragma once
#include <memory>
#include <vector>
#include <fstream>

#undef min
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <meshoptimizer.h>

#include "MeshletBuilder.h"



struct Vertex
{
	float x;
	float y;
	float z;
};

using Index = unsigned int;


struct Normal
{
	float x;
	float y;
	float z;
};

struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<Normal> normals;
	std::vector<Index> indices;
};

struct Header
{
	uint32_t verticesCount;
	uint32_t indicesCount;
};

inline void saveMeshFile(const Mesh& mesh, const std::string& filePath)
{
	const auto header = Header{ static_cast<uint32_t>(mesh.vertices.size()), static_cast<uint32_t>(mesh.indices.size()) };

	auto file = std::ofstream{ filePath, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc };
	file.write(reinterpret_cast<const char*>(&header), sizeof(Header));
	for(uint32_t i{}; i < mesh.vertices.size(); i++)
	{
		const auto& vertex = mesh.vertices.at(i);
		file.write(reinterpret_cast<const char*>(&vertex), sizeof(Vertex));
	}
	for (uint32_t i{}; i < mesh.normals.size(); i++)
	{
		const auto& normal = mesh.normals.at(i);
		file.write(reinterpret_cast<const char*>(&normal), sizeof(Normal));
	}
	for (uint32_t i{}; i < mesh.indices.size(); i++)
	{
		const auto& index = mesh.indices.at(i);
		file.write(reinterpret_cast<const char*>(&index), sizeof(Index));
	}
}

inline Mesh loadMeshFile(const std::string& filePath)
{
	auto header = Header{};
	auto file = std::ifstream{ filePath, std::ios_base::in | std::ios_base::binary };
	file.read(reinterpret_cast<char*>(&header), sizeof(Header));
	Mesh mesh;
	mesh.vertices.resize(header.verticesCount);
	mesh.normals.resize(header.verticesCount);
	mesh.indices.resize(header.indicesCount);
	for (uint32_t i{}; i < mesh.vertices.size(); i++)
	{
		auto& vertex = mesh.vertices.at(i);
		file.read(reinterpret_cast<char*>(&vertex), sizeof(Vertex));
	}
	for (uint32_t i{}; i < mesh.normals.size(); i++)
	{
		auto& normal = mesh.normals.at(i);
		file.read(reinterpret_cast<char*>(&normal), sizeof(Normal));
	}
	for (uint32_t i{}; i < mesh.indices.size(); i++)
	{
		auto& index = mesh.indices.at(i);
		file.read(reinterpret_cast<char*>(&index), sizeof(Index));
	}
	return mesh;
}



inline Mesh process(const aiMesh& aiMesh)
{
	assert(aiMesh.mVertices);
	assert(aiMesh.mNormals);
	assert(aiMesh.mAABB.mMin != aiMesh.mAABB.mMax);
	//optimize
	const auto faceCount = aiMesh.mNumFaces;
	const auto indexCount = faceCount * 3;
	auto unindexedVertices = std::vector<Vertex>{};
	auto unindexedNormals = std::vector<Normal>{};


	unindexedVertices.resize(indexCount, Vertex{});
	unindexedNormals.resize(indexCount, Normal{});
	auto vertexOffset = uint32_t{};

	const auto aabb = aiMesh.mAABB;
	const auto aabbCenter = (aabb.mMin + aabb.mMax) * 0.5f;
	const auto invL = 0.5f / ((aabb.mMax.Length() > aabb.mMin.Length() ? aabb.mMax : aabb.mMin)-aabbCenter).Length();
	for (unsigned i = 0; i < aiMesh.mNumFaces; i++)
	{
		for (unsigned j = 0; j < aiMesh.mFaces[i].mNumIndices; j++)
		{
			const auto aiVertex = aiMesh.mVertices[aiMesh.mFaces[i].mIndices[j]]-aabbCenter;

			const auto aiNormal = aiMesh.mNormals[aiMesh.mFaces[i].mIndices[j]];

			const auto v = Vertex
			{
				.x = aiVertex.x * invL,
				.y = aiVertex.y * invL,
				.z = aiVertex.z * invL
			};

			const auto n = Normal{ aiNormal.x, aiNormal.y , aiNormal.z};

			unindexedVertices[vertexOffset] = v;
			unindexedNormals[vertexOffset] = n;
			vertexOffset++;
		}
	}

	auto remap = std::vector<unsigned int>(indexCount);
	const auto vertexCount = meshopt_generateVertexRemap(
		&remap[0],
		nullptr,
		indexCount,
		&unindexedVertices[0],
		indexCount,
		sizeof(Vertex));


	auto mesh = Mesh
	{
		std::vector<Vertex>(vertexCount),
		std::vector<Normal>(vertexCount),
		std::vector<Index>(indexCount)
	};


	


	meshopt_remapIndexBuffer(mesh.indices.data(), nullptr, indexCount, &remap[0]);
	meshopt_remapVertexBuffer(mesh.vertices.data(), &unindexedVertices[0], indexCount, sizeof(Vertex), &remap[0]);

	MeshletBuilder b;
	b.process(mesh);


	meshopt_remapVertexBuffer(mesh.normals.data(), &unindexedNormals[0], indexCount, sizeof(Normal), &remap[0]);
	meshopt_optimizeVertexCache(
		mesh.indices.data(),
		mesh.indices.data(),
		indexCount,
		vertexCount);
	meshopt_optimizeOverdraw(
		mesh.indices.data(),
		mesh.indices.data(),
		indexCount,
		&mesh.vertices.data()[0].x,
		vertexCount,
		sizeof(Vertex),
		1.05f);


	/*auto remapVertex = std::vector<unsigned int>(vertexCount);
	meshopt_optimizeVertexFetchRemap(remapVertex.data(), mesh.indices->data(), mesh.indices->size(), vertexCount);


	auto ver = std::make_unique<std::vector<Vertex>>(vertexCount);
	auto norm = std::make_unique<std::vector<Normal>>(vertexCount);



	meshopt_remapVertexBuffer(ver->data(), mesh.vertices->data(), vertexCount, sizeof(Vertex), &remapVertex[0]);
	meshopt_remapVertexBuffer(norm->data(), mesh.normals->data(), vertexCount, sizeof(Normal), &remapVertex[0]);

	mesh.vertices.swap(ver);
	mesh.normals.swap(norm);*/
	//meshopt_optimizeVertexFetch(
	//	mesh.vertices->data(),
	//	mesh.indices->data(),
	//	indexCount, mesh.vertices->data(),
	//	vertexCount,
	//	sizeof(Vertex));

	return mesh;
}