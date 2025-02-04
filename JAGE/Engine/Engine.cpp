#include <World/World.h>
#include <Renderer/API.h>
#include <Core/AssetLoader.h>
#include <Core/AssetManager.h>
#include <Input/MouseInput.h>
#include <Input/KeyboardInput.h>
#include <ECS/Components/PlayerMovement.h>
#include <ECS/Components/Camera.h>
#include <ECS/Components/StaticMeshRenderer.h>
#include <ECS/Components/RigidBody.h>
#include <ECS/Components/PointLight.h>
#include <ECS/Components/DirectionalLight.h>
#include <ECS/Components/Transform.h>
#include <ECS/Systems/PhysicsSystem.h>
#include <ECS/Systems/RenderSystem.h>
#include <ECS/Systems/PlayerSystem.h>
#include <Core/Types/MeshModel.h>
#include <LogDisplay.h>
#include <Editor/Editor.h>
#include <Renderer/ShaderPreProcessor.h>
#include "Engine.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void HandleInput(GLFWwindow* window);

bool Engine::CreateWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "JAGE", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Could not create game window." << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
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

	if (!API::Get().Initialize()) {
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

	// IMGUI initialization
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	// load default assets
	MeshModel& DefaultPlane = AssetLoader::Get().LoadMeshModelFromFile(
		"Assets/Meshes/default_plane.obj");
	MeshModel& DefaultCube = AssetLoader::Get().LoadMeshModelFromFile(
		"Assets/Meshes/default_cube.obj");
	MeshModel& backpackModel = AssetLoader::Get().LoadMeshModelFromFile(
		"Assets/Meshes/SurvivalGuitarBackpack/backpack.obj");

	// register components
	world.RegisterComponent<Transform>();
	world.RegisterComponent<Camera>();
	world.RegisterComponent<PlayerMovement>();
	world.RegisterComponent<RigidBody>();
	world.RegisterComponent<StaticMeshRenderer>();
	world.RegisterComponent<PointLight>();
	world.RegisterComponent<DirectionalLight>();

	Signature any;
	any.set();

	// register systems
	PlayerSystem* playerSystem = world.RegisterSystem<PlayerSystem>();
	Signature playerRequiredSignature = world.MakeSignature<Transform, Camera, PlayerMovement>();
	world.SetSystemRequiredSignature<PlayerSystem>(playerRequiredSignature);
	world.SetSystemOptionalSignature<PlayerSystem>(any);

	RenderSystem* renderSystem = world.RegisterSystem<RenderSystem>();
	renderSystem->SetRenderApi(&API::Get());

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
	playerCamera.mFarClipPlane = 500.0f;
	renderSystem->SetActiveCamera(&playerCamera);

	// white ground
	Entity ground = world.CreateEntity();
	world.AddComponent(ground, Transform{glm::vec3(0.0f, -1.5f, 0.0f), QUAT_NO_ROTATION, glm::vec3(1000.0f, 1.0f, 1000.0f)});
	world.AddComponent(ground, StaticMeshRenderer{ DefaultPlane.meshes });
	
	// the backpack
	//Entity backpack = world.CreateEntity();
	//world.AddComponent(backpack, Transform{});
	//world.AddComponent(backpack, StaticMeshRenderer{ backpackModel->meshes });
	//Transform& backpackTransform = world.GetComponent<Transform>(backpack);
	//backpackTransform.scale = glm::vec3(0.1f); // scale down backpack size

	uint32_t nBackpacks = 5;
	std::vector<Transform*> bkpTransforms;
	for (uint32_t i = 0; i < nBackpacks; i++) {
		Entity backpack = world.CreateEntity();
		world.AddComponent(backpack, Transform{ Utils::RandomPointInSphere(15.f, glm::vec3(0.0f, 12.0f, 0.0f)), Utils::RandomQuaternion(), glm::vec3(Utils::RandomFloat() + glm::vec3(0.5f)) });
		world.AddComponent(backpack, StaticMeshRenderer{ backpackModel.meshes });
		bkpTransforms.push_back(&world.GetComponent<Transform>(backpack));
	}

	uint32_t nCubes = 30;
	std::vector<Transform*> cubeTransforms;
	for (uint32_t i = 0; i < nCubes; i++) {
		Entity cube = world.CreateEntity();
		world.AddComponent(cube, Transform{Utils::RandomPointInSphere(15.f, glm::vec3(0.0f, 12.0f, 0.0f)), Utils::RandomQuaternion(), glm::vec3(Utils::RandomFloat() + glm::vec3(0.5f))});
		world.AddComponent(cube, StaticMeshRenderer{ DefaultCube.meshes });
		cubeTransforms.push_back(&world.GetComponent<Transform>(cube));
	}

	Entity pointLight = world.CreateEntity();
	float intensity = 200.0f;
	glm::vec3 pos = glm::vec3(-5.0f, 12.0f, 0.0f);
	glm::vec3 color = glm::vec3(0.5f, 0.1f, 0.05f);
	world.AddComponent(pointLight, Transform{ pos });
	world.AddComponent(pointLight, PointLight{ pos, color, intensity });

	Entity pointLight1 = world.CreateEntity();
	pos = glm::vec3(0.0f, 12.0f, 0.0f);
	color = glm::vec3(0.1f, 0.5f, 0.05f);
	world.AddComponent(pointLight1, Transform{ pos });
	world.AddComponent(pointLight1, PointLight{ pos, color, intensity });

	// Entity pointLight2 = world.CreateEntity();
	// pos = glm::vec3(5.0f, 12.0f, 0.0f);
	// color = glm::vec3(0.05f, 0.1f, 0.5f);
	// world.AddComponent(pointLight2, Transform{ pos });
	// world.AddComponent(pointLight2, PointLight{ pos, color, intensity });

	// the sun
	Entity sun = world.CreateEntity();
	world.AddComponent(sun, Transform{});
	world.AddComponent(sun, DirectionalLight{});
	DirectionalLight& sunDirLight = world.GetComponent<DirectionalLight>(sun);
	sunDirLight.intensity = 0.0f;
	sunDirLight.orthoProjSizes = glm::vec4(5);

	Editor editor = Editor();

	uint32_t nFrames = 0;
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		HandleInput(window);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGui::ShowDemoWindow(); // Show demo window! :)

		const auto currentFrame = static_cast<float>(glfwGetTime());
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// rotate cubes around origin
		for (Transform* cubeTransform : cubeTransforms) {
			cubeTransform->position = cubeTransform->position * glm::angleAxis(0.4f * dt, glm::vec3{ 0.0f, 1.0f, 0.0f });
			cubeTransform->rotation = cubeTransform->rotation * glm::angleAxis(0.1f * dt, cubeTransform->Up());
		}

		playerSystem->Update(dt);
		renderSystem->Update(dt);

		float fps = 1.0f / dt;
		LOG_DISPLAY_KEYED(std::format("{:.2f}", fps), "FPS");
		LOG_DISPLAY_KEYED(std::format("{:.2f}ms", 1000.0f * dt), "Frame Time");
		LogDisplayWindow::Get().Update(dt);

		editor.Update(dt);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	//ImGui_ImplOpenGL3_Shutdown();
	//ImGui_ImplGlfw_Shutdown();
	//ImGui::DestroyContext();

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