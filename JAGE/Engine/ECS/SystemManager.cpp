#include "SystemManager.h"

void SystemManager::EntityDestroyed(Entity entity) {
	for (auto const& pair : mSystems) {
		auto const& system = pair.second;
		system->UnRegister(entity);
	}
}

void SystemManager::EntitySignatureChanged(Entity entity, Signature entitySignature) {
	for (auto const& pair : mSystems) {
		auto const& type = pair.first;
		auto const& system = pair.second;

		auto const& systemRequiredSignatureMask = mRequiredSignatures[type];
		auto const& systemOptionalSignatureMask = mOptionalSignatures[type];

		// 1. must have all required components
		// 2. must have at least one of the optional components
		if ((entitySignature & systemRequiredSignatureMask) == systemRequiredSignatureMask) {
			if (systemOptionalSignatureMask.none() || (entitySignature & systemOptionalSignatureMask).any()) {
				system->Register(entity);
				LOG(LogSystemManager, LOG_VERBOSE, "Registered entity " + std::to_string(entity) + " to " + (system->name));
			}
		}
		else {
			if (system->GetEntityCount() > 0) {
				LOG(LogSystemManager, LOG_VERBOSE, "Unregistered entity " + std::to_string(entity) + " from " + (system->name));
				system->UnRegister(entity);
			}
		}
	}
}