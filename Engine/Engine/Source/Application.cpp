#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include "Application.h"
#include <fstream>
#include <GlslRuntimeCompiler.h>
#include <Logger.h>
#include <RenderDocCapture.h>
#include <Scene.h>

#include <SDLWindow.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <thread>
#include <mutex>
#include <rigtorp/MPMCQueue.h>
#include <iostream>
#include <chrono>
#include <VirtualTextureStreaming.h>

#include <VulkanRenderInterface.h>
#include "SceneLoader.h"
#include <glm/ext/matrix_transform.hpp>

#include <execution>
#include <algorithm>
#include <span>

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
    auto graphicsDebugger = debugger::RenderDocCapture{};
    auto virtualTextureStreaming = VirtualTextureStreaming{};

    window.initialize(WindowDescriptor{ 1280, 720 });

    const auto workerCount = 10;

    const auto rendererDescriptor = RendererDescriptor
    {
        .version = 1,
        .instanceName = std::string{"ToyRenderer"},
        .handler = window.getWindowHandler(),
        .meta = window.getRendererMeta(),
        .windowExtentGetter = [&window]()
        {
            return WindowExtent{ window.width(), window.height()};
        },
        .threadWorkersCount = workerCount
    };
    renderer.initialize(rendererDescriptor);

    const auto renderDocDescriptor = debugger::RenderDocCaptureDescriptor
    {
        .nativeBackend = renderer.getNativeBackend()
    };
    graphicsDebugger.initialize(renderDocDescriptor);

    const auto virtualTextureStreamingDescriptor = VirtualTextureStreamingDescriptor{};
    virtualTextureStreaming.initialize(virtualTextureStreamingDescriptor);
    
#pragma region FrameRingLinearAllocator
    //FEATURE: This should moved into a Frame Linear Allocator
    //================================================================================
    auto usage = toy::core::Flags<BufferAccessUsage>{ BufferAccessUsage::storage };
    usage |= BufferAccessUsage::uniform;
    const auto frameData = renderer.createBuffer(BufferDescriptor
        {
            .size = 1024*1024*10,
            .accessUsage = usage,
            .memoryUsage = MemoryUsage::cpuOnly,
        });

    void* frameDataPtr = nullptr;
    renderer.map(frameData.nativeHandle, &frameDataPtr);
    //=================================================================================
#pragma endregion
#pragma region Pipeline creation 
    //R&D: Research about material system, (have in mind it should be compatible to a shader graph created materials.REFACTOR: extrude all material dependent stuff out of this function.
    const auto simpleTriangleGroup = BindGroupDescriptor
    {
	    .bindings =
	    {
		    {
			    .binding = 0,
			    .descriptor = BindingDescriptor{BindingType::UniformBuffer}
		    }
	    }
    };

    const auto simpleTrianglePerInstanceGroup = BindGroupDescriptor
    {
	    .bindings =
	    {
		    {
			    .binding = 0,
			    .descriptor = BindingDescriptor{BindingType::StorageBuffer}
		    }
	    }
    };

    const auto simpleTriangleMeshDataGroup = BindGroupDescriptor
    {
        .bindings =
        {
            {
                .binding = 0,
                .descriptor = BindingDescriptor{BindingType::StorageBuffer}
            },
            {
                .binding = 1,
                .descriptor = BindingDescriptor{BindingType::StorageBuffer}
            },
            {
                .binding = 2,
                .descriptor = BindingDescriptor{BindingType::StorageBuffer}
            },
            {
                .binding = 3,
                .descriptor = BindingDescriptor{BindingType::StorageBuffer}
            },
            {
                .binding = 4,
                .descriptor = BindingDescriptor{BindingType::StorageBuffer}
            }
        }
    };

    const auto simpleTriangleGroupLayout = renderer.createBindGroupLayout(simpleTriangleGroup);
    const auto simpleTriangleMeshDataGroupLayout = renderer.createBindGroupLayout(simpleTriangleMeshDataGroup);
    const auto simpleTrianglePerInstanceGroupLayout = renderer.createBindGroupLayout(simpleTrianglePerInstanceGroup);

	const auto vertexShaderGlslCode = loadShaderFile("Resources/Triangle.vert");
    const auto fragmentShaderGlslCode = loadShaderFile("Resources/Triangle.frag");

    const auto vertexShaderInfo =
	    GlslRuntimeCompiler::ShaderInfo
	    {
		    .entryPoint = "main",
		    .compilationDefines = {},
		    .shaderStage = compiler::ShaderStage::vertex,
		    .shaderCode = vertexShaderGlslCode,
		    .enableDebugCompilation = true
	    };

    const auto fragmentShaderInfo =
	    GlslRuntimeCompiler::ShaderInfo
	    {
		    .entryPoint = "main",
		    .compilationDefines = {},
		    .shaderStage = compiler::ShaderStage::fragment,
		    .shaderCode = fragmentShaderGlslCode,
		    .enableDebugCompilation = true
	    };


	auto simpleTriangleVsSpirvCode = ShaderByteCode{};
    auto simpleTriangleFsSpirvCode = ShaderByteCode{};

    auto result = GlslRuntimeCompiler::compileToSpirv(vertexShaderInfo, simpleTriangleVsSpirvCode);
    TOY_ASSERT(result == CompilationResult::success);

    result = GlslRuntimeCompiler::compileToSpirv(fragmentShaderInfo, simpleTriangleFsSpirvCode);
    TOY_ASSERT(result == CompilationResult::success);


    const auto vertexShaderModule = renderer.createShaderModule(toy::renderer::ShaderStage::vertex, { ShaderLanguage::spirv1_6, simpleTriangleVsSpirvCode });

    const auto fragmentShaderModule = renderer.createShaderModule(toy::renderer::ShaderStage::vertex, { ShaderLanguage::spirv1_6, simpleTriangleFsSpirvCode });

    const auto simpleTrianglePipeline = renderer.createPipeline(
        GraphicsPipelineDescriptor
        {
            .vertexShader = vertexShaderModule,
            .fragmentShader = fragmentShaderModule,
            .renderTargetDescriptor = RenderTargetsDescriptor
            {
                .colorRenderTargets = std::initializer_list
                {
                    ColorRenderTargetDescriptor{ ColorFormat::rgba8 }
                },
                .depthRenderTarget = DepthRenderTargetDescriptor{ DepthFormat::d32 }
            },
            .state = PipelineState
            {
                .depthTestEnabled = true
            }
        },
        {
            SetBindGroupMapping{0, simpleTriangleMeshDataGroupLayout},
            SetBindGroupMapping{1, simpleTriangleGroupLayout},
            SetBindGroupMapping{2, simpleTrianglePerInstanceGroupLayout}
        },
        {
            PushConstant({ .size = sizeof(u32)})
        });

#pragma endregion


    const auto imageDescriptor = ImageDescriptor
	{
        .format = Format::d32,
        .extent = Extent{window.width(), window.height()},
        .mips = 1,
        .layers = 1,
        .accessUsage = ImageAccessUsage::depthStencilAttachment,
    };
    auto depthFramebuffer = renderer.createImage(imageDescriptor);

    const auto depthFramebufferViewDescriptor = ImageViewDescriptor
    {
        .image = depthFramebuffer,
        .format = Format::d32,
        .type = ImageViewType::_2D,
        .aspect = ImageViewAspect::depth
    };
    auto depthFramebufferView = renderer.createImageView(depthFramebufferViewDescriptor);

    struct Camera
    {
        glm::vec3 position;
        glm::vec3 forward;
        glm::vec3 up;
        float movementSpeed{1.0f};
        float sensitivity{ 1.0f };
    };

    struct View
    {
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 viewProjection;
    };

    struct InstanceData
    {
        glm::mat4 model;
        glm::uint clusterOffset;
        glm::uint triangleOffset;
        glm::uint positionStreamOffset;
        glm::uint pad;
    };

    //REFACTOR: camera control
    //==============================
    auto camera = Camera
    {
        .position = glm::vec3{0.0f,0.0f,1.0f},
        .forward = glm::vec3{0.0f,0.0f,-1.0f},
        .up = glm::vec3{0.0f,1.0f,0.0f},
        .movementSpeed = 0.001f,
        .sensitivity = 0.2f
    };
    auto moveCameraFaster = false;
    auto onMousePressedScreenLocation = glm::vec2{ 0.0f,0.0f };
    auto mouseButtonPressed = false;
    //==============================
    const auto p1 = "E:\\Develop\\ToyEngineContent\\Pkg_E_Knight_anim\\Exports\\FBX\\Knight_USD_002.fbx";
    const auto p2 = "E:\\Develop\\ToyEngineContent\\crystal_palace.glb";
    const auto bistroExterior = "E:\\McGuireComputerGraphicsArchive\\Bistro\\exterior.obj";
    const auto bistroInterior = "E:\\McGuireComputerGraphicsArchive\\Bistro\\interior.obj";

    const auto bistroInteriorData = "E:\\Develop\\ToyEngineContent\\interior.dat";
    const auto bistroExteriorData = "E:\\Develop\\ToyEngineContent\\exterior.dat";
    const auto knightData = "E:\\Develop\\ToyEngineContent\\knight.dat";
    const auto anatomyData = "E:\\Develop\\ToyEngineContent\\Z-Anatomy.dat";

    const auto scene = Scene::loadSceneFromFile(renderer, bistroExteriorData);
    
    Handle<BindGroup> meshDataBindGroup = renderer.allocateBindGroup(simpleTriangleMeshDataGroupLayout, UsageScope::async);
	renderer.updateBindGroup(meshDataBindGroup, 
        {
				{
					0, UAV{BufferView{ scene.positionStream_.nativeHandle, 0, VK_WHOLE_SIZE}}
				},
			    {
					1, UAV{BufferView{ scene.uvStream_.nativeHandle, 0, VK_WHOLE_SIZE}}
				},
			    {
					2, UAV{BufferView{ scene.tangentFrameStream_.nativeHandle, 0, VK_WHOLE_SIZE}}
				},
				{
					3, UAV{BufferView{ scene.triangle_.nativeHandle, 0, VK_WHOLE_SIZE}}
				},
				{
					4, UAV{BufferView{ scene.clusters_.nativeHandle, 0, VK_WHOLE_SIZE}}
				}
		});

    //TODO: [#3] command submit should work also without calling nextFrame in a frame async scenario
    u32 objectToRender = scene.drawInstances_.size();

    


    auto time = 0.0f;
    auto frameNumber = u32{};
    bool stillRunning = true;
    auto captureTool = graphicsDebugger.getScopeCapture();

    {
        //playing with transfer queue

        auto t1 = renderer.acquireCommandList(QueueType::transfer);
        auto t2 = renderer.acquireCommandList(QueueType::transfer);

        auto d1 = renderer.submitCommandList(QueueType::transfer,{ t1 }, {});

        auto d2 = renderer.submitCommandList(QueueType::transfer, { t2 }, {d1.barrier()});
        

    }
    //const auto ii = renderer.createVirtualTexture({});

    auto frameStartTime = std::chrono::high_resolution_clock::now();
    auto frameEndTime = std::chrono::high_resolution_clock::now();

    using Batch = decltype(renderer)::SubmitBatchType;

    std::unique_ptr<Batch> prepareBatch;
    auto perThreadSubmits = std::array<std::unique_ptr<Batch>, 10>{};
    std::unique_ptr<Batch> postRenderingBatch;

    while (stillRunning)
    {
        const auto cpuFrameTime = frameEndTime - frameStartTime;
        frameStartTime = std::chrono::high_resolution_clock::now();
        const auto hertz = cpuFrameTime.count() / 1000000.0f;//ns -> s
        window.setWindowTitle(std::to_string(hertz)); // <- this couse memory allocation
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


        if(io.keyboardState.one == toy::io::ButtonState::pressed)
        {
            if(captureTool.isRenderDocInjected())
            {
                captureTool.captureNextMarkedScope();
                LOG(INFO) << "capturing frame " << frameNumber << "...";
            }
        } 

#pragma region Camera Control
        if (io.keyboardState.e == toy::io::ButtonState::pressed)
        {
            objectToRender += 1;
            std::cout << "visible objects: " << objectToRender << std::endl;
        }

        if (io.keyboardState.q == toy::io::ButtonState::pressed)
        {
            objectToRender -= 1;
            std::cout << "visible objects: " << objectToRender << std::endl;
        }

        if (io.keyboardState.shiftLeft == toy::io::ButtonState::pressed)
        {
            moveCameraFaster = true;
        }

        if (io.keyboardState.shiftLeft == toy::io::ButtonState::unpressed)
        {
            moveCameraFaster = false;
        }

        const auto fastSpeed = 25.0f;

        if(io.keyboardState.w == toy::io::ButtonState::pressed)
        {
            camera.position += camera.forward * camera.movementSpeed * (moveCameraFaster ? fastSpeed :1.0f);
        }
        if (io.keyboardState.s == toy::io::ButtonState::pressed)
        {
            camera.position -= camera.forward * camera.movementSpeed * (moveCameraFaster ? fastSpeed : 1.0f);
        }
        if (io.keyboardState.a == toy::io::ButtonState::pressed)
        {
            camera.position += glm::normalize(glm::cross(camera.forward, camera.up)) * camera.movementSpeed * (moveCameraFaster ? fastSpeed : 1.0f);
        }
        if (io.keyboardState.d == toy::io::ButtonState::pressed)
        {
            camera.position -= glm::normalize(glm::cross(camera.forward, camera.up)) * camera.movementSpeed * (moveCameraFaster ? fastSpeed : 1.0f);
        }
        if(io.mouseState.leftButton == toy::io::ButtonState::pressed && !mouseButtonPressed)
        {
            onMousePressedScreenLocation = glm::vec2{ io.mouseState.position.x, io.mouseState.position.y };
            mouseButtonPressed = true;
        }

        if (io.mouseState.leftButton == toy::io::ButtonState::unpressed && mouseButtonPressed)
        {
            mouseButtonPressed = false;
        }

        if(mouseButtonPressed)
        {
            auto mouseScreenLocation = glm::vec2{ io.mouseState.position.x, io.mouseState.position.y };

            auto delta = mouseScreenLocation - onMousePressedScreenLocation;
            delta.y *= -1.0;

            const auto right = glm::normalize(cross(camera.forward, camera.up));
            const auto up = glm::normalize(cross(right, camera.forward));

            const auto f = glm::normalize(camera.forward + right * delta.y + up * delta.x);//TODO:: [#4] swapped x and y??

            auto rotationAxis = glm::normalize(glm::cross(f, camera.forward));

            if(glm::length(rotationAxis) >= 0.1f)
            {
                const auto rotation = glm::rotate(glm::identity<glm::mat4>(), glm::radians(glm::length(delta) * camera.sensitivity), f);

                camera.forward = glm::normalize(glm::vec3(rotation * glm::vec4(camera.forward, 0.0f)));
            }

            onMousePressedScreenLocation = mouseScreenLocation;
        }
#pragma endregion

        frameNumber++;
        {
            captureTool.start();

            renderer.nextFrame();

            Handle<BindGroup> bindGroup = renderer.allocateBindGroup(simpleTriangleGroupLayout);

            auto frameDataBeginPtr = static_cast<u8*>(frameDataPtr);

            {
                const auto aspectRatio = static_cast<float>(window.width()) / static_cast<float>(window.height());
                const auto projection = glm::infinitePerspective(glm::radians(60.0f), aspectRatio, 0.001f);
                const auto view = glm::lookAt(camera.position, camera.position + camera.forward, camera.up);

                const auto viewData = View
                {
                    .view = view,
                    .projection = projection,
                    .viewProjection = projection * view
                };

                std::memcpy(frameDataBeginPtr, &viewData, sizeof(View));
                frameDataBeginPtr += sizeof(View);
            }

            const auto myConstantBufferView = BufferView{ frameData.nativeHandle, {}, sizeof(View) };

            renderer.updateBindGroup(bindGroup, {
                {
                    0,
                    CBV{ myConstantBufferView }
                }
                });


            const auto& swapchainImage = renderer.acquireNextSwapchainImage();

            const auto instances = std::span{ scene.drawInstances_ };

            const auto batchSize = 10;
            const auto itemsPerBatch = scene.drawInstances_.size() / batchSize;
            auto batchSpanOffset = std::size_t{ 0 };

            auto batches = std::vector<std::remove_const_t<decltype(instances)>>{};
            batches.resize(batchSize);
            for (auto i = u32{}; i < batchSize - 1; i++)
            {
                batches[i] = instances.subspan(batchSpanOffset, itemsPerBatch);
                batchSpanOffset += itemsPerBatch;
            }
            batches[batchSize - 1] = instances.subspan(batchSpanOffset, scene.drawInstances_.size() - batchSpanOffset);

            auto setIndicies = std::vector<int>{};
            setIndicies.resize(batchSize);
            std::iota(std::begin(setIndicies), std::end(setIndicies), 0);

            auto batchOffsets = std::vector<std::size_t>{};
            batchOffsets.resize(batchSize);
            auto itemsPerBatches = [&](std::size_t i) { return batches[i].size(); };

            std::transform_exclusive_scan(std::begin(setIndicies), std::end(setIndicies),
                std::begin(batchOffsets), std::size_t{ 0 }, std::plus<std::size_t>{}, itemsPerBatches);

            const auto renderingDescriptor = RenderingDescriptor
            {
                .colorRenderTargets = {
                    RenderTargetDescriptor
                    {
                        .imageView = swapchainImage.view,
                        .load = LoadOperation::clear,
                        .store = StoreOperation::store,
                        .resolveMode = ResolveMode::none,
                        .clearValue = ColorClear{ 100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f }
                    }
                },
                .depthRenderTarget = RenderTargetDescriptor
                {
                    .imageView = depthFramebufferView,
                    .load = LoadOperation::clear,
                    .store = StoreOperation::store,
                    .resolveMode = ResolveMode::none,
                    .clearValue = DepthClear{ 1.0f }
                }
            };
            assert(&renderingDescriptor);
            constexpr auto area = RenderArea{ 0,0,1280,720 };

            {
                auto cmd = renderer.acquireCommandList(toy::renderer::QueueType::graphics);
                cmd.begin();
                //TODO: this should performed on initial resource creation
                cmd.barrier(
                    {
                        ImageBarrierDescriptor
                        {
                            .srcLayout = Layout::undefined,
                            .dstLayout = Layout::present,
                            .image = swapchainImage.image
                        }
                    });
                cmd.barrier(
                    {
                        ImageBarrierDescriptor
                        {
                            .srcLayout = Layout::undefined,
                            .dstLayout = Layout::depthStencilRenderTarget,
                            .aspect = ImageViewAspect::depth,
                            .image = depthFramebuffer
                        }
                    });


                cmd.barrier(
                    {
                        ImageBarrierDescriptor
                        {
                            .srcLayout = Layout::present,
                            .dstLayout = Layout::colorRenderTarget,
                            .image = swapchainImage.image
                        }
                    });

                //perform render target clearing
                cmd.beginRendering(renderingDescriptor, area);
                cmd.endRendering();
                cmd.end();

                if (postRenderingBatch != nullptr)
                {
                    auto dependency = postRenderingBatch->barrier();
                    prepareBatch = std::make_unique<Batch>(renderer.submitCommandList(toy::renderer::QueueType::graphics, { cmd }, {dependency}));
                }
                else
                {
                    prepareBatch = std::make_unique<Batch>(renderer.submitCommandList(toy::renderer::QueueType::graphics, { cmd }, {}));
                }
            }
            renderer.submitBatches(QueueType::graphics, { *prepareBatch });


            Handle<BindGroup> perInstanceGroup = renderer.allocateBindGroup(simpleTrianglePerInstanceGroupLayout);
            const auto batchOffset = static_cast<u32>(frameDataBeginPtr - static_cast<u8*>(frameDataPtr));

            const auto dataMemSize = sizeof(InstanceData);
            renderer.updateBindGroup(perInstanceGroup,
                {
                    BindingDataMapping
                    {
                        .binding = 0,
                        .view = UAV
                        {
                            .bufferView = BufferView
                            {
                                .buffer = frameData.nativeHandle,
                                .offset = batchOffset,
                                .size = (u64)(dataMemSize * scene.drawInstances_.size())
                            }
                        }
                    }
                });

            std::for_each(std::execution::seq, std::begin(setIndicies), std::end(setIndicies), [&](auto& index)
                {
                    auto& drawInstances = batches[index];
                    auto cmd = renderer.acquireCommandList(toy::renderer::QueueType::graphics, WorkerThreadId{ .index = static_cast<u32>(index % workerCount)});
                    cmd.begin();

                    const auto renderingDescriptor = RenderingDescriptor
                    {
                        .colorRenderTargets = {
                            RenderTargetDescriptor
                            {
                                .imageView = swapchainImage.view,
                                .load = LoadOperation::load,
                                .store = StoreOperation::store,
                                .resolveMode = ResolveMode::none,
                                .clearValue = ColorClear{ 100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f }
                            }
                        },
                        .depthRenderTarget = RenderTargetDescriptor
                        {
                            .imageView = depthFramebufferView,
                            .load = LoadOperation::load,
                            .store = StoreOperation::store,
                            .resolveMode = ResolveMode::none,
                            .clearValue = DepthClear{ 1.0f }
                        }
                    };

                    cmd.beginRendering(renderingDescriptor, area);

                    {
                        constexpr auto scissor = Scissor{ 0,0,1280, 720 };
                        constexpr auto viewport = Viewport{ 0.0,0.0,1280.0,720.0 };


                        cmd.bindPipeline(simpleTrianglePipeline);
                        cmd.setScissor(scissor);
                        cmd.setViewport(viewport);
                        cmd.bindGroup(0, meshDataBindGroup);
                        cmd.bindGroup(1, bindGroup);
                        cmd.bindGroup(2, perInstanceGroup);

                        
                        auto setOffset = batchOffsets[index] * dataMemSize;
                        
                        for (auto j = u32{}; j < std::clamp(objectToRender, u32{}, static_cast<u32>(drawInstances.size())); j++)
                        {
                            const auto& drawInstance = drawInstances[j];


                            //scene.meshes_[drawInstance.meshIndex].lods[0].
                            const auto& mesh = scene.meshes_[drawInstance.meshIndex];

                            //this scope should be thread safe. More preciesly, memory allocation should be thread safe. Beter strategy is to allocate block of memory for each render thread up front. [see Miro board, multithreaded per frame dynamic allocator]

                            const auto instanceData = InstanceData
                            {
                                .model = glm::translate(drawInstance.model, glm::vec3(0.0f,0.0f,0.0f)),
                                .clusterOffset = mesh.clusterOffset,
                                .triangleOffset = mesh.triangleOffset,
                                .positionStreamOffset = mesh.positionStreamOffset
                            };

                            std::memcpy(frameDataBeginPtr + setOffset, &instanceData, dataMemSize);
                            setOffset += dataMemSize;

                        }
                        
                        for (auto j = u32{}; j < std::clamp(objectToRender, u32{}, static_cast<u32>(drawInstances.size())); j++)
                        {
                            const auto& drawInstance = drawInstances[j];
                            const auto& mesh = scene.meshes_[drawInstance.meshIndex];


                            const u32 value = j + static_cast<u32>(batchOffsets[index]);
                            cmd.pushConstant(value);
                            cmd.draw(mesh.vertexCount, 1, 0, 0);
                        }
                        
                    }

                    cmd.endRendering();
                    cmd.end();

                    auto perThreadBatch = renderer.submitCommandList(toy::renderer::QueueType::graphics, { cmd }, { prepareBatch->barrier() });
                    perThreadSubmits[index] = std::make_unique<Batch>(perThreadBatch);

                }
                );

            
            renderer.submitBatches(QueueType::graphics, 
                { 
                    *perThreadSubmits[0],
                    *perThreadSubmits[1],
                    *perThreadSubmits[2],
                    *perThreadSubmits[3],
                    * perThreadSubmits[4],
                    * perThreadSubmits[5],
                    * perThreadSubmits[6],
                    * perThreadSubmits[7],
                    * perThreadSubmits[8],
                    * perThreadSubmits[9]
                });



            {
                auto cmd = renderer.acquireCommandList(toy::renderer::QueueType::graphics);
                cmd.begin();
                cmd.barrier({
                        ImageBarrierDescriptor
                        {
                            .srcLayout = Layout::colorRenderTarget,
                            .dstLayout = Layout::present,
                            .image = swapchainImage.image
                        }
                    });

                cmd.end();
                postRenderingBatch = std::make_unique<Batch>(renderer.submitCommandList(toy::renderer::QueueType::graphics, { cmd }, 
                    {
                        perThreadSubmits[0]->barrier(),
                        perThreadSubmits[1]->barrier(),
                        perThreadSubmits[2]->barrier(),
                        perThreadSubmits[3]->barrier(),
                        perThreadSubmits[4]->barrier(),
                        perThreadSubmits[5]->barrier(),
                        perThreadSubmits[6]->barrier(),
                        perThreadSubmits[7]->barrier(),
                        perThreadSubmits[8]->barrier(),
                        perThreadSubmits[9]->barrier()
                    }));
            }


            renderer.submitBatches(QueueType::graphics, { *postRenderingBatch });
            renderer.present(postRenderingBatch->barrier());

            time += 0.01f;
            captureTool.stopAndOpenCapture();
        }
        frameEndTime = std::chrono::high_resolution_clock::now();
    }

    graphicsDebugger.deinitialize();
    renderer.deinitialize();
    window.deinitialize();
    logger::deinitialize();

    return EXIT_SUCCESS;
}
