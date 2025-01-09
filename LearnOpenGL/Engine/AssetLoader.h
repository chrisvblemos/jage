#pragma once

#include "MeshModel.h"
#include "StaticMesh.h"
#include "Texture.h"
#include "GameAsset.h"

#include <vector>
#include <iostream>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb/stb_image.h>
#include <filesystem>

class AssetLoader {
private:
	AssetLoader() = default;

	void ProcessObjNode(aiNode* node, const aiScene* scene, const std::string& path, MeshModel* meshModel);
	StaticMesh* ProcessMesh(aiMesh* mesh, const aiScene* scene, const std::string& path);
	std::vector<Texture*> LoadTexturesFromMaterial(aiMaterial* mat, aiTextureType aiType, const std::string& path);

public:
	// prevents copying
	AssetLoader(const AssetLoader&) = delete;
	AssetLoader& operator=(const AssetLoader&) = delete;

	static AssetLoader& Get() {
		static AssetLoader instance;
		return instance;
	}

	MeshModel* LoadMeshModelFromFile(const std::string& path);
	Texture* LoadTextureFromFile(const std::string& path);
};