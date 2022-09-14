#include "Application.h"

#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

// Tell SDL not to mess with main()
//#define SDL_MAIN_HANDLED
#define NOMINMAX
#define VULKAN_VALIDATION

#include <cassert>
#include <iostream>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include "VulkanRenderInterface.h"
#include "SDLWindow.h"
#include "GlslRuntimeCompiler.h"
#include <fstream>

namespace
{
    std::string loadTextShaderFile(const std::string& filePath)
    {
        auto length = uint32_t{};
        auto buffer = std::string{};
        {
            using namespace std::string_literals;
            auto fileStream = std::ifstream{ filePath,  std::ios::ate };
            assert(fileStream.is_open());
            length = static_cast<uint32_t>(fileStream.tellg());
            fileStream.seekg(0);

            buffer.resize(length);
            fileStream.read(buffer.data(), length);
            fileStream.close();
        }
        return buffer;
    }
}

int Application::run()
{
    auto window = toy::window::SDLWindow{ 1280, 720 };
    auto renderer = toy::renderer::api::vulkan::VulkanRenderInterface{};


    /*
     * Create surface
     *  - we need fetch supported formats and select the matched format
     */

    /*renderer.create
    renderer.createSurface(window.get)*/

    const auto rendererDescriptor = toy::renderer::RendererDescriptor
    {
        .version = 1,
        .instanceName = std::string{"ToyRenderer"},
        .handler = window.getWindowHandler(),
        .meta = window.getRendererMeta(),
        .windowExtentGetter = [&window]() { return toy::renderer::Extent{ window.width(), window.height()}; }
    };

    renderer.initialize(rendererDescriptor);

    bool stillRunning = true;
    while (stillRunning)
    {
        window.pollEvents();
        const auto events = window.getEvents();
        const auto io = window.getIO();

        if (io.keyboardState.zero == toy::io::ButtonState::pressed)
        {
            std::cout << "zero button has been pressed!" << std::endl;
        }
        
        for(const auto event : events)
        {
	        if(event == toy::window::Event::quit)
	        {
                stillRunning = false;
	        }
        }

        renderer.nextFrame();
        auto commandList = renderer.acquireCommandList(toy::renderer::QueueType::graphics, toy::renderer::CommandListType::primary);
        commandList->barrier({});

        using namespace toy::renderer;
        //This function should also be multi threaded.

        const auto vertexShader = ShaderModule
        {
        };

        const auto fragmentShader = ShaderModule
        {
        };

        auto someCoolProgram = renderer.createPipeline(
            GraphicsPipelineDescription
            {
	            ShaderModule{},
                ShaderModule{},
                RenderTargetsDescription
            	{
					.colorRenderTargets = std::initializer_list
            		{
            			ColorRenderTargetDescription{}
            		},
            		.depthRenderTarget = DepthRenderTargetDescription{},
            		.stencilRenderTarget = StencilRenderTargetDescription{}
            	},
            	{
            		.depthTestEnabled = true
            	}
            },
            {
                BindGroup
	            {
	                .set = 0, //rename to bind frequency to make opaque set assignment??
	                .bindings = 
	                {
                        {
                            .binding = 0,
							.uniform = UniformDeclaration{}
                        },
                        {
                            .binding = 1,
                            .sampler2D = Sampler2DDeclaration{}
                        }
	                }
	            },
                BindGroup
                {
					.set = 1,
                    .bindings =
                    {
                        {
                            .binding = 0,
							.bindlessArray = BindlessArrayDeclaration{}//?????
                        }
                    }
                }
            });

    }

    renderer.deinitialize();

    using namespace toy::renderer::compiler;
    const auto shaderCode = (GlslShaderCode)loadTextShaderFile("E:/Develop/VulkanMultithreading/RenderInterfaceTest/TestShaders/noopShader.frag");
    auto byteCode = ShaderByteCode{};
    const auto result = GlslRuntimeCompiler::compileToSpirv(
        GlslRuntimeCompiler::ShaderInfo
        {
            "entry",
            { "DEFINE1" },
            ShaderStage::fragment,
            shaderCode,
            true
        },
        byteCode);


    return EXIT_SUCCESS;


	/*
	 * Renderer initialization
	 *  fetch required extensions
	 *  handle window resize
	 *  pass window handler
	 */

	/*
	 * game loop?? Not really because of multithreading?
	 */
}
