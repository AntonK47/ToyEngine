#pragma once

#include <Common.h>

namespace toy::renderer::debugger
{
	class RenderDocCapture;

	namespace vk
	{
		class Instance;
	}

	struct RenderDocCaptureDescriptor
	{
		NativeBackend nativeBackend;

	};

	class ScopeCapture;

	class ScopeCapture
	{
		friend RenderDocCapture;
	public:
		void captureNextMarkedScope()
		{
			shouldCapture_ = true;
		}

		void start();
		void stopAndOpenCapture();
		[[nodiscard]]bool isRenderDocInjected() const;
	private:

		explicit ScopeCapture(RenderDocCapture* capture) : capture_{ capture } {}

		bool shouldCapture_{ false };
		RenderDocCapture* capture_;
	};

	class RenderDocCapture
	{
		friend ScopeCapture;
	public:
		void initialize(const RenderDocCaptureDescriptor& descriptor);
		void deinitialize();

		[[nodiscard]] ScopeCapture getScopeCapture()
		{
			return ScopeCapture(this);
		}
		
	private:

		void startCapture();
		void stopCapture();

		void launchRenderDocApplication();
		NativeBackend nativeBackend_{};
		bool hasInitialized_{ false };
	};

	
}


