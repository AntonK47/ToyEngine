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

#include <VulkanRenderInterface.h>
#include "SceneLoader.h"
#include <glm/ext/matrix_transform.hpp>
//#include <ozz/base/io/archive.h>
//#include <ozz/animation/offline/fbx/fbx_skeleton.h>
//#include <ozz/animation/offline/fbx/fbx.h>



using namespace toy::renderer;
using namespace toy::window;
using namespace compiler;


namespace 
{
    void heavyWork()
    {
        std::cout << "mega heavy job has started!!!!" << std::endl;
    }

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
    /*using namespace ozz::animation::offline::fbx;
    using namespace ozz::animation::offline;
    auto instanceManager = FbxManagerInstance{};
    auto fbxIoSettings = FbxDefaultIOSettings{ instanceManager };

    const auto file = std::string{ "C:\\Users\\AntonKi8\\Downloads\\Pkg_E_Knight_anim\\Exports\\FBX\\Knight_USD_002.fbx" };
    const auto pwd = std::string{ "" };

    auto sceneLoader = ozz::animation::offline::fbx::FbxSceneLoader
	{
		file.c_str(),
		pwd.c_str(),
		instanceManager,
		fbxIoSettings
	};

    RawSkeleton* skeleton;
    OzzImporter::NodeType type;
    ozz::animation::offline::fbx::ExtractSkeleton(sceneLoader, type, skeleton);*/

    logger::initialize();
    auto window = SDLWindow{};
    auto renderer = api::vulkan::VulkanRenderInterface{};
    auto graphicsDebugger = debugger::RenderDocCapture{};

    window.initialize(WindowDescriptor{ 1280, 720 });

    const auto rendererDescriptor = RendererDescriptor
    {
        .version = 1,
        .instanceName = std::string{"ToyRenderer"},
        .handler = window.getWindowHandler(),
        .meta = window.getRendererMeta(),
        .windowExtentGetter = [&window]()
        {
            return WindowExtent{ window.width(), window.height()};
        }
    };
    renderer.initialize(rendererDescriptor);

    const auto renderDocDescriptor = debugger::RenderDocCaptureDescriptor
    {
        .nativeBackend = renderer.getNativeBackend()
    };
    graphicsDebugger.initialize(renderDocDescriptor);

    


#pragma region Multithreading
    struct Job
    {
    public:

        Job() = default;
        Job(const std::function<void()>& jobJunction)
        {
            function = jobJunction;
        }
        Job(const Job& a) noexcept { function = a.function; }
        Job& operator=(const Job& a) noexcept { function = a.function; return *this; }
        Job(Job&&) = delete;
        std::function<void()> function;
    };

    auto queue = rigtorp::MPMCQueue<Job>{ 100 };
    auto shouldRun = true;

    auto workerProcessor = [&shouldRun, &queue]()
    {
        while (shouldRun)
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
            //std::cout << "thread id: " << std::this_thread::get_id() << std::endl;

            auto job = Job{};
            while (queue.try_pop(job))
            {
                if (job.function)
                {
                    job.function();
                }
            }
        }

    };

    auto workerA = std::thread
    {
        workerProcessor
    };

    auto workerB = std::thread
    {
        workerProcessor
    };
#pragma endregion
#pragma region FrameRingLinearAllocator
    //FEATURE: This should moved into a Frame Linear Allocator
    //================================================================================
    const auto frameData = renderer.createBuffer(BufferDescriptor
        {
            .size = 1024*1024*100,
            .accessUsage = BufferAccessUsage::uniform,
            .memoryUsage = MemoryUsage::cpuOnly,
        });

    void* frameDataPtr = nullptr;
    renderer.map(frameData.nativeHandle, &frameDataPtr);
    //=================================================================================
#pragma endregion
#pragma region Material 
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
			    .descriptor = BindingDescriptor{BindingType::UniformBuffer}
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
        u32 clusterOffset;
        u32 triangleOffset;
        u32 positionStreamOffset;
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

    const auto scene = Scene::loadSceneFromFile(renderer, "");
    
    Handle<BindGroup> meshDataBindGroup = renderer.allocateBindGroup(simpleTriangleMeshDataGroupLayout, UsageScope::async);
	renderer.updateBindGroup(meshDataBindGroup, {

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

    const auto gridSize = u32{ 1 };
    auto myTestObjects = std::vector<glm::vec3>{};
    myTestObjects.reserve(gridSize* gridSize* gridSize);
    const auto scale = 200.0f;
    for(auto i = u32{}; i < gridSize; i++)
    {
	    for(auto j = u32{}; j < gridSize; j++)
	    {
		    for(auto k = u32{}; k < gridSize; k++)
		    {
                myTestObjects.push_back(glm::vec3{ i * scale, j * scale, k * scale });
		    }
	    }
    }


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


    auto frameStartTime = std::chrono::high_resolution_clock::now();
    auto frameEndTime = std::chrono::high_resolution_clock::now();

    while (stillRunning)
    {
        const auto cpuFrameTime = frameEndTime - frameStartTime;
        frameStartTime = std::chrono::high_resolution_clock::now();
        const auto hertz = cpuFrameTime.count() / 1000000.0f;//ns -> s
        window.setWindowTitle(std::to_string(hertz));
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

        if (io.keyboardState.a == toy::io::ButtonState::pressed)
        {
            {
	            auto job = Job{&heavyWork};
				queue.push(job);
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

        if(io.keyboardState.w == toy::io::ButtonState::pressed)
        {
            camera.position += camera.forward * camera.movementSpeed * (moveCameraFaster ? 10.0f:1.0f);
        }
        if (io.keyboardState.s == toy::io::ButtonState::pressed)
        {
            camera.position -= camera.forward * camera.movementSpeed * (moveCameraFaster ? 10.0f : 1.0f);
        }
        if (io.keyboardState.a == toy::io::ButtonState::pressed)
        {
            camera.position += glm::normalize(glm::cross(camera.forward, camera.up)) * camera.movementSpeed * (moveCameraFaster ? 10.0f : 1.0f);
        }
        if (io.keyboardState.d == toy::io::ButtonState::pressed)
        {
            camera.position -= glm::normalize(glm::cross(camera.forward, camera.up)) * camera.movementSpeed * (moveCameraFaster ? 10.0f : 1.0f);
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
                const auto view = glm::lookAt(camera.position, camera.position+camera.forward, camera.up);

                const auto viewData = View
                {
                    .view = view,
                    .projection = projection,
                    .viewProjection = projection*view
                };

                std::memcpy(frameDataBeginPtr, &viewData, sizeof(View));
                frameDataBeginPtr += sizeof(View);
            }

            const auto myConstantBufferView = BufferView{ frameData.nativeHandle, {}, sizeof(View)};

            renderer.updateBindGroup(bindGroup, {
                {
                    0,
                    CBV{ myConstantBufferView }
                }
                });


            const auto& swapchainImage = renderer.acquireNextSwapchainImage();
            
            auto commandList = renderer.acquireCommandList(QueueType::graphics);
            commandList.begin();
            //TODO: this should performed on initial resource creation
            commandList.barrier({
                ImageBarrierDescriptor
                {
                    .srcLayout = Layout::undefined,
                    .dstLayout = Layout::present,
                    .image = swapchainImage.image
                }
                });
            commandList.barrier({
                ImageBarrierDescriptor
	            {
	                .srcLayout = Layout::undefined,
	                .dstLayout = Layout::depthStencilRenderTarget,
                    .aspect = ImageViewAspect::depth,
	                .image = depthFramebuffer
	            }
                });
            
                
            commandList.barrier({ 
                ImageBarrierDescriptor
            	{
            		.srcLayout = Layout::present,
                    .dstLayout = Layout::colorRenderTarget,
                    .image = swapchainImage.image
            	}
            });

            const auto renderingDescriptor = RenderingDescriptor
            {
                .colorRenderTargets = {
                    RenderTargetDescriptor
                    {
                        .imageView = swapchainImage.view,
                        .load = LoadOperation::clear,
                        .store = StoreOperation::store,
                        .resolveMode = ResolveMode::none,
                        .clearValue = ColorClear{ 100.0f/255.0f, 149.0f/255.0f, 237.0f/255.0f, 1.0f }
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

            constexpr auto area = RenderArea{ 0,0,1280,720 };

            commandList.beginRendering(renderingDescriptor, area);

            {
                constexpr auto scissor = Scissor{ 0,0,1280, 720};
                constexpr auto viewport = Viewport{ 0.0,0.0,1280.0,720.0 };


                commandList.bindPipeline(simpleTrianglePipeline);
                commandList.setScissor(scissor);
                commandList.setViewport(viewport);
                commandList.bindGroup(0, meshDataBindGroup);
                commandList.bindGroup(1, bindGroup);

                for(auto i = u32{}; i < myTestObjects.size(); i++)
                {
                    for (auto j = u32{}; j < std::clamp(objectToRender, u32{}, static_cast<u32>(scene.drawInstances_.size())); j++)
                    {
                        const auto& drawInstance = scene.drawInstances_[j];

                        Handle<BindGroup> perInstanceGroup = renderer.allocateBindGroup(simpleTrianglePerInstanceGroupLayout);

                        const auto mesh = scene.meshes_[drawInstance.meshIndex];



                        const auto instanceData = InstanceData
                        {
                            .model = glm::translate(drawInstance.model, myTestObjects[i]),
                            .clusterOffset = mesh.clusterOffset,
                            .triangleOffset = mesh.triangleOffset,
                            .positionStreamOffset = mesh.positionStreamOffset
                        };

                        const auto offset = static_cast<u32>(frameDataBeginPtr - static_cast<u8*>(frameDataPtr));
                        const auto cbv = BufferView{ frameData.nativeHandle, { offset }, sizeof(instanceData) };

                        auto ii = (void*)frameDataBeginPtr;
                        auto si = std::size_t{ 1024 * 10 * 10 };
                        frameDataBeginPtr = (u8*)std::align(64, sizeof(instanceData), ii, si);

                        std::memcpy(frameDataBeginPtr, &instanceData, sizeof(instanceData));
                        frameDataBeginPtr += sizeof(instanceData) + 64 - sizeof(instanceData) % 64;


                        renderer.updateBindGroup(perInstanceGroup, 
                            {
                            	BindingDataMapping
                            	{
                            		.binding = 0,
                            		.view = CBV{ cbv}
                            	}
                            });

                        commandList.bindGroup(2, perInstanceGroup);
                        commandList.draw(mesh.vertexCount, 1, 0, 0);
                    }
                }
            }

            commandList.endRendering();

            commandList.barrier({ 
                ImageBarrierDescriptor
            	{
            		.srcLayout = Layout::colorRenderTarget,
            		.dstLayout = Layout::present,
                    .image = swapchainImage.image
            	}
            });

            commandList.end();

            
            renderer.submitCommandList(commandList);

            //TODO: need somehow pass frame begin and end token as semaphore for present
            /*auto batch = renderer.submitCommandList(QueueType::graphics, { commandList }, {});
            renderer.submitBatches(QueueType::graphics, { batch });*/

            renderer.present();

            time += 0.01f;
            captureTool.stopAndOpenCapture();
        }
        frameEndTime = std::chrono::high_resolution_clock::now();
    }

    shouldRun = false;
    workerA.join();
    workerB.join();

    graphicsDebugger.deinitialize();
    renderer.deinitialize();
    window.deinitialize();
    logger::deinitialize();
   

    return EXIT_SUCCESS;
}
