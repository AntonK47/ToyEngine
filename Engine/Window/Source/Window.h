#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include <Core.h>
#include "WindowIO.h"

namespace toy::window
{
    enum class Event
    {
        quit,
        resize
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
        [[nodiscard]] std::vector<Event> getEvents(); //TODO: smallvector
        [[nodiscard]] io::WindowIo getIo();
        void resize(core::u32 width, core::u32 height)
        {
            resizeInternal(width, height);
        }

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

        std::vector<std::filesystem::path> getDraggedFilePaths()
        {
            return getDraggedFilePathsInternal();
        }

        void registerExternalDragExtension(const std::string& extension)
        {
            registerExternalDragExtensionInternal(extension);
        }

        void hide()
        {
            hideInternal();
        }
        void show()
        {
            showInternal();
        }

        void enableBorder()
        {
            enableBorderInternal();
        }
        void disableBorder()
        {
            disableBorderInternal();
        }

        [[nodiscard]] float getDiagonalDpiScale()
        {
            return getDiagonalDpiScaleInternal();
        }

    protected:
        virtual void initializeInternal(const WindowDescriptor& descriptor) = 0;
        virtual void deinitializeInternal() = 0;
        
        virtual void hideInternal() = 0;
        virtual void showInternal() = 0;

        virtual void enableBorderInternal() = 0;
        virtual void disableBorderInternal() = 0;

        virtual float getDiagonalDpiScaleInternal() = 0;

        virtual void pollEventsInternal() = 0;
        virtual [[nodiscard]] std::vector<Event> getEventsInternal() = 0;//TODO: smallvector
        virtual [[nodiscard]] io::WindowIo getIoInternal() = 0;
        virtual void resizeInternal(core::u32 width, core::u32 height) = 0;
        virtual void setWindowTitleInternal(const std::string& title) = 0;
        virtual [[nodiscard]] std::vector<std::filesystem::path> getDraggedFilePathsInternal() = 0;
        virtual void registerExternalDragExtensionInternal(const std::string& extension) = 0;
        WindowHandler handler_{};
        BackendRendererMeta meta_{};
        core::u32 width_{};
        core::u32 height_{};
    };
}
