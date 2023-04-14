#pragma once

#include <string>
#include <Core.h>
#include <variant>
#include <span>
#include <fstream>
#include <ValidationCommon.h>


namespace toy::io::loaders::dds
{
	enum class TextureFormat
	{
		bc7,
		bgra8888unorm
	};

	struct Texture1DDimensionInfo
	{
		core::u32 width;
	};
	struct Texture2DDimensionInfo
	{
		core::u32 width;
		core::u32 height;
	};
	struct Texture3DDimensionInfo
	{
		core::u32 width;
		core::u32 height;
		core::u32 depth;
	};
	struct TextureCubeDimensionInfo
	{
		core::u32 width;
		core::u32 height;
	};

	using TextureDimensionInfo = std::variant
		<
		Texture1DDimensionInfo,
		Texture2DDimensionInfo,
		Texture3DDimensionInfo,
		TextureCubeDimensionInfo
		>;

	struct TextureInfo
	{
		core::u32 arrayCount;
		core::u32 mipCount;
		TextureDimensionInfo dimensionInfo;
		TextureFormat format;
	};

	using TextureData = std::span<std::byte>;


	std::fstream& operator>>(std::fstream& stream, TextureInfo& info);

	TextureInfo readTextureInfo(std::fstream& file);

	struct FirstMip{};
	struct LastMip{};

	template<class T>
	concept TextureDataSourceDescriptor = requires(T t, const core::u32 dataOffset, std::span<std::byte>destination)
	{
		{t.getTextureInfo()} -> std::convertible_to<TextureInfo>;
		{t.copyDataTo(dataOffset, destination)};
	};

	struct FilestreamTextureDataSourceDescriptor final
	{
	public:
		explicit FilestreamTextureDataSourceDescriptor(const std::string& fileName)
		{
			file_.open( fileName, std::fstream::in | std::fstream::binary);
			file_ >> info_;
			dataBlockOffset_ = file_.tellg();
		}
		~FilestreamTextureDataSourceDescriptor()
		{
			file_.close();
		}
		
		inline auto getTextureInfo() const -> TextureInfo
		{
			return info_;
		}

		inline void copyDataTo(const core::u32 dataOffset, std::span<std::byte> destination)
		{
			const auto newPosition = dataBlockOffset_ + static_cast<std::streamoff>(dataOffset);

			file_.seekg(newPosition);
			file_.read((char*)destination.data(), destination.size_bytes());
		}

	private:
		std::fstream file_;
		TextureInfo info_;
		std::streampos dataBlockOffset_;
	};

	static_assert(TextureDataSourceDescriptor<FilestreamTextureDataSourceDescriptor>);


	inline bool hasCompressedFormat(const TextureFormat format)
	{
		auto hasCompression = false;
		switch (format)
		{
		case TextureFormat::bc7:
			hasCompression = true;
			break;
		case TextureFormat::bgra8888unorm:
			hasCompression = false;
			break;
		default:
			break;
		}
		return hasCompression;
	}

	inline core::u32 bytesPerTexel(const TextureFormat format)
	{
		auto bytesPerTexel = core::u32{};

		switch (format)
		{
		case TextureFormat::bgra8888unorm:

			bytesPerTexel = 4;
			break;
		case TextureFormat::bc7:
			bytesPerTexel = 1;
			break;
		default:
			break;
		}

		return bytesPerTexel;
	}

	inline core::u32 lodSize(const core::u32 width, const core::u32 height, bool hasCompression)
	{
		const auto w = core::u32{ ((width + 3) / 4) };
		const auto h = core::u32{ ((height + 3) / 4) };
		return hasCompression ?
		
			std::max(core::u32{ 1 }, w) *
			std::max(core::u32{ 1 }, h) *
			 core::u32{ 16 } : width * height;
	}

	inline core::u32 lodSizeInBytes(const TextureInfo& info, const core::u32 lodLevel)
	{
		const auto hasCompression = hasCompressedFormat(info.format);
		const auto bytes = bytesPerTexel(info.format);
		TOY_ASSERT(std::holds_alternative<Texture2DDimensionInfo>(info.dimensionInfo));
		const auto& dimension = std::get<Texture2DDimensionInfo>(info.dimensionInfo);
		const auto w = dimension.width >> lodLevel;
		const auto h = dimension.height >> lodLevel;
		const auto size = lodSize(w, h, hasCompression);
		return size * bytes;
	}

	inline void loadTexture2D(
		TextureDataSourceDescriptor auto& dataSource,
		std::span<std::byte> dataDestination,
		const std::variant<core::u8, FirstMip> first = FirstMip{},
		const std::variant<core::u8, LastMip> last = LastMip{})
	{
		auto firstMip = core::u8{};
		auto lastMip = core::u8{};

		const auto& info = dataSource.getTextureInfo();

		TOY_ASSERT(std::holds_alternative<Texture2DDimensionInfo>(info.dimensionInfo));

		const auto& dimension = std::get<Texture2DDimensionInfo>(info.dimensionInfo);

		TOY_ASSERT(dimension.width == dimension.height);
		TOY_ASSERT(std::has_single_bit(dimension.width));


		if (std::holds_alternative<core::u8>(first))
		{
			firstMip = std::get<core::u8>(first);
		}
		else
		{
			firstMip = 0;
		}

		if (std::holds_alternative<core::u8>(last))
		{
			lastMip = std::get<core::u8>(last);
		}
		else
		{
			lastMip = info.mipCount - 1;
		}

		TOY_ASSERT(firstMip < info.mipCount);
		TOY_ASSERT(lastMip < info.mipCount);

		auto offset = core::u32{};

		auto extent = std::max(dimension.width, dimension.height);

		const auto hasCompression = hasCompressedFormat(info.format);
		const auto bytes = bytesPerTexel(info.format);
		for (auto i = 0; i < firstMip; i++)
		{
			const auto mipSubTextureSize = extent >> i;
			const auto size = lodSize(mipSubTextureSize, mipSubTextureSize, hasCompression);

			offset += size * bytes;
		}

		auto dataSize = core::u32{};

		for (auto i = firstMip; i <= lastMip; i++)
		{
			const auto mipSubTextureSize = extent >> i;
			const auto size = lodSize(mipSubTextureSize, mipSubTextureSize, hasCompression);
			dataSize += size * bytes;
		}
		dataSource.copyDataTo(offset, dataDestination);
	}
}