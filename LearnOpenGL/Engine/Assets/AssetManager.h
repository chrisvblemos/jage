#pragma once

#include <Engine/Utils.h>
#include <Engine/Core.h>
#include "Types/MeshModel.h"
#include "Types/StaticMesh.h"
#include "Types/Texture.h"
#include "Types/Material.h"
#include "Types/GameAsset.h"

class AssetManager {
private:
	AssetManager() = default;

	uint32_t assetCount = 0;
	std::stack<uint32_t> assetIdPool;

	std::unordered_map<const char*, std::vector<std::unique_ptr<GameAsset>>> assets;

public:
	// prevents copying
	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;

	static AssetManager& Get() {
		static AssetManager instance;
		return instance;
	}

	template <typename T>
	std::vector<T*> GetAssets() {
		auto it = assets.find(typeid(T).name());
		if (it == assets.end()) {
			return {};
		}

		std::vector<T*> result;
		for (const auto& asset : it->second) {
			result.push_back(static_cast<T*>(asset.get()));
		}

		return result;
	}

	template <typename T>
	T* GetAssetById(AssetId id) {
		for (const auto& asset : GetAssets<T>()) {
			if (asset->assetId == id) {
				return asset;
			}
		}

		return nullptr;
	}

	template <typename T>
	T* GetAssetByName(const std::string& name) {
		for (const auto& asset : GetAssets<T>()) {
			if (asset->assetName == name) {
				return asset;
			}
		}

		return nullptr;
	}

	template <typename T>
	T* GetAssetByPath(const std::string& path) {
		for (const auto& asset : GetAssets<T>()) {
			if (asset->assetPath == path) {
				return asset;
			}
		}

		return nullptr;
	}

	template <typename T>
	T* CreateAsset() {
		uint32_t assetId = Utils::AllocateIdFromPool(assetIdPool, assetCount);
		const char* type_name = typeid(T).name();
		assets[type_name].push_back(std::make_unique<T>());

		return static_cast<T*>(assets[type_name].back().get());
	}
};