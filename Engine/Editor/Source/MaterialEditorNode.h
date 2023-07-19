#pragma once

#include <variant>
#include <string>
#include <array>

#include <Core.h>
#include <ImGuiNodeEditor.h>
#include <memory>

namespace toy::editor::resolver
{
	struct MaterialNodeResolver;
	struct PinResolveResult;
}

namespace toy::editor
{
	enum class PinType
	{
		scalarType,
		rangedScalarType,
		vector2Type,
		vector3Type,
		vector4Type,
		colorType
	};

	struct Link
	{
		ed::LinkId id{};
		ed::PinId fromPinId{};
		ed::PinId toPinId{};
		ed::NodeId fromNodeId{};
		ed::NodeId toNodeId{};
	};

	ImColor getTypePinColor(const PinType& type)
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

	struct Pin
	{
		ed::PinId id{};
		core::u8 referenceCount{ 0 };
		std::string name{};
		std::string description{};
		ImColor accentColor{};
		PinType valueType{};
		ValueType defaultValue{ ScalarType{} };
		bool isRequired{ true };
	};

	struct MaterialNode
	{
		ed::NodeId id{};
		std::string title{};
		std::string description{};
		ImColor nodeColor{};
		std::vector<Pin> inputPins{};
		std::vector<Pin> outputPins{};
		float width{ 200 };
		std::unique_ptr<resolver::MaterialNodeResolver> nodeResolver{ nullptr };

		virtual void draw() {};
		virtual void deferredDraw() {}

		auto resolve(core::u8 outputPinIndex) -> resolver::PinResolveResult*;

		virtual ~MaterialNode() {}
	};
}