#include "RenderInterface.h"
#include <Hash.h>

using namespace toy::renderer;

namespace toy::renderer
{
	BindGroup RenderInterface::allocateBindGroup(const BindGroupDescriptor& descriptor, const BindGroupLayout& layout)
	{
		return {};
	}

	BindGroup RenderInterface::allocateBindGroup(const BindGroupDescriptor& descriptor)
	{
		return {};
	}

	BindGroupLayout RenderInterface::allocateBindGroupLayout(const BindGroupDescriptor& descriptor)
	{

		const auto hash = Hasher::hash(descriptor);

		if(bindGroupLayoutCache_.contains(hash))
		{
			return bindGroupLayoutCache_[hash];
		}

		const auto groupLayout = allocateBindGroupLayoutInternal(descriptor);
		bindGroupLayoutCache_[hash] = groupLayout;

		return groupLayout;
	}
}
