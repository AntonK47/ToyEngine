#pragma once

#include <functional>
#include <string>
#include <memory>
#include <unordered_map>


namespace toy::editor
{
	struct MaterialNode;

	struct MaterialNodeEntry
	{
		MaterialNodeEntry(std::string name, std::function<std::unique_ptr<MaterialNode>(void)> create)
			: name(name), create(create){}
		std::string name;
		std::function<std::unique_ptr<MaterialNode>(void)> create;
	};
	inline static std::unordered_map<std::string, std::vector<MaterialNodeEntry>> nodeRegistry;

	template<typename T>
	void registerMaterialNode(const std::string group, const std::string name)
	{
		nodeRegistry[group].emplace_back(name, []() { return std::make_unique<T>(); });
	}
}