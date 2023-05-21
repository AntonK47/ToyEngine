#include "DynamicFrameAllocator.h"
#include <mutex>
using namespace toy::graphics::rhi;

namespace
{
	std::mutex allocationMutex_;
}

namespace toy::graphics
{
	void DynamicFrameAllocator::initialize(const AllocatorDescriptor& descriptor)
	{
		rhi_ = &descriptor.rhi;
		size_ = descriptor.size;
		framesInFlight_ = descriptor.framesInFlight;
		memoryAlignment_ = 16; //TODO: request this value from rhi

		ringSegments_.resize(framesInFlight_);

		TOY_ASSERT(size_ % 16 == 0);

		auto usage = toy::core::Flags<BufferAccessUsage>{ BufferAccessUsage::storage };
		usage |= BufferAccessUsage::uniform;
		usage |= BufferAccessUsage::index;

		const auto totalBufferSize = size_; //TODO: round up to power of two

		buffer_ = rhi_->createBuffer(BufferDescriptor
			{
				.size = totalBufferSize,
				.accessUsage = usage,
				.memoryUsage = MemoryUsage::cpuOnly,
			});

		rhi_->map(buffer_, &dataPtr_);
	}
	auto DynamicFrameAllocator::deinitialize() -> void
	{
		rhi_->unmap(buffer_);
	}

	auto DynamicFrameAllocator::allocate(const core::u32 size) -> Allocation
	{
		const auto lock = std::lock_guard<std::mutex>{allocationMutex_};
		TOY_ASSERT(currentHead_ % memoryAlignment_ == 0);
		const auto alignedSize = size + memoryAlignment_ - size % memoryAlignment_;
		auto allocation = Allocation{};

		if (currentHead_ < currentTail_)
		{
			TOY_ASSERT(currentHead_ + alignedSize < currentTail_ && "Not enough memory!");
			auto data = (std::byte*)dataPtr_ + currentHead_;

			allocation.bufferView = rhi::BufferView{ buffer_, currentHead_, alignedSize };

			allocation.dataPtr = data;
			currentHead_ += alignedSize;
		}

		else
		{
			if ((currentHead_ + alignedSize) < size_)
			{
				auto data = (std::byte*)dataPtr_ + currentHead_;

				allocation.bufferView = rhi::BufferView{ buffer_, currentHead_, alignedSize };

				allocation.dataPtr = data;
				currentHead_ += alignedSize;
			}
			else
			{
				TOY_ASSERT((alignedSize < currentTail_) && "Out of memory!");
				auto data = (std::byte*)dataPtr_;

				allocation.bufferView = rhi::BufferView{ buffer_, 0, alignedSize };
				allocation.dataPtr = data;

				currentHead_ = alignedSize;
			}
		}
		return allocation;
	}
	auto DynamicFrameAllocator::getBuffer() -> rhi::Handle<rhi::Buffer>
	{
		return buffer_;
	}
	auto DynamicFrameAllocator::nextFrame() -> void
	{
		currentFrame_ = (currentFrame_ + 1) % framesInFlight_;
		currentTail_ = ringSegments_[(currentFrame_ + 1) % framesInFlight_].head;
		ringSegments_[currentFrame_].head = currentHead_;
	}
}