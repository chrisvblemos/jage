#pragma once

#include "Common.h"
#include "Material.h"
#include "Mesh.h"
#include "MeshModel.h"
#include "Texture.h"

class AssetManager {
public:
	static AssetManager& Get() {
		static AssetManager instance;
		return instance;
	}

	template<typename T>
	T& CreateAsset() {
		auto& container = GetContainer<T>();
		auto asset = std::make_unique<T>();
		asset->assetId = nextId++;
		T& ref = *asset;
		container[asset->assetId] = std::move(asset);
		return ref;
	}

	template<typename T>
	T* GetAssetById(Asset id) {
		auto& container = GetContainer<T>();
		auto it = container.find(id);
		return it != container.end() ? it->second.get() : nullptr;
	}

	template<typename T>
	T* GetAssetByPath(const std::string& path) {
		auto& container = GetContainer<T>();
		for (const auto& pair : container) {
			if (pair.second->assetPath == path) {
				return pair.second.get();
			}
		}
		return nullptr;
	}

private:
	AssetManager() = default;
	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;

	template<typename T>
	std::unordered_map<Asset, std::unique_ptr<T>>& GetContainer();

	template<> std::unordered_map<Asset, std::unique_ptr<Mesh>>& GetContainer<Mesh>() { return meshes; }
	template<> std::unordered_map<Asset, std::unique_ptr<MeshModel>>& GetContainer<MeshModel>() { return meshModels; }
	template<> std::unordered_map<Asset, std::unique_ptr<Material>>& GetContainer<Material>() { return materials; }
	template<> std::unordered_map<Asset, std::unique_ptr<Texture>>& GetContainer<Texture>() { return textures; }

	Asset nextId = 0;

	std::unordered_map<Asset, std::unique_ptr<Mesh>> meshes;
	std::unordered_map<Asset, std::unique_ptr<MeshModel>> meshModels;
	std::unordered_map<Asset, std::unique_ptr<Material>> materials;
	std::unordered_map<Asset, std::unique_ptr<Texture>> textures;


};

