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

#include <imgui.h>

using namespace toy::renderer;
using namespace toy::window;
using namespace compiler;


namespace 
{

    auto mapWindowIoToImGuiIo(const toy::io::WindowIo& windowIo, ImGuiIO& io) -> void
    {

        io.AddMousePosEvent(windowIo.mouseState.position.x, windowIo.mouseState.position.y);
        io.AddMouseButtonEvent(0, windowIo.mouseState.leftButton == toy::io::ButtonState::pressed);
        io.AddMouseButtonEvent(1, windowIo.mouseState.rightButton == toy::io::ButtonState::pressed);
        io.AddMouseButtonEvent(2, windowIo.mouseState.middleButton == toy::io::ButtonState::pressed);
        io.AddMouseWheelEvent(windowIo.mouseState.wheel.x, windowIo.mouseState.wheel.y);

        io.AddKeyEvent(ImGuiKey::ImGuiKey_0, windowIo.keyboardState.zero == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_1, windowIo.keyboardState.one == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_2, windowIo.keyboardState.two == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_3, windowIo.keyboardState.three == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_4, windowIo.keyboardState.four == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_5, windowIo.keyboardState.five == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_6, windowIo.keyboardState.six == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_7, windowIo.keyboardState.seven == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_8, windowIo.keyboardState.eight == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_9, windowIo.keyboardState.nine == toy::io::ButtonState::pressed);

        io.AddKeyEvent(ImGuiKey::ImGuiKey_A, windowIo.keyboardState.a == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_B, windowIo.keyboardState.b == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_C, windowIo.keyboardState.c == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_D, windowIo.keyboardState.d == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_E, windowIo.keyboardState.e == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_F, windowIo.keyboardState.f == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_G, windowIo.keyboardState.g == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_H, windowIo.keyboardState.h == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_I, windowIo.keyboardState.i == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_J, windowIo.keyboardState.j == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_K, windowIo.keyboardState.k == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_L, windowIo.keyboardState.l == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_M, windowIo.keyboardState.m == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_O, windowIo.keyboardState.o == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_P, windowIo.keyboardState.p == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Q, windowIo.keyboardState.q == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_R, windowIo.keyboardState.r == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_S, windowIo.keyboardState.s == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_T, windowIo.keyboardState.t == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_U, windowIo.keyboardState.u == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_V, windowIo.keyboardState.v == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_W, windowIo.keyboardState.w == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_X, windowIo.keyboardState.x == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Y, windowIo.keyboardState.y == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Z, windowIo.keyboardState.z == toy::io::ButtonState::pressed);

        io.AddKeyEvent(ImGuiKey::ImGuiKey_Space, windowIo.keyboardState.space == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Backspace, windowIo.keyboardState.backspace == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Enter, windowIo.keyboardState.enter == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_LeftShift, windowIo.keyboardState.shiftLeft == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_RightShift, windowIo.keyboardState.shiftRight == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_LeftBracket, windowIo.keyboardState.bracketLeft == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_RightBracket, windowIo.keyboardState.bracketRight == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Backslash, windowIo.keyboardState.backslash == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_LeftAlt, windowIo.keyboardState.altLeft == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_RightAlt, windowIo.keyboardState.altRight == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Semicolon, windowIo.keyboardState.semicolon == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Apostrophe, windowIo.keyboardState.apostroph == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Comma, windowIo.keyboardState.comma == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Period, windowIo.keyboardState.period == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Slash, windowIo.keyboardState.slash == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_GraveAccent, windowIo.keyboardState.graveAccent == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Minus, windowIo.keyboardState.minus == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Equal, windowIo.keyboardState.equel == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_LeftCtrl, windowIo.keyboardState.controlLeft == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_RightCtrl, windowIo.keyboardState.controlRight == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Escape, windowIo.keyboardState.escape == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_CapsLock, windowIo.keyboardState.capsLock == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Tab, windowIo.keyboardState.tab == toy::io::ButtonState::pressed);
        
        io.AddKeyEvent(ImGuiKey::ImGuiKey_F1, windowIo.keyboardState.f1 == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_F2, windowIo.keyboardState.f2 == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_F3, windowIo.keyboardState.f3 == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_F4, windowIo.keyboardState.f4 == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_F5, windowIo.keyboardState.f5 == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_F6, windowIo.keyboardState.f6 == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_F7, windowIo.keyboardState.f7 == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_F8, windowIo.keyboardState.f8 == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_F9, windowIo.keyboardState.f9 == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_F10, windowIo.keyboardState.f10 == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_F11, windowIo.keyboardState.f11 == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_F12, windowIo.keyboardState.f12 == toy::io::ButtonState::pressed);
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
    logger::initialize();
    auto window = SDLWindow{};
    auto renderer = api::vulkan::VulkanRenderInterface{};
    auto graphicsDebugger = debugger::RenderDocCapture{};
    auto virtualTextureStreaming = VirtualTextureStreaming{};

    window.initialize(WindowDescriptor{ 1280, 720 });
    window.setWindowTitle("Toy Engine"); // <- this couse memory allocation

    
    


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




    ImGui::CreateContext();
    
        int width, height;
        unsigned char* pixels = NULL;
        ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
        const auto texelSize = 1;
        const auto fontImageSize = width * height * texelSize;

        Flags<ImageAccessUsage> accessUsage = ImageAccessUsage::sampled;
        accessUsage |= ImageAccessUsage::transferDst;


        const auto fontDescriptor = ImageDescriptor
        {
            .format = Format::r8,
            .extent = Extent{ static_cast<u32>(width), static_cast<u32>(height)},
            .mips = 1,
            .layers = 1,
            .accessUsage = accessUsage
        };

        auto fontImage = renderer.createImage(fontDescriptor);
        ImGui::GetIO().Fonts->SetTexID((void*)&fontImage);

        const auto stagingSize = 10 * 1024 * 1024; //10MB

        const auto stagingDescriptor = BufferDescriptor
        {
            .size = stagingSize,
            .accessUsage = BufferAccessUsage::transferSrc,
            .memoryUsage = MemoryUsage::cpuOnly
        };

        auto stagingBuffer = renderer.createBuffer(stagingDescriptor, DebugLabel{ "Upload Staging Buffer" });

        void* stagingBufferDataPtr;
        renderer.map(stagingBuffer.nativeHandle, &stagingBufferDataPtr);

        TOY_ASSERT(fontImageSize <= stagingSize);

        std::memcpy(stagingBufferDataPtr, pixels, fontImageSize);


        auto uploadCommandList = renderer.acquireCommandList(QueueType::transfer);

        uploadCommandList.begin();
        uploadCommandList.barrier({ ImageBarrierDescriptor
            {
                .srcLayout = Layout::undefined,
                .dstLayout = Layout::transferDst,
                .srcStage = ResourcePipelineStageUsageFlagBits::none,
                .dstStage = ResourcePipelineStageUsageFlagBits::none,
                .aspect = ImageViewAspect::color,
                .image = fontImage
            } });

        uploadCommandList.trasfer(
            SourceBufferDescrptor
            {
                .buffer = stagingBuffer.nativeHandle,
                .offset = 0
            },
            DestinationImageDescriptor
            {
                .image = fontImage,
                .regions = { Region{ 0, 0, 1, glm::uvec3{width, height, 1 }}}
            }
            );

        uploadCommandList.barrier({ ImageBarrierDescriptor
            {
                .srcLayout = Layout::transferDst,
                .dstLayout = Layout::shaderRead,
                .srcStage = ResourcePipelineStageUsageFlagBits::none,
                .dstStage = ResourcePipelineStageUsageFlagBits::fragment,
                .aspect = ImageViewAspect::color,
                .image = fontImage
            } });
        uploadCommandList.end();

        const auto submit = renderer.submitCommandList(toy::renderer::QueueType::transfer, { uploadCommandList }, {});
        renderer.submitBatches(toy::renderer::QueueType::transfer, { submit });

    
    
    

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
    usage |= BufferAccessUsage::index;
    const auto frameData = renderer.createBuffer(BufferDescriptor
        {
            .size = 1024*1024*100,
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
                .depthTestEnabled = true,
                .faceCulling = FaceCull::back
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

#pragma region gui pipeline


    const auto guiVertexShaderGlslCode = loadShaderFile("Resources/gui.vert");
    const auto guiFragmentShaderGlslCode = loadShaderFile("Resources/gui.frag");

    const auto guiVertexShaderInfo =
        GlslRuntimeCompiler::ShaderInfo
    {
        .entryPoint = "main",
        .compilationDefines = {},
        .shaderStage = compiler::ShaderStage::vertex,
        .shaderCode = guiVertexShaderGlslCode,
        .enableDebugCompilation = true
    };

    const auto guiFragmentShaderInfo =
        GlslRuntimeCompiler::ShaderInfo
    {
        .entryPoint = "main",
        .compilationDefines = {},
        .shaderStage = compiler::ShaderStage::fragment,
        .shaderCode = guiFragmentShaderGlslCode,
        .enableDebugCompilation = true
    };


    auto guiVsSpirvCode = ShaderByteCode{};
    auto guiFsSpirvCode = ShaderByteCode{};

    {
        const auto result = GlslRuntimeCompiler::compileToSpirv(guiVertexShaderInfo, guiVsSpirvCode);
        TOY_ASSERT(result == CompilationResult::success);
    }
    {
        const auto result = GlslRuntimeCompiler::compileToSpirv(guiFragmentShaderInfo, guiFsSpirvCode);
        TOY_ASSERT(result == CompilationResult::success);
    }

    


    const auto guiVertexShaderModule = renderer.createShaderModule(toy::renderer::ShaderStage::vertex, { ShaderLanguage::spirv1_6, guiVsSpirvCode });
    const auto guiFragmentShaderModule = renderer.createShaderModule(toy::renderer::ShaderStage::fragment, { ShaderLanguage::spirv1_6, guiFsSpirvCode });


    const auto guiVertexDataGroup = BindGroupDescriptor
    {
        .bindings =
        {
            {
                .binding = 0,
                .descriptor = BindingDescriptor{BindingType::StorageBuffer}
            }
        }
    };

    const auto guiFontGroup = BindGroupDescriptor
    {
        .bindings =
        {
            {
                .binding = 0,
                .descriptor = BindingDescriptor{BindingType::Texture2D}
            },
            {
                .binding = 1,
                .descriptor = BindingDescriptor{BindingType::Sampler}
            }
        }
    };

    
    const auto guiVertexDataGroupLayout = renderer.createBindGroupLayout(guiVertexDataGroup);
    const auto guiFontGroupLayout = renderer.createBindGroupLayout(guiFontGroup);

    const auto fontView = renderer.createImageView(ImageViewDescriptor
        {
            .image = fontImage,
            .format = Format::r8,
            .type = ImageViewType::_2D
        });


    const auto fontSampler = renderer.createSampler(SamplerDescriptor{ Filter::linear, Filter::linear, MipFilter::linear });

    Handle<BindGroup> guiFontBindGroup = renderer.allocateBindGroup(guiFontGroupLayout, UsageScope::async);
    renderer.updateBindGroup(guiFontBindGroup,
        {
                {
                    0, Texture2DSRV{fontView}
                },
                {
                    1, SamplerSRV{fontSampler}
                },
        });

    struct ScaleTranslate {
        glm::vec2 scale;
        glm::vec2 translate;
    };

    const auto guiPipeline = renderer.createPipeline(
        GraphicsPipelineDescriptor
        {
            .vertexShader = guiVertexShaderModule,
            .fragmentShader = guiFragmentShaderModule,
            .renderTargetDescriptor = RenderTargetsDescriptor
            {
                .colorRenderTargets = std::initializer_list
                {
                    ColorRenderTargetDescriptor{ ColorFormat::rgba8 }
                },
                //.depthRenderTarget = DepthRenderTargetDescriptor{ DepthFormat::d32 }
            },
            .state = PipelineState
            {
                .depthTestEnabled = false,
                .faceCulling = FaceCull::none,
                .blending = Blending::alphaBlend
            }
        },
        {
            SetBindGroupMapping{0, guiVertexDataGroupLayout},
            SetBindGroupMapping{1, guiFontGroupLayout}
        },
        {
            PushConstant({ .size = sizeof(ScaleTranslate) })
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
        
        window.pollEvents();
        const auto& events = window.getEvents();
        const auto& io = window.getIo();

        auto& imGuiIo = ImGui::GetIO();
        imGuiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        imGuiIo.DeltaTime = 1.0f / 60.0f;              // set the time elapsed since the previous frame (in seconds)
        imGuiIo.DisplaySize.x = window.width();             // set the current display width
        imGuiIo.DisplaySize.y = window.height();
        mapWindowIoToImGuiIo(io, imGuiIo);



        for (const auto& event : events)
        {
            if (event == Event::quit)
            {
                stillRunning = false;
            }
        }

        ImGui::NewFrame();
        ImGui::ShowDemoWindow();



        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        
        const float PAD = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (work_pos.x + PAD);
        window_pos.y = (work_pos.y + PAD);
        window_pos_pivot.x = 0.0f;
        window_pos_pivot.y = 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        window_flags |= ImGuiWindowFlags_NoMove;
        
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

        auto statisticsOpen = true;
        if (ImGui::Begin("Statistics", &statisticsOpen, window_flags))
        {
      
            ImGui::Text("Statistics");
            ImGui::Separator();
            ImGui::Text("frame time: %.2f ms (%.1f fps)", hertz, 1000.0f/hertz);
        }
        ImGui::End();




        ImGui::EndFrame();

        if(io.keyboardState.one == toy::io::ButtonState::pressed)
        {
            if(captureTool.isRenderDocInjected())
            {
                captureTool.captureNextMarkedScope();
                LOG(INFO) << "capturing frame " << frameNumber << "...";
            }
        } 

#pragma region Camera Control
        if (!imGuiIo.WantCaptureKeyboard)
        {
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

            if (io.keyboardState.w == toy::io::ButtonState::pressed)
            {
                camera.position += camera.forward * camera.movementSpeed * (moveCameraFaster ? fastSpeed : 1.0f);
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
        }
        
        if (!imGuiIo.WantCaptureMouse)
        {
            if (io.mouseState.leftButton == toy::io::ButtonState::pressed && !mouseButtonPressed)
            {
                onMousePressedScreenLocation = glm::vec2{ io.mouseState.position.x, io.mouseState.position.y };
                mouseButtonPressed = true;
            }

            if (io.mouseState.leftButton == toy::io::ButtonState::unpressed && mouseButtonPressed)
            {
                mouseButtonPressed = false;
            }
        }
        if (imGuiIo.WantCaptureMouse && mouseButtonPressed)
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

            
            


            





            
            ImGui::Render();
            ImDrawData* drawData = ImGui::GetDrawData();

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
                renderer.beginDebugLable(QueueType::graphics, {"prepare render target"});
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

            renderer.endDebugLable(QueueType::graphics);
            renderer.beginDebugLable(QueueType::graphics, { "object rendering"});
            std::for_each(std::execution::par, std::begin(setIndicies), std::end(setIndicies), [&](auto& index)
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


            renderer.endDebugLable(QueueType::graphics);

            auto guiBatch = std::unique_ptr<Batch>{};
            {


                renderer.beginDebugLable(QueueType::graphics, DebugLabel{ "GUI" });
                auto cmd = renderer.acquireCommandList(toy::renderer::QueueType::graphics);
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
                    }/*,
                    .depthRenderTarget = RenderTargetDescriptor
                    {
                        .imageView = depthFramebufferView,
                        .load = LoadOperation::load,
                        .store = StoreOperation::store,
                        .resolveMode = ResolveMode::none,
                        .clearValue = DepthClear{ 1.0f }
                    }*/
                };

                cmd.beginRendering(renderingDescriptor, area);
               
                constexpr auto viewport = Viewport{ 0.0,720.0,1280.0,-720.0 };


                cmd.bindPipeline(guiPipeline);
                cmd.setViewport(viewport);
                cmd.bindGroup(1, guiFontBindGroup);

                const auto dataMemSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
                const auto clipOff = drawData->DisplayPos;

                for (auto i = u32{}; i < drawData->CmdListsCount; i++)
                {
                    auto guiVertexDataGroup = renderer.allocateBindGroup(guiVertexDataGroupLayout);
                    const auto cmdList = drawData->CmdLists[i];

                    const auto& indexRawData = cmdList->IdxBuffer;
                    const auto& vertexRawData = cmdList->VtxBuffer;

                    const auto indexRawDataSize = indexRawData.Size * sizeof(ImDrawIdx);
                   
                    
                    auto size = std::size_t{ frameData.size };
                    auto p = (void*)frameDataBeginPtr;
                    frameDataBeginPtr = (u8*)std::align(16, indexRawDataSize, p, size) ;

                    if (frameDataBeginPtr + indexRawDataSize > frameDataBeginPtr + frameData.size)
                    {
                        frameDataBeginPtr = (u8*)frameDataPtr;
                    }
                    auto indexBufferOffset = static_cast<u32>(frameDataBeginPtr - static_cast<u8*>(frameDataPtr));
                    std::memcpy(frameDataBeginPtr, indexRawData.Data, indexRawDataSize);
                    frameDataBeginPtr += indexRawDataSize;

                    const auto vertexRawDataSize = vertexRawData.Size * sizeof(ImDrawVert);
                    
                    size = std::size_t{ frameData.size };
                    p = (void*)frameDataBeginPtr;
                    frameDataBeginPtr = (u8*)std::align(16, vertexRawDataSize, p, size);
                    if (frameDataBeginPtr + indexRawDataSize > frameDataBeginPtr + frameData.size)
                    {
                        frameDataBeginPtr = (u8*)frameDataPtr;
                    }

                    auto vertexBufferOffset = static_cast<u32>(frameDataBeginPtr - static_cast<u8*>(frameDataPtr));
                    std::memcpy(frameDataBeginPtr, vertexRawData.Data, vertexRawDataSize);
                    frameDataBeginPtr += vertexRawDataSize;

                    
                    renderer.updateBindGroup(guiVertexDataGroup,
                        {
                            BindingDataMapping
                            {
                                .binding = 0,
                                .view = UAV
                                {
                                    .bufferView = BufferView
                                    {
                                        .buffer = frameData.nativeHandle,
                                        .offset = vertexBufferOffset,
                                        .size = (u64)(vertexRawDataSize)
                                    }
                                }
                            }
                        });

                    float scale[2];
                    scale[0] = 2.0f / drawData->DisplaySize.x;
                    scale[1] = 2.0f / drawData->DisplaySize.y;

                    const auto scaleTranslate = ScaleTranslate
                    {
                        .scale = glm::vec2(scale[0], scale[1]),
                        .translate = glm::vec2(-1.0f - drawData->DisplayPos.x * scale[0], -1.0f - drawData->DisplayPos.y * scale[1])
                    };
                    cmd.bindGroup(0, guiVertexDataGroup);


                    cmd.pushConstant(scaleTranslate);

                    for (auto j = u32{}; j < cmdList->CmdBuffer.Size; j++)
                    {
                        const auto& drawCommand = cmdList->CmdBuffer[j];

                        const auto clipMin = ImVec2(drawCommand.ClipRect.x - clipOff.x, drawCommand.ClipRect.y - clipOff.y);
                        const auto clipMax = ImVec2(drawCommand.ClipRect.z - clipOff.x, drawCommand.ClipRect.w - clipOff.y);
                        if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                            continue;
                        
                        const auto scissor = Scissor{ static_cast<i32>(clipMin.x), static_cast<i32>(clipMin.y), static_cast<u32>(clipMax.x- clipMin.x), static_cast<u32>(clipMax.y-clipMin.y) };
                        cmd.setScissor(scissor);
                        cmd.bindIndexBuffer(frameData.nativeHandle, indexBufferOffset, sizeof(ImDrawIdx) == 2 ? IndexType::index16 : IndexType::index32);
                        cmd.drawIndexed(drawCommand.ElemCount, 1, drawCommand.IdxOffset, drawCommand.VtxOffset, 0);
                    }
                }
                cmd.endRendering();
                cmd.end();
                guiBatch = std::make_unique<Batch>(renderer.submitCommandList(toy::renderer::QueueType::graphics, { cmd },
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

                renderer.submitBatches(QueueType::graphics, { *guiBatch });
                renderer.endDebugLable(QueueType::graphics);
            }
            
            {
                renderer.beginDebugLable(QueueType::graphics, {"prepare present"});
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
                        guiBatch->barrier()
                    }));
            }


            renderer.submitBatches(QueueType::graphics, { *postRenderingBatch });
            renderer.present(postRenderingBatch->barrier());

            time += 0.01f;
            captureTool.stopAndOpenCapture();
        }
        frameEndTime = std::chrono::high_resolution_clock::now();
    }

    ImGui::DestroyContext();
    graphicsDebugger.deinitialize();
    renderer.deinitialize();
    window.deinitialize();
    logger::deinitialize();

    return EXIT_SUCCESS;
}
