#include <World/World.h>
#include <Renderer/OpenGlApi.h>
#include <Core/Config.h>
#include "Engine.h"

void Engine::Init() {
	bool shouldExit = false;

	Window&    windowManager = Window::Get();
	World&     world         = World::Get();
	Input&     input         = Input::Get();
	OpenGlApi& openglApi     = OpenGlApi::Get();

	shouldExit = !windowManager.Initialize();
	shouldExit = !openglApi.Initialize();
	shouldExit = !input.Initialize();
	shouldExit = !world.Initialize();

	if (shouldExit) LOG(LogEngine, LOG_ERROR, "Something went wrong.");

	while (!shouldExit) {
		frameTime = static_cast<float>(glfwGetTime());
		float dt = frameTime - frameTimePrev;
		frameTimePrev = frameTime;

		windowManager.BeginFrame();

		input.PollKeys();
		input.PollMouse();
		world.Update(dt);

		windowManager.EndFrame();

		shouldExit = windowManager.ShouldClose();
	}
}