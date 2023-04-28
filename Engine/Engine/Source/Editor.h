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
#include "IconsFontAwesome6Brands.h"
#include "ImageDataUploader.h"

#include <imgui_node_editor.h>
#include <Core.h>
#include <ValidationCommon.h>
#include <Window.h>
#include <Material.h>

#include "TextureManager.h"
#include "BackgroundTasksSystem.h"
using namespace toy::core;


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

		auto createNewMaterial(MaterialAssetDescriptor descriptor) -> toy::core::UID
		{
			auto& name = descriptor.name;
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
			const auto& oldName = material.name;
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

	namespace
	{
		BackgroundTasksSystem backgroundTasksSystem{ 3 };
	}


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
				if (ImGui::Button("Process All"))
				{
					for (auto i = u32{}; i < fileList_.size(); i++)
					{
						const auto& file = fileList_[i];
						if (!backgroundTasksSystem.hasTask(i))
						{
							backgroundTasksSystem.createTask(i, [](TaskFeedback& feedback)
								{
									using namespace std::chrono_literals;

									for (auto j = u32{}; j < 100; j++)
									{
										std::this_thread::sleep_for(50ms);
										feedback.report(j * 0.01f);
										if (feedback.stopRequested())
											return;
									}
								});

						}
					}
				}

				for (auto i = u32{}; i < fileList_.size(); i++)
				{
					const auto& file = fileList_[i];
					ImGui::PushID(i);
					ImGui::Text("%s", file.filename().generic_string().c_str()); 
					ImGui::SameLine();
					if (ImGui::Button(std::format("process##{}", i).c_str()))
					{
						if (!backgroundTasksSystem.hasTask(i))
						{
							backgroundTasksSystem.createTask(i, [](TaskFeedback& control)
								{
									using namespace std::chrono_literals;

									for (auto j = u32{}; j < 100; j++)
									{
										std::this_thread::sleep_for(50ms);
										control.report(j * 0.01f);
										if (control.stopRequested())
											return;
									}

								});
						}
					}
					if (backgroundTasksSystem.hasTask(i))
					{
						ImGui::ProgressBar(backgroundTasksSystem.requestProgress(i));

						if (backgroundTasksSystem.status(i) == TaskStatus::done)
						{
							backgroundTasksSystem.freeTask(i);
						}
					}
					ImGui::PopID();
				}
				

				if (&op && ImGui::Button("Close"))
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
		enum class ViewState
		{
			splash,
			sceneView
		};

		ViewState viewState_;
		

		core::UID splashScreenImage_;
		const float splashScreenAnimationDuration_ = 0.8f;
		const float splashScreenIdleDuration_ = 2.0f;
		bool isAnimationPlaying_ = false;
		float time_ = 0.0f;
		float totalTime_ = 0.0f;
		float animationOffsetBase_ = 0.0f;
		const core::u32 splashScreenSize_ = 400;

	public:
		auto initialize(RenderInterface& rhi, window::Window& windowInterface, ImageDataUploader& imageDataUploader, TextureManager& textureManager) -> void
		{
			//backgroundTasksSystem_ = std::make_unique<BackgroundTasksSystem>();
			window_ = &windowInterface;
			viewState_ = ViewState::splash;
			window_->resize(splashScreenSize_, splashScreenSize_);

			
			splashScreenImage_ = toy::helper::loadTexture("Resources/generated_splash_screen.DDS", rhi, imageDataUploader, textureManager);
			window_->setWindowDraggingRegion(core::Rectangle{0, 0, splashScreenSize_, splashScreenSize_});
		}

		auto navigateToSceneView()
		{
			viewState_ = ViewState::sceneView;
			window_->resize(1920, 1080);
			window_->enableBorder();
			ImGui::GetIO().BackendFlags = ImGuiBackendFlags_HasMouseCursors;
			ImGui::GetIO().MouseDrawCursor = true;
			window_->setWindowDraggingRegion();
		}

		auto easeInOutQuart(const float x) -> float
		{
			return x < 0.5f ? 8.0f * x * x * x * x : 1.0f - std::powf(-2.0f * x + 2.0f, 4.0f) / 2.0f;
		}


		auto showSplashScreen()
		{
			ImGuiWindowFlags windowFlags =
				ImGuiWindowFlags_NoDecoration |
				//ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoFocusOnAppearing |
				ImGuiWindowFlags_NoNav |
				ImGuiWindowFlags_NoBackground |
				ImGuiWindowFlags_NoMove;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(splashScreenSize_, splashScreenSize_), ImGuiCond_Always);
			static auto vis = true;
			if (ImGui::Begin("SplashScreen", &vis, windowFlags))
			{
				time_ += ImGui::GetIO().DeltaTime;
				totalTime_+= ImGui::GetIO().DeltaTime;
				auto animationTime = 0.0f;
				if (time_ > splashScreenIdleDuration_)
				{
					time_ = 0.0f;
					isAnimationPlaying_ = true;

				}
				if (isAnimationPlaying_)
				{
					animationTime = time_;
				}
				
				auto offset = easeInOutQuart(animationTime / splashScreenAnimationDuration_) * 1.0f / 3.0f;

				if (time_ > splashScreenAnimationDuration_ && isAnimationPlaying_)
				{
					time_ = 0.0f;
					isAnimationPlaying_ = false;
					animationTime = 0.0f;
					animationOffsetBase_ += 1.0f / 3.0f;
				}

				ImGui::Image((void*)splashScreenImage_, 
					ImVec2(splashScreenSize_, splashScreenSize_), 
					ImVec2(0 + animationOffsetBase_ + offset, 0), 
					ImVec2(1.0f / 3 + animationOffsetBase_ + offset, 1.0f));
				

				ImGui::SetCursorPos(ImVec2(10, 10));
				ImGui::Text("TOY ENGINE");
				ImGui::Text("");ImGui::SameLine(10);
				ImGui::Text(std::format("{} {}", ICON_FA_SQUARE_TWITTER, "@AntonKi8").c_str());
				auto textSize = ImGui::CalcTextSize("images by Kandinsky 2.1");
				ImGui::SetCursorPos(ImVec2(splashScreenSize_ - textSize.x - 10, splashScreenSize_ - textSize.y - 10));
				ImGui::Text("images by Kandinsky 2.1");
				ImGui::SetCursorPos(ImVec2(0, splashScreenSize_ - 10));
				ImGui::ProgressBar(totalTime_ /15.0f, ImVec2(splashScreenSize_, 10), "");

				
			}
			ImGui::End();
			ImGui::PopStyleVar(3);

			if (totalTime_ >= 15.0f)
			{
				navigateToSceneView();
			}
		}

		auto showEditorGui() -> void
		{
			switch (viewState_)
			{
			case toy::editor::Editor::ViewState::splash:
				showSplashScreen();
				break;
			case toy::editor::Editor::ViewState::sceneView:
				showMenuBar();
				tabManager_.showTabContent();
				break;
			default:
				break;
			}


			
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
		std::unique_ptr<BackgroundTasksSystem> backgroundTasksSystem_;
		RenderInterface* rhi_;
		window::Window* window_;
		ImageDataUploader* uploader_;
		TabManager tabManager_;

		struct SceneDrawStatistics
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
			SceneDrawStatistics scene{};
			GuiDrawStatistics gui{};
		};

		struct PerThreadDrawStatistics
		{
			alignas(std::hardware_destructive_interference_size) SceneDrawStatistics statistics {};
		};

		std::vector<PerThreadDrawStatistics> perRenderThreadDrawStatistics_{};
		DrawStatistics drawStatistics_{};
		std::vector<SceneDrawStatistics> gatheredStatistics_{};


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
						const auto arrowWidth = ImGui::CalcTextSize(ICON_FA_ANGLE_LEFT);
						const auto iconWidth = ImGui::CalcTextSize(ICON_FA_BARS_PROGRESS);

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
						const auto barWidth = windowWidth - progressQueuePos.x - style.FramePadding.x * 2 - style.ItemInnerSpacing.x - style.WindowPadding.x * 2 - menuPadding.x - iconWidth.x;
						ImGui::ProgressBar(progressValue_, ImVec2(barWidth, 4), "");
						ImGui::EndGroup();
						ImGui::SameLine();
						pos = ImGui::GetCursorPos();
						ImGui::SetCursorPos(ImVec2(pos.x - arrowWidth.x, pos.y - 4));
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
					ImGui::SetNextWindowSizeConstraints(ImVec2(progressQueueWidgetWidth_, 40), ImVec2(progressQueueWidgetWidth_, 400));

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
						const auto cancelButtonHeight = 20;
						const auto& style = ImGui::GetStyle();
						const auto iconWidth = ImGui::CalcTextSize(ICON_FA_XMARK).x;

						ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
						ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0, 0, 0, 0));

						ImGui::BeginChild("##list", ImGui::GetContentRegionAvail() - ImVec2(0, cancelButtonHeight), false, windowFlags | ImGuiWindowFlags_NoBackground);
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

						ImGui::Button("Cancel All Queued Tasks", ImVec2(-1, cancelButtonHeight));
					}
					ImGui::End();
				}
				
			}
		};
	};
}