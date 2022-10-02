#pragma once

#define SDL_MAIN_HANDLED
#include <Window.h>
#include <SDL2/SDL.h>

namespace toy::window
{
    
    class SDLWindow final : public Window
    {
    public:

        void pollEvents() override;

        void resize(core::u32 width, core::u32 height) override;
        void initializeInternal(const WindowDescriptor& descriptor) override;
        void deinitializeInternal() override;

    private:

        void resetPolledEventsAndIo();

    public:
        [[nodiscard]] io::WindowIo getIO() override
        {
            return windowIo_;
        }

        [[nodiscard]] std::vector<Event> getEvents() override
        {
            return currentPolledEvents_;
        }

        ~SDLWindow() override {}

    private:
        SDL_Window* window_;
        std::vector<Event> currentPolledEvents_;
        io::WindowIo windowIo_{};
    };
}
