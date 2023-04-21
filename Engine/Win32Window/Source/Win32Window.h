#pragma once

#include <Window.h>
#include <vector>
#include <oleidl.h>
#include <format>
#include <iostream>
namespace toy::window
{

    //NOTE: https://github.com/ocornut/imgui/issues/2602
    class DropManager : public IDropTarget
    {
        /*DropManager(DropManager&) = delete;
        DropManager& operator=(DropManager&) = delete;*/

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
            *pdwEffect &= DROPEFFECT_COPY;
            return S_OK;
        }

        auto DragOver(
            DWORD grfKeyState,
            POINTL pt,
            DWORD* pdwEffect) -> HRESULT override
        {
            *pdwEffect &= DROPEFFECT_COPY;
            return S_OK;
        }

        auto DragLeave(void) -> HRESULT override
        {
            return S_OK;
        }

        auto Drop(
            IDataObject* pDataObj,
            DWORD grfKeyState,
            POINTL pt,
            DWORD* pdwEffect) -> HRESULT  override
        {
            FORMATETC fmte = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
            STGMEDIUM stgm;

            if (SUCCEEDED(pDataObj->GetData(&fmte, &stgm)))
            {
                HDROP hdrop = (HDROP)stgm.hGlobal; // or reinterpret_cast<HDROP> if preferred
                UINT file_count = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);

                // we can drag more than one file at the same time, so we have to loop here
                for (UINT i = 0; i < file_count; i++)
                {
                    TCHAR szFile[MAX_PATH];
                    UINT cch = DragQueryFile(hdrop, i, szFile, MAX_PATH);
                    if (cch > 0 && cch < MAX_PATH)
                    {
                        std::cout << std::format("{}", szFile) << std::endl;
                        // szFile contains the full path to the file, do something useful with it
                        // i.e. add it to a vector or something
                    }
                }

                ReleaseStgMedium(&stgm);

            }


            *pdwEffect &= DROPEFFECT_COPY;
            return S_OK;
        }
    };

    class Win32Window final : public Window
    {
    public:

        ~Win32Window() override {}

        LRESULT handleMessage(INT msg, WPARAM wParam, LPARAM lParam);
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
        std::vector<Event> currentPolledEvents_{};//TODO: smallvector
        io::WindowIo windowIo_{};
    };
}
