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
#include <filesystem>
#include <execution>
#include <algorithm>
#include <span>
#include <stack>
#include <regex>
//#include <compressonator.h>

#include "SceneLoader.h"
#include "Gui.h"
#include "OutlineFeature.h"
#include "MaterialEditor.h"
#include "Editor.h"
#include "ImageDataUploader.h"
#include "TextureManager.h"
#include "DDSLoader.h"
#include "Material.h"
#include "DynamicFrameAllocator.h"
#include "TaskSystem.h"
#include "Statistics.h"
#include "Utils.h"

using namespace toy::io::loaders::dds;
using namespace toy::graphics::rhi;
using namespace toy::window;
using namespace toy::graphics::compiler;
using namespace toy::graphics;
using namespace toy::editor;
using namespace toy;


struct TextureDataSource
{
	std::string path;
};

template<toy::io::loaders::dds::TextureDataSourceDescriptor T>
struct TextureDescriptor
{
	 T source;
};

Scene loadScene(TaskSystem& taskSystem, RenderInterface& renderer, ImageDataUploader& textureUploader, TextureManager& textureManager, std::vector<UID>& assetTextures)
{
	//void* BC7block_encoder;
	//BC7block_encoder = NULL;
	//CMP_EncoderSetting encodeSettings{};
	//encodeSettings.format = CMP_FORMAT_BC7;
	//encodeSettings.quality = 1.0;
	//encodeSettings.width = 16;
	//encodeSettings.height = 16;

	//if (CMP_CreateBlockEncoder(&BC7block_encoder, encodeSettings) != 0) {
	//	//CMP_FreeMipSet(&MipSetIn);
	//	//CMP_FreeMipSet(&MipSetCmp);
	//	std::printf("Error: Creating BC7 block encoder");
	//	//return -1;
	//}

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

	const auto bladeRunnerCity = "E:\\Develop\\ToyEngineContent\\blade-runner-style-cityscapes.dat";

	auto scene = Scene::loadSceneFromFile(renderer, knightData);


	for (const auto& textureFile : knightTextureSet)
	{
		auto dataSource = FilestreamTextureDataSourceDescriptor{ textureFile };

		const auto& info = dataSource.getTextureInfo();
		const auto& texture2dInfo = std::get<Texture2DDimensionInfo>(info.dimensionInfo);
		TOY_ASSERT(info.format == TextureFormat::bc7);

		Flags<ImageAccessUsage> accessUsage = ImageAccessUsage::sampled;
		accessUsage |= ImageAccessUsage::transferDst;

		const auto imageDescriptor = ImageDescriptor
		{
			.format = Format::bc7,
			.extent = Extent{ static_cast<u32>(texture2dInfo.width), static_cast<u32>(texture2dInfo.height)},
			.mips = info.mipCount,
			.layers = info.arrayCount,
			.accessUsage = accessUsage
		};

		auto image = renderer.createImage(imageDescriptor);

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
		for (auto i = u32{}; i < info.mipCount; i++)
		{
			totalSize += lodSizeInBytes(info, i);
		}

		auto data = std::vector<std::byte>{};
		data.resize(totalSize);
		auto dataSpan = std::span(data);
		loadTexture2D(dataSource, (TextureData)dataSpan);


		auto uploadTask = toy::core::Task{};
		uploadTask.taskFunction = [&]() {
			textureUploader.upload(data, texture);
		};
		taskSystem.run({ &uploadTask }, WorkerTag::rhi).wait();

		const auto uid = textureManager.addTexture(texture);
		assetTextures.push_back(uid);
	}

#pragma warning(push)
#pragma warning(disable: 6001)

	const auto samplingGroupDescriptor = BindGroupDescriptor
	{
		.bindings =
		{
			{
				.binding = core::u32{0},
				.descriptor = BindingDescriptor
				{
					.type = BindingType::Sampler,
					.descriptorCount = core::u32{1}
				}
			}
		},
		.flags = BindGroupFlag::unboundLast
	};

	const auto samplingGroupLayout = renderer.createBindGroupLayout(samplingGroupDescriptor, DebugLabel{ .name = "textureSamplers" });
	const auto samplingGroup = renderer.allocateBindGroup(samplingGroupLayout, UsageScope::async, DebugLabel{ "textureSamplers" });

	//R&D: Research about material system, (have in mind it should be compatible to a shader graph created materials.REFACTOR: extrude all material dependent stuff out of this function.
	const auto simpleTriangleGroup = BindGroupDescriptor
	{
		.bindings =
		{
			{
				.binding = 0,
				.descriptor = BindingDescriptor
				{
					.type = BindingType::StorageBuffer, //TODO: should be uniform
					.descriptorCount = 1
				}
			}
		},
		.flags = BindGroupFlag::none
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
	const auto fragmentShaderGlsTemplateCode = loadShaderFile("Resources/StaticClusteredGeometry.frag");

	const auto materialTemplate = MaterialTemplate
	{
		.vertexShaderTemplate = vertexShaderGlslTemplateCode,
		.fragmentShaderTemplate = fragmentShaderGlsTemplateCode
	};

	const auto materialAsset = MaterialAsset{};

	const auto frameContext = FrameContext
	{
		.texturesLayout = textureManager.getTextureBindGroupLayout(),
		.geometryLayout = simpleTriangleMeshDataGroupLayout,
		.perInstanceDataLayout = simpleTrianglePerInstanceGroupLayout,
		.perFrameDataLayout = simpleTriangleGroupLayout,
		.samplerLayout = samplingGroupLayout
	};

	const auto sharedMaterial = MaterialGenerator::compilePipeline(renderer,
		MaterialCompilationDescriptor
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
#pragma warning(pop)

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
			PushConstant({.size = sizeof(u32)})
		});


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

	scene.sharedMaterial_.push_back(sharedMaterial);
	scene.sharedMaterial_.push_back(SharedMaterial{ simpleTrianglePipeline });
	scene.meshDataBindGroup_ = meshDataBindGroup;
	return scene;
}

int Application::run(const std::vector<std::string>& arguments)
{
	auto projectDirectory = std::optional<std::filesystem::path>{};
	if (arguments.size() > 1)
	{
		auto validationFailed = false;

		for (auto i = u32{ 1 }; i < arguments.size(); i+=2)
		{
			validationFailed |= arguments.size() < i + 1;
			if (arguments[i] == std::string{"-projectDirectory"})
			{
				const auto directory = std::filesystem::path{ arguments[i + 1] };
				validationFailed |= !std::filesystem::is_directory(directory);
				projectDirectory = directory;
			}
			else
			{
				validationFailed |= true;
			}
		}

		//if (validationFailed)
		//{
		//	return -1;
		//}
	}
	



#pragma region startup
	logger::initialize();
	/*auto window = SDLWindow{};
	auto renderer = RenderInterface{};
	auto graphicsDebugger = debugger::RenderDocCapture{};
	auto virtualTextureStreaming = VirtualTextureStreaming{};
	auto editor = Editor{};
	auto textureUploader = ImageDataUploader{};
	auto textureManager = TextureManager{};
	auto taskSystem = TaskSystem{};
	auto dynamicFrameAllocator = DynamicFrameAllocator{};*/

	auto windowPtr = std::make_unique<SDLWindow>();
	auto rendererPtr = std::make_unique<RenderInterface>();
	auto graphicsDebuggerPtr = std::make_unique<debugger::RenderDocCapture>();
	auto virtualTextureStreamingPtr = std::make_unique<VirtualTextureStreaming>();
	auto editorPtr = std::make_unique<Editor>();
	auto textureUploaderPtr = std::make_unique<ImageDataUploader>();
	auto textureManagerPtr = std::make_unique<TextureManager>();
	auto taskSystemPtr = std::make_unique<TaskSystem>();
	auto dynamicFrameAllocatorPtr = std::make_unique<DynamicFrameAllocator>();
	auto guiPtr = std::make_unique<Gui>();


	auto& window = *windowPtr;
	auto& renderer = *rendererPtr;
	auto& graphicsDebugger = *graphicsDebuggerPtr;
	auto& virtualTextureStreaming = *virtualTextureStreamingPtr;
	auto& editor = *editorPtr;
	auto& textureUploader = *textureUploaderPtr;
	auto& textureManager = *textureManagerPtr;
	auto& taskSystem = *taskSystemPtr;
	auto& dynamicFrameAllocator = *dynamicFrameAllocatorPtr;
	auto& gui = *guiPtr;

	auto windowWidth = u32{2560};
	auto windowHeight = u32{1440};
	auto ids = std::vector<std::thread::id>{};

	{
		taskSystem.initialize(TaskSystemDescriptor{});
		ids = taskSystem.workers();
		ids.push_back(std::this_thread::get_id());

		window.initialize(WindowDescriptor{ windowWidth, windowHeight });
		window.setWindowTitle("Toy Engine");
		window.registerExternalDragExtension(".png");

		auto materialEditor = toy::editor::materials::MaterialEditor{};

		materialEditor.initialize();
#pragma warning(push)
#pragma warning(disable: 6001)
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
			.workers = ids
		};
#pragma warning(pop)
		renderer.initialize(rendererDescriptor);

		dynamicFrameAllocator.initialize({ .rhi = renderer, .size = 10 * 1024 * 1024, .framesInFlight = 3 });
		textureUploader.initialize(renderer, 100 * 1024 * 1024);

		const auto textureManagerDescriptor = TextureManagerDescriptor
		{
			.rhi = renderer,
			.frameInFlights = 3 //TODO:: this should be defined upfront
		};

		textureManager.initialize(textureManagerDescriptor);

		editor.initialize(taskSystem, renderer, window, textureUploader, textureManager);
		const auto renderDocDescriptor = debugger::RenderDocCaptureDescriptor
		{
			.nativeBackend = renderer.getNativeBackend()
		};
		graphicsDebugger.initialize(renderDocDescriptor);


		const auto guiDescriptor = GuiDescriptor
		{
			.renderer = renderer,
			.textureManager = textureManager,
			.imageUploader = textureUploader,
			.dynamicFrameAllocator = dynamicFrameAllocator,
			.dpiScale = window.getDiagonalDpiScale()
		};
		gui.initialize(guiDescriptor);
	}
	
#pragma endregion
#pragma region scene pipeline 
#pragma warning(push)
#pragma warning(disable: 6001)
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
	const auto samplingGroupLayout = renderer.createBindGroupLayout(samplingGroupDescriptor, DebugLabel{ .name = "textureSamplers" });
	const auto samplingGroup = renderer.allocateBindGroup(samplingGroupLayout, UsageScope::async, DebugLabel{ "textureSamplers" });
	
	

	const auto simpleTriangleGroup = BindGroupDescriptor
	{
		.bindings =
		{
			Binding
			{
				.binding = 0,
				.descriptor = BindingDescriptor{BindingType::StorageBuffer} //TODO: should be uniform
			}
		},
		.flags = BindGroupFlag::none
	};
	const auto simpleTriangleGroupLayout = renderer.createBindGroupLayout(simpleTriangleGroup);


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

	const auto simpleTrianglePerInstanceGroupLayout = renderer.createBindGroupLayout(simpleTrianglePerInstanceGroup);
#pragma endregion


#pragma region preparation
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

#pragma region outline
	const auto visibilityBufferDescriptor = ImageDescriptor
	{
		.format = Format::r32u,
		.extent = Extent{window.width(), window.height()},
		.mips = 1,
		.layers = 1,
		.accessUsage = ImageAccessUsage::colorAttachment
	};

	const auto visibilityBuffer = renderer.createImage(visibilityBufferDescriptor);
	const auto visibilityBufferViewDescriptor = ImageViewDescriptor
	{
		.image = visibilityBuffer,
		.format = Format::r32u,
		.type = ImageViewType::_2D,
		.aspect = ImageViewAspect::color
	};
	const auto visibilityBufferView = renderer.createImageView(visibilityBufferViewDescriptor);

#pragma endregion

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
		.position = glm::vec3{0.0f,120.0f,-200.0f},
		.forward = glm::vec3{0.0f,0.0f,1.0f},
		.up = glm::vec3{0.0f,1.0f,0.0f},
		.movementSpeed = 0.01f,
		.sensitivity = 0.2f
	};
	auto moveCameraFaster = false;
	auto onMousePressedScreenLocation = glm::vec2{ 0.0f,0.0f };
	auto mouseButtonPressed = false;
	//==============================
	std::vector<UID> assetTextures;
	//auto scene = loadScene(renderer, textureUploader, textureManager, assetTextures);
	auto scene = Scene{};
	//===========================BINDLESS================================

	auto sampler = renderer.createSampler(SamplerDescriptor{ Filter::linear, Filter::linear, MipFilter::linear });
	renderer.updateBindGroup(samplingGroup, { BindingDataMapping{0, SamplerSRV{sampler}, 0 } });
	

	//===========================EDITOR==================================


	struct TransformComponent
	{
		glm::mat4 transform;
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

	


	static bool togglePipeline = false;


	
	

	//TODO: [#3] command submit should work also without calling nextFrame in a frame async scenario
	auto frameNumber = u32{};
	bool stillRunning = true;
	auto captureTool = graphicsDebugger.getScopeCapture();

	

	SubmitBatch prepareBatch;
	auto prepareBatchValid = false;
	auto perThreadSubmits = std::vector<SubmitBatch>{};
	perThreadSubmits.resize(10);
	SubmitBatch postRenderingBatch;


	

	struct PerThreadDrawStatistics
	{
		alignas(std::hardware_destructive_interference_size) SceneDrawStatistics statistics{};
	};

	auto perRenderThreadDrawStatistics = std::vector<PerThreadDrawStatistics>{};
	perRenderThreadDrawStatistics.resize(ids.size());

	auto drawStatistics = DrawStatistics{};



	

	std::atomic_flag isSceneReady;
	auto newScene = Scene{};

	auto sceneLoadTask = Task{};
	sceneLoadTask.taskFunction = [&]()
	{
		newScene = loadScene(taskSystem, renderer, textureUploader, textureManager, assetTextures);

		
		isSceneReady.test_and_set();

		editor.navigateToSceneView();
	};

	taskSystem.run({ &sceneLoadTask }, WorkerTag::background);

	
	auto frameStartTime = std::chrono::high_resolution_clock::now();
	auto frameEndTime = std::chrono::high_resolution_clock::now();
	window.show();
#pragma endregion
#pragma region gameloop


	
	auto readBackBuffer = renderer.createBuffer(BufferDescriptor
		{
			.size = 4,
			.accessUsage = BufferAccessUsage::transferDst,
			.memoryUsage = MemoryUsage::cpuOnly
		});




	bool loaded = false;

	while (stillRunning)
	{
		if (isSceneReady.test() && !loaded)
		{
			loaded = true;
			scene = newScene;
			for (const auto& object : scene.drawInstances_)
			{
				const auto editorObject = EditorSceneObject
				{
					.name = std::format("object_{}", object.meshIndex),
					.transform = TransformComponent{.transform = object.model },
					.mesh = MeshComponent{.meshIndex = object.meshIndex }
				};

				editorScene.scene.push_back(editorObject);

			}
		}


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
		//imGuiIo.MouseDrawCursor = true;

		for (const auto& event : events)
		{
			if (event == Event::quit)
			{
				stillRunning = false;
			}
			if (event == Event::resize)
			{
				windowHeight = window.height();
				windowWidth = window.width();
				renderer.resizeBackbuffer(windowWidth, windowHeight);
			}
		}

		ImGui::NewFrame();
		ImGui::ShowDemoWindow();

		
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

		if (!editorScene.scene.empty())
		{
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
					if (!hasMaterial && !onCreateNewMaterial && ImGui::Button(ICON_FA_PLUS))
					{
						onCreateNewMaterial = true;

					}

					if (!hasMaterial && onCreateNewMaterial)
					{
						auto flags = ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue;
						if (ImGui::InputTextWithHint("##", "Name", &materialName, flags))
						{
							hasMaterial = true;
							onCreateNewMaterial = false;

							auto& assetManager = AssetManager::get();
							materialUID = assetManager.createNewMaterial(MaterialAssetDescriptor{ .name = materialName });

						}
					}

					if (hasMaterial)
					{
						ImGui::Text(materialName.c_str());
						ImGui::SameLine();
						if (ImGui::Button(ICON_FA_TRASH_CAN))
						{
							hasMaterial = false;
							auto& assetManager = AssetManager::get();
							assetManager.deleteMaterial(materialUID);
						}
						ImGui::SameLine();
						if (ImGui::Button(ICON_FA_PEN_TO_SQUARE))
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
		}
		

		static bool drag = false;

		if (io.dragDropState == io::DragDropEvent::dragBegin)
		{
			drag = true;
		}
		if (io.dragDropState == io::DragDropEvent::dragEnd)
		{
			drag = false;
		}
		if (io.dragDropState == io::DragDropEvent::dragLeave)
		{
			drag = false;
		}

		if (io.dragDropState == io::DragDropEvent::dragEnd)
			LOG(INFO) << "END";
		if(io.dragDropState == io::DragDropEvent::dragBegin)
			LOG(INFO) << "BEGIN";
		if (io.dragDropState == io::DragDropEvent::dragLeave)
			LOG(INFO) << "LEAVE";
	
		

		if(drag)
		{
			//HACK
			ImGui::GetIO().MouseDown[ImGuiMouseButton_Left] = true;
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern))
			{
				ImGui::SetDragDropPayload("FILES", nullptr, 0);
				ImGui::BeginTooltip();
				ImGui::Text(ICON_FA_FILE_IMAGE);
				ImGui::EndTooltip();
				ImGui::EndDragDropSource();
			}
		}

		auto showAssetBrowser = true;

		static auto contentZoom = 1.0f;

		ImVec2 assetItemSize(40 * contentZoom, 40 * contentZoom);
		if (ImGui::Begin("AssetBrowser", &showAssetBrowser))
		{
			ImGui::SliderFloat("##", &contentZoom, 1.0f, 5.0f, "%.1f");
			auto& style = ImGui::GetStyle();

			ImGui::ProgressBar(0.3);
			const auto gridStartPosition = ImGui::GetCursorPos();
			ImGui::Dummy(ImGui::GetContentRegionAvail());
			
			if (ImGui::BeginDragDropTarget() )
			{
				if (auto t = ImGui::AcceptDragDropPayload("FILES") && io.dragDropState == io::DragDropEvent::dragEnd)  // or: const ImGuiPayload* payload = ... if you sent a payload in the block above
				{
					const auto& paths = window.getDraggedFilePaths();
					for (const auto& path : paths)
					{
						std::cout << path.generic_string() << std::endl;
					}
					editor.openTextureImport(paths);
				}

				ImGui::EndDragDropTarget();
			}
			ImGui::SetCursorPos(gridStartPosition);

			const auto contentWidth = ImGui::GetWindowPos().x
				+ ImGui::GetWindowContentRegionMax().x;
			ImGui::BeginGroup();
			for(auto i = u32{}; i < assetTextures.size(); i++)
			{
				ImGui::PushID(i);
				ImGui::ImageButton("b", reinterpret_cast<void*>(static_cast<core::u64>(assetTextures[i])), assetItemSize);

				const auto prevItemWidth = ImGui::GetItemRectMax().x;
				const auto widthAfterItemPush = prevItemWidth + style.ItemSpacing.x + assetItemSize.x;
				if(widthAfterItemPush < contentWidth)
				{
					ImGui::SameLine();
				}
				ImGui::PopID();
			}
			ImGui::EndGroup();
		}
		ImGui::End();
		/*
		materialEditor.drawMaterialEditor();
		*/
		
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
					const auto total = budget.budget[i].totalMemory / (static_cast<unsigned long long>(1024) * 1024);
					const auto available = budget.budget[i].availableMemory / (static_cast<unsigned long long>(1024) * 1024);
					const auto budgetValue = budget.budget[i].budgetMemory / (static_cast<unsigned long long>(1024) * 1024);

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

					if(ImGui::Button("deallocate memory"))
					{
						if(!allocations.empty())
						{
							const auto& b = allocations.top();
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
				ImGui::SliderFloat("Speed", &camera.movementSpeedScale, 1.0f, 1500.0f);
				nextWindowPos.y = (nextWindowPos.y + pad + ImGui::GetWindowHeight());
			}
			ImGui::End();
		}


		static auto showReadBack = true;
		if (ImGui::Begin("ToolsBar", &toolsBar, windowFlags))
		{
			std::byte* d;
			renderer.map(readBackBuffer, (void**)& d);

			//TODO: it depends on swapchain image format
			auto b = d[0];
			auto g = d[1];
			auto r = d[2];
			auto a = d[3];

			renderer.unmap(readBackBuffer);
			const auto color = ImColor(IM_COL32(r,g,b,a));
			ImGui::ColorButton("abs", color);

		}
		ImGui::End();

		/*

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
		if (ImGui::Begin("WindowControlBar", &windowControlBar, windowFlags))
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
		*/
		editor.showEditorGui();

		if (io.keyboardState.three == toy::io::ButtonState::pressed)
		{
			if (captureTool.isRenderDocInjected())
			{
				captureTool.captureNextMarkedScope();
				LOG(INFO) << "capturing frame " << frameNumber << "...";
			}
		}
		if (!imGuiIo.WantCaptureKeyboard)
		{
			if (io.keyboardState.two == toy::io::ButtonState::pressed)
			{
				window.resize(300, 500);
				
				windowHeight = window.height();
				windowWidth = window.width();
				renderer.resizeBackbuffer(windowWidth, windowHeight);
				
			}
			
			if (io.keyboardState.three == toy::io::ButtonState::pressed)
			{
				if (captureTool.isRenderDocInjected())
				{
					captureTool.captureNextMarkedScope();
					LOG(INFO) << "capturing frame " << frameNumber << "...";
				}
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


		frameNumber++;
		{
			captureTool.start();
			renderer.nextFrame();
			dynamicFrameAllocator.nextFrame();
			textureManager.nextFrame();
			textureManager.updateBindGroup();

			const auto b = renderer.requestMemoryBudget();

			gui.render();

			auto bindGroup = renderer.allocateBindGroup(simpleTriangleGroupLayout);

			
			const auto aspectRatio = static_cast<float>(window.width()/*/2*/) / static_cast<float>(window.height());
			const auto projection = glm::perspective(glm::radians(60.0f), aspectRatio, 100000.0f, 0.001f);//inverse z trick
			const auto view = glm::lookAt(camera.position, camera.position + camera.forward, camera.up);

			const auto viewData = View
			{
				.view = view,
				.projection = projection,
				.viewProjection = projection * view
			};

			const auto viewAllocation = dynamicFrameAllocator.allocate(sizeof(View));
			TOY_ASSERT(viewAllocation.bufferView.offset % 16 == 0);
			std::memcpy(viewAllocation.dataPtr, &viewData, sizeof(View));

			renderer.updateBindGroup(bindGroup, {
				{
					0,
					UAV{ viewAllocation.bufferView }
				}
				});

			const auto& swapchainImage = renderer.acquireNextSwapchainImage();

			const auto instances = std::span{ scene.drawInstances_ };
			const auto batchSize = ids.size();
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
#pragma warning(push)
#pragma warning(disable: 6001)
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
#pragma warning(pop)

			const auto area = RenderArea{ 0,0,windowWidth,windowHeight };

			{


				{
					auto submitTask = toy::core::Task{};
					submitTask.taskFunction = [&]() {
						renderer.beginDebugLabel(QueueType::graphics, { "prepare render target" });

					};
					taskSystem.run({ &submitTask }, WorkerTag::rhi).wait();

				}
				
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

				if(prepareBatchValid)
				{
					auto dependency = postRenderingBatch.barrier();
					prepareBatch = renderer.submitCommandList(QueueType::graphics, { cmd }, {dependency});
				}
				else
				{
					prepareBatch = renderer.submitCommandList(QueueType::graphics, { cmd }, {});
					prepareBatchValid = true;
				}
			}

			auto submitTask = toy::core::Task{};
			submitTask.taskFunction = [&]() {
				renderer.submitBatches(QueueType::graphics, { prepareBatch });
				renderer.endDebugLabel(QueueType::graphics);
			};
			taskSystem.run({ &submitTask }, WorkerTag::rhi).wait();


			if (!scene.drawInstances_.empty())
			{
				auto perInstanceGroup = renderer.allocateBindGroup(simpleTrianglePerInstanceGroupLayout);
				

				const auto perInstanceDrawAllocation = dynamicFrameAllocator.allocate(sizeof(InstanceData) * scene.drawInstances_.size());
				
				renderer.updateBindGroup(perInstanceGroup,
					{
						BindingDataMapping
						{
							.binding = 0,
							.view = UAV
							{
								perInstanceDrawAllocation.bufferView
							}
						}
					});

				{
					auto submitTask = toy::core::Task{};
					submitTask.taskFunction = [&]() {
						renderer.beginDebugLabel(QueueType::graphics, { "object rendering" });

					};
					taskSystem.run({ &submitTask }, WorkerTag::rhi).wait();

				}
				

				auto renderTasks = std::array<Task, 100>{};
				//renderTasks.resize(setIndicies.size());
				for (auto i = u32{}; i < setIndicies.size(); i++)
				{
					auto index = setIndicies[i];
					
					renderTasks[i].taskFunction = [&, index]()
					{
						auto& drawStatistics = perRenderThreadDrawStatistics[index].statistics;
						drawStatistics = SceneDrawStatistics{};
						auto& drawInstances = batches[index];
						auto cmd = renderer.acquireCommandList(toy::graphics::rhi::QueueType::graphics);
						cmd.begin();

#pragma warning(push)
#pragma warning(disable: 6001)
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
#pragma warning(pop)

						cmd.beginRendering(renderingDescriptor, area);

						{
							//const auto scissor = Scissor{ (i32)(windowWidth/2),0,windowWidth/2, windowHeight };
							//const auto viewport = Viewport{ (float)windowWidth*0.5f,0.0,(float)windowWidth*0.5f, (float)windowHeight };

							const auto scissor = Scissor{ 0,0,windowWidth, windowHeight };
							const auto viewport = Viewport{ 0.0,0.0,(float)windowWidth, (float)windowHeight };

							if (togglePipeline)
							{
								cmd.bindPipeline(scene.sharedMaterial_[1].pipeline);

							}
							else
							{
								cmd.bindPipeline(scene.sharedMaterial_[0].pipeline);

							}
							cmd.setScissor(scissor);
							cmd.setViewport(viewport);
							cmd.bindGroup(0, scene.meshDataBindGroup_);
							cmd.bindGroup(1, bindGroup);
							cmd.bindGroup(2, perInstanceGroup);
							cmd.bindGroup(3, textureManager.getTextureBindGroup());
							cmd.bindGroup(4, samplingGroup);


							auto setOffset = batchOffsets[index] * sizeof(InstanceData);

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

								std::memcpy((core::u8*)perInstanceDrawAllocation.dataPtr + setOffset, &instanceData, sizeof(InstanceData));
								setOffset += sizeof(InstanceData);

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

					};
				}
				taskSystem.run(std::span(renderTasks.begin(), setIndicies.size())).wait();
				
				auto gatheredStatistics = std::vector<SceneDrawStatistics>{};
				gatheredStatistics.resize(perRenderThreadDrawStatistics.size());

				std::transform(perRenderThreadDrawStatistics.begin(), perRenderThreadDrawStatistics.end(), gatheredStatistics.begin(), [](auto& a) {return a.statistics; });

				drawStatistics.scene = std::accumulate(gatheredStatistics.begin(), gatheredStatistics.end(), SceneDrawStatistics{},
					[](SceneDrawStatistics a, SceneDrawStatistics& b)
					{
						SceneDrawStatistics c;
						c.drawCalls = a.drawCalls + b.drawCalls;
						c.totalTrianglesCount = a.totalTrianglesCount + b.totalTrianglesCount;
						return c;
					});


				auto submitTask = toy::core::Task{};
				submitTask.taskFunction = [&]() 
				{
					renderer.submitBatches(QueueType::graphics, perThreadSubmits);
					renderer.endDebugLabel(QueueType::graphics);
					
				};
				taskSystem.run({ &submitTask }, WorkerTag::rhi).wait();
			}


			auto readBackBatch = SubmitBatch{};
			{
				auto cmd = renderer.acquireCommandList(toy::graphics::rhi::QueueType::graphics);
				cmd.begin();

				cmd.barrier(
					{
						ImageBarrierDescriptor
						{
							.srcLayout = Layout::colorRenderTarget,
							.dstLayout = Layout::transferSrc,
							.image = swapchainImage.image
						}
					});

				cmd.transfer(
					TransferImageDescriptor
					{
						.image = swapchainImage.image,
						.regions = { Region{ 0, 0, 1, glm::uvec3{ 1, 1, 1 }, glm::clamp(glm::ivec3{  io.mouseState.position.x, io.mouseState.position.y, 0 }, glm::ivec3(0,0,0), glm::ivec3(window.width() - 1, window.height() - 1,0)) }}
					},
					TransferBufferDescriptor
					{
						.buffer = readBackBuffer,
						.offset = 0
					});



				cmd.barrier(
					{
						ImageBarrierDescriptor
						{
							.srcLayout = Layout::transferSrc,
							.dstLayout = Layout::colorRenderTarget,
							.image = swapchainImage.image
						}
					});

				cmd.end();


				auto submits = std::vector<SubmitDependency>{};
				submits.resize(10);
				std::transform(perThreadSubmits.begin(), perThreadSubmits.end(), submits.begin(), [](auto& a) { return a.barrier(); });
				readBackBatch = renderer.submitCommandList(toy::graphics::rhi::QueueType::graphics, { cmd }, submits);

				auto submitTask = toy::core::Task{};
				submitTask.taskFunction = [&]() {
					renderer.submitBatches(QueueType::graphics, { readBackBatch });
					};
				taskSystem.run({ &submitTask }, WorkerTag::rhi).wait();
			}


			
			auto guiBatch = SubmitBatch{};
			{
				{
					auto submitTask = toy::core::Task{};
					submitTask.taskFunction = [&]() {
						renderer.beginDebugLabel(QueueType::graphics, DebugLabel{ "GUI" });

					};
					taskSystem.run({ &submitTask }, WorkerTag::rhi).wait();

				}
				drawStatistics.gui = GuiDrawStatistics{};
				auto cmd = renderer.acquireCommandList(toy::graphics::rhi::QueueType::graphics);
				cmd.begin();
#pragma warning(push)
#pragma warning(disable: 6001)
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
#pragma warning(pop)

				cmd.beginRendering(renderingDescriptor, area);
			   
				const auto viewport = Viewport{ 0.0, (float)windowHeight,(float)windowWidth, -(float)windowHeight };

				
				cmd.setViewport(viewport);
				drawStatistics.gui = gui.fillCommandList(cmd);
				cmd.endRendering();
				cmd.end();

				guiBatch = renderer.submitCommandList(toy::graphics::rhi::QueueType::graphics, { cmd }, { readBackBatch.barrier() });

				auto submitTask = toy::core::Task{};
				submitTask.taskFunction = [&]() {
					renderer.submitBatches(QueueType::graphics, { guiBatch }); 
					renderer.endDebugLabel(QueueType::graphics);
					renderer.beginDebugLabel(QueueType::graphics, { "prepare present" });
				};
				taskSystem.run({ &submitTask }, WorkerTag::rhi).wait();
			}
			
			{
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

			{
				auto submitTask = toy::core::Task{};
				submitTask.taskFunction = [&]()
				{
					renderer.submitBatches(QueueType::graphics, { postRenderingBatch });
					renderer.present(postRenderingBatch.barrier());
					renderer.endDebugLabel(QueueType::graphics);
				};
				taskSystem.run({ &submitTask }, WorkerTag::rhi).wait();
			}
			
			captureTool.stopAndOpenCapture();
		}
		frameEndTime = std::chrono::high_resolution_clock::now();
	}
#pragma endregion
#pragma region shotdown

	gui.deinitialize();
	taskSystem.deinitialize();
	textureUploader.deinitialize();
	dynamicFrameAllocator.deinitialize();
	graphicsDebugger.deinitialize();
	renderer.deinitialize();
	window.deinitialize();
	logger::deinitialize();

#pragma endregion
	return EXIT_SUCCESS;
}
