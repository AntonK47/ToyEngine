#include "Application.h"
#include <fstream>
#include <GlslRuntimeCompiler.h>
#include <Logger.h>
#include "SDLWindow.h"
#include "VulkanRenderInterface.h"

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

    const auto frameData = renderer.createBuffer(BufferDescriptor
        {
            .size = 1024,
            .accessUsage = AccessUsage::uniform,
            .memoryUsage = MemoryUsage::cpuOnly,
        });

    void* frameDataPtr = nullptr;
    renderer.map(frameData, &frameDataPtr);

    auto time = 0.0f;

    auto pipeline = std::unique_ptr<Pipeline>{};
    auto bindGroupLayout = Handle<BindGroupLayout>{};
    
    //resource loading
    {
        const auto vertexShaderGlslCode = loadShaderFile("Resources/FullscreenQuadWithoutUniforms.vert");
        const auto fragmentShaderGlslCode = loadShaderFile("Resources/HyperbolicPoincareWeave.frag");

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


        const auto group1 = BindGroupDescriptor
        {
            .bindings =
            {
                {
                    .binding = 0,
                    .descriptor = SimpleDeclaration{ BindingType::UniformBuffer}
                }
            }
        };

        bindGroupLayout = renderer.allocateBindGroupLayout(group1);

        pipeline = renderer.createPipeline(
            GraphicsPipelineDescriptor
            {
                .vertexShader = ShaderModuleRef(vertexShaderModule.get()),
                .fragmentShader = ShaderModuleRef(fragmentShaderModule.get()),
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
                SetBindGroupMapping{0, bindGroupLayout}
            });
    }
    const auto myTestPipeline = Ref(pipeline.get());


    bool stillRunning = true;
    while (stillRunning)
    {
        window.pollEvents();
        const auto& events = window.getEvents();
        const auto& io = window.getIo();

        for (const auto& event : events)
        {
            if (event == Event::quit)
            {
                stillRunning = false;
            }
        }

        {
        	renderer.nextFrame();

            Handle<BindGroup> bindGroup = renderer.allocateBindGroup(bindGroupLayout);

            const auto offset = u32{};
            struct FrameData
            {
                float resolution[2];
                float mouse[2];
                float time;
            };

            auto data = FrameData{};
            data.resolution[0] = static_cast<float>(window.width());
            data.resolution[1] = static_cast<float>(window.height());
            data.mouse[0] = static_cast<float>(io.mouseState.position.x);
            data.mouse[1] = static_cast<float>(io.mouseState.position.y);
            data.time = time;

            std::memcpy(frameDataPtr, &data, sizeof(FrameData));

            const auto myConstantBufferView = BufferView{ frameData, offset, sizeof(float)};

            renderer.updateBindGroup(bindGroup, {
                {
                    0,
                    CBV{ myConstantBufferView }
                }
                });


            const auto& swapchainImage = renderer.acquireNextSwapchainImage();

            //TODO: maybe I should use ref instead of unique_ptr?
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

            constexpr auto area = toy::RenderArea{ 0,0,1280,720 };

            commandList->beginRendering(renderingDescriptor, area);

            {
                constexpr auto scissor = toy::Scissor{ 0,0,1280, 720};
                constexpr auto viewport = toy::Viewport{ 0.0,0.0,1280.0,720.0 };

                commandList->bindPipeline(myTestPipeline);
                commandList->setScissor(scissor);
                commandList->setViewport(viewport);
                commandList->bindGroup(0, bindGroup);
        		commandList->draw(3, 1, 0, 0);
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

            time += 0.01f;
        }
    }

    renderer.deinitialize();
    window.deinitialize();
    logger::deinitialize();
   

    return EXIT_SUCCESS;
}
