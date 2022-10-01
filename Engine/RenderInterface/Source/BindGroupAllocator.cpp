#include "BindGroupAllocator.h"

#include <Hash.h>

using namespace toy::renderer;

void BindGroupAllocator::initialize(Device& device, const BindGroupAllocatorDescriptor& descriptor)
{
	device_ = &device;
	swapchainFrameCount_ = descriptor.swapchainFrameCount;

	//resizing cause compiling error, because of unique_ptr constructor
	//bindGroupCache_.resize(descriptor.swapchainFrameCount);
}

void BindGroupAllocator::nextFrame()
{
	frameIndex_ = (frameIndex_ + 1) % swapchainFrameCount_;
}

std::unique_ptr<BindGroup> BindGroupAllocator::allocateBindGroup(const BindGroupLayout& layout)
{
	return allocateBindGroupInternal(layout);
}

std::unique_ptr<BindGroup> BindGroupAllocator::allocateBindGroup(const BindGroupDescriptor& descriptor)
{
	const auto layout = allocateBindGroupLayout(descriptor);
	return allocateBindGroup(*layout);
}

BindGroupLayout* BindGroupAllocator::allocateBindGroupLayout(const BindGroupDescriptor& descriptor)
{
	const auto hash = core::Hasher::hash(descriptor);

	if (!bindGroupLayoutCache_.contains(hash))
	{
		bindGroupLayoutCache_[hash] = allocateBindGroupLayoutInternal(descriptor);
	}

	return bindGroupLayoutCache_[hash].get();
}
