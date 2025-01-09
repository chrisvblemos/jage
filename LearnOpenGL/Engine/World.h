#pragma once

#include "EntityManager.h"
#include "ComponentManager.h"
#include "SystemManager.h"
#include <memory>

class World {
private:
	World() = default;
public:
	// prevents copying
	World(const World&) = delete;
	World& operator=(const World&) = delete;

	static World& Get() {
		static World instance;
		return instance;
	}

	bool Initialize();
	void Update(float dt);
	Entity CreateEntity();
	void DestroyEntity(Entity entity);

	const Signature& GetEntitySignature(Entity entity) const;

	template<typename T>
	void RegisterComponent()
	{
		mComponentManager->RegisterComponent<T>();
	}

	template<typename T>
	void AddComponent(Entity entity, T component)
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
	void RemoveComponent(Entity entity)
	{
		mComponentManager->RemoveComponent<T>(entity);

		auto signature = mEntityManager->GetSignature(entity);
		signature.set(mComponentManager->GetComponentType<T>(), false);
		mEntityManager->SetSignature(entity, signature);

		mSystemManager->EntitySignatureChanged(entity, signature);
	}

	template<typename T>
	T& GetComponent(Entity entity)
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
		return mSystemManager->RegisterSystem<T>().get();
	}

	template<typename T>
	void SetSystemRequiredSignature(Signature signature)
	{
		mSystemManager->SetRequiredSignature<T>(signature);
	}

	template<typename T>
	void SetSystemOptionalSignature(Signature signature)
	{
		mSystemManager->SetOptionalSignature<T>(signature);
	}

private:
	std::unique_ptr<ComponentManager> mComponentManager;
	std::unique_ptr<EntityManager> mEntityManager;
	std::unique_ptr<SystemManager> mSystemManager;
};