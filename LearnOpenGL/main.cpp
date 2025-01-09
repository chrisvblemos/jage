//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//#include <iostream>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include "Engine/Shader.h"
//#include "Engine/Model.h"
//#include "Camera.h"
//#include "MouseInput.h"
//#include "Lights.h"
//#include "World.h"
//#include "Player.h"
//#include <vector>
//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>
//#include <memory>

#include "Engine/Engine.h"

int main() {
	Engine::Get().Init();
	return 0;
}

//
//// ===========================
//// ========CALLBACKS==========
//// ===========================
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//void processInput(GLFWwindow* window);
//
//void processInput(GLFWwindow* window) {
//	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
//		glfwSetWindowShouldClose(window, true);
//	}
//}
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
//	glViewport(0, 0, width, height);
//}
//
//// ===========================
//// ==========ENTRY============
//// ===========================
//
//int main() {
//	glfwInit();
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
//	if (window == nullptr)
//	{
//		std::cout << "Failed to create GLFW window." << std::endl;
//		glfwTerminate();
//		return -1;
//	}
//
//	glfwMakeContextCurrent(window);
//	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//
//	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
//		std::cout << "Failed to initialize GLAD." << std::endl;
//		return -1;
//	}
//
//	stbi_set_flip_vertically_on_load(true);
//	glEnable(GL_DEPTH_TEST);
//	auto ourShader = std::make_shared<Shader>("Shaders/default.vert", "Shaders/default.frag");
//	auto backpackModel = std::make_shared<Model>("Assets/Meshes/AbandonedCottage/cottage.obj", glm::vec3(0.0f));
//
//	// initialize input, camera, etc
//	auto mouseInput = std::make_shared<MouseInput>(window);
//	auto editorCamera = std::make_shared<Camera>(window, mouseInput, glm::vec3(0.0f));
//
//	// global stuff for now
//	float deltaTime = 0.0f;
//	float lastFrame = 0.0f;
//
//	// add directional light to the world
//	auto directionalLight = std::make_shared<DirectionalLight>(
//		glm::vec3(0.0f, -0.5f, 0.5f),
//		glm::vec3(0.05f, 0.05f, 0.05f),
//		glm::vec3(0.4f, 0.4f, 0.4f),
//		glm::vec3(0.5f, 0.5f, 0.5f)
//	);
//
//	std::shared_ptr<World> world = std::make_shared<World>();
//	world->cameras.push_back(editorCamera);
//	world->models.push_back(backpackModel);
//	world->directionalLight = directionalLight;
//
//	std::shared_ptr<Player> player = std::make_shared<Player>(window, mouseInput, glm::vec3(0.0f, 3.0f, 0.0f));
//	std::shared_ptr<Camera> camera = player->camera;
//
//	while (!glfwWindowShouldClose(window))
//	{
//		/* ========== FRAME LOGIC ========= */
//		float currentFrame = static_cast<float>(glfwGetTime());
//		deltaTime = currentFrame - lastFrame;
//		lastFrame = currentFrame;
//
//		/* ========== INPUT =========== */
//		
//		processInput(window); // closes the window on ESC
//		mouseInput->HandleInput(); // handle mouse movement
//
//		/* ========== TICKS ========== */
//		player->Tick(deltaTime);
//		//editorCamera->Tick(deltaTime); // controls camera logic every frame tick
//
//		/* ========== RENDERING =========== */
//
//		// clear the canvas
//		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//		ourShader->use();
//
//		glm::mat4 view = camera->GetViewMatrix();
//		glm::mat4 projection = camera->GetProjectionMatrix();
//		ourShader->setMat4("view", view);
//		ourShader->setMat4("projection", projection);
//		ourShader->setVec3("viewPos", camera->position);
//
//		// draw lights
//		if (world->directionalLight != nullptr) {
//			world->directionalLight->Draw(ourShader);
//		}
//		//directionalLight.Draw(ourShader);
//		//for (auto pointLight : pointLights) {
//		//	pointLights->Draw(ourShader);
//		//}
//
//		// render world models
//		for (std::shared_ptr<Model> model : world->models) {
//			model->Draw(ourShader);
//		}
//
//		// end frame
//		glfwSwapBuffers(window);
//		glfwPollEvents();
//	}
//
//	glfwTerminate();
//
//	return 0;
//
//}