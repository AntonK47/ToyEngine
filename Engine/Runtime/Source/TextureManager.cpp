#include "TextureManager.h"
#include <DDSLoader.h>
#include <ImageDataUploader.h>
using namespace toy::io::loaders::dds;

using namespace toy::graphics::rhi;
using namespace toy::core;

UID toy::helper::loadTexture(const std::string path, RenderInterface& rhi, ImageDataUploader& textureUploader, TextureManager& textureManager)
{
	auto dataSource = FilestreamTextureDataSourceDescriptor{ path };

	const auto& info = dataSource.getTextureInfo();
	const auto& texture2dInfo = std::get<Texture2DDimensionInfo>(info.dimensionInfo);
	TOY_ASSERT(info.format == TextureFormat::bc7);

	Flags<ImageAccessUsage> accessUsage = ImageAccessUsage::sampled;
	accessUsage |= ImageAccessUsage::transferDst;

	const auto imageDescriptor = ImageDescriptor
	{
		.format = Format::bc7,
		.extent = Extent{ static_cast<u32>(texture2dInfo.width), static_cast<u32>(texture2dInfo.height)},
		.mips = info.mipCount,
		.layers = info.arrayCount,
		.accessUsage = accessUsage
	};

	auto image = rhi.createImage(imageDescriptor);

	const auto view = rhi.createImageView(ImageViewDescriptor
		{
			.image = image,
			.format = Format::bc7,
			.type = ImageViewType::_2D
		});

	const auto texture = toy::Texture2D
	{
		.width = (u32)texture2dInfo.width,
		.height = (u32)texture2dInfo.height,
		.image = image,
		.view = view,
		.hasMips = info.mipCount > 1,
		.bytesPerTexel = bytesPerTexel(info.format)
	};


	auto totalSize = u32{};
	for (auto i = u32{}; i < info.mipCount; i++)
	{
		totalSize += lodSizeInBytes(info, i);
	}

	auto data = std::vector<std::byte>{};
	data.resize(totalSize);
	auto dataSpan = std::span(data);
	loadTexture2D(dataSource, (TextureData)dataSpan);

	textureUploader.upload(data, texture);

	const auto uid = textureManager.addTexture(texture);
	return uid;
}

namespace toy
{
	auto TextureManager::initialize(const TextureManagerDescriptor& descriptor) -> void
	{
		rhi_ = &descriptor.rhi;
		framesInFlight_ = descriptor.frameInFlights;

		texture2DGroupPerFrame_.resize(framesInFlight_);

		auto bindlessFlags = Flags<BindGroupFlag>{ BindGroupFlag::none };
		bindlessFlags |= BindGroupFlag::unboundLast;

		const auto bindlessTextureGroup = BindGroupDescriptor
		{
			.bindings =
			{
				{
					.binding = 0,
					.descriptor = BindingDescriptor{BindingType::Texture2D, maxDescriptorsPerGroup_}
				}
			},
			.flags = BindGroupFlag::unboundLast
		};
		texture2DGroupLayout_ = rhi_->createBindGroupLayout(bindlessTextureGroup, DebugLabel{ .name = "bindlessTextures2D" });

		for (auto i = u32{}; i < framesInFlight_; i++)
		{
			const auto group = rhi_->allocateBindGroup(texture2DGroupLayout_, UsageScope::async, DebugLabel{ std::format("bindlesTextures2DBindGroup_{}", i).c_str() });
			texture2DGroupPerFrame_[i] = group;
		}
	}

	auto TextureManager::nextFrame() -> void
	{
		currentFrameIndex_ = (currentFrameIndex_ + 1) % framesInFlight_;
	}

	auto TextureManager::addTexture(const Texture2D& texture) -> UID
	{
		needUpdate_ = true;
		const auto uid = UIDGenerator::generate();

		textures_.push_back(texture);

		const auto index = static_cast<core::u32>(textures_.size() - 1);
		uidToIndexMap_.insert(std::make_pair(uid, index));
		return uid;
	}

	auto TextureManager::get(const UID uid) -> Texture2D&
	{
		TOY_ASSERT(uidToIndexMap_.contains(uid));
		return textures_[uidToIndexMap_.at(uid)];
	}

	auto TextureManager::getBindIndex(const UID uid) -> core::u32
	{
		TOY_ASSERT(uidToBindingMap_.contains(uid));
		return uidToBindingMap_.at(uid);
	}


	auto TextureManager::updateBindGroup() -> void
	{
		if (needUpdate_)
		{
			auto i = u32{};
			//TODO: it should be done carefully and update just non used bind groups
			for (auto& [uid, index] : uidToIndexMap_)
			{
				for (auto j = u32{}; j < framesInFlight_; j++)
				{
					rhi_->updateBindGroup(texture2DGroupPerFrame_[j], { BindingDataMapping{0, Texture2DSRV{textures_[i].view}, i} });

				}

				if (uidToBindingMap_.contains(uid))
				{
					uidToBindingMap_[uid] = i;

				}
				else
				{
					uidToBindingMap_.insert(std::make_pair(uid, i));
				}

				i++;
			}
		}
		needUpdate_ = false;
	}

	auto TextureManager::getTextureBindGroup() -> Handle<BindGroup>
	{
		return texture2DGroupPerFrame_[currentFrameIndex_];
	}

	auto TextureManager::getTextureBindGroupLayout() -> Handle<BindGroupLayout>
	{
		return texture2DGroupLayout_;
	}
}