#pragma once
#include <memory>
#include <ImGuiNodeEditor.h>
#include <vector>

namespace toy::editor
{
	struct MaterialNode;
	struct Link;
	struct OutputPin;
	struct InputPin;

	struct MaterialModel
	{
		virtual auto nodes() -> std::vector<std::unique_ptr<MaterialNode>> & = 0;
		virtual auto links() -> std::vector<std::unique_ptr<Link>> & = 0;

		MaterialNode* findNode(const ed::NodeId id);
		OutputPin* findOutputPin(const ed::PinId id);
		InputPin* findInputPin(const ed::PinId id);
	};
}