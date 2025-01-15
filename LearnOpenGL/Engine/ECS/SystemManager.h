#pragma once

/* Manages all systems in the game. */

#include <Engine/Core.h>
#include "Systems/System.h"

class SystemManager {
public:
	template <typename T>
	std::shared_ptr<T> RegisterSystem() {
		const char* typeName = typeid(T).name();
		assert(mSystems.find(typeName) == mSystems.end() && "SystemManager: System already registered.");

		auto system = std::make_shared<T>();
		mSystems.insert({ typeName, system });
		return system;
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

	void EntityDestroyed(Entity entity);
	void EntitySignatureChanged(Entity entity, Signature entitySignature);

private:
	std::unordered_map<const char*, Signature> mRequiredSignatures{};
	std::unordered_map<const char*, Signature> mOptionalSignatures{};
	std::unordered_map<const char*, std::shared_ptr<System>> mSystems{};
};