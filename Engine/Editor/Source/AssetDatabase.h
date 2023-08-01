#pragma once

#include <Core.h>
#include <Logger.h>
#include <crossguid/guid.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using uuid = xg::Guid;


namespace nlohmann {
	template<>
	struct adl_serializer<uuid> {
		static void to_json(json& j, const uuid& id) {
			j = id.str();
		}

		static void from_json(const json& j, uuid& id) {
			if (j.is_null()) {
				id = uuid{};
			}
			else {
				id = uuid{ j.template get<std::string>() };
			}
		}
	};
}

	/*inline void to_json(nlohmann::json& j, const uuid& p) 
	{
		j = nlohmann::json{ {"uuid", p.str()}};
	}

	inline void from_json(const nlohmann::json& j, uuid& p) 
	{
		auto s = std::string{};
		j.at("uuid").get_to(s);
		p = uuid(s);
	}*/


namespace toy::editor
{
	

	struct LoaderVersion
	{
		core::u8 major{};
		core::u8 minor{};
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LoaderVersion, major, minor)

	struct ObjectReference
	{
		std::string relativePath{};
		std::string objectName{};
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ObjectReference, relativePath, objectName)

	struct TextureAsset
	{
		core::u8 mipLevels{};
		core::u32 width{};
		core::u32 height{};
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextureAsset, mipLevels, width, height)

	struct ObjectCoreEntry
	{
		LoaderVersion version{};
		std::string type{};
		std::string name{};
		uuid uuid{};
		nlohmann::json object{};
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ObjectCoreEntry, type, name, uuid, object)

	struct CoreFile
	{
		LoaderVersion version{};
		std::vector<ObjectCoreEntry> objects{};
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CoreFile, version, objects)

	struct AssetEntry
	{
		std::string type{};
		std::string name{};
		std::string uuid{};
		std::string coreFilePath{};

		bool operator==(const AssetEntry& other) const
		{
			return type == other.type && name == other.name && uuid == other.uuid && coreFilePath == other.coreFilePath;
		}
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AssetEntry, type, name, uuid, coreFilePath)

	struct AssetDatabaseEntries
	{
		std::vector<AssetEntry> entries{};
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AssetDatabaseEntries, entries)

	struct AssetDatabase
	{
		inline AssetDatabase(const std::string_view projectRootPath) : 
			projectRootPath{ std::filesystem::path{projectRootPath} },
			cacheFilePath{ std::filesystem::path{projectRootPath} / cacheFileName } {}

		
		AssetDatabaseEntries db{};
		std::filesystem::path projectRootPath;
		const std::string cacheFileName{ "assets.db.cache" };
		std::filesystem::path cacheFilePath;

		bool hasCache();
		void rebuildCache();
		void flush();
		void loadCachedData();
		void addEntry(const AssetEntry& entry);
		void removeEntry(const AssetEntry& entry);
	};

	struct CreateTexture2DAssetInfo
	{
		std::string name;
		std::filesystem::path sourceImagePath;
	};

	/*auto createTexture2DAsset(const CreateTexture2DAssetInfo& info)
	{

	}*/

	auto doAssetStuff(std::string_view directoryPath) -> void;
}