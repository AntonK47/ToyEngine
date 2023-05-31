#include "EditorApplication.h"
#include <iostream>

#include <RenderInterface.h>
#include <SDLWindow.h>
#include <TaskSystem.h>
#include <DynamicFrameAllocator.h>
#include <DDSLoader.h>
#include <Gui.h>
#include <Editor.h>
#include <Logger.h>
#include <RenderDocCapture.h>
#include <glm/ext/matrix_common.hpp>

using namespace toy::io::loaders::dds;
using namespace toy::graphics::rhi;
using namespace toy::window;
using namespace toy::graphics;
using namespace toy::editor;
using namespace toy;

int toy::editor::EditorApplication::run(const std::vector<std::string>& arguments)
{
	logger::initialize();
	auto windowPtr = std::make_unique<SDLWindow>();
	auto rendererPtr = std::make_unique<RenderInterface>();
	auto graphicsDebuggerPtr = std::make_unique<debugger::RenderDocCapture>();
	auto editorPtr = std::make_unique<Editor>();
	auto textureUploaderPtr = std::make_unique<ImageDataUploader>();
	auto textureManagerPtr = std::make_unique<TextureManager>();
	auto taskSystemPtr = std::make_unique<TaskSystem>();
	auto dynamicFrameAllocatorPtr = std::make_unique<DynamicFrameAllocator>();
	auto guiPtr = std::make_unique<Gui>();

	auto& window = *windowPtr;
	auto& renderer = *rendererPtr;
	auto& graphicsDebugger = *graphicsDebuggerPtr;
	auto& editor = *editorPtr;
	auto& textureUploader = *textureUploaderPtr;
	auto& textureManager = *textureManagerPtr;
	auto& taskSystem = *taskSystemPtr;
	auto& dynamicFrameAllocator = *dynamicFrameAllocatorPtr;
	auto& gui = *guiPtr;

	auto windowWidth = u32{ 1920 };//u32{2560};
	auto windowHeight = u32{ 1080 };//u32{1440};
	auto ids = std::vector<std::thread::id>{};

	{
		taskSystem.initialize(TaskSystemDescriptor{});
		ids = taskSystem.workers();
		ids.push_back(std::this_thread::get_id());

		window.initialize(WindowDescriptor{ windowWidth, windowHeight });
		window.setWindowTitle("Toy Engine");
		window.registerExternalDragExtension(".png");

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
		renderer.initialize(rendererDescriptor);

		dynamicFrameAllocator.initialize({ .rhi = renderer, .size = 10 * 1024 * 1024, .framesInFlight = 3 });
		textureUploader.initialize(renderer, 100 * 1024 * 1024);

		const auto textureManagerDescriptor = TextureManagerDescriptor
		{
			.rhi = renderer,
			.frameInFlights = 3 //TODO:: this should be defined upfront
		};

		textureManager.initialize(textureManagerDescriptor);

		//editor.initialize(taskSystem, renderer, window, textureUploader, textureManager);
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
	struct Camera
	{
		glm::vec3 position;
		glm::vec3 forward;
		glm::vec3 up;
		float movementSpeed{ 1.0f };
		float movementSpeedScale{ 1.0f };
		float sensitivity{ 1.0f };
	};
	struct View
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 viewProjection;
	};

	auto camera = Camera
	{
		.position = glm::vec3{0.0f,120.0f,-200.0f},
		.forward = glm::vec3{0.0f,0.0f,1.0f},
		.up = glm::vec3{0.0f,1.0f,0.0f},
		.movementSpeed = 0.01f,
		.sensitivity = 0.2f
	};

	auto captureTool = graphicsDebugger.getScopeCapture();

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
	const auto perViewBindGroupLayoutDescriptor = BindGroupDescriptor
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
	const auto perViewBindGroupLayout = renderer.createBindGroupLayout(perViewBindGroupLayoutDescriptor);


	auto frameStartTime = std::chrono::high_resolution_clock::now();
	auto frameEndTime = std::chrono::high_resolution_clock::now();
	window.show();
	window.enableBorder();
	bool stillRunning = true;
	while(stillRunning)
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
		for (const auto& event : events)
		{
			if (event == Event::quit)
			{
				stillRunning = false;
			}
			if (event == Event::resize)
			{
				renderer.resizeBackbuffer(window.width(), window.height());

			}
		}

		ImGui::NewFrame();
		ImGui::ShowDemoWindow();


		{
			captureTool.start();
			renderer.nextFrame();
			dynamicFrameAllocator.nextFrame();
			textureManager.nextFrame();
			textureManager.updateBindGroup();


			gui.render();

			

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

			auto perViewBindGroup = renderer.allocateBindGroup(perViewBindGroupLayout);


			renderer.updateBindGroup(perViewBindGroup, {
				{
					0,
					UAV{ viewAllocation.bufferView }
				}
				});

			const auto& swapchainImage = renderer.acquireNextSwapchainImage();

			
			
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
			SubmitBatch prepareBatch;
			const auto area = RenderArea{ 0,0,windowWidth,windowHeight };
			{

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

				prepareBatch = renderer.submitCommandList(QueueType::graphics, { cmd }, { });

			}

			auto submitTask = toy::core::Task{};
			submitTask.taskFunction = [&]() {
				renderer.beginDebugLabel(QueueType::graphics, { "prepare render target" });
				renderer.submitBatches(QueueType::graphics, { prepareBatch });
				renderer.endDebugLabel(QueueType::graphics);
				};
			taskSystem.run({ &submitTask }, WorkerTag::rhi).wait();



			auto guiBatch = SubmitBatch{};
			{
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


				cmd.setViewport(viewport);
				gui.fillCommandList(cmd);
				cmd.endRendering();
				cmd.end();

				guiBatch = renderer.submitCommandList(toy::graphics::rhi::QueueType::graphics, { cmd }, { prepareBatch.barrier()});

				auto submitTask = toy::core::Task{};
				submitTask.taskFunction = [&]() {
					renderer.beginDebugLabel(QueueType::graphics, DebugLabel{ "GUI" });
					renderer.submitBatches(QueueType::graphics, { guiBatch });
					renderer.endDebugLabel(QueueType::graphics);
					renderer.beginDebugLabel(QueueType::graphics, { "prepare present" });
					};
				taskSystem.run({ &submitTask }, WorkerTag::rhi).wait();
			}
			

			{
				auto submitTask = toy::core::Task{};
				submitTask.taskFunction = [&]()
					{
						SubmitBatch postRenderingBatch;
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
							postRenderingBatch = renderer.submitCommandList(QueueType::graphics, { cmd }, { guiBatch.barrier() });
						}
						renderer.submitBatches(QueueType::graphics, { postRenderingBatch });
						renderer.present(postRenderingBatch.barrier());
					};
				taskSystem.run({ &submitTask }, WorkerTag::rhi).wait();
			}

			captureTool.stopAndOpenCapture();
		}
		frameEndTime = std::chrono::high_resolution_clock::now();
	}
	textureUploader.deinitialize();
	gui.deinitialize();
	taskSystem.deinitialize();
	dynamicFrameAllocator.deinitialize();
	graphicsDebugger.deinitialize();
	renderer.deinitialize();
	window.deinitialize();
	logger::deinitialize();

#pragma endregion
	return EXIT_SUCCESS;

}
