#include "Window.h"

using namespace toy::window;

void Window::pollEvents()
{
    pollEventsInternal();
}

folly::small_vector<Event> Window::getEvents()
{
    return getEventsInternal();
}

toy::io::WindowIo Window::getIo()
{
    return getIoInternal();
}

void Window::deinitialize()
{
    deinitializeInternal();
}

void Window::initialize(const WindowDescriptor& descriptor)
{
    width_ = descriptor.width;
    height_ = descriptor.height;

    initializeInternal(descriptor);
}
