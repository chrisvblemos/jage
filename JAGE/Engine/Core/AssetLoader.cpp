#include <Core/Core.h>
#include <Renderer/OpenGL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb/stb_image.h>
#include <filesystem>

#include "Types/MeshModel.h"
#include "Types/Material.h"
#include "Types/Mesh.h"
#include "Types/Texture.h"

#include "AssetManager.h"
#include "AssetLoader.h"

MeshModel& AssetLoader::LoadMeshModelFromFile(const std::string& path) {
	auto& assetManager = AssetManager::Get();
	auto* existingModel = assetManager.GetAssetByPath<MeshModel>(path);

	if (existingModel) {
		return *existingModel;
	}

	std::unique_ptr<Assimp::Importer> importer = std::make_unique<Assimp::Importer>();
	const aiScene* scene = importer->ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG("AssimpImporter", LOG_ERROR, importer->GetErrorString());
		LOG(LogAssetLoader, LOG_CRITICAL, "Failed to load model from file at " + path);
	}

	auto& meshModel = assetManager.CreateAsset<MeshModel>();
	meshModel.assetPath = path;

	ProcessObjNode(scene->mRootNode, scene, path, meshModel);

	if (meshModel.meshes.size() == 0) {
		LOG(LogAssetLoader, LOG_CRITICAL, std::format("Failed to load model from file {}", path));
	}

	return meshModel;
}

void AssetLoader::ProcessObjNode(aiNode* node, const aiScene* scene, const std::string& path, MeshModel& meshModel) {
	for (uint32_t i = 0; i < node->mNumMeshes; i++) {
		aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];
		AssetId meshAssetId = ProcessMesh(aimesh, scene, path);
		meshModel.meshes.push_back(meshAssetId);
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++) {
		ProcessObjNode(node->mChildren[i], scene, path, meshModel);
	}
}

AssetId AssetLoader::ProcessMesh(aiMesh* aiMesh, const aiScene* scene, const std::string& path) {
	auto& assetManager = AssetManager::Get();
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (uint32_t i = 0; i < aiMesh->mNumVertices; i++) {
		Vertex vertex;

		glm::vec3 vector;
		vector.x = aiMesh->mVertices[i].x;
		vector.y = aiMesh->mVertices[i].y;
		vector.z = aiMesh->mVertices[i].z;

		vertex.Position = vector;

		vector.x = aiMesh->mNormals[i].x;
		vector.y = aiMesh->mNormals[i].y;
		vector.z = aiMesh->mNormals[i].z;

		vertex.Normal = vector;

		if (aiMesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			vec.x = aiMesh->mTextureCoords[0][i].x;
			vec.y = aiMesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	};

	for (uint32_t i = 0; i < aiMesh->mNumFaces; i++) {
		aiFace face = aiMesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	};

	Mesh& newMesh = assetManager.CreateAsset<Mesh>();
	newMesh.assetPath = path;
	newMesh.vertices = vertices;
	newMesh.indices = indices;

	if (aiMesh->mMaterialIndex >= 0) {
		aiMaterial* aiMat = scene->mMaterials[aiMesh->mMaterialIndex];
		std::vector<AssetId> diffuseTextures = LoadTexturesFromMaterial(aiMat, aiTextureType_DIFFUSE, path);
		std::vector<AssetId> specularTextures = LoadTexturesFromMaterial(aiMat, aiTextureType_SPECULAR, path);
		std::vector<AssetId> normalTextures = LoadTexturesFromMaterial(aiMat, aiTextureType_NORMALS, path);

		// create material assigned to this mesh
		if (!diffuseTextures.empty())
			newMesh.diffuseTexture = diffuseTextures[0];
		if (!specularTextures.empty())
			newMesh.specularTexture = specularTextures[0];
		if (!normalTextures.empty())
			newMesh.specularTexture = normalTextures[0];
	}

	OpenGL::Get().RegisterMesh(&newMesh);

	return newMesh.assetId;
}

std::vector<AssetId> AssetLoader::LoadTexturesFromMaterial(aiMaterial* mat, aiTextureType aiType, const std::string& path) {
	std::vector<AssetId> textures;

	for (uint32_t i = 0; i < mat->GetTextureCount(aiType); i++) {
		aiString fileName;
		mat->GetTexture(aiType, i, &fileName);
		std::filesystem::path pathObj(path);
		std::filesystem::path texturePathObj = pathObj.parent_path() / std::string(fileName.C_Str());

		Texture& texture = LoadTextureFromFile(texturePathObj.string());
		textures.push_back(texture.assetId);
	}

	return textures;
}

Texture& AssetLoader::LoadTextureFromFile(const std::string& path) {
	auto& assetManager = AssetManager::Get();
	Texture* existingTexture = assetManager.GetAssetByPath<Texture>(path);
	if (existingTexture) {
		return *existingTexture;
	}

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.data(), &width, &height, &nrChannels, 0);

	if (!data) {
		LOG(LogAssetLoader, LOG_CRITICAL, "Failed to load texture from " + path);
		stbi_image_free(data);
	}

	LOG(LogAssetLoader, LOG_VERBOSE, std::format("Loaded texture from {}. Width: {} | Height: {}", path, width, height));
	auto& texture = assetManager.CreateAsset<Texture>();
	texture.assetPath = path;
	texture.data = data;
	texture.height = height;
	texture.width = width;
	texture.nrChannels = nrChannels;

	OpenGL::Get().RegisterTexture2D(&texture);

	stbi_image_free(data);

	if (texture.width == 0) {
		LOG(LogAssetLoader, LOG_CRITICAL, std::format("Failed to load texture from {}", path));
	}

	return texture;
}