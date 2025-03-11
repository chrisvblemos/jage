#pragma once

#include <Core/Core.h>
#include <ECS/EntityManager.h>
#include <ECS/ComponentManager.h>
#include <ECS/SystemManager.h>
#include <ECS/Systems/CharacterSystem.h>
#include <ECS/Systems/TransformSystem.h>
#include <ECS/Systems/RenderSystem.h>
#include <ECS/Systems/PhysicsSystem.h>

class World {
private:
	World() = default;
	PhysicsSystem* physicsSys;
	CharacterSystem* characterSys;
	TransformSystem* transformSys;
	RenderSystem* renderSys;

public:
	// prevents copying
	World(const World&) = delete;
	World& operator=(const World&) = delete;

	static World& Get() {
		static World instance;
		return instance;
	}

	bool Initialize();
	Entity CreateEntity();
	void DestroyEntity(const Entity entity);
	void Update(float dt);
	const std::string GetEntityName(const Entity entity) const;
	const std::vector<Entity> GetEntities() const;

	const Signature GetEntitySignature(Entity entity) const;

	template<typename T>
	void RegisterComponent()
	{
		mComponentManager->RegisterComponent<T>();
	}

	template<typename T>
	void AddComponent(const Entity entity, T component)
	{
		mComponentManager->AddComponent<T>(entity, component);

		auto signature = mEntityManager->GetSignature(entity);
		signature.set(mComponentManager->GetComponentType<T>(), true);
		mEntityManager->SetSignature(entity, signature);

		mSystemManager->EntitySignatureChanged(entity, signature);
	}

	template <typename... ComponentType>
	Signature MakeSignature() {
		Signature sig;
		(sig.set(GetComponentType<ComponentType>()), ...);
		return sig;
	}

	template<typename T>
	void RemoveComponent(const Entity entity)
	{
		mComponentManager->RemoveComponent<T>(entity);

		auto signature = mEntityManager->GetSignature(entity);
		signature.set(mComponentManager->GetComponentType<T>(), false);
		mEntityManager->SetSignature(entity, signature);

		mSystemManager->EntitySignatureChanged(entity, signature);
	}

	template<typename T>
	T& GetComponent(const Entity entity)
	{
		return mComponentManager->GetComponent<T>(entity);
	}

	template<typename T>
	ComponentType GetComponentType()
	{
		return mComponentManager->GetComponentType<T>();
	}


	template<typename T>
	T* RegisterSystem()
	{
		return mSystemManager->RegisterSystem<T>();
	}

	template<typename T>
	void SetSystemRequiredSignature(const Signature signature)
	{
		mSystemManager->SetRequiredSignature<T>(signature);
	}

	template<typename T>
	void SetSystemOptionalSignature(const Signature signature)
	{
		mSystemManager->SetOptionalSignature<T>(signature);
	}

private:
	std::unique_ptr<ComponentManager> mComponentManager;
	std::unique_ptr<EntityManager> mEntityManager;
	std::unique_ptr<SystemManager> mSystemManager;
};