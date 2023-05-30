#pragma once
#include <fstream>
#include <string>
#include <assert.h>
#include <vector>
namespace toy
{
	inline std::string loadShaderFile(const std::string& filePath)
	{

		auto fileStream = std::ifstream{ filePath, std::ios::ate };
		assert(fileStream.is_open());

		const auto length = static_cast<uint32_t>(fileStream.tellg());
		fileStream.seekg(0);

		auto code = std::vector<char>{};
		code.resize(length);

		fileStream.read(code.data(), length);
		return std::string{ code.data(), length };
	}
}