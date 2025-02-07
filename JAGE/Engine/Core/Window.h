#pragma once

#include <Core/Core.h>

class Window {
public:
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	static Window& Get() {
		static Window instance;
		return instance;
	}

	bool Initialize();
	void BeginFrame();
	void EndFrame();
	bool ShouldClose();
	void Close();
	void FramebufferSizeCallback(int width, int height);

	void SetResolution(const size_t width, const size_t height);
	void SetVsyncIsEnabled(const bool enabled);
	void SetIsFullscreen(const bool fullscreen);
	void SetMouseCursorVisibility(const bool visible);

	Vec2 GetResolution() const { return Vec2(width, height); }
	bool GetIsFullscren() const { return isFullscreen; }
	bool GetIsVsyncEnabled() const { return isVsyncEnabled; }
	GLFWwindow* GetNativeWindow() const { return window; }

private:
	Window() = default;
	size_t width;
	size_t height;
	bool isFullscreen;
	bool isVsyncEnabled;
	bool isMouseVisible;

	GLFWwindow* window;

};