#pragma once
#include <utility>
#include <RenderInterface.h>

namespace toy::graphics
{
	struct AllocatorDescriptor
	{
		toy::graphics::rhi::RenderInterface& rhi;
		core::u32 size;
		core::u32 framesInFlight;
	};

	struct Allocation
	{
		rhi::BufferView bufferView;
		void* dataPtr;
	};

	class DynamicFrameAllocator final
	{
	public:
		auto initialize(const AllocatorDescriptor& descriptor) -> void;
		auto deinitialize() -> void;

		/*template <typename T, typename... ARGS>
		[[nodiscard]] auto create(ARGS&&... args) -> T*
		{
			return new(allocate(sizeof(T)) T(std::forward<ARGS>(args)...);
		}*/
		[[nodiscard]] auto allocate(const core::u32 size) -> Allocation;

		[[nodiscard]] auto getBuffer() -> rhi::Handle<rhi::Buffer>;

		auto nextFrame() -> void;

	private:
		rhi::RenderInterface* rhi_{ nullptr };
		rhi::Handle<rhi::Buffer> buffer_;
		
		void* dataPtr_{ nullptr };

		core::u32 currentHead_{};
		core::u32 currentTail_{};
		core::u32 memoryAlignment_{}; //TODO: alignment is dependent frome a usage type, e.x. storage buffer requre 16 byte and uniform buffer 64 byte alignment

		struct Segment
		{
			core::u32 head{0};
		};

		std::vector<Segment> ringSegments_;

		core::u32 size_;
		core::u32 framesInFlight_;
		core::u32 currentFrame_;
	};
}

