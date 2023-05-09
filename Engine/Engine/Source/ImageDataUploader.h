#pragma once
#include <RenderInterface.h>
#include <RenderInterfaceTypes.h>
#include "Texture2D.h"
using namespace toy::graphics::rhi;

namespace toy
{

	struct ImageDataUploader
	{
		auto initialize(RenderInterface& rhi, const core::u32 stagingBufferSize) -> void
		{
			rhi_ = &rhi;
			stagingSize = stagingBufferSize;

			const auto stagingDescriptor = BufferDescriptor
			{
				.size = stagingSize,
				.accessUsage = BufferAccessUsage::transferSrc,
				.memoryUsage = MemoryUsage::cpuOnly
			};

			stagingBuffer = rhi_->createBuffer(stagingDescriptor, DebugLabel{ "Upload Staging Buffer" });
			rhi_->map(stagingBuffer, &stagingBufferDataPtr);
		}

		auto deinitialize() -> void
		{
			rhi_->unmap(stagingBuffer);
			rhi_->destroyBuffer(stagingBuffer);
		}


		auto upload(const std::vector<std::byte>& srcData, const Texture2D& dstTexture)
		{

			auto width = dstTexture.width;
			auto height = dstTexture.height;

			const auto maxMips = static_cast<u32>(std::ceilf(std::log2f(std::max(dstTexture.width, dstTexture.height))) + 1);
			const auto mips = dstTexture.hasMips ? maxMips : u32{ 1 };
			auto uploadRegions = std::vector<Region>{};
			uploadRegions.resize(mips);

			auto totalImageSize = u32{};
			for (auto i = u32{}; i < mips; i++)
			{
				uploadRegions[i] = Region{ i, 0, 1, glm::uvec3{width, height, 1 },{}, totalImageSize };
				totalImageSize += std::max(u32{ 16 }, ((width) * (height)*dstTexture.bytesPerTexel));


				width >>= 1;
				height >>= 1;
			}

			TOY_ASSERT(totalImageSize <= stagingSize);
			std::memcpy(stagingBufferDataPtr, srcData.data(), totalImageSize);


			auto uploadCommandList = rhi_->acquireCommandList(QueueType::transfer, UsageScope::async);

			uploadCommandList.begin();
			uploadCommandList.barrier({ ImageBarrierDescriptor
				{
					.srcLayout = Layout::undefined,
					.dstLayout = Layout::transferDst,
					.srcStage = ResourcePipelineStageUsageFlagBits::none,
					.dstStage = ResourcePipelineStageUsageFlagBits::none,
					.aspect = ImageViewAspect::color,
					.image = dstTexture.image
				} });

			uploadCommandList.transfer(
				SourceBufferDescriptor
				{
					.buffer = stagingBuffer,
					.offset = 0
				},
				DestinationImageDescriptor
				{
					.image = dstTexture.image,
					.regions = uploadRegions
				});

			uploadCommandList.barrier({ ImageBarrierDescriptor
				{
					.srcLayout = Layout::transferDst,
					.dstLayout = Layout::shaderRead,
					.srcStage = ResourcePipelineStageUsageFlagBits::none,
					.dstStage = ResourcePipelineStageUsageFlagBits::fragment,
					.aspect = ImageViewAspect::color,
					.image = dstTexture.image
				} });
			uploadCommandList.end();

			const auto submit = rhi_->submitCommandList(QueueType::transfer, { uploadCommandList }, {});
			rhi_->submitBatches(QueueType::transfer, { submit });
		}

	private:
		void* stagingBufferDataPtr;
		Handle<Buffer> stagingBuffer;
		u32 stagingSize;
		RenderInterface* rhi_;
	};
}