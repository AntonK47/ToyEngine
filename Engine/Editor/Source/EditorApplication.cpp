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
#include <ImageDataUploader.h>

#include "MaterialEditor.h"
#include "AssetDatabase.h"

using namespace toy::io::loaders::dds;
using namespace toy::graphics::rhi;
using namespace toy::window;
using namespace toy::graphics;
using namespace toy::editor;
using namespace toy;

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

		for (const auto& c : windowIo.textState.text)
		{
			io.AddInputCharacter(c);
		}
	}
}


int toy::editor::EditorApplication::run(const std::vector<std::string>& arguments)
{
	logger::initialize();
	auto windowPtr = std::make_unique<SDLWindow>();
	auto rendererPtr = std::make_unique<RenderInterface>();
	auto graphicsDebuggerPtr = std::make_unique<debugger::RenderDocCapture>();
	auto textureUploaderPtr = std::make_unique<ImageDataUploader>();
	auto textureManagerPtr = std::make_unique<TextureManager>();
	//auto taskSystemPtr = std::make_unique<TaskSystem>();
	auto dynamicFrameAllocatorPtr = std::make_unique<DynamicFrameAllocator>();
	auto guiPtr = std::make_unique<Gui>();

	auto materialEditor = MaterialEditor{};
	materialEditor.initialize();

	auto& window = *windowPtr;
	auto& renderer = *rendererPtr;
	auto& graphicsDebugger = *graphicsDebuggerPtr;
	auto& textureUploader = *textureUploaderPtr;
	auto& textureManager = *textureManagerPtr;
	//auto& taskSystem = *taskSystemPtr;
	auto& dynamicFrameAllocator = *dynamicFrameAllocatorPtr;
	auto& gui = *guiPtr;

	auto windowWidth = u32{2560};
	auto windowHeight = u32{1440};
	//auto ids = std::vector<std::thread::id>{};

	{
		//taskSystem.initialize(TaskSystemDescriptor{});
		//ids = taskSystem.workers();
		//ids.push_back(std::this_thread::get_id());

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
			.workers = { std::this_thread::get_id()} //ids
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
	
	doAssetStuff(arguments[1]);
	auto db = AssetDatabase(arguments[1]);
	db.rebuildCache();
	db.flush();

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
		::mapWindowIoToImGuiIo(io, imGuiIo);
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

		if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Z))
		{
			materialEditor.undo();
		}

		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Y))
		{
			materialEditor.redo();
		}

		ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_FirstUseEver);

		ImGui::Begin("History");
		if (ImGui::Button("Undo"))
		{
			materialEditor.undo();
		}
		ImGui::SameLine();
		if (ImGui::Button("Redo"))
		{
			materialEditor.redo();
		}
		if (ImGui::BeginListBox("##historyList", ImVec2(-FLT_MIN, -FLT_MIN)))
		{
			const auto& history = materialEditor.getUndoHistory();
			for (int n = 0; n < history.size(); n++)
			{
				const bool is_selected = n == materialEditor.pointer()-1;
				ImGui::Selectable(history[n]->toString().c_str(), is_selected);

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}
		ImGui::End();

		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Node Editor"))
		{
			materialEditor.drawGui();
		}
		
		ImGui::End();
		ImGui::EndFrame();

		if(window.isWindowVisible())
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

			//auto submitTask = toy::core::Task{};
			//submitTask.taskFunction = [&]() 
			{
				renderer.beginDebugLabel(QueueType::graphics, { "prepare render target" });
				renderer.submitBatches(QueueType::graphics, { prepareBatch });
				renderer.endDebugLabel(QueueType::graphics);
			};
			//taskSystem.run({ &submitTask }, WorkerTag::rhi).wait();



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

				//auto submitTask = toy::core::Task{};
				//submitTask.taskFunction = [&]() 
				{
					renderer.beginDebugLabel(QueueType::graphics, DebugLabel{ "GUI" });
					renderer.submitBatches(QueueType::graphics, { guiBatch });
					renderer.endDebugLabel(QueueType::graphics);
					renderer.beginDebugLabel(QueueType::graphics, { "prepare present" });
				};
				//taskSystem.run({ &submitTask }, WorkerTag::rhi).wait();
			}
			

			{
				//auto submitTask = toy::core::Task{};
				//submitTask.taskFunction = [&]()
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
				//taskSystem.run({ &submitTask }, WorkerTag::rhi).wait();
			}

			captureTool.stopAndOpenCapture();
		}
		frameEndTime = std::chrono::high_resolution_clock::now();
	}
	textureUploader.deinitialize();
	gui.deinitialize();
	//taskSystem.deinitialize();
	dynamicFrameAllocator.deinitialize();
	graphicsDebugger.deinitialize();
	renderer.deinitialize();
	window.deinitialize();
	logger::deinitialize();

#pragma endregion
	return EXIT_SUCCESS;

}
