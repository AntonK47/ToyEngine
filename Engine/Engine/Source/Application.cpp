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
        if (io.keyboardState.nine == toy::io::ButtonState::pressed)
        {
            std::cout << "9!" << std::endl;
        }

        for (const auto event : events)
        {
            if (event == toy::window::Event::quit)
            {
                stillRunning = false;
            }
        }

        {
        	renderer.nextFrame();
            using namespace toy::renderer;

            const auto swapchainImage = renderer.acquireNextSwapchainImage();

            auto commandList = renderer.acquireCommandList(QueueType::graphics, CommandListType::primary);
         

            commandList->barrier({
                ImageBarrierDescriptor
                {
                    .srcLayout = Layout::Undefined,
                    .dstLayout = Layout::Present,
                    .image = swapchainImage.image.get()
                }
                });
            
            commandList->barrier({ 
                ImageBarrierDescriptor
            	{
            		.srcLayout = Layout::Present,
                    .dstLayout = Layout::ColorRenderTarget,
                    .image = swapchainImage.image.get()
            	}
            });

            const auto renderingDescriptor = RenderingDescriptor
            {
                .colorRenderTargets = {
                    RenderTargetDescriptor
                    {
                        .renderTargetImageAccessor = swapchainImage.view.get(),
                        .load = LoadOperation::clear,
                        .store = StoreOperation::store,
                        .resolveMode = ResolveMode::none,
                        .clearValue = ColorClear{ 0.5, 0.1f, 0.2f, 0.5f }
                    }
                }
            };

            constexpr auto area = RenderArea{ 100,200,300,400 };

            commandList->beginRendering(renderingDescriptor, area);

            {
	            //commandList->draw(...)
            }

            commandList->endRendering();

            commandList->barrier({ 
                ImageBarrierDescriptor
            	{
            		.srcLayout = Layout::ColorRenderTarget,
            		.dstLayout = Layout::Present,
                    .image = swapchainImage.image.get()
            	}
            });

            renderer.submitCommandList(std::move(commandList));
            renderer.present();
        }
        

        //==========================================
        using namespace toy::renderer;
        //This function should also be multi threaded.

        const auto vertexShader = ShaderModule
        {
        };

        const auto fragmentShader = ShaderModule
        {
        };

        auto someCoolProgram = renderer.createPipeline(
            GraphicsPipelineDescriptor
            {
                ShaderModule{},
                ShaderModule{},
                RenderTargetsDescription
                {
                    .colorRenderTargets = std::initializer_list
                    {
                        ColorRenderTargetDescriptor{}
                    },
                    .depthRenderTarget = DepthRenderTargetDescriptor{},
                    .stencilRenderTarget = StencilRenderTargetDescriptor{}
                },
                {
                    .depthTestEnabled = true
                }
            },
            {
                
            });





        








        const auto group1 = BindGroupDescriptor
        {
            .bindings =
            {
                {
                    .binding = 0,
                    .descriptor = SimpleDeclaration{ BindingType::UniformBuffer}
                },
                {
                    .binding = 1,
                    .descriptor = SimpleDeclaration{ BindingType::Texture2D }
                }
            }
        };
        const auto group2 = BindGroupDescriptor
        {
            .bindings =
            {
                {
                    .binding = 0,
                    .descriptor = BindlessDeclaration{ BindingType::Texture2D }
                }
            }
        };


        const auto group1Layout = renderer.allocateBindGroupLayout(group1);
        using Matrix = std::array<float, 9>;

        struct ViewData
        {
            int i;
            Matrix m;
        };


        

        auto view = renderer.allocateBindGroup(group1);
        
        auto cmd = renderer.acquireCommandList(QueueType::graphics, CommandListType::primary);
        
        //cmd->bindGroup(view);
        //cmd->setTextureSrv(view, 1);
        

        //bind groups can be shared across multiple pipelines, so I can use BindGroup caching????????????????????????????

        // => so I need BindGroupDescriptor =D

        

        struct MemoryCheck
        {
            ViewData view;
        } bindGroup0Memory;

        bindGroup0Memory.view = ViewData{ 1, {} };

    }

    renderer.deinitialize();


    {
        enum class BindingType
        {
	        Texture2D,
            StorageBuffer
        };

        struct Binding
        {
	        toy::core::u32 binding;
            BindingType type;
        };
        struct BindingGroupDescriptor
        {
            std::vector<Binding> bindings;
        };
        const auto a = BindingGroupDescriptor
        {
            {
	            {
	                .binding = 0,
	                .type = BindingType::Texture2D
	            },
	            {
	                .binding = 1,
	                .type = BindingType::StorageBuffer
	            }
            }
        };




        struct SrvTexture2D{};
        struct SrvStorageBuffer{};

        struct MyStruct
        {
            SrvTexture2D a;
            SrvStorageBuffer b;
        };



        struct A
        {
            using BindingGroupType = int;
            BindingGroupDescriptor bgd;
            
        };


#define BINDING_GROUP_DESCRIPTOR_FIELD(binding, type) \
	Binding{ binding, BindingType::type }

#define FIELDS

#define BINDING_GROUP_DESCRIPTOR_FIELDS_0(binding, type, ...)
#define BINDING_GROUP_DESCRIPTOR_FIELDS_N(binding, type, ...) \
	BINDING_GROUP_DESCRIPTOR_FIELD(binding, type), BINDING_GROUP_DESCRIPTOR_FIELDS(##__VA_ARGS__)
 
#define BINDING_GROUP_DESCRIPTOR(...) BindingGroupDescriptor{ { BINDING_GROUP_DESCRIPTOR_FIELDS(##__VA_ARGS__) }}
#define DECLARE_BINDING_GROUP(...) BINDING_GROUP_DESCRIPTOR(##__VA_ARGS__);

      /*  auto d = DECLARE_BINDING_GROUP(0, Texture2D)
    	auto dd = BINDING_GROUP_DESCRIPTOR_FIELDS_N(0, Texture2D, 1, StorageBuffer);*/
    }

    return EXIT_SUCCESS;
}
