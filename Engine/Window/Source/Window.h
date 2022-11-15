#pragma once

#include <vector>

#include "Common.h"
#include "WindowIO.h"
#include "folly/small_vector.h"

namespace toy::window
{
    struct WindowHandler
    {
#ifdef WIN32
        HWND hwnd;
        HINSTANCE hinstance;
#endif
    };

    struct BackendRendererMeta
    {
#ifdef TOY_ENGINE_VULKAN_BACKEND
        std::vector<const char*> requiredExtensions;
#endif
    };

    enum class Event
    {
        quit
    };

    struct WindowDescriptor
    {
        core::u32 width;
        core::u32 height;
    };

    class Window
    {
    public:
        virtual ~Window() = default;

        void pollEvents();
        [[nodiscard]] folly::small_vector<Event> getEvents();
        [[nodiscard]] io::WindowIo getIo();
        void resize(core::u32 width, core::u32 height);

        void setWindowTitle(const std::string& title)
        {
            setWindowTitleInternal(title);
        }

        [[nodiscard]] WindowHandler getWindowHandler() const
        {
            return handler_;
        }

        [[nodiscard]] BackendRendererMeta getRendererMeta() const
        {
            return meta_;
        }

        [[nodiscard]] core::u32 width() const { return width_; }
        [[nodiscard]] core::u32 height() const { return height_; }


        void initialize(const WindowDescriptor& descriptor);
        void deinitialize();

    protected:
        virtual void initializeInternal(const WindowDescriptor& descriptor) = 0;
        virtual void deinitializeInternal() = 0;

        virtual void pollEventsInternal() = 0;
        virtual [[nodiscard]] folly::small_vector<Event> getEventsInternal() = 0;
        virtual [[nodiscard]] io::WindowIo getIoInternal() = 0;
        virtual void resizeInternal(core::u32 width, core::u32 height) = 0;
        virtual void setWindowTitleInternal(const std::string& title) = 0;
        WindowHandler handler_{};
        BackendRendererMeta meta_{};
        core::u32 width_{};
        core::u32 height_{};
    };
}
