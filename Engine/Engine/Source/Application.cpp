#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define IMGUI_DEFINE_MATH_OPERATORS
#include <glm/glm.hpp>
#include <glm/ext/matrix_common.hpp>
#include <glm/gtc/matrix_transform.hpp>

//TODO: Move glm defines into its own common header

#include "Application.h"
#include <Core.h>
#include <fstream>
#include <GlslRuntimeCompiler.h>
#include <Logger.h>
#include <RenderDocCapture.h>
#include <Scene.h>

#include <SDLWindow.h>

#include <thread>
#include <mutex>
#include <rigtorp/MPMCQueue.h>
#include <iostream>
#include <chrono>
#include <VirtualTextureStreaming.h>

#include <RenderInterface.h>
#include <RenderInterfaceTypes.h>

#include "SceneLoader.h"

#include <execution>
#include <algorithm>
#include <span>
#include <stack>

#define IMGUI_USER_CONFIG "toyimconfig.h"
#include <imgui.h>
#include "IconsFontAwesome6.h"

#include <imgui_node_editor.h>

#include "OutlineFeature.h"

using namespace toy::graphics::rhi;

using namespace toy::window;
using namespace toy::graphics::compiler;
using namespace toy::graphics;

namespace ed = ax::NodeEditor;


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
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Apostrophe, windowIo.keyboardState.apostrophe == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Comma, windowIo.keyboardState.comma == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Period, windowIo.keyboardState.period == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Slash, windowIo.keyboardState.slash == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_GraveAccent, windowIo.keyboardState.graveAccent == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Minus, windowIo.keyboardState.minus == toy::io::ButtonState::pressed);
        io.AddKeyEvent(ImGuiKey::ImGuiKey_Equal, windowIo.keyboardState.equal == toy::io::ButtonState::pressed);
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

namespace ImGui
{
    void ToggleButton(const char* label, bool* toggleValue, const ImVec2& size = ImVec2(0, 0))
    {
        const auto color1 = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
        const auto color2 = ImGui::GetStyle().Colors[ImGuiCol_Button];

        ImGui::PushID(1);
        
        if (*toggleValue)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, color1);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, color2);
            if (ImGui::Button(label, size))
            {
                
                *toggleValue = false;
            }
            ImGui::PopStyleColor(2);
        }
        else
        {            
            ImGui::PushStyleColor(ImGuiCol_Button, color2);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, color1);
            if (ImGui::Button(label, size))
            {
                *toggleValue = true;
            }
            ImGui::PopStyleColor(2);
        }
        ImGui::PopID();
    }
}

int Application::run()
{
    logger::initialize();
    auto window = SDLWindow{};
    auto renderer = RenderInterface{};
    auto graphicsDebugger = debugger::RenderDocCapture{};
    auto virtualTextureStreaming = VirtualTextureStreaming{};


    const auto windowWidth = u32{1920};//u32{2560};
    const auto windowHeight = u32{1080};//u32{1440};

    window.initialize(WindowDescriptor{ windowWidth, windowHeight });
    window.setWindowTitle("Toy Engine"); // <- this course memory allocation

    auto materialEditor = toy::editor::materials::MaterialEditor{};

    materialEditor.initialize();


    const auto workerCount = 10;

    const auto rendererDescriptor = toy::graphics::rhi::RendererDescriptor
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
    
    auto outlineFeature = features::OutlineFeature(renderer);
    outlineFeature.initialize();


    ImGui::CreateContext();

    float baseFontSize = 16.0f;
    float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

    ImGui::GetIO().Fonts->AddFontFromFileTTF("Resources/Fonts/Roboto-Medium.ttf", baseFontSize);

    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig config;
    config.MergeMode = true;
    config.PixelSnapH = true;
    config.GlyphMinAdvanceX = iconFontSize;
    
    ImGui::GetIO().Fonts->AddFontFromFileTTF("Resources/fa-solid-900.ttf", baseFontSize, &config, icons_ranges);

    int width, height;
    unsigned char* pixels = NULL;
    ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
    const auto texelSize = 1;
    const auto fontImageSize = width * height * texelSize;

    ed::Config nodeConfig;
    nodeConfig.SettingsFile = "Simple.json";
    auto nodeContext = ed::CreateEditor(&nodeConfig);


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
    renderer.map(stagingBuffer, &stagingBufferDataPtr);

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

    uploadCommandList.transfer(
        SourceBufferDescriptor
        {
            .buffer = stagingBuffer,
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

    const auto submit = renderer.submitCommandList(QueueType::transfer, { uploadCommandList }, {});
    renderer.submitBatches(QueueType::transfer, { submit });

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

    const auto frameDataSize = u32{ 1024 * 1024 * 100 };

    const auto frameData = renderer.createBuffer(BufferDescriptor
        {
            .size = frameDataSize,
            .accessUsage = usage,
            .memoryUsage = MemoryUsage::cpuOnly,
        });

    void* frameDataPtr = nullptr;
    renderer.map(frameData, &frameDataPtr);
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

    const auto vertexShaderModule = renderer.createShaderModule(toy::graphics::rhi::ShaderStage::vertex, { ShaderLanguage::spirv1_6, simpleTriangleVsSpirvCode });

    const auto fragmentShaderModule = renderer.createShaderModule(toy::graphics::rhi::ShaderStage::vertex, { ShaderLanguage::spirv1_6, simpleTriangleFsSpirvCode });

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

    


    const auto guiVertexShaderModule = renderer.createShaderModule(toy::graphics::rhi::ShaderStage::vertex, { ShaderLanguage::spirv1_6, guiVsSpirvCode });
    const auto guiFragmentShaderModule = renderer.createShaderModule(toy::graphics::rhi::ShaderStage::fragment, { ShaderLanguage::spirv1_6, guiFsSpirvCode });


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
        float movementSpeedScale{ 1.0f };
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
    const auto splashData = "E:\\Develop\\ToyEngineContent\\splash.dat";

    const auto scene = Scene::loadSceneFromFile(renderer, splashData);
    
    Handle<BindGroup> meshDataBindGroup = renderer.allocateBindGroup(simpleTriangleMeshDataGroupLayout, UsageScope::async);
	renderer.updateBindGroup(meshDataBindGroup, 
        {
				{
					0, UAV{BufferView{ scene.positionStream_, 0, (~0ULL)}}
				},
			    {
					1, UAV{BufferView{ scene.uvStream_, 0, (~0ULL)}}
				},
			    {
					2, UAV{BufferView{ scene.tangentFrameStream_, 0, (~0ULL)}}
				},
				{
					3, UAV{BufferView{ scene.triangle_, 0, (~0ULL)}}
				},
				{
					4, UAV{BufferView{ scene.clusters_, 0, (~0ULL)}}
				}
		});

    //TODO: [#3] command submit should work also without calling nextFrame in a frame async scenario
    auto time = 0.0f;
    auto frameNumber = u32{};
    bool stillRunning = true;
    auto captureTool = graphicsDebugger.getScopeCapture();

    auto frameStartTime = std::chrono::high_resolution_clock::now();
    auto frameEndTime = std::chrono::high_resolution_clock::now();

    SubmitBatch prepareBatch;
    auto prepareBatchValide = false;
    auto perThreadSubmits = std::vector<SubmitBatch>{};
	perThreadSubmits.resize(10);
    SubmitBatch postRenderingBatch;


    struct SceneDrawStaticstics
    {
        u32 drawCalls{};
        u32 totalTrianglesCount{};
    };

    struct GuiDrawStatistics
    {
        u32 drawCalls{};
        u32 totalIndicesCount{};
        u32 totalVerticesCount{};
    };

    struct DrawStatistics
    {
        SceneDrawStaticstics scene{};
        GuiDrawStatistics gui{};
    };

    struct PerThreadDrawStatistics
    {
        alignas(std::hardware_destructive_interference_size) SceneDrawStaticstics statistics {};
    };

    auto perRenderThreadDrawStatistics = std::vector<PerThreadDrawStatistics>{};
    perRenderThreadDrawStatistics.resize(workerCount);

    auto drawStatistics = DrawStatistics{};

    while (stillRunning)
    {
        const auto cpuFrameTime = frameEndTime - frameStartTime;
        frameStartTime = std::chrono::high_resolution_clock::now();
        const auto hertz = cpuFrameTime.count() / 1000000000.0f;//ns -> s
        
        window.pollEvents();
        const auto& events = window.getEvents();
        const auto& io = window.getIo();

        auto& imGuiIo = ImGui::GetIO();
        imGuiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        imGuiIo.DisplaySize.x = window.width();
        imGuiIo.DisplaySize.y = window.height();
        imGuiIo.DeltaTime = hertz;
        mapWindowIoToImGuiIo(io, imGuiIo);
        imGuiIo.MouseDrawCursor = true;

        for (const auto& event : events)
        {
            if (event == Event::quit)
            {
                stillRunning = false;
            }
        }

        ImGui::NewFrame();
        ImGui::ShowDemoWindow();

#pragma region scene editor

        auto showSceneHierarchy = true;
        //ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Scene Hierarchy", &showSceneHierarchy))
        {
            static int selected = 0;
            {
                ImGui::BeginChild("left pane");
                for (int i = 0; i < 100; i++)
                {
                    // FIXME: Good candidate to use ImGuiSelectableFlags_SelectOnNav
                    char label[128];
                    sprintf(label, "MyObject %d", i);
                    if (ImGui::Selectable(label, selected == i))
                        selected = i;
                }
                ImGui::EndChild();
            }
        }
        ImGui::End();



#pragma endregion
        if (ImGui::Begin("Node Editor"))
        {
            ed::SetCurrentEditor(nodeContext);
            ed::Begin("My Editor", ImVec2(0.0, 0.0f));
            int uniqueId = 1;
            // Start drawing nodes.
            ed::BeginNode(uniqueId++);
            ImGui::Text("Node A");
            ed::BeginPin(uniqueId++, ed::PinKind::Input);
            ImGui::Text("-> In");
            ed::EndPin();
            ImGui::SameLine();
            ed::BeginPin(uniqueId++, ed::PinKind::Output);
            ImGui::Text("Out ->");
            ed::EndPin();
            ed::EndNode();
            ed::End();
            ed::SetCurrentEditor(nullptr);
        }
        ImGui::End();

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        
        const float pad = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const auto workPos = viewport->WorkPos;
        const auto workSize = viewport->WorkSize;
        auto nextWindowPos = ImVec2{};
        auto windowPosPivot = ImVec2{};
        nextWindowPos.x = (workPos.x + pad);
        nextWindowPos.y = (workPos.y + pad);
        windowPosPivot.x = 0.0f;
        windowPosPivot.y = 0.0f;
        ImGui::SetNextWindowPos(nextWindowPos, ImGuiCond_Always, windowPosPivot);
        windowFlags |= ImGuiWindowFlags_NoMove;
        ImGui::SetNextWindowBgAlpha(0.55f);

        auto toolsBar = true;
        static auto showStatistics = false;
        static auto showCameraControls = false;
        if (ImGui::Begin("ToolsBar", &toolsBar, windowFlags))
        {
#ifdef TOY_ENGINE_ENABLE_RENDER_DOC_CAPTURING
            ImGui::PushID(1);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(150.0 / 360.0, 68.0f / 100.0f, 72.0f / 100.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(150.0 / 360.0, 68.0f / 100.0f, 90.0f / 100.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(150.0 / 360.0, 68.0f / 100.0f, 60.0f / 100.0f));
            if (ImGui::Button(ICON_FA_BUG))
            {
                if (captureTool.isRenderDocInjected())
                {
                    captureTool.captureNextMarkedScope();
                    LOG(INFO) << "capturing frame " << frameNumber << "...";
                }
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("RenderDoc capture");
            ImGui::PopStyleColor(3);
            ImGui::PopID();
            ImGui::SameLine();
#endif
            ImGui::ToggleButton(ICON_FA_VIDEO, &showCameraControls);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Show camera settings");
            ImGui::SameLine();
            ImGui::ToggleButton(ICON_FA_CHART_LINE, &showStatistics);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Show statistics");
            nextWindowPos.y = (nextWindowPos.y + pad + ImGui::GetWindowHeight());
        }
        ImGui::End();

        if (showStatistics)
        {
            ImGui::SetNextWindowPos(nextWindowPos, ImGuiCond_Always, windowPosPivot);
            windowFlags |= ImGuiWindowFlags_NoMove;
            ImGui::SetNextWindowBgAlpha(0.35f);

            auto windowOpen = true;
            if (ImGui::Begin("Statistics", &windowOpen, windowFlags))
            {
                ImGui::SeparatorText("Timings");
                ImGui::Text("frame time: %.2f ms (%.1f fps)", hertz * 1000.0f, 1.0f / hertz);
                ImGui::SeparatorText("Scene");
                ImGui::Text("Total triangles: %d", drawStatistics.scene.totalTrianglesCount);
                ImGui::Text("Total drawcalls: %d", drawStatistics.scene.drawCalls);
                ImGui::SeparatorText("GUI");
                ImGui::Text("Total indices: %d", drawStatistics.gui.totalIndicesCount);
                ImGui::Text("Total vertices: %d", drawStatistics.gui.totalVerticesCount);
                ImGui::Text("Total drawcalls: %d", drawStatistics.gui.drawCalls);
                ImGui::SeparatorText("GPU Memory");

                auto drawList = ImGui::GetWindowDrawList();

                const auto itemWidth = 200u;
                const auto itemHeight = 20u;

                const auto budget = renderer.requestMemoryBudget();

                for(auto i = u32{}; i < budget.heapCount; i++)
                {
                    const auto total = budget.budget[i].totalMemory/(1024*1024);
                    const auto available = budget.budget[i].availableMemory/(1024*1024);
                    const auto budgetValue = budget.budget[i].budgetMemory/(1024*1024);

                    auto type = std::string{};
                    switch(budget.budget[i].type)
                    {
                    case HeapType::device:
                        type = "VRAM";
                        break;
                    case HeapType::host:
                        type = "RAM";
                        break;
                    case HeapType::coherent:
                        type = "ReBAR";
                        break;
                    }

	                ImGui::Text("");
	                ImGui::SameLine(60);
	                ImGui::Text("heap %i, (%s), (%i MB/%i MB/%i MB)", i,type.c_str(),  available, budgetValue, total);


	                ImVec2 p = ImGui::GetCursorScreenPos();
	                drawList->AddRectFilled(p, ImVec2(p.x + itemWidth, p.y + 20), glm::packUnorm4x8(glm::vec4(0.2, 0.2, 0.2, 0.7)));
	                drawList->AddRectFilled(p, ImVec2(p.x + itemWidth*(budgetValue)/(float)total, p.y + 20), glm::packUnorm4x8(glm::vec4(0.1, 0.7, 0.1, 0.7)));
	                drawList->AddRectFilled(p, ImVec2(p.x + itemWidth*(available)/(float)total,p.y + 20), glm::packUnorm4x8(glm::vec4(0.7,0.1,0.1,0.7)));
	                ImGui::Dummy(ImVec2(itemWidth, 20));

                    
                }

                nextWindowPos.y = (nextWindowPos.y + pad + ImGui::GetWindowHeight());

                static std::stack<Handle<Buffer>> allocations;

                    if(ImGui::Button("allocate memory"))
                    {
                        const auto desc = BufferDescriptor
                        {
                            .size = 1024*1024*100,
                            .accessUsage = BufferAccessUsage::storage,
                            .memoryUsage = MemoryUsage::gpuOnly
                        };
                        auto b = renderer.createBuffer(desc);
	                    allocations.push(b);
                    }

                    if(ImGui::Button("dealocate memory"))
                    {
                        if(!allocations.empty())
                        {
                            const auto b = allocations.top();
                            renderer.destroyBuffer(b);
                            allocations.pop();
                        }
                    }

                    ImGui::LabelText("allocations", "%i", allocations.size());
            }
            ImGui::End();
        }
        if (showCameraControls)
        {
            ImGui::SetNextWindowPos(nextWindowPos, ImGuiCond_Always, windowPosPivot);
            windowFlags |= ImGuiWindowFlags_NoMove;
            ImGui::SetNextWindowBgAlpha(0.35f);

            auto windowOpen = true;
            if (ImGui::Begin("Camera controls", &windowOpen, windowFlags))
            {
                ImGui::SeparatorText("Camera");
                ImGui::SliderFloat("Speed", &camera.movementSpeedScale, 1.0f, 15.0f);
                nextWindowPos.y = (nextWindowPos.y + pad + ImGui::GetWindowHeight());
            }
            ImGui::End();
        }




        viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (work_pos.x + work_size.x);
        window_pos.y = (work_pos.y);
        window_pos_pivot.x = 1.0f;
        window_pos_pivot.y = 0.0f;
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    	auto windowControlBar = true;
        ImGui::PushID(1);
    	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, glm::packUnorm4x8(glm::vec4(0.9, 0.1, 0.1, 0.2)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, glm::packUnorm4x8(glm::vec4(0.9, 0.1, 0.1, 0.4)));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, glm::packUnorm4x8(glm::vec4(0.9, 0.1, 0.1, 0.8)));
        ImGui::PushStyleColor(ImGuiCol_Text, glm::packUnorm4x8(glm::vec4(0.9, 0.1, 0.1, 1.0)));
        if (ImGui::Begin("WindowControBar", &windowControlBar, windowFlags))
        {
            if (ImGui::Button(ICON_FA_CIRCLE_XMARK))
            {
                stillRunning = false;
            }
        }
        ImGui::End();
        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();
        ImGui::PopID();

        ImGui::EndFrame();

#pragma region Camera Control
        if (!imGuiIo.WantCaptureKeyboard)
        {
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
                camera.position += camera.forward * camera.movementSpeed * (moveCameraFaster ? fastSpeed : 1.0f) * camera.movementSpeedScale;
            }
            if (io.keyboardState.s == toy::io::ButtonState::pressed)
            {
                camera.position -= camera.forward * camera.movementSpeed * (moveCameraFaster ? fastSpeed : 1.0f) * camera.movementSpeedScale;
            }
            if (io.keyboardState.a == toy::io::ButtonState::pressed)
            {
                camera.position += glm::normalize(glm::cross(camera.forward, camera.up)) * camera.movementSpeed * (moveCameraFaster ? fastSpeed : 1.0f) * camera.movementSpeedScale;
            }
            if (io.keyboardState.d == toy::io::ButtonState::pressed)
            {
                camera.position -= glm::normalize(glm::cross(camera.forward, camera.up)) * camera.movementSpeed * (moveCameraFaster ? fastSpeed : 1.0f) * camera.movementSpeedScale;
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

            const auto b = renderer.requestMemoryBudget();

            ImGui::Render();
            const auto drawData = ImGui::GetDrawData();

            auto bindGroup = renderer.allocateBindGroup(simpleTriangleGroupLayout);

            auto frameDataBeginPtr = static_cast<u8*>(frameDataPtr);

            {
                const auto aspectRatio = static_cast<float>(window.width()) / static_cast<float>(window.height());
                const auto projection = glm::perspective(glm::radians(60.0f), aspectRatio, 1000.0f, 0.001f);//inverse z trick
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

            const auto myConstantBufferView = BufferView{ frameData, {}, sizeof(View) };

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
            batches[batchSize - std::size_t{1}] = instances.subspan(batchSpanOffset, scene.drawInstances_.size() - batchSpanOffset);

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
                    .clearValue = DepthClear{ 0.0f }
                }
            };

            constexpr auto area = RenderArea{ 0,0,windowWidth,windowHeight };

            {
                renderer.beginDebugLabel(QueueType::graphics, {"prepare render target"});
                auto cmd = renderer.acquireCommandList(toy::graphics::rhi::QueueType::graphics);
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

                if(prepareBatchValide)
                {
                    auto dependency = postRenderingBatch.barrier();
	                prepareBatch = renderer.submitCommandList(QueueType::graphics, { cmd }, {dependency});
                }
                else
                {
	                prepareBatch = renderer.submitCommandList(QueueType::graphics, { cmd }, {});
                    prepareBatchValide = true;
                }
            }
            renderer.submitBatches(QueueType::graphics, { prepareBatch });


            auto perInstanceGroup = renderer.allocateBindGroup(simpleTrianglePerInstanceGroupLayout);
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
                                .buffer = frameData,
                                .offset = batchOffset,
                                .size = (u64)(dataMemSize * scene.drawInstances_.size())
                            }
                        }
                    }
                });

            renderer.endDebugLabel(QueueType::graphics);
            renderer.beginDebugLabel(QueueType::graphics, { "object rendering"});
            std::for_each(std::execution::par, std::begin(setIndicies), std::end(setIndicies), [&](auto& index)
            {
                auto& drawStatistics = perRenderThreadDrawStatistics[index].statistics;
                drawStatistics = SceneDrawStaticstics{};
                auto& drawInstances = batches[index];
                auto cmd = renderer.acquireCommandList(toy::graphics::rhi::QueueType::graphics, WorkerThreadId{ .index = static_cast<u32>(index % workerCount)});
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
                    const auto scissor = Scissor{ 0,0,windowWidth, windowHeight };
                    const auto viewport = Viewport{ 0.0,0.0,(float)windowWidth, (float)windowHeight };


                    cmd.bindPipeline(simpleTrianglePipeline);
                    cmd.setScissor(scissor);
                    cmd.setViewport(viewport);
                    cmd.bindGroup(0, meshDataBindGroup);
                    cmd.bindGroup(1, bindGroup);
                    cmd.bindGroup(2, perInstanceGroup);

                        
                    auto setOffset = batchOffsets[index] * dataMemSize;
                        
                    for (auto j = u32{}; j < static_cast<u32>(drawInstances.size()); j++)
                    {
                        const auto& drawInstance = drawInstances[j];


                        //scene.meshes_[drawInstance.meshIndex].lods[0].
                        const auto& mesh = scene.meshes_[drawInstance.meshIndex];

                        //this scope should be thread safe. More precisely, memory allocation should be thread safe. Appropriate strategy is to allocate block of memory for each render thread up front. [see Miro board, multithreaded per frame dynamic allocator]

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
                        
                    for (auto j = u32{}; j < static_cast<u32>(drawInstances.size()); j++)
                    {
                        const auto& drawInstance = drawInstances[j];
                        const auto& mesh = scene.meshes_[drawInstance.meshIndex];


                        const u32 value = j + static_cast<u32>(batchOffsets[index]);
                        cmd.pushConstant(value);
                        cmd.draw(mesh.vertexCount, 1, 0, 0);
                        drawStatistics.totalTrianglesCount += mesh.vertexCount / 3;
                        drawStatistics.drawCalls++;
                    }
                        
                }

                cmd.endRendering();
                cmd.end();

                auto perThreadBatch = renderer.submitCommandList(QueueType::graphics, { cmd }, { prepareBatch.barrier() });
                perThreadSubmits[index] = perThreadBatch;

            });

            auto gatheredStatistics = std::vector<SceneDrawStaticstics>{};
            gatheredStatistics.resize(perRenderThreadDrawStatistics.size());
                
            std::transform(perRenderThreadDrawStatistics.begin(), perRenderThreadDrawStatistics.end(), gatheredStatistics.begin(), [](auto& a) {return a.statistics; });

            drawStatistics.scene = std::accumulate(gatheredStatistics.begin(), gatheredStatistics.end(), SceneDrawStaticstics{},
                [](SceneDrawStaticstics a, SceneDrawStaticstics& b)
                {
                    SceneDrawStaticstics c;
                    c.drawCalls= a.drawCalls + b.drawCalls;
                    c.totalTrianglesCount = a.totalTrianglesCount + b.totalTrianglesCount;
                    return c;
                });


            renderer.submitBatches(QueueType::graphics, perThreadSubmits);

            renderer.endDebugLabel(QueueType::graphics);

            auto guiBatch = SubmitBatch{};
            {
                drawStatistics.gui = GuiDrawStatistics{};
                renderer.beginDebugLabel(QueueType::graphics, DebugLabel{ "GUI" });
                auto cmd = renderer.acquireCommandList(toy::graphics::rhi::QueueType::graphics);
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
                    }
                };

                cmd.beginRendering(renderingDescriptor, area);
               
                const auto viewport = Viewport{ 0.0, (float)windowHeight,(float)windowWidth, -(float)windowHeight };

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

                    drawStatistics.gui.totalIndicesCount += indexRawData.Size;
                    drawStatistics.gui.totalVerticesCount += vertexRawData.Size;

                    const auto indexRawDataSize = indexRawData.Size * sizeof(ImDrawIdx);
                   
                    
                    auto size = std::size_t{ frameDataSize };
                    auto p = (void*)frameDataBeginPtr;
                    frameDataBeginPtr = (u8*)std::align(16, indexRawDataSize, p, size) ;

                    if (frameDataBeginPtr + indexRawDataSize > frameDataBeginPtr + frameDataSize)
                    {
                        frameDataBeginPtr = (u8*)frameDataPtr;
                    }
                    auto indexBufferOffset = static_cast<u32>(frameDataBeginPtr - static_cast<u8*>(frameDataPtr));
                    std::memcpy(frameDataBeginPtr, indexRawData.Data, indexRawDataSize);
                    frameDataBeginPtr += indexRawDataSize;

                    const auto vertexRawDataSize = vertexRawData.Size * sizeof(ImDrawVert);
                    
                    size = std::size_t{ frameDataSize };
                    p = (void*)frameDataBeginPtr;
                    frameDataBeginPtr = (u8*)std::align(16, vertexRawDataSize, p, size);
                    if (frameDataBeginPtr + indexRawDataSize > frameDataBeginPtr + frameDataSize)
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
                                        .buffer = frameData,
                                        .offset = vertexBufferOffset,
                                        .size = (u64)(vertexRawDataSize)
                                    }
                                }
                            }
                        });

                    const auto scale = std::array<float, 2>{2.0f / drawData->DisplaySize.x, 2.0f / drawData->DisplaySize.y};
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
                        cmd.bindIndexBuffer(frameData, indexBufferOffset, sizeof(ImDrawIdx) == 2 ? IndexType::index16 : IndexType::index32);
                        cmd.drawIndexed(drawCommand.ElemCount, 1, drawCommand.IdxOffset, drawCommand.VtxOffset, 0);
                        drawStatistics.gui.drawCalls++;
                    }
                }
                cmd.endRendering();
                cmd.end();

                auto submits = std::vector<SubmitDependency>{};
            	submits.resize(10);
                std::transform(perThreadSubmits.begin(), perThreadSubmits.end(), submits.begin(), [](auto& a){ return a.barrier();} );
                guiBatch = renderer.submitCommandList(toy::graphics::rhi::QueueType::graphics, { cmd }, submits);

                renderer.submitBatches(QueueType::graphics, { guiBatch });
                renderer.endDebugLabel(QueueType::graphics);
            }
            
            {
                renderer.beginDebugLabel(QueueType::graphics, {"prepare present"});
                auto cmd = renderer.acquireCommandList(QueueType::graphics);
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
                postRenderingBatch = renderer.submitCommandList(QueueType::graphics, { cmd }, {guiBatch.barrier() });
            }

            renderer.submitBatches(QueueType::graphics, { postRenderingBatch} );


            outlineFeature.render();

            renderer.present(postRenderingBatch.barrier());

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
