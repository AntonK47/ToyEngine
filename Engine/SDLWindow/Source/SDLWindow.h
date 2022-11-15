#pragma once

#include <Window.h>
#include <SDL2/SDL.h>
#include <folly/small_vector.h>
struct SDL_Window;

namespace toy::window
{

    class SDLWindow final : public Window
    {
    public:

        ~SDLWindow() override {}
    private:
        [[nodiscard]] io::WindowIo getIoInternal() override;
        [[nodiscard]] folly::small_vector<Event> getEventsInternal() override;
        void pollEventsInternal() override;
        void initializeInternal(const WindowDescriptor& descriptor) override;
        void deinitializeInternal() override;
        void resizeInternal(core::u32 width, core::u32 height) override;

        void resetPolledEventsAndIo();
    protected:
        void setWindowTitleInternal(const std::string& title) override;
    private:
        SDL_Window* window_ = nullptr;
        folly::small_vector<Event> currentPolledEvents_{};
        io::WindowIo windowIo_{};
    };
}
