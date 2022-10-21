#pragma once

#include <Window.h>
#include <SDL2/SDL.h>
struct SDL_Window;

namespace toy::window
{

    class SDLWindow final : public Window
    {
    public:

        ~SDLWindow() override {}
    private:
        [[nodiscard]] io::WindowIo getIoInternal() override;
        [[nodiscard]] std::vector<Event> getEventsInternal() override;
        void pollEventsInternal() override;
        void resizeInternal(core::u32 width, core::u32 height) override;
        void initializeInternal(const WindowDescriptor& descriptor) override;
        void deinitializeInternal() override;
    
        void resetPolledEventsAndIo();


        SDL_Window* window_ = nullptr;
        std::vector<Event> currentPolledEvents_{};
        io::WindowIo windowIo_{};
    };
}
