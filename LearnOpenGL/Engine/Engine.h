#pragma once

#include <Engine/Core.h>

class Engine {
private:
	Engine() = default;
public:
	// prevents copying
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	static Engine& Get() {
		static Engine instance;
		return instance;
	}

	GLFWwindow* window = nullptr;

	void Init();
	void Update(float deltaTime);

private:
	bool CreateWindow();
};