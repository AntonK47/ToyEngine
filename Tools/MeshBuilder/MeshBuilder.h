#pragma once
#include <iostream>
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <boost/program_options.hpp>

constexpr float defaultFormat = 0.1f;
constexpr std::array supportedFormats = { 0.1f };

struct ProcessOptions
{
	float format;
};

enum class ProcessResult
{
	success,
	error
};

struct SceneObject
{
	Mesh mesh;
	float transform[16]{};
};

struct SceneHeader
{
	uint32_t objectCount;
};

inline void saveSceneFile(const std::vector<SceneObject>& scene, const std::string& filePath)
{
	const auto totalObjects = static_cast<uint32_t>(scene.size());
	const auto header = SceneHeader{ totalObjects };

	auto perObjectHeader = std::vector<Header>{};
	perObjectHeader.resize(totalObjects);
	for(uint32_t i{}; i < totalObjects; i++)
	{
		perObjectHeader[i].indicesCount = static_cast<uint32_t>(scene[i].mesh.indices.size());
		perObjectHeader[i].verticesCount = static_cast<uint32_t>(scene[i].mesh.vertices.size());
	}
	auto file = std::ofstream{ filePath, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc };

	file.write(reinterpret_cast<const char*>(&header), sizeof(SceneHeader));
	file.write(reinterpret_cast<const char*>(perObjectHeader.data()), sizeof(Header) * perObjectHeader.size());

	for (uint32_t i{}; i < totalObjects; i++)
	{
		file.write(reinterpret_cast<const char*>(&scene[i].transform), sizeof(float) * 16);
	}

	for (uint32_t i{}; i < totalObjects; i++)
	{
		file.write(reinterpret_cast<const char*>(scene[i].mesh.vertices.data()), sizeof(Vertex) * scene[i].mesh.vertices.size());
		file.write(reinterpret_cast<const char*>(scene[i].mesh.normals.data()), sizeof(Normal) * scene[i].mesh.normals.size());
		file.write(reinterpret_cast<const char*>(scene[i].mesh.indices.data()), sizeof(Index) * scene[i].mesh.indices.size());
	}
}

inline std::vector<SceneObject> loadSceneFile(const std::string& filePath)
{
	auto header = SceneHeader{};
	auto file = std::ifstream{ filePath, std::ios_base::in | std::ios_base::binary };
	file.read(reinterpret_cast<char*>(&header), sizeof(SceneHeader));

	const auto totalObjects = static_cast<uint32_t>(header.objectCount);
	auto scene = std::vector<SceneObject>{};
	scene.resize(totalObjects);

	auto perObjectHeader = std::vector<Header>{};
	perObjectHeader.resize(totalObjects);
	
	file.read(reinterpret_cast<char*>(perObjectHeader.data()), sizeof(Header) * perObjectHeader.size());
	

	for (uint32_t i{}; i < totalObjects; i++)
	{
		file.read(reinterpret_cast<char*>(&scene[i].transform), sizeof(float) * 16);
	}

	for (uint32_t i{}; i < totalObjects; i++)
	{
		scene[i].mesh.vertices.resize(perObjectHeader[i].verticesCount);
		scene[i].mesh.normals.resize(perObjectHeader[i].verticesCount);
		scene[i].mesh.indices.resize(perObjectHeader[i].indicesCount);
		file.read(reinterpret_cast<char*>(scene[i].mesh.vertices.data()), sizeof(Vertex) * scene[i].mesh.vertices.size());
		file.read(reinterpret_cast<char*>(scene[i].mesh.normals.data()), sizeof(Normal) * scene[i].mesh.normals.size());
		file.read(reinterpret_cast<char*>(scene[i].mesh.indices.data()), sizeof(Index) * scene[i].mesh.indices.size());
	}
	
	return scene;
}

void processNode(const aiNode& node, const aiMatrix4x4& parentAbsoluteTransform, const aiScene& scene, std::vector<SceneObject>& scenePacked)
{
	if(node.mNumChildren > 0)
	{
		for(uint32_t i {}; i < node.mNumChildren; i++)
		{
			processNode(*node.mChildren[i],  node.mTransformation * parentAbsoluteTransform, scene, scenePacked);
		}

	}

	const auto localTransform = node.mTransformation;
	if (node.mNumMeshes>0)
	{
		const auto pMesh = scene.mMeshes[node.mMeshes[0]];
		const auto sceneObject = SceneObject
		{
			.mesh = process(*pMesh),
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

ProcessResult processScene(const aiScene& scene, std::vector<SceneObject>& scenePacked)
{
	processNode(*scene.mRootNode, aiMatrix4x4{}, scene, scenePacked);

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

ProcessResult process(const std::string& input, const std::string& output, const ProcessOptions& options)
{
	Assimp::Importer importer;

	//const auto filePath = "C:/Users/AntonKi8/source/repos/CandleFlameEngine/Resources/bun_zipper.ply";
	//const auto filePath = "C:/Users/AntonKi8/source/repos/CandleFlameEngine/Resources/xyzrgb_dragon.ply";
	const auto scene = importer.ReadFile(input,
		aiProcess_GenSmoothNormals|
		aiProcess_Triangulate |
		aiProcess_GenBoundingBoxes |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);
	importer.ApplyPostProcessing(aiPostProcessSteps::aiProcess_CalcTangentSpace);
	const auto error = importer.GetErrorString();
	assert(scene && error);
	//const auto mesh = loadMeshFile("aa.mesh");

	
	
	std::vector<SceneObject> scenePacked;
	processScene(*scene, scenePacked);
	/*if (scene->HasMeshes())
	{
		const auto pMesh = scene->mMeshes[0];
		const auto mesh = process(*pMesh);
		saveMeshFile(mesh, output);
		return ProcessResult::success;
	}*/

	saveSceneFile(scenePacked, output);

	return ProcessResult::success;
}

class MeshBuilderApplication
{
public:
	static int run(int argc, char* argv[])
	{
		namespace po = boost::program_options;



		auto format = float{ defaultFormat };
		auto desc = po::options_description{ "Options:" };
		desc.add_options()
			("help", "print this usage message")
			("format", po::value<float>(&format), "specify a target format. Possible formats are 0.1.");

		auto hidden = po::options_description{ "Hidden" };
		hidden.add_options()
			("input-output-file", po::value<std::vector<std::string>>(), "input output file pair");

		auto p = po::positional_options_description{};
		p.add("input-output-file", -1);

		auto allOptions = po::options_description{};
		allOptions.add(desc).add(hidden);

		try
		{
			po::variables_map vm;
			po::store(po::command_line_parser(argc, argv).
				options(allOptions).positional(p).run(), vm);
			po::notify(vm);

			if (vm.count("input-output-file"))
			{
				auto files = vm["input-output-file"].as<std::vector<std::string>>();

				if (files.size() != 2)
				{
					std::cout << "input-output-file should specify two file paths." << std::endl;
					return EXIT_SUCCESS;
				}
				if (std::find(std::begin(supportedFormats), std::end(supportedFormats), format) != std::end(supportedFormats))
				{
					const auto input = files[0];
					const auto output = files[1];

					if (process(input, output, ProcessOptions{ format }) == ProcessResult::success)
					{
						std::cout << output << " generation finished." << std::endl;
						return EXIT_SUCCESS;
					}
					std::cout << output << " generation failed." << std::endl;
					return EXIT_FAILURE;

				}
				std::cout << "chosen format is not supported" << std::endl;
				return EXIT_SUCCESS;
			}
		}
		catch (std::exception& e)
		{
		}

		std::cout << "Usage: meshBuilder [option]... [src file] [dst file]" << std::endl;
		std::cout << desc << std::endl;
		return EXIT_SUCCESS;
	}
};