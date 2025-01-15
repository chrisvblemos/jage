#include <Engine/World/World.h>
#include <Engine/Defaults/Models.h>
#include <Engine/Renderer/OpenGL.h>
#include <Engine/Assets/AssetLoader.h>
#include <Engine/Assets/AssetManager.h>
#include <Engine/Input/MouseInput.h>
#include <Engine/Input/KeyboardInput.h>
#include <Engine/ECS/Components/PlayerMovement.h>
#include <Engine/ECS/Components/Camera.h>
#include <Engine/ECS/Components/StaticMeshRenderer.h>
#include <Engine/ECS/Components/RigidBody.h>
#include <Engine/ECS/Components/PointLight.h>
#include <Engine/ECS/Components/DirectionalLight.h>
#include <Engine/ECS/Components/Transform.h>
#include <Engine/ECS/Systems/PhysicsSystem.h>
#include <Engine/ECS/Systems/RenderSystem.h>
#include <Engine/ECS/Systems/PlayerSystem.h>
#include <Engine/Assets/Types/MeshModel.h>
#include "Engine.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void HandleInput(GLFWwindow* window);

bool Engine::CreateWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Could not create game window." << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize glad." << std::endl;
		glfwTerminate();
		return false;
	}

	return true;
}

void Engine::Init() {
	if (!CreateWindow()) {
		std::cout << "Failed to create game window. Exiting..." << std::endl;
		glfwTerminate();
		return;
	}

	if (!MouseInput::Get().Initialize(window) || !KeyboardInput::Get().Initialize(window)) {
		std::cout << "Failed to initialize player input. Exiting..." << std::endl;
		glfwTerminate();
		return;
	}

	if (!OpenGL::Get().Initialize()) {
		std::cout << "Failed to initialize renderer. Exiting..." << std::endl;
		glfwTerminate();
		return;
	}

	float dt = 0.0f;
	float lastFrame = 0.0f;

	World& world = World::Get();

	if (!world.Initialize()) {
		std::cout << "Failed to initialize world. Exiting..." << std::endl;
		glfwTerminate();
		return;
	}

	// load default assets
	MeshModel* DefaultPlane = AssetLoader::Get().LoadMeshModelFromFile("Assets/Meshes/default_plane.obj");
	MeshModel* DefaultCube = AssetLoader::Get().LoadMeshModelFromFile("Assets/Meshes/default_cube.obj");
	MeshModel* backpackModel = AssetLoader::Get().LoadMeshModelFromFile("Assets/Meshes/SurvivalGuitarBackpack/backpack.obj");
	
	// register components
	world.RegisterComponent<Transform>();
	world.RegisterComponent<Camera>();
	world.RegisterComponent<PlayerMovement>();
	world.RegisterComponent<RigidBody>();
	world.RegisterComponent<StaticMeshRenderer>();
	world.RegisterComponent<PointLight>();
	world.RegisterComponent<DirectionalLight>();

	// register systems
	PlayerSystem* playerSystem = world.RegisterSystem<PlayerSystem>();
	Signature playerRequiredSignature = world.MakeSignature<Transform, Camera, PlayerMovement>();
	world.SetSystemRequiredSignature<PlayerSystem>(playerRequiredSignature);

	RenderSystem* renderSystem = world.RegisterSystem<RenderSystem>();
	renderSystem->SetRenderApi(&OpenGL::Get());

	Signature renderRequiredSignature = world.MakeSignature<Transform>();
	Signature renderOptionalSignature = world.MakeSignature<StaticMeshRenderer, DirectionalLight, PointLight>();
	world.SetSystemRequiredSignature<RenderSystem>(renderRequiredSignature);
	world.SetSystemOptionalSignature<RenderSystem>(renderOptionalSignature);

	// create entities
	Entity player = world.CreateEntity();
	world.AddComponent(player, Transform{ glm::vec3(0.0f, 3.0f, 0.0f) });
	world.AddComponent(player, Camera{});
	world.AddComponent(player, PlayerMovement{});
	Camera& playerCamera = world.GetComponent<Camera>(player);
	playerCamera.mFOV = 60.0f;
	renderSystem->SetActiveCamera(&playerCamera);

	// white ground
	Entity ground = world.CreateEntity();
	world.AddComponent(ground, Transform{glm::vec3(0.0f, -0.5f, 0.0f), QUAT_NO_ROTATION, glm::vec3(1000.0f, 1.0f, 1000.0f)});
	world.AddComponent(ground, StaticMeshRenderer{ DefaultPlane->meshes });
	
	// the backpack
	//Entity backpack = world.CreateEntity();
	//world.AddComponent(backpack, Transform{});
	//world.AddComponent(backpack, StaticMeshRenderer{ backpackModel->meshes });
	//Transform& backpackTransform = world.GetComponent<Transform>(backpack);
	//backpackTransform.scale = glm::vec3(0.1f); // scale down backpack size

	uint32_t nCubes = 100;
	std::vector<Transform*> cubeTransforms;
	for (uint32_t i = 0; i < nCubes; i++) {
		Entity cube = world.CreateEntity();
		world.AddComponent(cube, Transform{Utils::RandomPointInSphere(15.0f), Utils::RandomQuaternion(), glm::vec3(Utils::RandomFloat())});
		world.AddComponent(cube, StaticMeshRenderer{ DefaultCube->meshes });
		cubeTransforms.push_back(&world.GetComponent<Transform>(cube));
	}

	//int nPointLights = 32;
	//for (unsigned int i = 0; i < nPointLights; i++) {
	//	Entity pointLight = world.CreateEntity();
	//	glm::vec3 randomPos = Utils::RandomPointInSphere(8.0f);
	//	glm::vec3 randomColor = glm::vec3(Utils::RandomFloat(), Utils::RandomFloat(), Utils::RandomFloat());
	//	float randomIntensity = Utils::RandomFloat();
	//	float randomRadius = 10.0f * Utils::RandomFloat();
	//	world.AddComponent(pointLight, Transform{ randomPos });
	//	world.AddComponent(pointLight, PointLight{ randomPos, randomColor, randomIntensity, randomRadius });
	//}

	// the sun
	Entity sun = world.CreateEntity();
	world.AddComponent(sun, Transform{});
	world.AddComponent(sun, DirectionalLight{});
	DirectionalLight& sunDirLight = world.GetComponent<DirectionalLight>(sun);
	sunDirLight.intensity = 0.7f;

	uint32_t nFrames = 0;
	while (!glfwWindowShouldClose(window)) {
		const auto currentFrame = static_cast<float>(glfwGetTime());
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// rotate cubes around origin
		for (Transform* cubeTransform : cubeTransforms) {
			cubeTransform->rotation = glm::angleAxis(0.05f * dt, glm::vec3(0.0f, 1.0f, 0.0f)) * cubeTransform->rotation;
		}

		playerSystem->Update(dt);
		renderSystem->Update(dt);

		// process player input
		HandleInput(window);
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
}

void Engine::Update(float dt) {
	
}

void HandleInput(GLFWwindow* window) {
	KeyboardInput::Get().HandleInput();
	MouseInput::Get().HandleInput();

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}