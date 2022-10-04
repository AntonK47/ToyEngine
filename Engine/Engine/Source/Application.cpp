#include "Application.h"

#include <Logger.h>

#include "VulkanRenderInterface.h"
#include "SDLWindow.h"

#include <GlslRuntimeCompiler.h>
#include <fstream>

using namespace toy::renderer;
using namespace toy::window;
using namespace compiler;

namespace 
{
	std::string loadShaderFile(const std::string& filePath)
	{
        auto fileStream = std::ifstream{ filePath, std::ios::ate };
        assert(fileStream.is_open());

        const auto length = static_cast<uint32_t>(fileStream.tellg());
        fileStream.seekg(0);

        auto code = std::vector<char>{};
        code.resize(length);

        fileStream.read(code.data(), length);
        return std::string{ code.data(), length };
	}
}

int Application::run()
{
    logger::initialize();
    auto window = SDLWindow{};
    auto renderer = api::vulkan::VulkanRenderInterface{};
    

    window.initialize(WindowDescriptor{ 1280, 720 });

    const auto rendererDescriptor = RendererDescriptor
    {
        .version = 1,
        .instanceName = std::string{"ToyRenderer"},
        .handler = window.getWindowHandler(),
        .meta = window.getRendererMeta(),
        .windowExtentGetter = [&window]()
        {
	        return Extent{ window.width(), window.height()};
        }
    };


    renderer.initialize(rendererDescriptor);

    //resource loading
    {
        const auto vertexShaderGlslCode = loadShaderFile("Resources/FullscreenQuadWithoutUniforms.vert");
        const auto fragmentShaderGlslCode = loadShaderFile("Resources/Lit.frag");

        const auto vertexShaderInfo = GlslRuntimeCompiler::ShaderInfo
        {
            .entryPoint = "main",
            .compilationDefines = {},
            .shaderStage = compiler::ShaderStage::vertex,
            .shaderCode = vertexShaderGlslCode,
            .enableDebugCompilation = false
        };

        const auto fragmentShaderInfo = GlslRuntimeCompiler::ShaderInfo
        {
            .entryPoint = "main",
            .compilationDefines = {},
            .shaderStage = compiler::ShaderStage::fragment,
            .shaderCode = fragmentShaderGlslCode,
            .enableDebugCompilation = false
        };

        auto vertexShaderSpirvCode = ShaderByteCode{};
        auto fragmentShaderSpirvCode = ShaderByteCode{};

        auto result = GlslRuntimeCompiler::compileToSpirv(vertexShaderInfo, vertexShaderSpirvCode);
        assert(result == CompilationResult::success);

        result = GlslRuntimeCompiler::compileToSpirv(fragmentShaderInfo, fragmentShaderSpirvCode);
        assert(result == CompilationResult::success);


        

        const auto vertexShaderModule = renderer.createShaderModule(toy::renderer::ShaderStage::vertex, { ShaderLanguage::Spirv1_6, vertexShaderSpirvCode });

        const auto fragmentShaderModule = renderer.createShaderModule(toy::renderer::ShaderStage::vertex, { ShaderLanguage::Spirv1_6, fragmentShaderSpirvCode });


        const auto pipeline = renderer.createPipeline(
            GraphicsPipelineDescriptor
            {
                .vertexShader = vertexShaderModule.get(),
                .fragmentShader = fragmentShaderModule.get(),
                .renderTargetDescriptor = RenderTargetsDescriptor
                {
                    .colorRenderTargets = std::initializer_list
                    {
                        ColorRenderTargetDescriptor{ Format::RGBA8 }
                    }
                },
                .state = PipelineState
                {
                    .depthTestEnabled = true
                }
            },
            {

            });
    }



    bool stillRunning = true;
    while (stillRunning)
    {
        window.pollEvents();
        const auto events = window.getEvents();
        [[maybe_unused]] const auto io = window.getIo();

        for (const auto event : events)
        {
            if (event == Event::quit)
            {
                stillRunning = false;
            }
        }

        {
        	renderer.nextFrame();

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
		/*{
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

		}*/

    }

    renderer.deinitialize();
    window.deinitialize();
    logger::deinitialize();
    /*{
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

		//auto d = DECLARE_BINDING_GROUP(0, Texture2D)
		//auto dd = BINDING_GROUP_DESCRIPTOR_FIELDS_N(0, Texture2D, 1, StorageBuffer);
    }*/

    return EXIT_SUCCESS;
}
