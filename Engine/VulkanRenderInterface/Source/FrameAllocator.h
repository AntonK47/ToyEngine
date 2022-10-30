#pragma once

#include<vector>

namespace toy::renderer::api::vulkan
{

	struct FrameAllocator;
	struct FrameAllocator;

	struct LiniarAllocator
	{
		//... do liniar allocation stuff


		friend FrameAllocator;
	private:
		FrameAllocator* parent;
	};

	struct MemoryMapper
	{
		
	};

	struct FrameAllocator
	{
		void nextFrame();
		void flush();

		

	private:
		u32 size;
		u32 frames;

		std::vector<u8> data;
		Handle<Buffer> buffer;
		MemoryMapper mapper;
	};
}