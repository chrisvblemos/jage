#pragma once

/* Manages all systems in the game. */

#include <Core/Core.h>
#include "Systems/System.h"

class SystemManager {
public:
	template <typename T>
	T* RegisterSystem() {
		const char* typeName = typeid(T).name();
		assert(mSystems.find(typeName) == mSystems.end() && "SystemManager: System already registered.");

		auto system = std::make_shared<T>();
		mSystems.insert({ typeName, system });
		return system.get();
	}

	template <typename T>
	void SetRequiredSignature(Signature signature) {
		const char* typeName = typeid(T).name();
		assert(mSystems.find(typeName) != mSystems.end() && "SystemManager: System not registered.");

		mRequiredSignatures.insert({ typeName, signature });
	}

	template <typename T>
	void SetOptionalSignature(Signature signature) {
		const char* typeName = typeid(T).name();
		assert(mSystems.find(typeName) != mSystems.end() && "SystemManager: System not registered.");

		mOptionalSignatures.insert({ typeName, signature });
	}

	std::vector<std::weak_ptr<System>> GetSystems() const {
		std::vector<std::weak_ptr<System>> systems;
		for (auto& it : mSystems) {
			systems.push_back(std::shared_ptr<System>(it.second));
		}

		return systems;
	}

	template <typename T>
	std::weak_ptr<T> GetSystem() const {
		const char* typeName = typeid(T).name();
		auto& it = mSystems.find(typeName);
		assert(it != mSystems.end() && "SystemManager: System not registered.");

		return std::weak_ptr<T>(it.second);
	}

	void EntityDestroyed(Entity entity);
	void EntitySignatureChanged(Entity entity, Signature entitySignature);

private:
	std::unordered_map<const char*, Signature> mRequiredSignatures{};
	std::unordered_map<const char*, Signature> mOptionalSignatures{};
	std::unordered_map<const char*, std::shared_ptr<System>> mSystems{};
};