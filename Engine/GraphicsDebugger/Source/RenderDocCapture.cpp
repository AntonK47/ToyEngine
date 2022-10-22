#include "RenderDocCapture.h"
#include "Vulkan.h"

#include <array>
#include <mutex>

#include "renderdoc_app.h"
#include <cstdlib>
#include <Logger.h>
#include <filesystem>

#include "ValidationCommon.h"

namespace 
{
	RENDERDOC_API_1_5_0* renderDocApi = nullptr;
    HMODULE dllHandle = nullptr;
}

using namespace toy::renderer::debugger;

void ScopeCapture::start()
{
    if(shouldCapture_)
    {
        capture_->startCapture();
    }
	
}

void ScopeCapture::stopAndOpenCapture()
{
    if(shouldCapture_)
    {
        capture_->stopCapture();
        capture_->launchRenderDocApplication();
    }
    shouldCapture_ = false;
}

bool ScopeCapture::isRenderDocInjected() const
{
    return capture_->hasInitialized_;
}

void RenderDocCapture::initialize(const RenderDocCaptureDescriptor& descriptor)
{
#ifndef TOY_ENGINE_ENABLE_RENDER_DOC_CAPTURING
    return;
#endif
    auto pathBuffer = std::vector<wchar_t>{};
	pathBuffer.resize(256);
    const auto length = GetEnvironmentVariableW(L"RENDERDOC_PATH", pathBuffer.data(), static_cast<DWORD>(pathBuffer.size()));
    const auto hasError = GetLastError();
    pathBuffer.resize(length);
    
    if(hasError == ERROR_ENVVAR_NOT_FOUND)
    {
        LOG(WARNING) << "System environment variable RENDERDOC_PATH is not found!";
        return;
    }

    const auto pathString = std::wstring{ pathBuffer.data(), length };
    const auto path = std::filesystem::path{ pathString };
    auto allNeededFilesExists = std::filesystem::exists(path);

#ifdef WIN32
    const auto executable = std::filesystem::path{ path }.append("qrenderdoc.exe");
    const auto dynamicLibrary = std::filesystem::path{ path }.append("renderdoc.dll");
#endif

    allNeededFilesExists |= std::filesystem::exists(executable);
    allNeededFilesExists |= std::filesystem::exists(dynamicLibrary);

    if(!allNeededFilesExists)
    {
        LOG(WARNING) << "RenderDoc path seams to be corrupted. Check RENDERDOC_PATH or try to reinstall RenderDoc.";
        return;
    }

    {
#ifdef WIN32
        
        dllHandle = GetModuleHandleA("renderdoc.dll");
        if(!dllHandle)
        {
            LOG(WARNING) << "VK_LAYER_RENDERDOC_Capture layer is not included into a Vulkan backend";
        	return;
        }
        

        const auto renderdocGetApi =
            reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(  // NOLINT(clang-diagnostic-cast-function-type)
	            dllHandle,
	            "RENDERDOC_GetAPI"));

#endif

       const auto hasNoError = renderdocGetApi(eRENDERDOC_API_Version_1_1_2, reinterpret_cast<void**>(&renderDocApi));

        if(!hasNoError)
        {
            LOG(WARNING) << "Can't load RenderDoc dynamic library!";
            return;
        } 

        nativeBackend_ = descriptor.nativeBackend;

        auto capturePath = std::array<char,256>{};
        auto capturePathLength = size_t{};

    	const auto error = wcstombs_s(&capturePathLength, capturePath.data(), capturePath.size(), std::filesystem::current_path().append("ToyEngineCaptures\\frame_").native().c_str(), 255);

        if (error != 0)
        {
            LOG(WARNING) << "Invalid capture storage folder path!";
            return;
        }

        renderDocApi->SetCaptureFilePathTemplate(capturePath.data());
        renderDocApi->SetFocusToggleKeys(nullptr, 0);
        renderDocApi->SetCaptureKeys(nullptr, 0);
        renderDocApi->SetCaptureOptionU32(eRENDERDOC_Option_CaptureCallstacks, 1);
        renderDocApi->SetCaptureOptionU32(eRENDERDOC_Option_RefAllResources, 1 );
        renderDocApi->SetCaptureOptionU32(eRENDERDOC_Option_SaveAllInitials, 1 );
        renderDocApi->MaskOverlayBits(eRENDERDOC_Overlay_None, eRENDERDOC_Overlay_None);
        hasInitialized_ = true;
    }
}

void RenderDocCapture::deinitialize()
{
    if(hasInitialized_)
    {
        
    }
}

void RenderDocCapture::startCapture()
{
    if (hasInitialized_)
    {
#ifdef TOY_ENGINE_VULKAN_BACKEND
        const auto nativeBackend = api::vulkan::getVulkanNativeBackend(nativeBackend_);
        const auto instance = static_cast<VkInstance>(nativeBackend.instance);
        const auto devicePointer = RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(
            &instance);
#endif
        renderDocApi->StartFrameCapture(devicePointer, nullptr);
    }
}

void RenderDocCapture::stopCapture()
{
    if (hasInitialized_)
    {
#ifdef TOY_ENGINE_VULKAN_BACKEND
        const auto nativeBackend = api::vulkan::getVulkanNativeBackend(nativeBackend_);
        const auto instance = static_cast<VkInstance>(nativeBackend.instance);
        const auto devicePointer = RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(
	        &instance);
#endif
        const auto hasNoError = renderDocApi->EndFrameCapture(devicePointer, nullptr);

        LOG_IF(WARNING, hasNoError == 0) << "Trying to stop non running capturing process!";
    }
}

void RenderDocCapture::launchRenderDocApplication()
{
    if(!hasInitialized_)
    {
        return;
    }
    auto captureFilePath = std::array<char,512>{};
    auto captureFilePathLength = core::u32{ 512 };

    auto timestamp = core::u64{};

    const auto lastCapture = renderDocApi->GetNumCaptures()-1;

    renderDocApi->GetCapture(lastCapture, captureFilePath.data(), &captureFilePathLength, &timestamp);

    const auto path = std::string{ captureFilePath.data(), captureFilePathLength } + ".rdc";

    if(!exists(std::filesystem::path{ path }))
    {
        LOG(WARNING) << "Can't find capture file!";
        return;
    }

    if (!renderDocApi->IsRemoteAccessConnected())
    {
        const auto result = renderDocApi->LaunchReplayUI(true, path.c_str());
        if(result == 0)
        {
            LOG_IF(WARNING, result == 0) << "Could not launch RenderDoc!";
        }
    }
}
