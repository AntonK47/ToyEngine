#include "Gui.h"
#include "Utils.h"

#include <GlslRuntimeCompiler.h>
#include <ImageDataUploader.h>

using namespace toy::graphics::rhi;
using namespace toy::graphics::compiler;
using namespace toy::core;

namespace
{
	struct ScaleTranslate {
		glm::vec2 scale;
		glm::vec2 translate;
		u32 textureId;
	};
}

auto toy::Gui::initialize(const GuiDescriptor& descriptor) -> void
{
	renderer_ = &descriptor.renderer;
	textureManager_ = &descriptor.textureManager;
	dynamicFrameAllocator_ = &descriptor.dynamicFrameAllocator;

	auto fontSampler = Handle<Sampler>{};
	auto fontTexture = Texture2D{};
	{
		ImGui::CreateContext();

		ImGui::GetIO().ConfigWindowsResizeFromEdges = true;
		//ImGui::GetIO().BackendFlags = ImGuiBackendFlags_HasMouseCursors;

		const auto dpiScale = std::floorf(descriptor.dpiScale) / 96;
		float baseFontSize = 16.0f;
		float iconFontSize = dpiScale * baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
		ImFontConfig config;
		
		config.OversampleH = 8;
		config.OversampleV = 8;
		ImGui::GetIO().Fonts->AddFontFromFileTTF("Resources/Fonts/Roboto-Medium.ttf", dpiScale * baseFontSize, &config);

		const auto iconsRanges = std::vector<ImWchar>{ ICON_MIN_FA, ICON_MAX_16_FA, 0 };
		
		auto brandRanges = ImVector<ImWchar>{};
		ImFontGlyphRangesBuilder builder;
		builder.AddText(ICON_FA_SQUARE_TWITTER);
		builder.BuildRanges(&brandRanges);


		config.MergeMode = true;
		config.PixelSnapH = true;
		config.OversampleH = 2;
		config.OversampleV = 2;
		config.GlyphMinAdvanceX = iconFontSize;

		ImGui::GetIO().Fonts->AddFontFromFileTTF("Resources/fa-solid-900.ttf", dpiScale * baseFontSize, &config, iconsRanges.data());
		
		config.OversampleH = 8;
		config.OversampleV = 8;
		ImGui::GetIO().Fonts->AddFontFromFileTTF("Resources/fa-brands-400.ttf", dpiScale * baseFontSize, &config, brandRanges.Data);

		int width, height;
		unsigned char* pixels = NULL;
		ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
		const auto texelSize = 1;
		const auto fontImageSize = width * height * texelSize;

		ImGui::GetStyle().ScaleAllSizes(dpiScale);

		Flags<ImageAccessUsage> accessUsage = ImageAccessUsage::sampled;
		accessUsage |= ImageAccessUsage::transferDst;


		const auto fontDescriptor = ImageDescriptor
		{
			.format = Format::r8,
			.extent = Extent{ static_cast<u32>(width), static_cast<u32>(height)},
			.mips = 1,
			.layers = 1,
			.accessUsage = accessUsage
		};

		auto fontImage = renderer_->createImage(fontDescriptor);

		const auto fontView = renderer_->createImageView(ImageViewDescriptor
			{
				.image = fontImage,
				.format = Format::r8,
				.type = ImageViewType::_2D
			});

		fontSampler = renderer_->createSampler(SamplerDescriptor{ Filter::cubic, Filter::cubic, MipFilter::linear });


		fontTexture = toy::Texture2D
		{
			.width = (u32)width,
			.height = (u32)height,
			.image = fontImage,
			.view = fontView,
			.hasMips = false,
			.bytesPerTexel = texelSize
		};

		descriptor.imageUploader.upload(std::vector<std::byte>((std::byte*)pixels, (std::byte*)pixels + fontImageSize), fontTexture);

	}

	const auto guiVertexShaderGlslCode = loadShaderFile("Resources/gui.vert");
	const auto guiFragmentShaderGlslCode = loadShaderFile("Resources/gui.frag");

	const auto guiVertexShaderInfo =
		GlslRuntimeCompiler::ShaderInfo
	{
		.entryPoint = "main",
		.compilationDefines = {},
		.shaderStage = toy::graphics::compiler::ShaderStage::vertex,
		.shaderCode = guiVertexShaderGlslCode,
		.enableDebugCompilation = true
	};

	const auto guiFragmentShaderInfo =
		GlslRuntimeCompiler::ShaderInfo
	{
		.entryPoint = "main",
		.compilationDefines = {},
		.shaderStage = toy::graphics::compiler::ShaderStage::fragment,
		.shaderCode = guiFragmentShaderGlslCode,
		.enableDebugCompilation = true
	};



	auto guiVsSpirvCode = ShaderByteCode{};
	auto guiFsSpirvCode = ShaderByteCode{};

	{
		const auto result = GlslRuntimeCompiler::compileToSpirv(guiVertexShaderInfo, guiVsSpirvCode);
		TOY_ASSERT(result == CompilationResult::success);
	}
	{
		const auto result = GlslRuntimeCompiler::compileToSpirv(guiFragmentShaderInfo, guiFsSpirvCode);
		TOY_ASSERT(result == CompilationResult::success);
	}


	const auto guiVertexShaderModule = renderer_->createShaderModule(toy::graphics::rhi::ShaderStage::vertex, { ShaderLanguage::spirv1_6, guiVsSpirvCode });
	const auto guiFragmentShaderModule = renderer_->createShaderModule(toy::graphics::rhi::ShaderStage::fragment, { ShaderLanguage::spirv1_6, guiFsSpirvCode });

	const auto guiVertexDataGroup = BindGroupDescriptor
	{
		.bindings =
		{
			{
				.binding = 0,
				.descriptor = BindingDescriptor{BindingType::StorageBuffer}
			}
		}
	};

	const auto guiFontGroup = BindGroupDescriptor
	{
		.bindings =
		{
			{
				.binding = 0,
				.descriptor = BindingDescriptor{BindingType::Sampler}
			}
		}
	};

	guiVertexDataGroupLayout_ = renderer_->createBindGroupLayout(guiVertexDataGroup);
	const auto guiFontGroupLayout = renderer_->createBindGroupLayout(guiFontGroup);






	guiFontBindGroup_ = renderer_->allocateBindGroup(guiFontGroupLayout, UsageScope::async);
	renderer_->updateBindGroup(guiFontBindGroup_,
		{

				{
					0, SamplerSRV{fontSampler}
				},
		});

	guiPipeline_ = renderer_->createPipeline(
		GraphicsPipelineDescriptor
		{
			.vertexShader = guiVertexShaderModule,
			.fragmentShader = guiFragmentShaderModule,
			.renderTargetDescriptor = RenderTargetsDescriptor
			{
				.colorRenderTargets =
				{
					ColorRenderTargetDescriptor{ ColorFormat::rgba8 }
				},
		//.depthRenderTarget = DepthRenderTargetDescriptor{ DepthFormat::d32 }
	},
	.state = PipelineState
	{
		.depthTestEnabled = false,
		.faceCulling = FaceCull::none,
		.blending = Blending::alphaBlend
	}
		},
		{
			SetBindGroupMapping{0, guiVertexDataGroupLayout_},
			SetBindGroupMapping{1, guiFontGroupLayout},
			SetBindGroupMapping{2, textureManager_->getTextureBindGroupLayout() }
		},
		{
			PushConstant({.size = sizeof(ScaleTranslate) })
		});


	const auto fontUid = textureManager_->addTexture(fontTexture);

	ImGui::GetIO().Fonts->SetTexID(ImTextureID{ reinterpret_cast<void*>(static_cast<core::u64>(fontUid)) });
}

auto toy::Gui::deinitialize() -> void
{
	ImGui::DestroyContext();
}

auto toy::Gui::fillCommandList(CommandList& cmd) -> GuiDrawStatistics
{
	auto staticstics = GuiDrawStatistics{};
	const auto drawData = ImGui::GetDrawData();
	
	cmd.bindPipeline(guiPipeline_);
	cmd.bindGroup(1, guiFontBindGroup_);
	cmd.bindGroup(2, textureManager_->getTextureBindGroup());

	const auto dataMemSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
	const auto clipOff = drawData->DisplayPos;

	for (auto i = u32{}; i < drawData->CmdListsCount; i++)
	{
		auto guiVertexDataGroup = renderer_->allocateBindGroup(guiVertexDataGroupLayout_);
		const auto cmdList = drawData->CmdLists[i];

		const auto& indexRawData = cmdList->IdxBuffer;
		const auto& vertexRawData = cmdList->VtxBuffer;

		staticstics.totalIndicesCount += indexRawData.Size;
		staticstics.totalVerticesCount += vertexRawData.Size;

		const auto indexRawDataSize = indexRawData.Size * sizeof(ImDrawIdx);

		const auto indexRawDataAllocation = dynamicFrameAllocator_->allocate(indexRawDataSize);
		std::memcpy(indexRawDataAllocation.dataPtr, indexRawData.Data, indexRawDataSize);

		const auto vertexRawDataSize = vertexRawData.Size * sizeof(ImDrawVert);
		const auto vertexRawDataAllocation = dynamicFrameAllocator_->allocate(vertexRawDataSize);
		std::memcpy(vertexRawDataAllocation.dataPtr, vertexRawData.Data, vertexRawDataSize);
		renderer_->updateBindGroup(guiVertexDataGroup,
			{
				BindingDataMapping
				{
					.binding = 0,
					.view = UAV
					{
						vertexRawDataAllocation.bufferView
					}
				}
			});

		const auto scale = std::array<float, 2>{2.0f / drawData->DisplaySize.x, 2.0f / drawData->DisplaySize.y};

		cmd.bindGroup(0, guiVertexDataGroup);


		for (auto j = u32{}; j < cmdList->CmdBuffer.Size; j++)
		{
			auto scaleTranslate = ScaleTranslate
			{
				.scale = glm::vec2(scale[0], scale[1]),
				.translate = glm::vec2(-1.0f - drawData->DisplayPos.x * scale[0], -1.0f - drawData->DisplayPos.y * scale[1])
			};
			const auto& drawCommand = cmdList->CmdBuffer[j];

			const auto clipMin = ImVec2(drawCommand.ClipRect.x - clipOff.x, drawCommand.ClipRect.y - clipOff.y);
			const auto clipMax = ImVec2(drawCommand.ClipRect.z - clipOff.x, drawCommand.ClipRect.w - clipOff.y);
			if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
				continue;
			scaleTranslate.textureId = textureManager_->getBindIndex(static_cast<core::u32>(reinterpret_cast<core::u64>(drawCommand.TextureId)));
			cmd.pushConstant(scaleTranslate);
			const auto scissor = Scissor{ static_cast<i32>(clipMin.x), static_cast<i32>(clipMin.y), static_cast<u32>(clipMax.x - clipMin.x), static_cast<u32>(clipMax.y - clipMin.y) };
			cmd.setScissor(scissor);
			cmd.bindIndexBuffer(indexRawDataAllocation.bufferView.buffer, indexRawDataAllocation.bufferView.offset, sizeof(ImDrawIdx) == 2 ? IndexType::index16 : IndexType::index32);
			cmd.drawIndexed(drawCommand.ElemCount, 1, drawCommand.IdxOffset, drawCommand.VtxOffset, 0);
			staticstics.drawCalls++;
		}
	}
	return staticstics;
}
