#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <string>
#include <vector>
#include <variant>
#include <format>
#include <optional>
#include <unordered_map>
#include <array>

#define IMGUI_USER_CONFIG "toyimconfig.h"
#include <imgui.h>
#include "IconsFontAwesome6.h"
#include "ImageDataUploader.h"

#include <imgui_node_editor.h>
#include <Core.h>
#include <ValidationCommon.h>


namespace ed = ax::NodeEditor;

namespace ImGui
{
	void ToggleButton(const char* label, bool* toggleValue, const ImVec2& size = ImVec2(0, 0))
	{
		const auto color1 = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
		const auto color2 = ImGui::GetStyle().Colors[ImGuiCol_Button];

		ImGui::PushID(1);

		if (*toggleValue)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, color1);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, color2);
			if (ImGui::Button(label, size))
			{

				*toggleValue = false;
			}
			ImGui::PopStyleColor(2);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Button, color2);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, color1);
			if (ImGui::Button(label, size))
			{
				*toggleValue = true;
			}
			ImGui::PopStyleColor(2);
		}
		ImGui::PopID();
	}
}


auto mapWindowIoToImGuiIo(const toy::io::WindowIo& windowIo, ImGuiIO& io) -> void;

namespace toy::editor
{
	

	

	struct AssetManager
	{
		static auto get() -> AssetManager&
		{
			static AssetManager manager;
			return manager;
		}

		auto createNewMaterial(MaterialAssetDescriptor descriptor) -> UID
		{
			auto name = descriptor.name;
			auto nameCounter = 0;
			while(materialNameToUIDMap.contains(name))
			{
				nameCounter++;
				name = descriptor.name + "_" + std::to_string(nameCounter); 
			}

			const auto uid = core::UIDGenerator::generate();

			const auto material = MaterialAsset
			{
				.name = name
			};

			materialNameToUIDMap.insert(std::make_pair(name, uid));
			materials.insert(std::make_pair(uid, material));
			
			

			return uid;
		}

		auto deleteMaterial(const UID uid) -> void
		{
			TOY_ASSERT(materials.contains(uid));
			materialNameToUIDMap.erase(materials.at(uid).name);
			materials.erase(uid);
			//TODO: all referenced to this Asset should be notified
		}

		auto renameMaterial(const UID uid, const std::string& newName) -> void
		{
			TOY_ASSERT(!materialNameToUIDMap.contains(newName)); //TODO: return error code on this case
			TOY_ASSERT(materials.contains(uid));
			auto& material = materials.at(uid);
			const auto oldName = material.name;
			material.name = newName;

			TOY_ASSERT(materialNameToUIDMap.contains(oldName));

			materialNameToUIDMap.erase(oldName);
			materialNameToUIDMap.insert(std::make_pair(newName, uid));
		}

		auto getMaterialName(const UID uid) -> std::string
		{
			TOY_ASSERT(materials.contains(uid));
			return materials.at(uid).name;
		}

		auto getMaterialUIDByName(const std::string& name) -> UID
		{
			TOY_ASSERT(materialNameToUIDMap.contains(name));
			return materialNameToUIDMap.at(name);
		}

		std::unordered_map<std::string, UID> materialNameToUIDMap;
		std::unordered_map<UID, MaterialAsset> materials;
	};
	
	
	using TabUID = UID;
	using MaterialUID = UID;
	using AssetUID = UID;

	class Tab
	{
	public:
		Tab(const std::string& title) : title_{title}, hasChanges_{false}, isOpen_{true}{}

		auto show() -> void
		{
			showInternal();
		}

		[[nodiscard]] auto title() -> std::string
		{
			return title_;
		}

		[[nodiscard]] auto isOpen() -> bool&
		{
			return isOpen_;
		}

		auto saveAndClose()
		{
			saveAndCloseInternal();
		}

		virtual ~Tab(){}
	protected:
		virtual auto showInternal() -> void = 0;
		virtual auto saveAndCloseInternal() -> void {}
	private:
		std::string title_;
		bool isOpen_;
		bool hasChanges_;
	};


	struct MaterialEditorTab : public Tab
	{
		MaterialEditorTab(const MaterialUID uid) : Tab(AssetManager::get().getMaterialName(uid))
		{
		}
	protected:
		auto showInternal() -> void override
		{
			ImGui::Text("Tab-------------");
		}
	};

	struct TextureImportTab : public Tab
	{
		TextureImportTab() : Tab(std::string{"Texture Import"})
		{}
		auto addFiles(const std::vector<std::filesystem::path>& files) -> void
		{
			for (const auto& f : files)
			{
				fileList_.push_back(f);
			}
		}

	protected:
		auto showInternal() -> void override
		{
			ImGui::PushStyleColor(ImGuiCol_ResizeGrip, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, ImVec4(0, 0, 0, 0));

			
			static bool op = true;
			static bool use_work_area = true;
			static ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

			const ImGuiViewport* viewport = ImGui::GetMainViewport();

			auto windowSize = viewport->WorkSize;
			windowSize.x = windowSize.x / 4;

			ImGui::SetNextWindowSizeConstraints(ImVec2(0, -1), ImVec2(FLT_MAX, -1));
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(windowSize, ImGuiCond_Appearing);

			if (ImGui::Begin("Example: Fullscreen window", &op, flags))
			{
				for (const auto& file : fileList_)
				{
					ImGui::Text("%s", file.filename().generic_string().c_str());
				}

				if (&op && ImGui::Button("Close this window"))
					op = false;
			}
			ImGui::End();
			
			

			ImGui::PopStyleColor(3);
		}
	private:
		std::vector<std::filesystem::path> fileList_{};
	};

	struct TabManager
	{
		auto showTabBar(const float barWidth) -> void
		{

			const auto tabBarFlags = ImGuiTabBarFlags_None;// | ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_Reorderable;
			if (ImGui::BeginTabBar("TabBar###1", tabBarFlags))
			{
				for(auto& [tabUID, tab] : tabs)
				{
					if (tab->isOpen() && ImGui::BeginTabItem(tab->title().c_str(), &tab->isOpen(), ImGuiTabItemFlags_None))
					{
						ImGui::EndTabItem();
					}
				}
				
				ImGui::EndTabBar();
			}

			applyTabClose();
			
		}

		auto showTabContent() -> void
		{
			for (auto& [tabUID, tab] : tabs)
			{
				tab->show();
			}
		}

		auto openTextureImportTab() -> TextureImportTab*
		{
			auto tab = std::make_unique<TextureImportTab>();
			const auto tabUID = core::UIDGenerator::generate();
			tabs.insert(std::make_pair(tabUID, std::move(tab)));
			return (TextureImportTab*)tabs.at(tabUID).get();
		}

		auto openMaterialEditorTab(const MaterialUID materialUID) -> void
		{
			if (!assetUIDToTabUID.contains(materialUID))
			{
				auto tab = std::make_unique<MaterialEditorTab>(materialUID);
				const auto tabUID = core::UIDGenerator::generate();

				assetUIDToTabUID.insert(std::make_pair(materialUID, tabUID));
				tabUIDToAssetUID.insert(std::make_pair(tabUID, materialUID));
				tabs.insert(std::make_pair(tabUID, std::move(tab)));
				activTab = tabUID;
			}
			else
			{
				activTab = assetUIDToTabUID.at(materialUID);
			}
		}

		auto applyTabClose() -> void
		{
			auto delitionQueue = std::vector<TabUID>{};

			for(auto& [tabUID, tab] : tabs)
			{
				if(!tab->isOpen())
				{
					tab->saveAndClose();
					delitionQueue.push_back(tabUID);
				}
			}

			for(const auto& uid : delitionQueue)
			{
				/*const auto assetUID = tabUIDToAssetUID.at(uid);
				assetUIDToTabUID.erase(assetUID);
				tabUIDToAssetUID.erase(uid);*/
				tabs.erase(uid);
			}


			
		}
		

		std::unordered_map<AssetUID, TabUID> assetUIDToTabUID;
		std::unordered_map<TabUID, AssetUID> tabUIDToAssetUID;
		std::unordered_map<TabUID, std::unique_ptr<Tab>> tabs;
		TabUID activTab;

	};

	struct Asset
	{
		std::string relativePath;
		std::string name;
		UID uid;
		std::string creationTime;
	};

	struct Texture2DAsset : public Asset
	{
		core::u32 width;
		core::u32 height;
		Format format;
		core::u32 mips;
		core::u32 previewMipBase;
	};

	/*class AssetManager
	{


	private:
		std::vector<std::unique_ptr<Asset>> assets;
	};

	class AssetBrowser
	{
	private:
		std::vector<UID> assetTextures;
	};*/

	class Editor
	{

	public:
		auto initialize(RenderInterface& rhi, ImageDataUploader& imageDataUploader) -> void
		{
			//rhi_ = &rhi;
			//uploader_ = &imageDataUploader;

			//ImGui::CreateContext();

			//float baseFontSize = 16.0f;
			//float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

			//ImGui::GetIO().Fonts->AddFontFromFileTTF("Resources/Fonts/Roboto-Medium.ttf", baseFontSize);

			//static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
			//ImFontConfig config;
			//config.MergeMode = true;
			//config.PixelSnapH = true;
			//config.GlyphMinAdvanceX = iconFontSize;

			//ImGui::GetIO().Fonts->AddFontFromFileTTF("Resources/fa-solid-900.ttf", baseFontSize, &config, icons_ranges);

			//int width, height;
			//unsigned char* pixels = NULL;
			//ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
			//const auto texelSize = 1;
			//const auto fontImageSize = width * height * texelSize;

			//Flags<ImageAccessUsage> accessUsage = ImageAccessUsage::sampled;
			//accessUsage |= ImageAccessUsage::transferDst;


			//const auto fontDescriptor = ImageDescriptor
			//{
			//	.format = Format::r8,
			//	.extent = Extent{ static_cast<u32>(width), static_cast<u32>(height)},
			//	.mips = 1,
			//	.layers = 1,
			//	.accessUsage = accessUsage
			//};

			//auto fontImage = rhi_->createImage(fontDescriptor);
			//ImGui::GetIO().Fonts->SetTexID((void*)&fontImage);

			//const auto fontView = rhi_->createImageView(ImageViewDescriptor
			//	{
			//		.image = fontImage,
			//		.format = Format::r8,
			//		.type = ImageViewType::_2D
			//	});

			//const auto fontSampler = rhi_->createSampler(SamplerDescriptor{ Filter::linear, Filter::linear, MipFilter::linear });


			//const auto fontTexture = Texture2D
			//{
			//	.width = (u32)width,
			//	.height = (u32)height,
			//	.image = fontImage,
			//	.view = fontView,
			//	.hasMips = false,
			//	.bytesPerTexel = texelSize
			//};

			//uploader_->upload(std::vector<std::byte>((std::byte*)pixels, (std::byte*)pixels + fontImageSize), fontTexture);



			//const auto guiVertexShaderGlslCode = loadShaderFile("Resources/gui.vert");
			//const auto guiFragmentShaderGlslCode = loadShaderFile("Resources/gui.frag");

			//const auto guiVertexShaderInfo =
			//	GlslRuntimeCompiler::ShaderInfo
			//{
			//	.entryPoint = "main",
			//	.compilationDefines = {},
			//	.shaderStage = compiler::ShaderStage::vertex,
			//	.shaderCode = guiVertexShaderGlslCode,
			//	.enableDebugCompilation = true
			//};

			//const auto guiFragmentShaderInfo =
			//	GlslRuntimeCompiler::ShaderInfo
			//{
			//	.entryPoint = "main",
			//	.compilationDefines = {},
			//	.shaderStage = compiler::ShaderStage::fragment,
			//	.shaderCode = guiFragmentShaderGlslCode,
			//	.enableDebugCompilation = true
			//};


			//auto guiVsSpirvCode = ShaderByteCode{};
			//auto guiFsSpirvCode = ShaderByteCode{};

			//{
			//	const auto result = GlslRuntimeCompiler::compileToSpirv(guiVertexShaderInfo, guiVsSpirvCode);
			//	TOY_ASSERT(result == CompilationResult::success);
			//}
			//{
			//	const auto result = GlslRuntimeCompiler::compileToSpirv(guiFragmentShaderInfo, guiFsSpirvCode);
			//	TOY_ASSERT(result == CompilationResult::success);
			//}




			//const auto guiVertexShaderModule = rhi_->createShaderModule(toy::graphics::rhi::ShaderStage::vertex, { ShaderLanguage::spirv1_6, guiVsSpirvCode });
			//const auto guiFragmentShaderModule = rhi_->createShaderModule(toy::graphics::rhi::ShaderStage::fragment, { ShaderLanguage::spirv1_6, guiFsSpirvCode });


			//const auto guiVertexDataGroup = BindGroupDescriptor
			//{
			//	.bindings =
			//	{
			//		{
			//			.binding = 0,
			//			.descriptor = BindingDescriptor{BindingType::StorageBuffer}
			//		}
			//	}
			//};

			//const auto guiFontGroup = BindGroupDescriptor
			//{
			//	.bindings =
			//	{
			//		{
			//			.binding = 0,
			//			.descriptor = BindingDescriptor{BindingType::Texture2D}
			//		},
			//		{
			//			.binding = 1,
			//			.descriptor = BindingDescriptor{BindingType::Sampler}
			//		}
			//	}
			//};


			//const auto guiVertexDataGroupLayout = rhi_->createBindGroupLayout(guiVertexDataGroup);
			//const auto guiFontGroupLayout = rhi_->createBindGroupLayout(guiFontGroup);






			//Handle<BindGroup> guiFontBindGroup = rhi_->allocateBindGroup(guiFontGroupLayout, UsageScope::async);
			//rhi_->updateBindGroup(guiFontBindGroup,
			//	{
			//			{
			//				0, Texture2DSRV{fontView}
			//			},
			//			{
			//				1, SamplerSRV{fontSampler}
			//			},
			//	});

			//struct ScaleTranslate {
			//	glm::vec2 scale;
			//	glm::vec2 translate;
			//};

			//const auto guiPipeline = rhi_->createPipeline(
			//	GraphicsPipelineDescriptor
			//	{
			//		.vertexShader = guiVertexShaderModule,
			//		.fragmentShader = guiFragmentShaderModule,
			//		.renderTargetDescriptor = RenderTargetsDescriptor
			//		{
			//			.colorRenderTargets = std::initializer_list
			//			{
			//				ColorRenderTargetDescriptor{ ColorFormat::rgba8 }
			//			},
			//	//.depthRenderTarget = DepthRenderTargetDescriptor{ DepthFormat::d32 }
			//	},
			//	.state = PipelineState
			//	{
			//		.depthTestEnabled = false,
			//		.faceCulling = FaceCull::none,
			//		.blending = Blending::alphaBlend
			//	}
			//		},
			//	{
			//		SetBindGroupMapping{0, guiVertexDataGroupLayout},
			//		SetBindGroupMapping{1, guiFontGroupLayout}
			//	},
			//	{
			//		PushConstant({.size = sizeof(ScaleTranslate) })
			//	});


		}

		/*auto renderGui() -> void
		{
			ImGui::Render();
			const auto drawData = ImGui::GetDrawData();






			gatheredStatistics_.resize(perRenderThreadDrawStatistics_.size());

			std::transform(perRenderThreadDrawStatistics_.begin(), perRenderThreadDrawStatistics_.end(), gatheredStatistics_.begin(), [](auto& a) {return a.statistics; });

			drawStatistics_.scene = std::accumulate(gatheredStatistics_.begin(), gatheredStatistics_.end(), SceneDrawStaticstics{},
				[](SceneDrawStaticstics a, SceneDrawStaticstics& b)
				{
					SceneDrawStaticstics c;
					c.drawCalls = a.drawCalls + b.drawCalls;
					c.totalTrianglesCount = a.totalTrianglesCount + b.totalTrianglesCount;
					return c;
				});




			auto guiBatch = SubmitBatch{};
			{
				drawStatistics_.gui = GuiDrawStatistics{};
				rhi_->beginDebugLabel(QueueType::graphics, DebugLabel{ "GUI" });
				auto cmd = rhi_->acquireCommandList(toy::graphics::rhi::QueueType::graphics);
				cmd.begin();
				const auto renderingDescriptor = RenderingDescriptor
				{
					.colorRenderTargets = {
						RenderTargetDescriptor
						{
							.imageView = swapchainImage.view,
							.load = LoadOperation::load,
							.store = StoreOperation::store,
							.resolveMode = ResolveMode::none,
							.clearValue = ColorClear{ 100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f }
						}
					}
				};

				cmd.beginRendering(renderingDescriptor, area);

				const auto viewport = Viewport{ 0.0, (float)windowHeight,(float)windowWidth, -(float)windowHeight };

				cmd.bindPipeline(guiPipeline);
				cmd.setViewport(viewport);
				cmd.bindGroup(1, guiFontBindGroup);

				const auto dataMemSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
				const auto clipOff = drawData->DisplayPos;

				for (auto i = u32{}; i < drawData->CmdListsCount; i++)
				{
					auto guiVertexDataGroup = renderer.allocateBindGroup(guiVertexDataGroupLayout);
					const auto cmdList = drawData->CmdLists[i];

					const auto& indexRawData = cmdList->IdxBuffer;
					const auto& vertexRawData = cmdList->VtxBuffer;

					drawStatistics.gui.totalIndicesCount += indexRawData.Size;
					drawStatistics.gui.totalVerticesCount += vertexRawData.Size;

					const auto indexRawDataSize = indexRawData.Size * sizeof(ImDrawIdx);


					auto size = std::size_t{ frameDataSize };
					auto p = (void*)frameDataBeginPtr;
					frameDataBeginPtr = (u8*)std::align(16, indexRawDataSize, p, size);

					if (frameDataBeginPtr + indexRawDataSize > frameDataBeginPtr + frameDataSize)
					{
						frameDataBeginPtr = (u8*)frameDataPtr;
					}
					auto indexBufferOffset = static_cast<u32>(frameDataBeginPtr - static_cast<u8*>(frameDataPtr));
					std::memcpy(frameDataBeginPtr, indexRawData.Data, indexRawDataSize);
					frameDataBeginPtr += indexRawDataSize;

					const auto vertexRawDataSize = vertexRawData.Size * sizeof(ImDrawVert);

					size = std::size_t{ frameDataSize };
					p = (void*)frameDataBeginPtr;
					frameDataBeginPtr = (u8*)std::align(16, vertexRawDataSize, p, size);
					if (frameDataBeginPtr + indexRawDataSize > frameDataBeginPtr + frameDataSize)
					{
						frameDataBeginPtr = (u8*)frameDataPtr;
					}

					auto vertexBufferOffset = static_cast<u32>(frameDataBeginPtr - static_cast<u8*>(frameDataPtr));
					std::memcpy(frameDataBeginPtr, vertexRawData.Data, vertexRawDataSize);
					frameDataBeginPtr += vertexRawDataSize;

					renderer.updateBindGroup(guiVertexDataGroup,
						{
							BindingDataMapping
							{
								.binding = 0,
								.view = UAV
								{
									.bufferView = BufferView
									{
										.buffer = frameData,
										.offset = vertexBufferOffset,
										.size = (u64)(vertexRawDataSize)
									}
								}
							}
						});

					const auto scale = std::array<float, 2>{2.0f / drawData->DisplaySize.x, 2.0f / drawData->DisplaySize.y};
					const auto scaleTranslate = ScaleTranslate
					{
						.scale = glm::vec2(scale[0], scale[1]),
						.translate = glm::vec2(-1.0f - drawData->DisplayPos.x * scale[0], -1.0f - drawData->DisplayPos.y * scale[1])
					};
					cmd.bindGroup(0, guiVertexDataGroup);
					cmd.pushConstant(scaleTranslate);

					for (auto j = u32{}; j < cmdList->CmdBuffer.Size; j++)
					{
						const auto& drawCommand = cmdList->CmdBuffer[j];

						const auto clipMin = ImVec2(drawCommand.ClipRect.x - clipOff.x, drawCommand.ClipRect.y - clipOff.y);
						const auto clipMax = ImVec2(drawCommand.ClipRect.z - clipOff.x, drawCommand.ClipRect.w - clipOff.y);
						if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
							continue;

						const auto scissor = Scissor{ static_cast<i32>(clipMin.x), static_cast<i32>(clipMin.y), static_cast<u32>(clipMax.x - clipMin.x), static_cast<u32>(clipMax.y - clipMin.y) };
						cmd.setScissor(scissor);
						cmd.bindIndexBuffer(frameData, indexBufferOffset, sizeof(ImDrawIdx) == 2 ? IndexType::index16 : IndexType::index32);
						cmd.drawIndexed(drawCommand.ElemCount, 1, drawCommand.IdxOffset, drawCommand.VtxOffset, 0);
						drawStatistics.gui.drawCalls++;
					}
				}
				cmd.endRendering();
				cmd.end();

				auto submits = std::vector<SubmitDependency>{};
				submits.resize(10);
				std::transform(perThreadSubmits.begin(), perThreadSubmits.end(), submits.begin(), [](auto& a) { return a.barrier(); });
				guiBatch = renderer.submitCommandList(toy::graphics::rhi::QueueType::graphics, { cmd }, submits);

				renderer.submitBatches(QueueType::graphics, { guiBatch });
				renderer.endDebugLabel(QueueType::graphics);
			}
		}*/


		auto showSplashScreen();

		auto showEditorGui() -> void
		{
			showMenuBar();

			tabManager_.showTabContent();
			//showAssetBrowser();
		}


		auto openTextureImport(const std::vector<std::filesystem::path>& files) -> void
		{
			auto textureImportTab = tabManager_.openTextureImportTab();
			textureImportTab->addFiles(files);
		}

		auto tabManager() -> TabManager&
		{
			return tabManager_;
		}
	private:

		RenderInterface* rhi_;
		ImageDataUploader* uploader_;
		TabManager tabManager_;

		struct SceneDrawStaticstics
		{
			u32 drawCalls{};
			u32 totalTrianglesCount{};
		};

		struct GuiDrawStatistics
		{
			u32 drawCalls{};
			u32 totalIndicesCount{};
			u32 totalVerticesCount{};
		};

		struct DrawStatistics
		{
			SceneDrawStaticstics scene{};
			GuiDrawStatistics gui{};
		};

		struct PerThreadDrawStatistics
		{
			alignas(std::hardware_destructive_interference_size) SceneDrawStaticstics statistics {};
		};

		std::vector<PerThreadDrawStatistics> perRenderThreadDrawStatistics_{};
		DrawStatistics drawStatistics_{};
		std::vector<SceneDrawStaticstics> gatheredStatistics_{};


		bool progressQueueEnabled_{ true };
		bool progressQueueListOpened_{ false };
		core::u32 progressQueueWidgetWidth_{ 300 };
		float progressValue_{ 0.0f };

		auto showMenuBar() -> void
		{
			auto pos = ImGui::GetCursorPos();
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			const auto windowWidth = viewport->WorkSize.x;


			auto progressQueuePos = ImVec2(0, 0);

			if (ImGui::BeginMainMenuBar())
			{
				pos = ImGui::GetCursorPos();
				if (ImGui::BeginMenu("File"))
				{

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Edit"))
				{
					if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
					if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
					ImGui::Separator();
					if (ImGui::MenuItem("Cut", "CTRL+X")) {}
					if (ImGui::MenuItem("Copy", "CTRL+C")) {}
					if (ImGui::MenuItem("Paste", "CTRL+V")) {}
					ImGui::EndMenu();
				}
				ImGui::Separator();
				tabManager_.showTabBar(200);

				

				/*ImGui::Button("toggle");
				ImGui::ProgressBar(0.32);*/

				
				
				/*auto pos = ImGui::GetCursorPos();
				

				const ImGuiViewport* viewport = ImGui::GetMainViewport();

				auto windowSize = viewport->WorkSize;

				ImGui::SetNextWindowPos(pos + ImGui::)
				*/

				struct TaskStatus
				{
					std::string taskName;
					float progress;
				};

				auto inProcessingList = std::vector<TaskStatus>{};
				auto inWaitList = std::vector<TaskStatus>{};

				inProcessingList.push_back({ "testTexture MipMap generation", 0.76f });
				inProcessingList.push_back({ "testTexture2 BC7 compressing", 0.12f });
				
				inWaitList.push_back({ "testTexture3 MipMap generation", 0.0f });
				inWaitList.push_back({ "testTexture4 MipMap generation", 0.0f });
				//const auto& style = ImGui::GetStyle();

				progressValue_ += ImGui::GetIO().DeltaTime / 4.0;
				progressValue_ = std::fmodf(progressValue_, 1.0f);

				if (progressQueueEnabled_)
				{
					const auto& style = ImGui::GetStyle();

					ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
						ImGuiWindowFlags_AlwaysAutoResize |
						ImGuiWindowFlags_NoSavedSettings |
						ImGuiWindowFlags_NoFocusOnAppearing |
						ImGuiWindowFlags_NoNav |
						ImGuiWindowFlags_NoMove;

					pos.x = windowWidth - progressQueueWidgetWidth_ - style.WindowPadding.x;

					const auto menuPadding = style.FramePadding;
					ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
					ImGui::SetNextWindowBgAlpha(0.1);

					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
					ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0);
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));

					progressQueuePos = pos;
					

					if (ImGui::BeginChild("##processQueue", ImVec2(progressQueueWidgetWidth_, 20), true, windowFlags))
					{
						auto availableSize = ImGui::GetContentRegionAvail();
						const auto arrowWitdh = ImGui::CalcTextSize(ICON_FA_ANGLE_LEFT);
						const auto iconWitdh = ImGui::CalcTextSize(ICON_FA_BARS_PROGRESS);

						pos = ImGui::GetCursorPos() + style.FramePadding;
						ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
						if (ImGui::Selectable("##processQueueSelectable", progressQueueListOpened_))
						{
							progressQueueListOpened_ = !progressQueueListOpened_;
						};
						ImGui::SetItemAllowOverlap();
						ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
						ImGui::Text(ICON_FA_BARS_PROGRESS);//TODO: design a better one, use drawList
						ImGui::SameLine();
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
						ImGui::BeginGroup();
						pos = ImGui::GetCursorPos();
						ImGui::SetCursorPos(ImVec2(pos.x, pos.y - 2));
						ImGui::Text("processing..");
						const auto barWidth = windowWidth - progressQueuePos.x - style.FramePadding.x * 2 - style.ItemInnerSpacing.x - style.WindowPadding.x * 2 - menuPadding.x - iconWitdh.x;
						ImGui::ProgressBar(progressValue_, ImVec2(barWidth, 4), "");
						ImGui::EndGroup();
						ImGui::SameLine();
						pos = ImGui::GetCursorPos();
						ImGui::SetCursorPos(ImVec2(pos.x - arrowWitdh.x, pos.y - 4));
						if (progressQueueListOpened_)
						{
							ImGui::Text(ICON_FA_ANGLE_DOWN);
						}
						else
						{
							ImGui::Text(ICON_FA_ANGLE_LEFT);
						}
						
						ImGui::PopStyleVar();
					}

					ImGui::EndChild();
					ImGui::PopStyleVar(3);
				}

				const auto menuHeight = ImGui::GetFrameHeight();

				ImGui::EndMainMenuBar();


				if (progressQueueListOpened_)
				{
					const auto& style = ImGui::GetStyle();

					ImGui::SetNextWindowPos(progressQueuePos + ImVec2(0, menuHeight), ImGuiCond_Always);
					ImGui::SetNextWindowSizeConstraints(ImVec2(progressQueueWidgetWidth_, 40), ImVec2(progressQueueWidgetWidth_, 100));

					ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
						ImGuiWindowFlags_NoResize |
						ImGuiWindowFlags_NoCollapse |
						ImGuiWindowFlags_AlwaysAutoResize |
						ImGuiWindowFlags_NoSavedSettings |
						ImGuiWindowFlags_NoFocusOnAppearing |
						ImGuiWindowFlags_NoNav |
						ImGuiWindowFlags_NoMove;


					if (ImGui::Begin("progressQueueList", &progressQueueListOpened_, windowFlags | ImGuiWindowFlags_NoScrollbar))
					{
						const auto canselButtonHeight = 20;
						const auto& style = ImGui::GetStyle();
						const auto iconWidth = ImGui::CalcTextSize(ICON_FA_XMARK).x;

						ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
						ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0, 0, 0, 0));

						ImGui::BeginChild("##list", ImGui::GetContentRegionAvail() - ImVec2(0, canselButtonHeight), false, windowFlags | ImGuiWindowFlags_NoBackground);
						for (auto i = core::u32{}; i < inProcessingList.size(); i++)
						{
							ImGui::PushID(std::format("queue{}", i).c_str());

							ImGui::BeginGroup();
							ImGui::Text(inProcessingList[i].taskName.c_str());
							ImGui::SameLine(ImGui::GetContentRegionAvail().x - iconWidth - style.FramePadding.x - style.WindowPadding.x);
							ImGui::Button(ICON_FA_XMARK);

							ImGui::ProgressBar(inProcessingList[i].progress, ImVec2(-1, 4), "");
							
							ImGui::EndGroup();
							ImGui::PopID();
							if (i < inProcessingList.size() - 1)
							{
								ImGui::Separator();
							}
						}
						ImGui::SeparatorText("On Wait List");
						for (auto i = core::u32{}; i < inWaitList.size(); i++)
						{
							ImGui::PushID(std::format("queue{}", i).c_str());

							ImGui::BeginGroup();
							ImGui::Text(inWaitList[i].taskName.c_str());
							ImGui::SameLine(ImGui::GetContentRegionAvail().x - iconWidth - style.FramePadding.x - style.WindowPadding.x);
							ImGui::Button(ICON_FA_XMARK);
							ImGui::EndGroup();
							ImGui::PopID();
							if (i < inWaitList.size() - 1)
							{
								ImGui::Separator();
							}
						}
						ImGui::EndChild();

						ImGui::PopStyleColor();
						ImGui::PopStyleVar();

						ImGui::Button("Cansel All Queued Tasks", ImVec2(-1, canselButtonHeight));


					}
					ImGui::End();
				}
				
			}
		};

		//auto showAssetBrowser() -> void
		//{
		//	if (ImGui::IsMouseDown(ImGuiMouseButton_Right) && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern))
		//	{
		//		ImGui::SetDragDropPayload("FILES", nullptr, 0);
		//		ImGui::BeginTooltip();
		//		ImGui::Text(ICON_FA_FILE_IMAGE);
		//		ImGui::EndTooltip();
		//		ImGui::EndDragDropSource();
		//	}


		//	auto showAssetBrowser = true;

		//	static auto contentZoom = 1.0f;

		//	ImVec2 assetItemSize(40 * contentZoom, 40 * contentZoom);
		//	if (ImGui::Begin("AssetBrowser", &showAssetBrowser))
		//	{
		//		ImGui::SliderFloat("##", &contentZoom, 1.0f, 5.0f, "%.1f");
		//		auto& style = ImGui::GetStyle();

		//		ImGui::ProgressBar(0.3);
		//		const auto gridStartPosition = ImGui::GetCursorPos();
		//		ImGui::Dummy(ImGui::GetContentRegionAvail());

		//		if (ImGui::BeginDragDropTarget())
		//		{
		//			if (auto t = ImGui::AcceptDragDropPayload("FILES"))  // or: const ImGuiPayload* payload = ... if you sent a payload in the block above
		//			{
		//				std::cout << "new File" << std::endl;
		//				// draggedFiles is my vector of strings, how you handle your payload is up to you

		//			}

		//			ImGui::EndDragDropTarget();
		//		}
		//		ImGui::SetCursorPos(gridStartPosition);

		//		const auto contentWidth = ImGui::GetWindowPos().x
		//			+ ImGui::GetWindowContentRegionMax().x;
		//		ImGui::BeginGroup();
		//		for (auto i = u32{}; i < assetTextures.size(); i++)
		//		{
		//			ImGui::PushID(i);
		//			ImGui::ImageButton("b", (void*)assetTextures[i], assetItemSize);

		//			const auto prevItemWidth = ImGui::GetItemRectMax().x;
		//			const auto widthAfterItemPush = prevItemWidth + style.ItemSpacing.x + assetItemSize.x;
		//			if (widthAfterItemPush < contentWidth)
		//			{
		//				ImGui::SameLine();
		//			}
		//			ImGui::PopID();
		//		}
		//		ImGui::EndGroup();


		//	}
		//	ImGui::End();
		//}
	};
}