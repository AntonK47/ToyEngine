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
        void pollEventsInternal() override;
        void initializeInternal(const WindowDescriptor& descriptor) override;
        void deinitializeInternal() override;
        void resizeInternal(core::u32 width, core::u32 height) override;

        void resetPolledEventsAndIo();
    protected:
        void setWindowTitleInternal(const std::string& title) override;
    private:
        SDL_Window* window_ = nullptr;
        std::vector<Event> currentPolledEvents_{};//TODO: smallvector
        io::WindowIo windowIo_{};
    };
}
