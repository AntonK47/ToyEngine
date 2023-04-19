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
#include <regex>

#define IMGUI_USER_CONFIG "toyimconfig.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include "IconsFontAwesome6.h"

#include <imgui_node_editor.h>

#include "OutlineFeature.h"

#include "MaterialEditor.h"
#include "Editor.h"
#include "ImageDataUploader.h"
#include "TextureManager.h"

#include "DDSLoader.h"
using namespace toy::io::loaders::dds;

//#include <compressonator.h>

using namespace toy::graphics::rhi;

using namespace toy::window;
using namespace toy::graphics::compiler;
using namespace toy::graphics;
using namespace toy::editor;
using namespace toy;

namespace 
{

	/*struct Material
	{
	};*/

	struct SharedMaterial
	{
		Handle<Pipeline> pipeline;
	};

	struct MaterialInstance
	{
		u32 sharedMaterialIndex;
		u32 instanceDataIndex;
	};

	struct MaterialTemplate
	{
		std::string vertexShaderTemplate;
		std::string fragmentShaderTemplate;
	};

	struct FrameContext
	{
		Handle<BindGroupLayout> texturesLayout;
		Handle<BindGroupLayout> geometryLayout;
		Handle<BindGroupLayout> perInstanceDataLayout;
		Handle<BindGroupLayout> perFrameDataLayout;
	};

	struct MaterialCompiolationDescriptor
	{
		const MaterialTemplate& materialTemplate;
		const FrameContext& frameContext;
		const MaterialAsset& asset;
	};


	struct MaterialGenerator
	{
		static auto compilePipeline(RenderInterface& renderer, const MaterialCompiolationDescriptor& descriptor) ->  SharedMaterial
		{
			const std::string generatedFragmentCode =
				"vec3 evaluateMaterialLighting(Attributes attributes)"
				"{"
				"	vec3 diffuse = texture(sampler2D(textures2D[6], textureSampler), vec2(uv)).xyz;"
				"   vec3 normalTS = texture(sampler2D(textures2D[7], textureSampler), vec2(uv)).xyz;"
				"    vec3 normalWS = ntb * normalTS;"
				"    vec3 lightDirection = normalize(vec3(1.0, 1.0, 1.0));"
				"    float radiance = max(0, dot(lightDirection, normalize(normalWS)));"
				"    vec3 shadowColor = vec3(130.0/255.0, 163.0/255.0, 255.0/255.0) * 0.15;"
				"    vec3 diffuseColor = vec3(0.9, 0.4, 0.1);"
				"    vec3 color = mix(shadowColor, diffuse, radiance);"
				"    return color;"
				"}";

			const std::string evaluateMaterialFragment
				= "evaluateMaterialLighting";

			auto generatedVertexShaderGlslCode = descriptor.materialTemplate.vertexShaderTemplate;
			auto generatedFragmentShaderGlslCode = descriptor.materialTemplate.fragmentShaderTemplate;

			generatedFragmentShaderGlslCode = std::regex_replace(generatedFragmentShaderGlslCode, std::regex{ std::string{"<\\[generated_fragment_code\\]>"} }, generatedFragmentCode);
			generatedFragmentShaderGlslCode = std::regex_replace(generatedFragmentShaderGlslCode, std::regex{ std::string{"<\\[evaluate_material_fragment\\]>"} }, evaluateMaterialFragment);
			


			const auto vertexShaderInfo =
				GlslRuntimeCompiler::ShaderInfo
			{
				.entryPoint = "main",
				.compilationDefines = {},
				.shaderStage = compiler::ShaderStage::vertex,
				.shaderCode = generatedVertexShaderGlslCode,
				.enableDebugCompilation = true
			};

			const auto fragmentShaderInfo =
				GlslRuntimeCompiler::ShaderInfo
			{
				.entryPoint = "main",
				.compilationDefines = {},
				.shaderStage = compiler::ShaderStage::fragment,
				.shaderCode = generatedFragmentShaderGlslCode,
				.enableDebugCompilation = true
			};


			auto vsSpirvCode = ShaderByteCode{};
			auto fsSpirvCode = ShaderByteCode{};

			auto result = GlslRuntimeCompiler::compileToSpirv(vertexShaderInfo, vsSpirvCode);
			TOY_ASSERT(result == CompilationResult::success);

			result = GlslRuntimeCompiler::compileToSpirv(fragmentShaderInfo, fsSpirvCode);
			TOY_ASSERT(result == CompilationResult::success);

			const auto vertexShaderModule = renderer.createShaderModule(
				toy::graphics::rhi::ShaderStage::vertex, 
				{ 
					ShaderLanguage::spirv1_6,
					vsSpirvCode
				}
			);

			const auto fragmentShaderModule = renderer.createShaderModule(
				toy::graphics::rhi::ShaderStage::vertex,
				{
					ShaderLanguage::spirv1_6,
					fsSpirvCode 
				}
			);

			const auto pipeline = renderer.createPipeline(
				GraphicsPipelineDescriptor
				{
					.vertexShader = vertexShaderModule,
					.fragmentShader = fragmentShaderModule,
					.renderTargetDescriptor = RenderTargetsDescriptor
					{
						.colorRenderTargets = std::vector
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
					SetBindGroupMapping{0, descriptor.frameContext.geometryLayout},
					SetBindGroupMapping{1, descriptor.frameContext.perFrameDataLayout},
					SetBindGroupMapping{2, descriptor.frameContext.perInstanceDataLayout},
					SetBindGroupMapping{3, descriptor.frameContext.texturesLayout}
				},
				{
					PushConstant({.size = sizeof(u32)})
				}
			);

			SharedMaterial sharedMaterial;
			sharedMaterial.pipeline = pipeline;
			return sharedMaterial;
		}
	};


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


struct TextureDataSource
{
	std::string path;
};

template<toy::io::loaders::dds::TextureDataSourceDescriptor T>
struct TextureDescriptor
{
	 T source;
};


//struct TextureManager
//{
//	template<toy::io::loaders::dds::TextureDataSourceDescriptor T>
//	auto addTextureFromSource(const TextureDescriptor<T> descriptor);
//};


int Application::run()
{
	//CMP_InitFramework();

	logger::initialize();
	auto window = SDLWindow{};
	auto renderer = RenderInterface{};
	auto graphicsDebugger = debugger::RenderDocCapture{};
	auto virtualTextureStreaming = VirtualTextureStreaming{};
	auto editor = Editor{};
	auto textureUploader = ImageDataUploader{};
	auto textureManager = TextureManager{};


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
	textureUploader.initialize(renderer, 100 * 1024 * 1024);

	const auto textureManagerDescriptor = TextureManagerDescriptor
	{
		.rhi = renderer,
		.frameInFlights = 3 //TODO:: this should be defined upfront
	};

	textureManager.initialize(textureManagerDescriptor);

	//editor.initialize(renderer, textureUploader);




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

	const auto fontView = renderer.createImageView(ImageViewDescriptor
	{
		.image = fontImage,
		.format = Format::r8,
		.type = ImageViewType::_2D
	});

	const auto fontSampler = renderer.createSampler(SamplerDescriptor{ Filter::linear, Filter::linear, MipFilter::linear });


	const auto fontTexture = toy::Texture2D
	{
		.width = (u32)width,
		.height = (u32)height,
		.image = fontImage,
		.view = fontView,
		.hasMips = false,
		.bytesPerTexel = texelSize
	};

	textureUploader.upload(std::vector<std::byte>( (std::byte*)pixels, (std::byte*)pixels + fontImageSize ), fontTexture);

	const auto renderDocDescriptor = debugger::RenderDocCaptureDescriptor
	{
		.nativeBackend = renderer.getNativeBackend()
	};
	graphicsDebugger.initialize(renderDocDescriptor);
	
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

	
	
	const auto samplingGroupDescriptor = BindGroupDescriptor
	{
		.bindings =
		{
			{
				.binding = 0,
				.descriptor = BindingDescriptor{BindingType::Sampler}
			}
		},
		.flags = BindGroupFlag::unboundLast
	};
	const auto samplingGroupLayout = renderer.createBindGroupLayout(samplingGroupDescriptor, DebugLabel{ .name = "textureSemplers" });
	const auto samplingGroup = renderer.allocateBindGroup(samplingGroupLayout, UsageScope::async, DebugLabel{ "textureSemplers" });
	
	

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


	const auto vertexShaderGlslTemplateCode = loadShaderFile("Resources/StaticClusteredGeometry.vert");
	const auto fragmentShaderGlsTemplatelCode = loadShaderFile("Resources/StaticClusteredGeometry.frag");

	const auto materialTemplate = MaterialTemplate
	{
		.vertexShaderTemplate = vertexShaderGlslTemplateCode,
		.fragmentShaderTemplate = fragmentShaderGlsTemplatelCode
	};

	const auto materialAsset = MaterialAsset{};

	const auto frameContext = FrameContext
	{
		.texturesLayout = textureManager.getTextureBindGroupLayout(),
		.geometryLayout = simpleTriangleMeshDataGroupLayout,
		.perInstanceDataLayout = simpleTrianglePerInstanceGroupLayout,
		.perFrameDataLayout = simpleTriangleGroupLayout
	};

	const auto sharedMaterial = MaterialGenerator::compilePipeline(renderer,
			MaterialCompiolationDescriptor
			{
				.materialTemplate = materialTemplate,
				.frameContext = frameContext,
				.asset = materialAsset
			}
		);

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
			SetBindGroupMapping{2, simpleTrianglePerInstanceGroupLayout},
			SetBindGroupMapping{3, textureManager.getTextureBindGroupLayout()},
			SetBindGroupMapping{4, samplingGroupLayout }
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
				.descriptor = BindingDescriptor{BindingType::Sampler}
			}
		}
	};

	
	const auto guiVertexDataGroupLayout = renderer.createBindGroupLayout(guiVertexDataGroup);
	const auto guiFontGroupLayout = renderer.createBindGroupLayout(guiFontGroup);

	


	

	Handle<BindGroup> guiFontBindGroup = renderer.allocateBindGroup(guiFontGroupLayout, UsageScope::async);
	renderer.updateBindGroup(guiFontBindGroup,
		{
		
				{
					0, SamplerSRV{fontSampler}
				},
		});

	struct ScaleTranslate {
		glm::vec2 scale;
		glm::vec2 translate;
		core::u32 textureId;
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
			SetBindGroupMapping{1, guiFontGroupLayout},
			SetBindGroupMapping{2, textureManager.getTextureBindGroupLayout() }
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
		.position = glm::vec3{0.0f,0.0f,-1.0f},
		.forward = glm::vec3{0.0f,0.0f,1.0f},
		.up = glm::vec3{0.0f,1.0f,0.0f},
		.movementSpeed = 0.001f,
		.sensitivity = 0.2f
	};
	auto moveCameraFaster = false;
	auto onMousePressedScreenLocation = glm::vec2{ 0.0f,0.0f };
	auto mouseButtonPressed = false;
	//==============================

	using namespace std::string_literals;
	const auto knightTextureSet = std::array
	{
		"E:\\Develop\\ToyEngineContent\\Pkg_E_Knight_anim\\Textures\\results\\brushed_metal_rough_D.DDS"s,
		"E:\\Develop\\ToyEngineContent\\Pkg_E_Knight_anim\\Textures\\results\\brushed_metal_rough_F.DDS"s,
		"E:\\Develop\\ToyEngineContent\\Pkg_E_Knight_anim\\Textures\\results\\brushed_metal_rough_H.DDS"s,
		"E:\\Develop\\ToyEngineContent\\Pkg_E_Knight_anim\\Textures\\results\\cloth_diffuse_D.DDS"s,
		"E:\\Develop\\ToyEngineContent\\Pkg_E_Knight_anim\\Textures\\results\\cloth_normal_D.DDS"s,
		"E:\\Develop\\ToyEngineContent\\Pkg_E_Knight_anim\\Textures\\results\\leather_diffuse_D.DDS"s,
		"E:\\Develop\\ToyEngineContent\\Pkg_E_Knight_anim\\Textures\\results\\shield_final_diffuse_A.DDS"s,
		"E:\\Develop\\ToyEngineContent\\Pkg_E_Knight_anim\\Textures\\results\\shield_final_normal_A.DDS"s,
		"E:\\Develop\\ToyEngineContent\\Pkg_E_Knight_anim\\Textures\\results\\sword_diffuse_C.DDS"s,
		"E:\\Develop\\ToyEngineContent\\Pkg_E_Knight_anim\\Textures\\results\\sword_rough_D.DDS"s,
	};

	const auto editorTextureResources = std::array
	{
		"E:\\Develop\\ToyEngineContent\\generated_splash_screen.DDS"s,
	};


	const auto p1 = "E:\\Develop\\ToyEngineContent\\Pkg_E_Knight_anim\\Exports\\FBX\\Knight_USD_002.fbx";
	const auto p2 = "E:\\Develop\\ToyEngineContent\\crystal_palace.glb";
	const auto bistroExterior = "E:\\McGuireComputerGraphicsArchive\\Bistro\\exterior.obj";
	const auto bistroInterior = "E:\\McGuireComputerGraphicsArchive\\Bistro\\interior.obj";

	const auto bistroInteriorData = "E:\\Develop\\ToyEngineContent\\interior.dat";
	const auto bistroExteriorData = "E:\\Develop\\ToyEngineContent\\exterior.dat";
	const auto knightData = "E:\\Develop\\ToyEngineContent\\knight.dat";
	const auto anatomyData = "E:\\Develop\\ToyEngineContent\\Z-Anatomy.dat";
	const auto splashData = "E:\\Develop\\ToyEngineContent\\splash.dat";

	const auto blaseRunnderCity = "E:\\Develop\\ToyEngineContent\\blade-runner-style-cityscapes.dat";

	const auto scene = Scene::loadSceneFromFile(renderer, knightData);


	std::vector<UID> assetTextures;


	for(const auto& textureFile : knightTextureSet)
	{
		auto dataSource = FilestreamTextureDataSourceDescriptor{ textureFile };

		const auto& info = dataSource.getTextureInfo();
		const auto& texture2dInfo = std::get<Texture2DDimensionInfo>(info.dimensionInfo);
		TOY_ASSERT(info.format == TextureFormat::bc7);

		Flags<ImageAccessUsage> accessUsage = ImageAccessUsage::sampled;
		accessUsage |= ImageAccessUsage::transferDst;

		const auto fontDescriptor = ImageDescriptor
		{
			.format = Format::bc7,
			.extent = Extent{ static_cast<u32>(texture2dInfo.width), static_cast<u32>(texture2dInfo.height)},
			.mips = info.mipCount,
			.layers = info.arrayCount,
			.accessUsage = accessUsage
		};

		auto image = renderer.createImage(fontDescriptor);

		const auto view = renderer.createImageView(ImageViewDescriptor
		{
			.image = image,
			.format = Format::bc7,
			.type = ImageViewType::_2D
		});

		const auto texture = toy::Texture2D
		{
			.width = (u32)texture2dInfo.width,
			.height = (u32)texture2dInfo.height,
			.image = image,
			.view = view,
			.hasMips = info.mipCount > 1,
			.bytesPerTexel = bytesPerTexel(info.format)
		};


		auto totalSize = u32{};
		for(auto i = u32{}; i < info.mipCount; i++)
		{
			totalSize += lodSizeInBytes(info, i);
		}

		auto data = std::vector<std::byte>{};
		data.resize(totalSize);
		auto dataSpan = std::span(data);
		loadTexture2D(dataSource, (TextureData)dataSpan);


		textureUploader.upload(data, texture);

		const auto uid = textureManager.addTexture(texture);
		assetTextures.push_back(uid);
	}

	//===========================BINDLESS================================

	renderer.updateBindGroup(samplingGroup, { BindingDataMapping{0, SamplerSRV{fontSampler}, 0 } });
	

	//===========================EDITOR==================================





	struct TransformComponent
	{
		glm::mat4 tranform;
	};

	struct MeshComponent
	{
		u32 meshIndex;
	};

	struct EditorSceneObject
	{
		std::string name;
		TransformComponent transform;
		MeshComponent mesh;
	};

	struct EditorScene
	{
		std::vector<EditorSceneObject> scene;
	};


	auto editorScene = EditorScene();

	for(const auto& object : scene.drawInstances_)
	{
		const auto editorObject = EditorSceneObject
		{
			.name = std::format("object_{}", object.meshIndex),
			.transform = TransformComponent{ .tranform = object.model },
			.mesh = MeshComponent{ .meshIndex = object.meshIndex }
		};

		editorScene.scene.push_back(editorObject);

	}


	static bool togglePipeline = false;


	
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














	const auto fontUid = textureManager.addTexture(fontTexture);

	ImGui::GetIO().Fonts->SetTexID(ImTextureID{ (void*)fontUid });











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

		
		
		static auto selectedObject = u32{0};

		auto showSceneHierarchy = true;

		if (ImGui::Begin("Scene Hierarchy", &showSceneHierarchy))
		{
			//TODO: remove table
			if (ImGui::BeginTable("table_item_width", 2, ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("name");
				ImGui::TableSetupColumn("visible", ImGuiTableColumnFlags_WidthFixed, 26);
				//ImGui::TableHeadersRow();

				for (int i = 0; i < editorScene.scene.size(); i++)
				{
					const auto height = ImGui::GetTextLineHeightWithSpacing();
					ImGui::TableNextRow(ImGuiTableRowFlags_None, height);
					
					ImGui::PushID(i);
					ImGui::TableSetColumnIndex(0);
					if (ImGui::Selectable(editorScene.scene[i].name.c_str(), selectedObject == i))
					{
						selectedObject = i;
					}
					ImGui::TableSetColumnIndex(1);
					static bool v = true;
					if(v)
					{
						ImGui::ToggleButton(ICON_FA_EYE, &v);
					}
					else
					{
						ImGui::ToggleButton(ICON_FA_EYE_SLASH, &v);
					}

					ImGui::PopID();
				}
			}
			ImGui::EndTable();
		}
		
		ImGui::End();

		auto showInspector = true;
		ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Inspector", &showInspector))
		{
			auto& editorObject = editorScene.scene[selectedObject];
		
			ImGui::InputText("##", &editorObject.name, ImGuiInputTextFlags_CharsNoBlank);

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			
		  
			ImGui::PushID(selectedObject);
				
			if (ImGui::TreeNodeEx("Object", ImGuiTreeNodeFlags_DefaultOpen, "%s Transform", ICON_FA_CUBE))
			{
				ImGui::PushID(0);
				ImGui::InputFloat4("##", (float*)&scene.drawInstances_[editorObject.mesh.meshIndex].model[0], "%.2f", ImGuiInputTextFlags_None);
				ImGui::PopID();
				ImGui::PushID(1);
				ImGui::InputFloat4("##", (float*)&scene.drawInstances_[editorObject.mesh.meshIndex].model[1], "%.2f", ImGuiInputTextFlags_None);
				ImGui::PopID();
				ImGui::PushID(2);
				ImGui::InputFloat4("##", (float*)&scene.drawInstances_[editorObject.mesh.meshIndex].model[2], "%.2f", ImGuiInputTextFlags_None);
				ImGui::PopID();
				ImGui::PushID(3);
				ImGui::InputFloat4("##", (float*)&scene.drawInstances_[editorObject.mesh.meshIndex].model[3], "%.2f", ImGuiInputTextFlags_None);
				ImGui::PopID();
				ImGui::TreePop();
			}


			

			if (ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_DefaultOpen, "%s Material", ICON_FA_CARROT))
			{
				ImGui::PushID(0);
				static auto onCreateNewMaterial = false;
				static auto hasMaterial = false;
				static auto materialName = std::string{};
				static auto materialUID = u32{};
				if(!hasMaterial && !onCreateNewMaterial && ImGui::Button(ICON_FA_PLUS))
				{
					onCreateNewMaterial = true;
					
				}

				if(!hasMaterial && onCreateNewMaterial)
				{
					auto flags = ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue;
					if(ImGui::InputTextWithHint("##", "Name", &materialName, flags))
					{
						hasMaterial = true;
						onCreateNewMaterial = false;

						auto& assetManager = AssetManager::get();
						materialUID = assetManager.createNewMaterial(MaterialAssetDescriptor{.name = materialName });

					}
				}

				if(hasMaterial)
				{
					ImGui::Text(materialName.c_str());
					ImGui::SameLine();
					if(ImGui::Button(ICON_FA_TRASH_CAN))
					{
						hasMaterial = false;
						auto& assetManager = AssetManager::get();
						assetManager.deleteMaterial(materialUID);
					}
					ImGui::SameLine();
					if(ImGui::Button(ICON_FA_PEN_TO_SQUARE))
					{
						editor.tabManager().openMaterialEditorTab(materialUID);
					}
				}

				ImGui::PopID();
				ImGui::TreePop();
			}
			ImGui::PopID();
			ImGui::PopStyleVar();
		}
		ImGui::End();

		auto showAssetBrowser = true;

		static auto contentZoom = 1.0f;

		ImVec2 assetItemSize(40 * contentZoom, 40 * contentZoom);
		if (ImGui::Begin("AssetBrowser", &showAssetBrowser))
		{
			ImGui::SliderFloat("##", &contentZoom, 1.0f, 5.0f, "%.1f");

			const auto contentWidth = ImGui::GetWindowPos().x
				+ ImGui::GetWindowContentRegionMax().x;
			const auto& style = ImGui::GetStyle();
			for(auto i = u32{}; i < assetTextures.size(); i++)
			{
				ImGui::PushID(i);
				ImGui::ImageButton((void*)assetTextures[i], assetItemSize);

				const auto prevItemWidth = ImGui::GetItemRectMax().x;
				const auto widthAfterItemPush = prevItemWidth + style.ItemSpacing.x + assetItemSize.x;
				if(widthAfterItemPush < contentWidth)
				{
					ImGui::SameLine();
				}
				ImGui::PopID();
			}
				
		}
		ImGui::End();

		editor.showEditorGui();


#pragma endregion

		materialEditor.drawMaterialEditor();
		

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
			
			if (io.keyboardState.one == toy::io::ButtonState::pressed)
			{
				togglePipeline = true;
			}
			if (io.keyboardState.one == toy::io::ButtonState::unpressed)
			{
				togglePipeline = false;
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
			textureManager.nextFrame();
			textureManager.updateBindGroup();

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

					if (togglePipeline)
					{
						cmd.bindPipeline(sharedMaterial.pipeline);

					}
					else
					{
						cmd.bindPipeline(simpleTrianglePipeline);

					}
					cmd.setScissor(scissor);
					cmd.setViewport(viewport);
					cmd.bindGroup(0, meshDataBindGroup);
					cmd.bindGroup(1, bindGroup);
					cmd.bindGroup(2, perInstanceGroup);
					cmd.bindGroup(3, textureManager.getTextureBindGroup());

						
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
				cmd.bindGroup(2, textureManager.getTextureBindGroup());

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
					
					cmd.bindGroup(0, guiVertexDataGroup);
					

					for (auto j = u32{}; j < cmdList->CmdBuffer.Size; j++)
					{
						auto scaleTranslate = ScaleTranslate
						{
							.scale = glm::vec2(scale[0], scale[1]),
							.translate = glm::vec2(-1.0f - drawData->DisplayPos.x * scale[0], -1.0f - drawData->DisplayPos.y * scale[1])
						};
						const auto& drawCommand = cmdList->CmdBuffer[j];
						
						const auto clipMin = ImVec2(drawCommand.ClipRect.x - clipOff.x, drawCommand.ClipRect.y - clipOff.y);
						const auto clipMax = ImVec2(drawCommand.ClipRect.z - clipOff.x, drawCommand.ClipRect.w - clipOff.y);
						if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
							continue;
						scaleTranslate.textureId = textureManager.getBindIndex((core::u32)drawCommand.TextureId);
						cmd.pushConstant(scaleTranslate);
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

			renderer.present(postRenderingBatch.barrier());

			time += 0.01f;
			captureTool.stopAndOpenCapture();
		}
		frameEndTime = std::chrono::high_resolution_clock::now();
	}

	ImGui::DestroyContext();

	textureUploader.deinitialize();
	graphicsDebugger.deinitialize();
	renderer.deinitialize();
	window.deinitialize();
	logger::deinitialize();

	return EXIT_SUCCESS;
}
