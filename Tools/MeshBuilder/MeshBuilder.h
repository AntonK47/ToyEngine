#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <boost/program_options.hpp>
#include "Scene.h"

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

void processNode(const aiNode& node, const aiMatrix4x4& parentAbsoluteTransform, const aiScene& scene, std::vector<toy::core::scene::SceneObject>& scenePacked);

ProcessResult processScene(const aiScene& scene, std::vector<toy::core::scene::SceneObject>& scenePacked);

ProcessResult process(const std::string& input, const std::string& output, const ProcessOptions& options);

class MeshBuilderApplication
{
public:
	static int run(int argc, char* argv[]);
};
