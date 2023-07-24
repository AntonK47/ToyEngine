#pragma once

#include <variant>
#include <string>
#include <array>

#include <Core.h>
#include <ImGuiNodeEditor.h>
#include <memory>
#include <optional>
#include <format>

#include <Undo.h>
#include <MaterialModel.h>
#include <MaterialEditorResolver.h>

namespace toy::editor::resolver
{
	struct MaterialNodeResolver;
	struct PinResolveResult;
}

namespace toy::editor
{
	struct Pin;
	struct OutputPin;
	struct InputPin;

	enum class PinType
	{
		scalarType,
		rangedScalarType,
		vector2Type,
		vector3Type,
		vector4Type,
		colorType
	};

	struct Link final
	{
		ed::LinkId id{};
		ed::PinId fromPinId{};
		ed::PinId toPinId{};
		OutputPin* fromPin{};
		InputPin* toPin{};
		ImColor color{ 255,255,255,255 };
		float thickness{ 1.0f };

		~Link();

	};

	inline ImColor getTypePinColor(const PinType& type)
	{
		auto color = ImColor{};
		switch (type)
		{
		case PinType::scalarType:
			color = ImColor(150, 150, 150);
			break;
		case PinType::colorType:
			color = ImColor(140, 60, 50);
		default:
			break;
		}
		return color;
	}

	using Vec4Type = std::array<float, 4>;
	using Vec3Type = std::array<float, 3>;
	using Vec2Type = std::array<float, 2>;
	using ScalarType = float;

	struct RangedScalarType
	{
		float value{};
		float min{ std::numeric_limits<float>::min() };
		float man{ std::numeric_limits<float>::max() };
	};

	using ValueType = std::variant<Vec4Type, Vec3Type, Vec2Type, ScalarType, RangedScalarType>;

	struct MaterialNode;

	struct Pin
	{
		ed::PinId id{};
		MaterialNode* nodeOwner;
		ImColor color{};
		PinType type;
		std::string name{};
		std::string description{};
	};

	struct OutputPin final : Pin
	{
		core::u8 connectedLinksCount{ 0 };
	};

	struct InputPin final : Pin
	{
		std::vector<PinType> acceptedTypes{};
		Link* connectedLink{};

	};

	struct NodeState
	{
		inline virtual ~NodeState(){}
		auto operator<=>(const NodeState&) const = default;
		NodeState& operator=(NodeState&) = default;
		NodeState() = default;
		NodeState(const NodeState&) = default;
		NodeState(NodeState&&) = default;
		virtual std::unique_ptr<NodeState> clone() { return nullptr; }
	};

	struct MaterialNode
	{
		ed::NodeId id{};
		std::string title{};
		std::string description{};
		ImColor nodeColor{};
		std::vector<InputPin> inputPins{};
		std::vector<OutputPin> outputPins{};
		float width{ 200 };
		std::unique_ptr<resolver::MaterialNodeResolver> nodeResolver{ nullptr };
		ImVec2 position{};

		bool hasStateChanged{ false };

		inline virtual std::unique_ptr<NodeState> getStateCopy() = 0;
		inline virtual void setState(NodeState* state) = 0;
		inline virtual void submitState() = 0;
		inline virtual void drawNodeContent() {};
		inline virtual void drawPinContent(float maxWidth, core::u8 pinIndex){}
		inline virtual void deferredDraw() {}
		inline virtual void notifyInputPinConnection(){}
		inline virtual void notifyStateChange(){}
		inline virtual std::string toString() { return std::format("[{}]", id.Get()); }

		auto resolve(core::u8 outputPinIndex) -> resolver::PinResolveResult*;

		virtual ~MaterialNode();
	};

	struct MoveNodeAction final : public UndoAction
	{
	public:
		MoveNodeAction(MaterialModel* model, ed::NodeId nodeId, const ImVec2 lastPosition, const ImVec2 newPosition) :
			lastPosition(lastPosition),
			newPosition(newPosition),
			model(model),
			nodeId(nodeId) {}

		void redo() override;
		void undo() override;
		std::string toString() override;
	private:
		ImVec2 lastPosition;
		ImVec2 newPosition;
		MaterialModel* model;
		ed::NodeId nodeId;
	};


	struct NodeStateChangeAction final : public UndoAction
	{
	public:
		NodeStateChangeAction(MaterialModel* model, ed::NodeId nodeId, std::unique_ptr<NodeState> lastState, std::unique_ptr<NodeState> newState) :
			lastState(std::move(lastState)),
			newState(std::move(newState)),
			model(model),
			nodeId(nodeId){}

		void redo() override;
		void undo() override;
		std::string toString() override;
	private:
		std::unique_ptr<NodeState> lastState;
		std::unique_ptr<NodeState> newState;
		MaterialModel* model;
		ed::NodeId nodeId;
	};

	struct GroupAction final : public UndoAction
	{
	public:
		GroupAction(std::vector<std::unique_ptr<UndoAction>>& actions) :
			actions(std::move(actions)){}

		void redo() override;
		void undo() override;
		std::string toString() override;
	private:
		std::vector<std::unique_ptr<UndoAction>> actions;
	};
}