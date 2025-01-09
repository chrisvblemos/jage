#pragma once

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <vector>
#include <unordered_map>
#include <memory>

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