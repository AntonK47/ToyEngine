#pragma once
#include "Editor.h"
#include "Core.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <array>
#include <memory>
#include <format>
#include <concepts>

#include <Logger.h>

#include <MaterialEditorResolver.h>
//#include <MaterialEditorNode.h>

#include <ImGuiNodeEditor.h>

namespace toy::editor
{
	/*struct Vector4dNode final : public MaterialNode
	{

	};

	struct Vector3dNode final : public MaterialNode
	{

	};

	struct Vector3dDecompose final : public MaterialNode
	{

	};

	struct UnaryArithmeticNode final : public MaterialNode
	{
		enum class UniraArithmeticOperations
		{
			sign,
			reciprocal
		};
	};

	struct ArithmeticNode final : public MaterialNode
	{
		enum class ArithmeticOperations
		{
			addition,
			substraction,
			multiplication,
			division,
			dotproduct,
			crossproduct,
			vectorScalarMultiplication
		};

	private:
		float a;
		float b;
	};

	struct Texture2dNode final : public MaterialNode
	{

	};*/


	class MaterialEditor final : public Editor, Document, resolver::MaterialModel
	{
	public:
		auto initialize() -> void;
		auto deinitialize() -> void;
		template<std::derived_from<MaterialNode> T>
		auto registerMaterialNode(const std::vector<std::string> category)
		{

		}

		inline auto nodes() -> std::vector<std::unique_ptr<MaterialNode>>& override
		{
			return nodes_;
		}
		
		inline auto links() -> std::vector<Link>& override
		{
			return links_;
		}
	private:

		auto drawNode(MaterialNode& node) -> void;
		auto drawMaterialEditor() -> void;
		

		void onDrawGui() override;


		ed::NodeId selectedNodeId{};
		ed::PinId selectedPinId{};
		ed::LinkId selectedLinkId{};

		ed::PinId pulledInputPin_{};
		ed::PinId pulledOutputPin_{};

		ImVec2 openPopupPosition_{};

		ed::EditorContext* context_;
		bool isFirstFrame_ = true;
		std::vector<std::unique_ptr<MaterialNode>> nodes_;
		std::vector<Link> links_;

		std::unique_ptr<resolver::MaterialEditorResolver> resolver_;

	};
}

