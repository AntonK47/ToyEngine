#include "Scene.h"
#include <fstream>
#include <iostream>
#include <zpp_bits.h>
#include <assert.h>

using namespace toy::core::scene;

void toy::core::scene::saveSceneFile(const std::vector<SceneObject>& scene,
	const std::string& filePath)
{
	auto [data, out] = zpp::bits::data_out();
	auto result = out(scene);
	if (failure(result)) 
	{
		assert(false);
	}

	auto file = std::ofstream{ filePath,std::ios_base::binary | std::ios_base::trunc };
	file.write(reinterpret_cast<const char*>(data.data()), data.size());
	file.close();
}

std::vector<SceneObject> toy::core::scene::loadSceneFile(
	const std::string& filePath)
{
	auto data = std::vector<std::byte>{};
	auto file = std::ifstream{ filePath,std::ios::binary | std::ios::ate };
	const auto totalSize = file.tellg();
	file.seekg(0);
	data.resize(totalSize);
	file.read(reinterpret_cast<char*>(data.data()), data.size());
	file.close();

	auto scene = std::vector<SceneObject>{};
	auto in = zpp::bits::in(data);
	auto result = in(scene);
	if (failure(result))
	{
		assert(false);
	}

	return scene;
}
