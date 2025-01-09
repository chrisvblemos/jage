#pragma once

#include "Utils.h"
#include "StaticMesh.h"
#include "Material.h"
#include "StaticMesh.h"
#include "Shader.h"
#include "Texture.h"
#include "MeshModel.h"

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <stack>
#include <typeindex>
#include <typeinfo>

class AssetManager {
private:
	AssetManager() = default;

	uint32_t assetCount = 0;
	std::stack<uint32_t> assetIdPool;

	std::unordered_map<ASSET_ID, StaticMesh> staticMeshes;
	std::unordered_map<ASSET_ID, Material> materials;
	std::unordered_map<ASSET_ID, Texture> textures;
	std::unordered_map<ASSET_ID, Shader> shaders;
	std::unordered_map<ASSET_ID, MeshModel> meshModels;

public:
	// prevents copying
	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;

	static AssetManager& Get() {
		static AssetManager instance;
		return instance;
	}

	template <typename T> 
	T* GetAssetById(ASSET_ID id);

	template<typename T>
	T* GetAssetByPath(const std::string& path);

	template<typename T>
	std::vector<T*> GetAssets();

	template <typename T>
	T* CreateAsset();
	
	
};