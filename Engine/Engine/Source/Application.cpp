#include "Application.h"

#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define NOMINMAX

#include <iostream>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include "VulkanRenderInterface.h"
#include "SDLWindow.h"
#include <fstream>



int Application::run()
{
    auto window = toy::window::SDLWindow{ 1280, 720 };
    auto renderer = toy::renderer::api::vulkan::VulkanRenderInterface{};

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

    return EXIT_SUCCESS;
}
