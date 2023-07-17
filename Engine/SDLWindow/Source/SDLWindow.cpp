#include "SDLWindow.h"
#define SDL_MAIN_HANDLED

#include <Core.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_syswm.h>
#include <Logger.h>
#include <filesystem>
#include <set>
#include <WindowIO.h>

using namespace toy::window;
using namespace toy::core;



#ifdef WIN32

//NOTE: https://github.com/ocornut/imgui/issues/2602
class DropManager : public IDropTarget
{
    auto QueryInterface(
        REFIID riid,
        void** ppvObject) -> HRESULT override
    {
        if (riid == IID_IDropTarget)
        {
            *ppvObject = this;
            return S_OK;
        }

        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    auto AddRef(void) -> ULONG override
    {
        return 1;
    }

    auto Release(void) -> ULONG override
    {
        return 0;
    }

    auto DragEnter(
        IDataObject* pDataObj,
        DWORD grfKeyState,
        POINTL pt,
        DWORD* pdwEffect) -> HRESULT override
    {
        FORMATETC format = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM medium;

        draggedFiles.clear();
        isDragAccepted = true;

        if (SUCCEEDED(pDataObj->GetData(&format, &medium)))
        {
            HDROP drop = (HDROP)medium.hGlobal; // or reinterpret_cast<HDROP> if preferred
            UINT file_count = DragQueryFile(drop, 0xFFFFFFFF, NULL, 0);

            auto paths = std::vector<std::string>{};

            // we can drag more than one file at the same time, so we have to loop here
            for (UINT i = 0; i < file_count; i++)
            {
                TCHAR szFile[MAX_PATH];
                UINT cch = DragQueryFile(drop, i, szFile, MAX_PATH);
                if (cch > 0 && cch < MAX_PATH)
                {
                    const auto path = std::filesystem::path{ std::format("{}", szFile) };
                    const auto extension = path.extension().generic_string();
                    
                    if (!registeredExtensions.contains(extension))
                    {
                        LOG(INFO) << std::format("File extention is not supported by application: {}", path.generic_string());
                        isDragAccepted = false;
                    }
                    else
                    {
                        draggedFiles.push_back(path);
                    }
                }


            }

            ReleaseStgMedium(&medium);

        }

        if (!isDragAccepted)
        {
            *pdwEffect &= DROPEFFECT_NONE;
        }
        else
        {
            *pdwEffect &= DROPEFFECT_COPY;
            
        }
        events.push_back(toy::io::DragDropEvent::dragBegin);
        return S_OK;
    }

    auto DragOver(
        DWORD grfKeyState,
        POINTL pt,
        DWORD* pdwEffect) -> HRESULT override
    {

        POINT p{};
        p.x = pt.x;
        p.y = pt.y;
        ScreenToClient(windowOwner, &p);
        mouseX = p.x;
        mouseY = p.y;

        if (!isDragAccepted)
        {
            *pdwEffect &= DROPEFFECT_NONE;
        }
        else
        {
            *pdwEffect &= DROPEFFECT_COPY;
        }

        return S_OK;
    }

    auto DragLeave(void) -> HRESULT override
    {
        events.push_back(toy::io::DragDropEvent::dragLeave);
        return S_OK;
    }

    auto Drop(
        IDataObject* pDataObj,
        DWORD grfKeyState,
        POINTL pt,
        DWORD* pdwEffect) -> HRESULT  override
    {
        
        events.push_back(toy::io::DragDropEvent::dragEnd);
        *pdwEffect &= DROPEFFECT_COPY;
        
        //TODO: window should become active
        return S_OK;
    }

    


    bool isDragAccepted{ false };
    bool isDragging{ false };
    friend class SDLWindow;

    std::vector<toy::io::DragDropEvent> events;
    std::set<std::string> registeredExtensions;
    std::vector<std::filesystem::path> draggedFiles;
    
    u32 mouseX;
    u32 mouseY;

    HWND windowOwner;
};

namespace
{
    DropManager dropManager;

    SDL_HitTestResult dragAreaTest(SDL_Window* win, const SDL_Point* area, void* data)
    {
        auto region = (toy::core::Rectangle*)data;

        if (area->x > region->offset.x && area->x < (region->offset.x + region->size.width) &&
            area->y > region->offset.y && area->y < (region->offset.y + region->size.height))
        {
            return SDL_HITTEST_DRAGGABLE;
        }
        return SDL_HITTEST_NORMAL;
    }
}
#endif

toy::io::DragDropEvent SDLWindow::pollDragDropEvent()
{
    auto lastEvent = toy::io::DragDropEvent::none;
    for (auto e : dropManager.events)
    {
        if (lastEvent == toy::io::DragDropEvent::dragEnd)
        {
            continue;
        }

        lastEvent = e;
    }
    dropManager.events.clear();

    if (lastEvent == toy::io::DragDropEvent::dragBegin)
    {
        dropManager.isDragging = true;
    }

    if (lastEvent == toy::io::DragDropEvent::dragEnd)
    {
        dropManager.isDragging = false;
    }

    if (dropManager.isDragging)
    {
        windowIo_.mouseState.position.x = dropManager.mouseX;
        windowIo_.mouseState.position.y = dropManager.mouseY;
    }
    
    return lastEvent;
}

std::vector<std::filesystem::path> SDLWindow::getDraggedFilePathsInternal()
{
    return dropManager.draggedFiles;
}

void SDLWindow::registerExternalDragExtensionInternal(const std::string& extension)
{
    if (!dropManager.registeredExtensions.contains(extension))
    {
        dropManager.registeredExtensions.insert(extension);
    }
}

float SDLWindow::getDiagonalDpiScaleInternal()
{
    float ddpi;
    const auto result = SDL_GetDisplayDPI(0, &ddpi, nullptr, nullptr);
    TOY_ASSERT(result == 0);
    return ddpi;
}

void SDLWindow::initializeInternal(const WindowDescriptor& descriptor)
{
#ifdef WIN32
    OleInitialize(NULL);
    SetProcessDPIAware();//This ignores display scale setting on windows
#endif
    {
        auto result = SDL_Init(SDL_INIT_VIDEO);
        if (result)
        {
            LOG(FATAL) << SDL_GetError();
        }
#ifdef TOY_ENGINE_VULKAN_BACKEND
        window_ = SDL_CreateWindow("Vulkan Window",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            static_cast<int>(width_),
            static_cast<int>(height_),
            SDL_WINDOW_VULKAN |
            SDL_WINDOW_RESIZABLE |
            SDL_WINDOW_BORDERLESS |
            SDL_WINDOW_HIDDEN |
            SDL_WINDOW_ALLOW_HIGHDPI);

        

        auto extensionsCount = u32{};
        result = SDL_Vulkan_GetInstanceExtensions(window_, &extensionsCount, nullptr);
        if (result == SDL_FALSE)
        {
            LOG(FATAL) << SDL_GetError();
        }
        auto extensions = std::vector<const char*>(extensionsCount);
        result = SDL_Vulkan_GetInstanceExtensions(window_, &extensionsCount, extensions.data());
        if (result == SDL_FALSE)
        {
            LOG(FATAL) << SDL_GetError();
        }

        meta_.requiredExtensions = extensions;
        //SDL_ShowCursor(SDL_DISABLE);
#endif

#ifdef WIN32
        SDL_SysWMinfo info{};
        SDL_VERSION(&info.version);
        SDL_GetWindowWMInfo(window_, &info);

        handler_.hwnd = info.info.win.window;
        handler_.hinstance = info.info.win.hinstance;
        
        {
            const auto result = RegisterDragDrop(handler_.hwnd, &dropManager);
            TOY_ASSERT(result == S_OK);
        }

        dropManager.windowOwner = handler_.hwnd;
#endif

        currentPolledEvents_ = std::vector<Event>{};//TODO: smallvector

        windowIo_.keyboardState.reset();
		windowIo_.mouseState.reset();
        windowIo_.textState.reset();

        result = SDL_SetWindowHitTest(window_, dragAreaTest, &this->draggableRegion_);
        TOY_ASSERT(result == S_OK);
    }
}


void SDLWindow::resetPolledEventsAndIo()
{
    if (windowIo_.keyboardState.b == toy::io::ButtonState::pressed)
    {
        SDL_SetWindowBordered(window_, SDL_TRUE);
    }
    if (windowIo_.keyboardState.n == toy::io::ButtonState::pressed)
    {
        SDL_SetWindowBordered(window_, SDL_FALSE);
    }
    currentPolledEvents_.clear();
    windowIo_.textState.reset();

    
    /*windowIo_.keyboardState.reset();
    windowIo_.mouseState.reset();*/
}

void SDLWindow::setWindowTitleInternal(const std::string& title)
{
    SDL_SetWindowTitle(window_, title.c_str());
}

toy::io::WindowIo SDLWindow::getIoInternal()
{
	return windowIo_;
}

std::vector<Event> SDLWindow::getEventsInternal()//TODO: smallvector
{
	return currentPolledEvents_;
}

void SDLWindow::deinitializeInternal()
{
#ifdef WIN32
    RevokeDragDrop(handler_.hwnd);
#endif

    SDL_DestroyWindow(window_);
    SDL_Quit();
#ifdef WIN32
    OleUninitialize();
#endif
    
}

void SDLWindow::resizeInternal(core::u32 width, core::u32 height)
{
    newWidth_ = width;
    newHeight_ = height;
    shouldApplyNewSizeOnNextFrame_ = true;
}

void SDLWindow::hideInternal()
{
    SDL_HideWindow(window_);
}

void SDLWindow::showInternal()
{
    SDL_ShowWindow(window_);
}

void SDLWindow::enableBorderInternal()
{
    SDL_SetWindowBordered(window_, SDL_TRUE);
}

void SDLWindow::disableBorderInternal()
{
    SDL_SetWindowBordered(window_, SDL_FALSE);
}

void SDLWindow::setWindowDraggingRegionInternal(const core::Rectangle& region)
{
    TOY_ASSERT(region.offset.x >= 0);
    TOY_ASSERT(region.offset.x >= 0);
    TOY_ASSERT(region.size.width >= 0);
    TOY_ASSERT(region.size.height >= 0);
    draggableRegion_ = region;
}