#include "MeshBuilder.h"
#include <iostream>
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assert.h>

void processNode(const aiNode& node,
	const aiMatrix4x4& parentAbsoluteTransform,
	const aiScene& scene,
	std::vector<toy::core::scene::SceneObject>& scenePacked)
{
	if(node.mNumChildren > 0)
	{
		for(uint32_t i {}; i < node.mNumChildren; i++)
		{
			processNode(*node.mChildren[i], parentAbsoluteTransform * node.mTransformation , scene, scenePacked);
		}

	}
	
	const auto localTransform = parentAbsoluteTransform*node.mTransformation;
	for (auto i = uint32_t{}; i < node.mNumMeshes; i++)
	{
		const auto pMesh = scene.mMeshes[node.mMeshes[i]];
		auto result = process(*pMesh);

		if(result.result == Result::error)
		{
			continue;
		}

		const auto sceneObject = toy::core::scene::SceneObject
		{
			.mesh = result.value,
			.transform = {
				localTransform.a1,
				localTransform.a2,
				localTransform.a3,
				localTransform.a4,
				localTransform.b1,
				localTransform.b2,
				localTransform.b3,
				localTransform.b4,
				localTransform.c1,
				localTransform.c2,
				localTransform.c3,
				localTransform.c4,
				localTransform.d1,
				localTransform.d2,
				localTransform.d3,
				localTransform.d3
			}
		};

		scenePacked.push_back(sceneObject);
	}
	

}

ProcessResult processScene(const aiScene& scene,
	std::vector<toy::core::scene::SceneObject>& scenePacked)
{
	processNode(*scene.mRootNode, aiMatrix4x4{}, scene, scenePacked);
	return  ProcessResult::success;
	if (scene.HasMaterials())
	{
		for (uint32_t i{}; i < scene.mNumMaterials; i++)
		{
			const auto& material = scene.mMaterials[i];
			std::cout << material->GetName().C_Str() << std::endl;
			for (uint32_t j{}; j < material->mNumProperties; j++)
			{
				std::cout << "_________________________________________" << std::endl;
				const auto& prop = material->mProperties[j];
				std::cout << prop->mKey.C_Str() << std::endl;

				auto data = std::vector<char>{};
				if (prop->mDataLength != 0)
				{
					data.resize(prop->mDataLength);
					std::memcpy(data.data(), prop->mData, prop->mDataLength);
				}

				switch (prop->mType)
				{
				case aiPTI_String:
					{
						aiString aiStr;
						aiStr.Set(data.data());
						auto str = std::string{ data.begin() + 1, data.end() };
						std::cout << "\tstring: " << str << std::endl;
					}
					break;
				case aiPTI_Float:
					{
						const auto fl = *reinterpret_cast<float*>(data.data());
						std::cout << "\tfloat: " << fl << std::endl;
					}
					break;
				case aiPTI_Double:
					{
						const auto dob = *reinterpret_cast<double*>(data.data());
						std::cout << "\tdouble: " << dob << std::endl;
					}
					break;
				case aiPTI_Integer:
					{
						const auto inte = *reinterpret_cast<uint32_t*>(data.data());
						std::cout << "\tinteger: " << inte << std::endl;
					}
					break;
				default:;
				}

				if (prop->mSemantic != aiTextureType_NONE)
				{
					std::cout << "\ttexture index [" << prop->mIndex << "]" << std::endl;
				}

			}
		}
	}
	return  ProcessResult::success;
}

ProcessResult process(const std::string& input,
	const std::string& output,
	const ProcessOptions& options)
{
	Assimp::Importer importer;

	const auto scene = importer.ReadFile(input
		, aiProcess_GenBoundingBoxes | aiProcess_Triangulate | aiProcess_GenSmoothNormals /*,
										 aiProcess_GenSmoothNormals |
										  |
										 aiProcess_GenBoundingBoxes |
										 aiProcess_JoinIdenticalVertices |
										 aiProcess_SortByPType*/);
	importer.ApplyPostProcessing(aiPostProcessSteps::aiProcess_CalcTangentSpace);
	const auto error = importer.GetErrorString();
	assert(scene && error);
	
	std::vector<toy::core::scene::SceneObject> scenePacked;
	processScene(*scene, scenePacked);
	saveSceneFile(scenePacked, output);

	return ProcessResult::success;
}

int MeshBuilderApplication::run(int argc, char* argv[])
{
	auto output = std::string{ "E:\\Develop\\ToyEngineContent\\interior.dat" };
	auto input = std::string{ "E:\\McGuireComputerGraphicsArchive\\Bistro\\interior.obj" };

	auto output1 = std::string{ "E:\\Develop\\ToyEngineContent\\exterior.dat" };
	auto input1 = std::string{ "E:\\McGuireComputerGraphicsArchive\\Bistro\\exterior.obj" };

	auto format = float{ defaultFormat };
	if (process(input1, output1, ProcessOptions{ format }) == ProcessResult::success)
	{
		std::cout << output << " generation finished." << std::endl;
		return EXIT_SUCCESS;
	}

	return EXIT_SUCCESS;
}
