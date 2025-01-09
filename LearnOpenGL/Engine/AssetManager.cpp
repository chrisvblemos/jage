#include "Renderer/OpenGL.h"
#include <assimp/postprocess.h>
#include <filesystem>
#include "AssetManager.h"

template <>
MeshModel* AssetManager::GetAssetById<MeshModel>(ASSET_ID assetId) {
	auto it = meshModels.find(assetId);
	return (it != meshModels.end()) ? &it->second : nullptr;
}

template <>
StaticMesh* AssetManager::GetAssetById<StaticMesh>(ASSET_ID assetId) {
	auto it = staticMeshes.find(assetId);
	return (it != staticMeshes.end()) ? &it->second : nullptr;
}

template <>
Material* AssetManager::GetAssetById<Material>(ASSET_ID assetId) {
	auto it = materials.find(assetId);
	return (it != materials.end()) ? &it->second : nullptr;
}

template <>
Texture* AssetManager::GetAssetById<Texture>(ASSET_ID assetId) {
	auto it = textures.find(assetId);
	return (it != textures.end()) ? &it->second : nullptr;
}

template <>
Shader* AssetManager::GetAssetById<Shader>(ASSET_ID assetId) {
	auto it = shaders.find(assetId);
	return (it != shaders.end()) ? &it->second : nullptr;
}

template <>
MeshModel* AssetManager::GetAssetByPath<MeshModel>(const std::string& assetPath) {
	for (auto& [id, asset] : meshModels) {
		if (asset.assetPath == assetPath) {
			return &asset;
		}
	}

	return nullptr;
}

template <>
StaticMesh* AssetManager::GetAssetByPath<StaticMesh>(const std::string& assetPath) {
	for (auto& [id, asset] : staticMeshes) {
		if (asset.assetPath == assetPath) {
			return &asset;
		}
	}

	return nullptr;
}

template <>
Material* AssetManager::GetAssetByPath<Material>(const std::string& assetPath) {
	for (auto& [id, asset] : materials) {
		if (asset.assetPath == assetPath) {
			return &asset;
		}
	}

	return nullptr;
}

template <>
Texture* AssetManager::GetAssetByPath<Texture>(const std::string& assetPath) {
	for (auto& [id, asset] : textures) {
		if (asset.assetPath == assetPath) {
			return &asset;
		}
	}

	return nullptr;
}

template <>
Shader* AssetManager::GetAssetByPath<Shader>(const std::string& assetPath) {
	for (auto& [id, asset] : shaders) {
		if (asset.assetPath == assetPath) {
			return &asset;
		}
	}

	return nullptr;
}

template<>
std::vector<MeshModel*> AssetManager::GetAssets<MeshModel>() {
	std::vector<MeshModel*> result;
	for (auto& [id, asset] : meshModels) {
		result.push_back(&asset);
	}

	return result;
}

template<>
std::vector<StaticMesh*> AssetManager::GetAssets<StaticMesh>() {
	std::vector<StaticMesh*> result;
	for (auto& [id, asset] : staticMeshes) {
		result.push_back(&asset);
	}

	return result;
}

template<>
std::vector<Material*> AssetManager::GetAssets<Material>() {
	std::vector<Material*> result;
	for (auto& [id, asset] : materials) {
		result.push_back(&asset);
	}

	return result;
}

template<>
std::vector<Texture*> AssetManager::GetAssets<Texture>() {
	std::vector<Texture*> result;
	for (auto& [id, asset] : textures) {
		result.push_back(&asset);
	}

	return result;
}

template<>
std::vector<Shader*> AssetManager::GetAssets<Shader>() {
	std::vector<Shader*> result;
	for (auto& [id, asset] : shaders) {
		result.push_back(&asset);
	}

	return result;
}

template<>
MeshModel* AssetManager::CreateAsset<MeshModel>() {
	uint32_t assetId = Utils::AllocateIdFromPool(assetIdPool, assetCount);
	auto [it, result] = meshModels.emplace(assetId, MeshModel());
	it->second.assetId = assetId;
	return &it->second;
}

template<>
StaticMesh* AssetManager::CreateAsset<StaticMesh>() {
	uint32_t assetId = Utils::AllocateIdFromPool(assetIdPool, assetCount);
	auto [it, result] = staticMeshes.emplace(assetId, StaticMesh());
	it->second.assetId = assetId;
	return &it->second;
}

template<>
Material* AssetManager::CreateAsset<Material>() {
	uint32_t assetId = Utils::AllocateIdFromPool(assetIdPool, assetCount);
	auto [it, result] = materials.emplace(assetId, Material());
	it->second.assetId = assetId;
	return &it->second;
}

template<>
Texture* AssetManager::CreateAsset<Texture>() {
	uint32_t assetId = Utils::AllocateIdFromPool(assetIdPool, assetCount);
	auto [it, result] = textures.emplace(assetId, Texture());
	it->second.assetId = assetId;
	return &it->second;
}

template<>
Shader* AssetManager::CreateAsset<Shader>() {
	uint32_t assetId = Utils::AllocateIdFromPool(assetIdPool, assetCount);
	auto [it, result] = shaders.emplace(assetId, Shader());
	it->second.assetId = assetId;
	return &it->second;
}
