#include <ECS/Components/Transform.h>
#include <ECS/Components/Camera.h>
#include <ECS/Components/Character.h>
#include <ECS/Components/RigidBody.h>
#include <ECS/Components/StaticMeshRenderer.h>
#include <ECS/Components/PointLight.h>
#include <ECS/Components/DirectionalLight.h>
#include <ECS/Systems/CharacterSystem.h>
#include <ECS/Systems/TransformSystem.h>
#include <ECS/Systems/RenderSystem.h>
#include <ECS/Systems/PhysicsSystem.h>
#include "World.h"

bool World::Initialize()
{
	mComponentManager = std::make_unique<ComponentManager>();
	mEntityManager = std::make_unique<EntityManager>();
	mSystemManager = std::make_unique<SystemManager>();

	RegisterComponent<Transform>();
	RegisterComponent<Camera>();
	RegisterComponent<Character>();
	RegisterComponent<RigidBody>();
	RegisterComponent<StaticMeshRenderer>();
	RegisterComponent<PointLight>();
	RegisterComponent<DirectionalLight>();

	Signature any;
	any.set();

	CharacterSystem* characterSys = RegisterSystem<CharacterSystem>();
	Signature charSysReqSign = MakeSignature<Transform, Character>();
	SetSystemRequiredSignature<CharacterSystem>(charSysReqSign);
	SetSystemOptionalSignature<CharacterSystem>(any);

	RenderSystem* renderSys = RegisterSystem<RenderSystem>();
	Signature renderSysReqSign = MakeSignature<Transform>();
	Signature renderSysOptSign = MakeSignature<StaticMeshRenderer, DirectionalLight, PointLight, Camera>();
	SetSystemRequiredSignature<RenderSystem>(renderSysReqSign);
	SetSystemOptionalSignature<RenderSystem>(renderSysOptSign);
	renderSys->SetRenderApi(&OpenGlApi::Get());

	TransformSystem* transformSys = RegisterSystem<TransformSystem>();
	Signature transformSysReqSign = MakeSignature<Transform>();
	SetSystemRequiredSignature<TransformSystem>(transformSysReqSign);
	SetSystemOptionalSignature<TransformSystem>(any);

	PhysicsSystem* physicsSys = RegisterSystem<PhysicsSystem>();
	Signature physicsSysReqSign = MakeSignature<Transform, RigidBody>();
	SetSystemRequiredSignature<PhysicsSystem>(physicsSysReqSign);
	SetSystemOptionalSignature<PhysicsSystem>(any);

	MeshModel& DefaultPlane = AssetLoader::Get().LoadMeshModelFromFile(
		"Assets/Meshes/default_plane.obj");
	MeshModel& DefaultCube = AssetLoader::Get().LoadMeshModelFromFile(
		"Assets/Meshes/default_cube.obj");
	MeshModel& backpackModel = AssetLoader::Get().LoadMeshModelFromFile(
		"Assets/Meshes/SurvivalGuitarBackpack/backpack.obj");

	Window::Get().SetMouseCursorVisibility(false);

	Entity firstPersonCameraEntity = CreateEntity();
	AddComponent(firstPersonCameraEntity, Camera{});
	AddComponent(firstPersonCameraEntity, Transform{ Vec3(0), Vec3(0) });
	Camera& firstPersonCamera = GetComponent<Camera>(firstPersonCameraEntity);
	firstPersonCamera.fov = 60.0f;
	renderSys->SetActiveCamera(firstPersonCameraEntity);

	Entity character = CreateEntity();
	AddComponent(character, Transform{ Vec3(0, 3, 0), Vec3(0) });
	AddComponent(character, Character{});
	Character& characterC = GetComponent<Character>(character);
	characterC.camera = firstPersonCameraEntity;

	Entity ground = CreateEntity();
	AddComponent(ground, Transform{ Vec3(0.0f, -1.5f, 0.0f), NullVec3, Vec3(1000.0f, 1.0f, 1000.0f) });
	AddComponent(ground, StaticMeshRenderer{ DefaultPlane.meshes });

	uint32_t nBackpacks = 5;
	std::vector<Transform*> bkpTransforms;
	for (uint32_t i = 0; i < nBackpacks; i++) {
		Entity backpack = CreateEntity();
		AddComponent(backpack, Transform{ Utils::RandomPointInSphere(15.f, Vec3(0.0f, 12.0f, 0.0f)), Utils::RandomEulerRotation(), Vec3(Utils::RandomFloat() + Vec3(0.5f)) });
		AddComponent(backpack, StaticMeshRenderer{ backpackModel.meshes });
		bkpTransforms.push_back(&GetComponent<Transform>(backpack));
	}

	uint32_t nCubes = 10;
	std::vector<Transform*> cubeTransforms;
	for (uint32_t i = 0; i < nCubes; i++) {
		Entity cube = CreateEntity();
		AddComponent(cube, Transform{ Utils::RandomPointInSphere(15.f, Vec3(0.0f, 100.0f, 0.0f)), Vec3(0), Vec3(1) });
		// AddComponent(cube, Transform{ Vec3(0, 100, 0), Vec3(0.0f), Vec3(1.0) });
		AddComponent(cube, StaticMeshRenderer{ DefaultCube.meshes });
		AddComponent(cube, RigidBody());
		cubeTransforms.push_back(&GetComponent<Transform>(cube));
	}

	Entity pointLight = CreateEntity();
	float intensity = 200.0f;
	Vec3 pos = Vec3(-5.0f, 12.0f, 0.0f);
	Vec3 color = Vec3(0.5f, 0.1f, 0.05f);
	AddComponent(pointLight, Transform{ pos, Vec3(0) });
	AddComponent(pointLight, PointLight{ color, intensity });

	Entity pointLight1 = CreateEntity();
	pos = Vec3(0.0f, 12.0f, 0.0f);
	color = Vec3(0.1f, 0.5f, 0.05f);
	AddComponent(pointLight1, Transform{ pos, Vec3(0) });
	AddComponent(pointLight1, PointLight{ color, intensity });

	Entity sun = CreateEntity();
	AddComponent(sun, Transform{});
	AddComponent(sun, DirectionalLight{});
	DirectionalLight& sunDirLight = GetComponent<DirectionalLight>(sun);
	sunDirLight.intensity = 0.0f;
	sunDirLight.orthoProjSizes = Vec4(5);

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

	for (auto& sys : systems) {
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
