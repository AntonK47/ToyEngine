#include "AssetDatabase.h"

using namespace toy::editor;

bool AssetDatabase::hasCache()
{
	if (std::filesystem::is_directory(projectRootPath))
	{
		auto found = std::find_if(projectRootPath.begin(), projectRootPath.end(),
			[&](const std::filesystem::path& path)
			{
				if (path.has_filename())
				{
					return path.filename() == cacheFileName;

				}
				return false;
			});

		return found != projectRootPath.end();
	}
	return false;
}

void AssetDatabase::rebuildCache()
{
	namespace fs = std::filesystem;
	using namespace nlohmann;
	TOY_ASSERT(is_directory(projectRootPath));

	auto coreFiles = std::vector<fs::path>{};

	//TODO: we can report scanning status
	auto totalCoreFiles = std::count_if(fs::recursive_directory_iterator(projectRootPath),
		fs::recursive_directory_iterator{}, [](auto& path) { return std::filesystem::is_regular_file(path); });

	for (const auto& entry : fs::recursive_directory_iterator(projectRootPath))
	{
		if (entry.is_regular_file())
		{
			auto p = entry.path();

			if (p.has_extension())
			{
				if (p.extension() == ".core")
				{
					coreFiles.push_back(p);
				}
			}
		}
	}

	for (const auto& path : coreFiles)
	{
		std::cout << path << std::endl;
		auto stream = std::ifstream{ path };


		try
		{
			const auto coreFile = json::parse(stream);
			auto coreFileData = coreFile.template get<CoreFile>();

			for (const auto& object : coreFileData.objects)
			{
				auto entry = AssetEntry{};
				entry.name = object.name;
				entry.type = object.type;
				entry.uuid = object.uuid;
				entry.coreFilePath = path.string();
				addEntry(entry);
			}

		}
		catch (json::type_error& e)
		{
			std::cout << e.what() << '\n';
		}
	}
}

void AssetDatabase::flush()
{
	using namespace nlohmann;
	auto stream = std::ofstream{ cacheFilePath };
	auto j = json{ db.entries };
	stream << std::setw(4) << j << std::endl;
}

void AssetDatabase::loadCachedData()
{
	using namespace nlohmann;
	auto stream = std::ifstream{ cacheFilePath };
	auto j = json::parse(stream);
	db = j.template get<AssetDatabaseEntries>();
}

void AssetDatabase::addEntry(const AssetEntry& entry)
{
	db.entries.push_back(entry);
}

void AssetDatabase::removeEntry(const AssetEntry& entry)
{
	auto found = std::find(db.entries.begin(), db.entries.end(), entry);

	if (found != db.entries.end())
	{
		db.entries.erase(found);
	}

}

auto toy::editor::doAssetStuff(std::string_view directoryPath) -> void
{
	namespace fs = std::filesystem;

	auto dir = fs::path{ directoryPath };

	TOY_ASSERT(is_directory(dir));

	auto coreFiles = std::vector<fs::path>{};

	for (const auto& entry : fs::directory_iterator(dir))
	{
		if (entry.is_regular_file())
		{
			auto p = entry.path();

			if (p.has_extension())
			{
				if (p.extension() == ".core")
				{
					coreFiles.push_back(p);
				}
			}
		}
	}

	using namespace nlohmann;

	auto gameObjects = std::vector<ObjectCoreEntry>{};

	json::parser_callback_t cb = [](int depth, json::parse_event_t event, json& parsed)
		{
			if (event == json::parse_event_t::key and parsed == json("object"))
			{
				return false;
			}
			else
			{
				return true;
			}
		};

	for (const auto& p : coreFiles)
	{
		std::cout << p << std::endl;
		auto stream = std::ifstream{ p };

		json kk;

		try
		{
			kk = json::parse(stream);
			std::string s = kk.dump();
			std::cout << s << '\n';
		}
		catch (json::type_error& e)
		{
			std::cout << e.what() << '\n';
		}

		auto pp = kk.template get<CoreFile>();
	}

	auto guid = xg::newGuid();

	struct Version
	{
		int major;
		int minor;
	};

	auto a = TextureAsset
	{
		.mipLevels = 2,
		.width = 2,
		.height = 2
	};



	auto jj = json(a);

	LOG(INFO) << jj.dump();
}
