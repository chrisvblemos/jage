#include <Core/Config.h>
#include "Window.h"

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	Window* ptr = static_cast<Window*>(glfwGetWindowUserPointer(window));
	if (ptr)
		ptr->FramebufferSizeCallback(width, height);
}

bool Window::Initialize()
{
	if (window) return true;

	size_t width = Cfg::Rendering.Read<size_t>("Video", "video.resX", 800);
	size_t height = Cfg::Rendering.Read<size_t>("Video", "video.resY", 600);
	bool isFullscreen = Cfg::Rendering.Read<bool>("Video", "fullscreen", true);
	bool isVsyncEnabled = Cfg::Rendering.Read<bool>("Video", "vsync", false);

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(
		width,
		height,
		"JAGE",
		nullptr,
		nullptr);

	if (window == nullptr)
	{
		std::cout << "Could not create game window." << std::endl;
		Close();
		return false;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(isVsyncEnabled? 1 : 0);
	glfwSetWindowUserPointer(window, (void*)this);
	glfwSetFramebufferSizeCallback(window, ::framebufferSizeCallback);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize glad." << std::endl;
		glfwTerminate();
	}

	this->width = width;
	this->height = height;
	this->isVsyncEnabled = isVsyncEnabled;
	this->isFullscreen = isFullscreen;

	return true;
}

void Window::BeginFrame()
{
	glfwPollEvents();
}

void Window::EndFrame()
{
	glfwSwapBuffers(window);
}

bool Window::ShouldClose()
{
	if (!window) return true;

	return glfwWindowShouldClose(window);
}

void Window::Close()
{
	glfwTerminate();
}

void Window::SetResolution(const size_t width, const size_t height)
{
	if (this->width == width && this->height == height) return;

	this->width = width;
	this->height = height;
	glfwSetWindowSize(window, width, height);
}

void Window::SetVsyncIsEnabled(const bool enabled)
{
	this->isVsyncEnabled = enabled;
	glfwSwapInterval(isVsyncEnabled ? 1 : 0);
}

void Window::SetIsFullscreen(const bool fullscreen)
{
	this->isFullscreen = fullscreen;
}

void Window::SetMouseCursorVisibility(const bool visible)
{
	this->isMouseVisible = visible;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
}

void Window::FramebufferSizeCallback(int width, int height) {
	glViewport(0, 0, width, height);
}
