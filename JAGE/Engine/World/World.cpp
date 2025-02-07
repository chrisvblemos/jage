#include <ECS/Components/Transform.h>
#include <ECS/Components/Camera.h>
#include <ECS/Components/PlayerMovement.h>
#include <ECS/Components/RigidBody.h>
#include <ECS/Components/StaticMeshRenderer.h>
#include <ECS/Components/PointLight.h>
#include <ECS/Components/DirectionalLight.h>
#include <ECS/Systems/PlayerSystem.h>
#include <ECS/Systems/RenderSystem.h>
#include "World.h"

bool World::Initialize()
{
	mComponentManager = std::make_unique<ComponentManager>();
	mEntityManager = std::make_unique<EntityManager>();
	mSystemManager = std::make_unique<SystemManager>();

	RegisterComponent<Transform>();
	RegisterComponent<Camera>();
	RegisterComponent<PlayerMovement>();
	RegisterComponent<RigidBody>();
	RegisterComponent<StaticMeshRenderer>();
	RegisterComponent<PointLight>();
	RegisterComponent<DirectionalLight>();

	Signature any;
	any.set();

	PlayerSystem* playerSystem = RegisterSystem<PlayerSystem>();
	Signature playerRequiredSignature = MakeSignature<Transform, Camera, PlayerMovement>();
	SetSystemRequiredSignature<PlayerSystem>(playerRequiredSignature);
	SetSystemOptionalSignature<PlayerSystem>(any);

	RenderSystem* renderSystem = RegisterSystem<RenderSystem>();
	renderSystem->SetRenderApi(&OpenGlApi::Get());

	Signature renderRequiredSignature = MakeSignature<Transform>();
	Signature renderOptionalSignature = MakeSignature<StaticMeshRenderer, DirectionalLight, PointLight>();
	SetSystemRequiredSignature<RenderSystem>(renderRequiredSignature);
	SetSystemOptionalSignature<RenderSystem>(renderOptionalSignature);

	MeshModel& DefaultPlane = AssetLoader::Get().LoadMeshModelFromFile(
		"Assets/Meshes/default_plane.obj");
	MeshModel& DefaultCube = AssetLoader::Get().LoadMeshModelFromFile(
		"Assets/Meshes/default_cube.obj");
	MeshModel& backpackModel = AssetLoader::Get().LoadMeshModelFromFile(
		"Assets/Meshes/SurvivalGuitarBackpack/backpack.obj");

	Window::Get().SetMouseCursorVisibility(false);

	Entity player = CreateEntity();
	AddComponent(player, Transform{ glm::vec3(0.0f, 3.0f, 0.0f) });
	AddComponent(player, Camera{});
	AddComponent(player, PlayerMovement{});
	Camera& playerCamera = GetComponent<Camera>(player);
	playerCamera.mFOV = 60.0f;
	playerCamera.mFarClipPlane = 500.0f;
	renderSystem->SetActiveCamera(&playerCamera);

	Entity ground = CreateEntity();
	AddComponent(ground, Transform{ glm::vec3(0.0f, -1.5f, 0.0f), QUAT_NO_ROTATION, glm::vec3(1000.0f, 1.0f, 1000.0f) });
	AddComponent(ground, StaticMeshRenderer{ DefaultPlane.meshes });

	uint32_t nBackpacks = 5;
	std::vector<Transform*> bkpTransforms;
	for (uint32_t i = 0; i < nBackpacks; i++) {
		Entity backpack = CreateEntity();
		AddComponent(backpack, Transform{ Utils::RandomPointInSphere(15.f, glm::vec3(0.0f, 12.0f, 0.0f)), Utils::RandomQuaternion(), glm::vec3(Utils::RandomFloat() + glm::vec3(0.5f)) });
		AddComponent(backpack, StaticMeshRenderer{ backpackModel.meshes });
		bkpTransforms.push_back(&GetComponent<Transform>(backpack));
	}

	uint32_t nCubes = 30;
	std::vector<Transform*> cubeTransforms;
	for (uint32_t i = 0; i < nCubes; i++) {
		Entity cube = CreateEntity();
		AddComponent(cube, Transform{ Utils::RandomPointInSphere(15.f, glm::vec3(0.0f, 12.0f, 0.0f)), Utils::RandomQuaternion(), glm::vec3(Utils::RandomFloat() + glm::vec3(0.5f)) });
		AddComponent(cube, StaticMeshRenderer{ DefaultCube.meshes });
		cubeTransforms.push_back(&GetComponent<Transform>(cube));
	}

	Entity pointLight = CreateEntity();
	float intensity = 200.0f;
	glm::vec3 pos = glm::vec3(-5.0f, 12.0f, 0.0f);
	glm::vec3 color = glm::vec3(0.5f, 0.1f, 0.05f);
	AddComponent(pointLight, Transform{ pos });
	AddComponent(pointLight, PointLight{ pos, color, intensity });

	Entity pointLight1 = CreateEntity();
	pos = glm::vec3(0.0f, 12.0f, 0.0f);
	color = glm::vec3(0.1f, 0.5f, 0.05f);
	AddComponent(pointLight1, Transform{ pos });
	AddComponent(pointLight1, PointLight{ pos, color, intensity });

	Entity sun = CreateEntity();
	AddComponent(sun, Transform{});
	AddComponent(sun, DirectionalLight{});
	DirectionalLight& sunDirLight = GetComponent<DirectionalLight>(sun);
	sunDirLight.intensity = 0.1f;
	sunDirLight.orthoProjSizes = glm::vec4(5);

	return true;
}

Entity World::CreateEntity()
{
	return mEntityManager->CreateEntity();
}

const Signature World::GetEntitySignature(const Entity entity) const{
	return mEntityManager->GetSignature(entity);
}

void World::DestroyEntity(const Entity entity)
{
	mEntityManager->DestroyEntity(entity);
	mComponentManager->EntityDestroyed(entity);
	mSystemManager->EntityDestroyed(entity);
}

void World::Update(float dt)
{
	using WeakSystemPtr = std::weak_ptr<System>;
	std::vector<WeakSystemPtr> systems = mSystemManager->GetSystems();

	for (auto sys : systems) {
		auto ptr = sys.lock();
		if (ptr) 
			ptr->Update(dt);
	}
}

const std::string World::GetEntityName(const Entity entity) const {
	return mEntityManager->GetEntityName(entity);
}

const std::vector<Entity> World::GetEntities() const
{
	return mEntityManager->GetActiveEntities();
}
