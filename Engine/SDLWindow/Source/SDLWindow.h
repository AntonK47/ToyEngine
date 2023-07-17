#pragma once

#include <Window.h>
#include <SDL2/SDL.h>
#include <vector>
struct SDL_Window;

namespace toy::window
{
    
    class SDLWindow final : public Window
    {
    public:

        ~SDLWindow() override {}
    private:
        [[nodiscard]] io::WindowIo getIoInternal() override;
        [[nodiscard]] std::vector<Event> getEventsInternal() override;//TODO: smallvector
        [[nodiscard]] std::vector<std::filesystem::path> getDraggedFilePathsInternal() override;
        void registerExternalDragExtensionInternal(const std::string& extension) override;
        void pollEventsInternal() override;
        void initializeInternal(const WindowDescriptor& descriptor) override;
        void deinitializeInternal() override;
        void resizeInternal(core::u32 width, core::u32 height) override;

        void setWindowDraggingRegionInternal(const core::Rectangle& region) override;

        void hideInternal() override;
        void showInternal() override;
        bool isWindowVisibleInternal() override;

        void enableBorderInternal() override;
        void disableBorderInternal() override;

        float getDiagonalDpiScaleInternal() override;

        void resetPolledEventsAndIo();
        io::DragDropEvent pollDragDropEvent();
    protected:
        void setWindowTitleInternal(const std::string& title) override;
    private:
        SDL_Window* window_ = nullptr;
        std::vector<Event> currentPolledEvents_{};//TODO: smallvector
        io::WindowIo windowIo_{};
        bool shouldApplyNewSizeOnNextFrame_{ false };
        core::u32 newWidth_;
        core::u32 newHeight_;
        core::Rectangle draggableRegion_ = core::Rectangle{ 0,0,0,0 };
        bool isVisible_{ true };
    };
}
