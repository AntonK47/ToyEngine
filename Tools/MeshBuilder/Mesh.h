#pragma once
#include <vector>

#undef min
#include <assimp/scene.h>
#include <meshoptimizer.h>

#include "MeshletBuilder.h"




inline toy::core::scene::RuntimeMesh process(const aiMesh& aiMesh)
{
	assert(aiMesh.mVertices);
	/*assert(aiMesh.mNormals);
	assert(aiMesh.mAABB.mMin != aiMesh.mAABB.mMax);
	//optimize
	const auto faceCount = aiMesh.mNumFaces;
	const auto indexCount = faceCount * 3;
	auto unindexedVertices = std::vector<toy::core::scene::Position>{};
	auto unindexedNormals = std::vector<toy::core::scene::Normal>{};


	unindexedVertices.resize(indexCount, toy::core::scene::Position{});
	unindexedNormals.resize(indexCount, toy::core::scene::Normal{});
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

			const auto v = toy::core::scene::Position
			{
				.x = aiVertex.x * invL,
				.y = aiVertex.y * invL,
				.z = aiVertex.z * invL
			};

			const auto n = toy::core::scene::Normal{ aiNormal.x, aiNormal.y , aiNormal.z};

			unindexedVertices[vertexOffset] = v;
			unindexedNormals[vertexOffset] = n;
			vertexOffset++;
		}
	}
	*/

	const auto faceCount = aiMesh.mNumFaces;
	const auto indexCount = faceCount * 3;
	auto unindexedVertices = std::vector<toy::core::scene::Position>{};





	//auto unindexedNormals = std::vector<toy::core::scene::Normal>{};


	unindexedVertices.resize(indexCount, toy::core::scene::Position{});
	//unindexedNormals.resize(indexCount, toy::core::scene::Normal{});
	auto vertexOffset = uint32_t{};

	const auto aabb = aiMesh.mAABB;
	const auto aabbCenter = (aabb.mMin + aabb.mMax) * 0.5f;
	const auto invL = 0.5f / ((aabb.mMax.Length() > aabb.mMin.Length() ? aabb.mMax : aabb.mMin) - aabbCenter).Length();
	for (unsigned i = 0; i < aiMesh.mNumFaces; i++)
	{
		for (unsigned j = 0; j < aiMesh.mFaces[i].mNumIndices; j++)
		{
			const auto aiVertex = aiMesh.mVertices[aiMesh.mFaces[i].mIndices[j]]- aabbCenter;
			

			const auto v = toy::core::scene::Position
			{
				.x = aiVertex.x * invL,
				.y = aiVertex.y * invL,
				.z = aiVertex.z * invL
			};
			
			unindexedVertices[vertexOffset] = v;
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
		sizeof(toy::core::scene::Position));


	auto mesh = toy::core::scene::Mesh
	{
		.vertices = std::vector<toy::core::scene::Position>(vertexCount),
		//std::vector<toy::core::scene::Normal>(vertexCount),
		.indices = std::vector<toy::core::scene::Index>(indexCount)
	};


	


	meshopt_remapIndexBuffer(mesh.indices.data(), nullptr, indexCount, &remap[0]);
	meshopt_remapVertexBuffer(mesh.vertices.data(), &unindexedVertices[0], indexCount, sizeof(toy::core::scene::Position), &remap[0]);

	MeshletBuilder b;
	toy::core::scene::RuntimeMesh runtimeMesh;
	b.process(mesh, runtimeMesh);


	/*meshopt_remapVertexBuffer(mesh.normals.data(), &unindexedNormals[0], indexCount, sizeof(Normal), &remap[0]);
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
		sizeof(Position),
		1.05f);*/


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

	return runtimeMesh;
}