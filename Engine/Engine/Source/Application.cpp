#include "Application.h"
#include <fstream>
#include <GlslRuntimeCompiler.h>
#include <Logger.h>

#include "Scene.h"
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

    

    void uploadDataToBuffer(RenderInterface& renderer, void* uploadData, size_t dataSize, const Handle<Buffer>& buffer, const u32 byteOffset)
    {
        void* data;
        renderer.map(buffer, &data);

        std::memcpy(static_cast<u8*>(data)+byteOffset, uploadData, dataSize);
        renderer.unmap(buffer);
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


    const auto filePath = "E:\\Develop\\ToyEngine\\out\\build\\x64-Release\\Tools\\MeshBuilder\\dragon.dat";
    const auto scene = scene::loadSceneFile(filePath);


    const auto frameData = renderer.createBuffer(BufferDescriptor
        {
            .size = 1024,
            .accessUsage = AccessUsage::uniform,
            .memoryUsage = MemoryUsage::cpuOnly,
        });

    void* frameDataPtr = nullptr;
    renderer.map(frameData, &frameDataPtr);

    auto time = 0.0f;
    

    const auto physicalMemorySize = u32{ 1024 * 1024 };

    /*
     *
     *table entry size 32bit = 4 byte
     *pageAddressable size 7 bit
     *
     */

    const auto pageSize = u32{ 256 };
    const auto pageCountInPhysicalMemory = u32{ physicalMemorySize / pageSize };
    const auto totalPageCount = u32{ physicalMemorySize / pageSize };
    const auto pageTableEntrySize = u32{ 4 };
    const auto pageTableByteSize = pageTableEntrySize * totalPageCount;

    const auto unifiedGeometryBufferPhysical = renderer.createBuffer(
        BufferDescriptor
        {
        	.size = physicalMemorySize,
        	.accessUsage =  AccessUsage::storage,
        	.memoryUsage = MemoryUsage::cpuWrite,
        });

    

    const auto unifiedGeometryBufferPageTable = renderer.createBuffer(
        BufferDescriptor
        {
            .size = pageTableByteSize,
            .accessUsage = AccessUsage::storage,
            .memoryUsage = MemoryUsage::cpuWrite,
        });


    const auto simpleTriangleGroup = BindGroupDescriptor
    {
        .bindings =
        {
            {
                .binding = 0,
                .descriptor = SimpleDeclaration{ BindingType::UniformBuffer}
            }
        }
    };

    const auto simpleTriangleMeshDataGroup = BindGroupDescriptor
    {
        .bindings =
        {
            {
                .binding = 0,
                .descriptor = SimpleDeclaration{ BindingType::StorageBuffer }
            },
            {
                .binding = 1,
                .descriptor = SimpleDeclaration{ BindingType::StorageBuffer }
            },
            {
                .binding = 2,
                .descriptor = SimpleDeclaration{ BindingType::StorageBuffer }
            }
        }
    };

    const auto simpleTriangleGroupLayout = renderer.allocateBindGroupLayout(simpleTriangleGroup);
    const auto simpleTriangleMeshDataGroupLayout = renderer.allocateBindGroupLayout(simpleTriangleMeshDataGroup);


	const auto vertexShaderGlslCode = loadShaderFile("Resources/Triangle.vert");
    const auto fragmentShaderGlslCode = loadShaderFile("Resources/Triangle.frag");

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


	auto simpleTriangleVSSpirvCode = ShaderByteCode{};
    auto simpleTriangleFSSpirvCode = ShaderByteCode{};

    auto result = GlslRuntimeCompiler::compileToSpirv(vertexShaderInfo, simpleTriangleVSSpirvCode);
    assert(result == CompilationResult::success);

    result = GlslRuntimeCompiler::compileToSpirv(fragmentShaderInfo, simpleTriangleFSSpirvCode);
    assert(result == CompilationResult::success);


    const auto vertexShaderModule = renderer.createShaderModule(toy::renderer::ShaderStage::vertex, { ShaderLanguage::Spirv1_6, simpleTriangleVSSpirvCode });

    const auto fragmentShaderModule = renderer.createShaderModule(toy::renderer::ShaderStage::vertex, { ShaderLanguage::Spirv1_6, simpleTriangleFSSpirvCode });

    const auto simpleTrianglePipeline = renderer.createPipeline(
        GraphicsPipelineDescriptor
        {
            .vertexShader = vertexShaderModule,
            .fragmentShader = fragmentShaderModule,
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
                SetBindGroupMapping{0, simpleTriangleMeshDataGroupLayout},
                SetBindGroupMapping{1, simpleTriangleGroupLayout}
            });

    
    const auto vertexBufferSize = static_cast<u32>(scene[0].mesh.positionVertexStream.size() * sizeof(
        scene::Position));

    const auto vertexBuffer = renderer.createBuffer(BufferDescriptor
        {
        	.size = vertexBufferSize,
            .accessUsage = AccessUsage::storage,
            .memoryUsage = MemoryUsage::cpuOnly,
        });

    uploadDataToBuffer(renderer, (void*)scene[0].mesh.positionVertexStream.data(), vertexBufferSize, vertexBuffer, 0);


    const auto triangleBufferSize = static_cast<u32>(scene[0].mesh.triangles.size() * sizeof(
        u8));

    const auto triangleBuffer = renderer.createBuffer(BufferDescriptor
        {
            .size = triangleBufferSize,
            .accessUsage = AccessUsage::storage,
            .memoryUsage = MemoryUsage::cpuOnly,
        });

    uploadDataToBuffer(renderer, (void*)scene[0].mesh.triangles.data(), triangleBufferSize, triangleBuffer, 0);

    const auto meshletsBufferSize = static_cast<u32>(scene[0].mesh.lods[0].meshlets.size() * sizeof(
	    scene::Meshlet));

    const auto meshletsBuffer = renderer.createBuffer(BufferDescriptor
        {
            .size = meshletsBufferSize,
            .accessUsage = AccessUsage::storage,
            .memoryUsage = MemoryUsage::cpuOnly,
        });

    uploadDataToBuffer(renderer, (void*)scene[0].mesh.lods[0].meshlets.data(), meshletsBufferSize, meshletsBuffer, 0);

    using Entry = u32;

    using PageTable = std::array<Entry, totalPageCount>;

    auto pageTable = PageTable{};


    struct Surface
    {
        u32 pageOffset;
    };

    

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

            //TODO: make global bind group allocator
            Handle<BindGroup> meshDataBindGroup = renderer.allocateBindGroup(simpleTriangleMeshDataGroupLayout);
            renderer.updateBindGroup(meshDataBindGroup, {

                        {
                            0, UAV{BufferView{ vertexBuffer, 0, vertexBufferSize}}
                        },
                        {
                            1, UAV{BufferView{ triangleBuffer, 0, triangleBufferSize}}
                        },
                        {
                            2, UAV{BufferView{ meshletsBuffer, 0, meshletsBufferSize}}
                        }
                });

            Handle<BindGroup> bindGroup = renderer.allocateBindGroup(simpleTriangleGroupLayout);

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
                        .clearValue = ColorClear{ 100.0f/255.0f, 149.0f/255.0f, 237.0f/255.0f, 1.0f }
                    }
                }
            };

            constexpr auto area = toy::RenderArea{ 0,0,1280,720 };

            commandList->beginRendering(renderingDescriptor, area);

            {
                constexpr auto scissor = toy::Scissor{ 0,0,1280, 720};
                constexpr auto viewport = toy::Viewport{ 0.0,0.0,1280.0,720.0 };


                commandList->bindPipeline(simpleTrianglePipeline);
                commandList->setScissor(scissor);
                commandList->setViewport(viewport);
                commandList->bindGroup(0, meshDataBindGroup);
                commandList->bindGroup(1, bindGroup);

                commandList->draw(scene[0].mesh.header.totalTriangles*3, 1, 0, 0);
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
