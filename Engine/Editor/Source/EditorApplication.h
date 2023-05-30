#pragma once
#include <string>
#include <vector>

namespace toy::editor
{
	class EditorApplication
	{
	public:
		static int run(const std::vector<std::string>& arguments);
	};
}