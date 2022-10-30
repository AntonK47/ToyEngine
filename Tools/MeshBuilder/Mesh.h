#pragma once
#include <vector>

#undef min
#include <assimp/scene.h>
#include <meshoptimizer.h>

#include "MeshletBuilder.h"




inline toy::core::scene::RuntimeMesh process(const aiMesh& aiMesh)
{
	assert(aiMesh.mVertices);
	assert(aiMesh.mNormals);
	assert(aiMesh.mTangents);
	assert(aiMesh.mBitangents);
	assert(aiMesh.mTextureCoords);
	/*
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
	auto unindexedPositions = std::vector<toy::core::scene::Position>{};
	unindexedPositions.resize(indexCount);

	auto unindexedTextureCoordinates = std::vector<toy::core::scene::TextureCoordinate>{};
	unindexedTextureCoordinates.resize(indexCount);

	auto unindexedTangentFrames = std::vector<toy::core::scene::TangentFrame>{};
	unindexedTangentFrames.resize(indexCount);

	auto vertexOffset = uint32_t{};
	for (unsigned i = 0; i < aiMesh.mNumFaces; i++)
	{
		for (unsigned j = 0; j < aiMesh.mFaces[i].mNumIndices; j++)
		{
			const auto uvChannel = uint32_t{ 0 };
			const auto aiPosition = aiMesh.mVertices[aiMesh.mFaces[i].mIndices[j]];
			const auto aiNormal = aiMesh.mNormals[aiMesh.mFaces[i].mIndices[j]];
			const auto aiBitangent = aiMesh.mBitangents[aiMesh.mFaces[i].mIndices[j]];
			const auto aiTangent = aiMesh.mTangents[aiMesh.mFaces[i].mIndices[j]];
			const auto aiTextureCoordinate = aiMesh.mTextureCoords[uvChannel][aiMesh.mFaces[i].mIndices[j]];
			

			const auto v = toy::core::scene::Position
			{
				.x = aiPosition.x,
				.y = aiPosition.y,
				.z = aiPosition.z
			};
			
			unindexedPositions[vertexOffset] = v;

			unindexedTextureCoordinates[vertexOffset] = toy::core::scene::TextureCoordinate
			{
				.u = aiTextureCoordinate.x,
				.v = aiTextureCoordinate.y
			};

			unindexedTangentFrames[vertexOffset].normal = toy::core::scene::Vector
			{
				.x = aiNormal.x,
				.y = aiNormal.y,
				.z = aiNormal.z
			};

			unindexedTangentFrames[vertexOffset].bitangent = toy::core::scene::Vector
			{
				.x = aiBitangent.x,
				.y = aiBitangent.y,
				.z = aiBitangent.z
			};

			unindexedTangentFrames[vertexOffset].tangent = toy::core::scene::Vector
			{
				.x = aiTangent.x,
				.y = aiTangent.y,
				.z = aiTangent.z
			};

			vertexOffset++;
		}
	}

	auto remap = std::vector<unsigned int>(indexCount);
	const auto vertexCount = meshopt_generateVertexRemap(
		&remap[0],
		nullptr,
		indexCount,
		&unindexedPositions[0],
		indexCount,
		sizeof(toy::core::scene::Position));


	auto mesh = toy::core::scene::Mesh
	{
		.indices = std::vector<toy::core::scene::Index>(indexCount),
		.positionsVertexStream = std::vector<toy::core::scene::Position>(vertexCount),
		.uvVertexStream = std::vector<toy::core::scene::TextureCoordinate>(vertexCount),
		.tangentFrameVertexStream = std::vector<toy::core::scene::TangentFrame>(vertexCount)
	};



	meshopt_remapIndexBuffer(
		mesh.indices.data(),
		nullptr,
		indexCount,
		remap.data());

	meshopt_remapVertexBuffer(
		mesh.positionsVertexStream.data(),
		unindexedPositions.data(),
		indexCount,
		sizeof(toy::core::scene::Position),
		remap.data());

	meshopt_remapVertexBuffer(
		mesh.uvVertexStream.data(),
		unindexedTextureCoordinates.data(),
		indexCount,
		sizeof(toy::core::scene::TextureCoordinate),
		remap.data());

	meshopt_remapVertexBuffer(
		mesh.tangentFrameVertexStream.data(),
		unindexedTangentFrames.data(),
		indexCount,
		sizeof(toy::core::scene::TangentFrame),
		remap.data());

	MeshletBuilder b;
	toy::core::scene::RuntimeMesh runtimeMesh;
	b.process(mesh, runtimeMesh);

	return runtimeMesh;
}