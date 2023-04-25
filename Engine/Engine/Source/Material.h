#pragma once
#include <string>
#include <Core.h>
#include <RenderInterfaceTypes.h>

using namespace toy::core;
using namespace toy::graphics;
using namespace toy::graphics::rhi;


namespace toy
{
	struct MaterialAsset
	{
		std::string name;
	};

	struct MaterialAssetDescriptor
	{
		std::string name;
	};

	struct SharedMaterial
	{
		Handle<Pipeline> pipeline;
	};

	struct MaterialInstance
	{
		u32 sharedMaterialIndex;
		u32 instanceDataIndex;
	};

	struct MaterialTemplate
	{
		std::string vertexShaderTemplate;
		std::string fragmentShaderTemplate;
	};

	struct FrameContext
	{
		Handle<BindGroupLayout> texturesLayout;
		Handle<BindGroupLayout> geometryLayout;
		Handle<BindGroupLayout> perInstanceDataLayout;
		Handle<BindGroupLayout> perFrameDataLayout;
		Handle<BindGroupLayout> samplerLayout;
	};

	struct MaterialCompiolationDescriptor
	{
		const MaterialTemplate& materialTemplate;
		const FrameContext& frameContext;
		const MaterialAsset& asset;
	};


	struct MaterialGenerator
	{
		static auto compilePipeline(RenderInterface& renderer, const MaterialCompiolationDescriptor& descriptor) -> SharedMaterial;
	};
}