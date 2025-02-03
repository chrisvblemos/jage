#pragma once

#include <Core/Core.h>

struct Texture;
struct MeshModel;
struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

class AssetLoader {
private:
	AssetLoader() = default;

	void ProcessObjNode(aiNode* node, const aiScene* scene, const std::string& path, MeshModel& meshModel);
	AssetId ProcessMesh(aiMesh* mesh, const aiScene* scene, const std::string& path);
	std::vector<AssetId> LoadTexturesFromMaterial(aiMaterial* mat, const aiTextureType aiType, const std::string& path);

public:
	// prevents copying
	AssetLoader(const AssetLoader&) = delete;
	AssetLoader& operator=(const AssetLoader&) = delete;

	static AssetLoader& Get() {
		static AssetLoader instance;
		return instance;
	}

	MeshModel& LoadMeshModelFromFile(const std::string& path);
	Texture& LoadTextureFromFile(const std::string& path, const uint8_t texType);
};