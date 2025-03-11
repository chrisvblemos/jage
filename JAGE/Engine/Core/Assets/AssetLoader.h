#pragma once

#include "Common.h"
#include "Texture.h"
#include "MeshModel.h"
#include "Mesh.h"
#include <stb/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class AssetLoader {
private:
	AssetLoader() = default;

	void ProcessObjNode(aiNode* node, const aiScene* scene, const std::string& path, MeshModel& meshModel);
	Asset ProcessMesh(aiMesh* mesh, const aiScene* scene, const std::string& path);
	std::vector<Asset> LoadTexturesFromMaterial(aiMaterial* mat, const aiTextureType aiType, const std::string& path);

public:
	// prevents copying
	AssetLoader(const AssetLoader&) = delete;
	AssetLoader& operator=(const AssetLoader&) = delete;

	static AssetLoader& Get() {
		static AssetLoader instance;
		return instance;
	}

	MeshModel& LoadMeshModelFromFile(const std::string& path, bool flipUV = false);
	Texture& LoadTextureFromFile(const std::string& path, const uint8_t texType);
};