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

#include <imgui_node_editor.h>
#include <Core.h>
#include <ValidationCommon.h>


namespace ed = ax::NodeEditor;

namespace toy::editor
{
	using UID = u32;

	struct MaterialAsset
	{
		std::string name;
	};

	struct MaterialAssetDescriptor
	{
		std::string name;
	};

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

	struct MenuTabManager
	{
		auto showTabBar(const float barWidth) -> void
		{


			//const auto tabBarFlags = ImGuiTabBarFlags_None;// | ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_Reorderable;
			//if (ImGui::BeginTabBar("TabBar###1", tabBarFlags))
			//{
			//	for(auto& [tabUID, tab] : tabs)
			//	{
			//		auto flags = 0;
			//		if(tabUID == activTab)
			//		{
			//			flags |= ImGuiTabItemFlags_SetSelected;
			//		}

			//		static bool t = true;
			//		if (t && ImGui::BeginTabItem(tab->title().c_str()), &t, ImGuiTabItemFlags_None )
			//		{
			//			ImGui::EndTabItem();
			//		}


			//	}
			//	ImGui::EndTabBar();
			//}

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
				const auto assetUID = tabUIDToAssetUID.at(uid);
				assetUIDToTabUID.erase(assetUID);
				tabUIDToAssetUID.erase(uid);
				tabs.erase(uid);
			}
		}
		

		std::unordered_map<AssetUID, TabUID> assetUIDToTabUID;
		std::unordered_map<TabUID, AssetUID> tabUIDToAssetUID;
		std::unordered_map<TabUID, std::unique_ptr<Tab>> tabs;
		TabUID activTab;

	};

	class Editor
	{

	public:
		auto showEditorGui() -> void
		{
			showMenuBar();
		}

		auto tabManager() -> MenuTabManager&
		{
			return tabManager_;
		}
	private:

		MenuTabManager tabManager_;

		auto showMenuBar() -> void
		{
			if (ImGui::BeginMainMenuBar())
			{
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

				ImGui::EndMainMenuBar();
			}
		};
	};
}