#include <Engine/Core.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb/stb_image.h>
#include <filesystem>

#include "Types/MeshModel.h"
#include "Types/Material.h"
#include "Types/StaticMesh.h"
#include "Types/Texture.h"

#include "AssetManager.h"
#include "AssetLoader.h"

MeshModel* AssetLoader::LoadMeshModelFromFile(const std::string& path) {
	if (MeshModel* mm = AssetManager::Get().GetAssetByPath<MeshModel>(path)) {
		return mm; // meshmodel already loaded, ignore it
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "AssetLoader: Failed to load model from file at " << path << std::endl;
		std::cout << "AssimpImporter: " << importer.GetErrorString() << std::endl;
		return nullptr;
	}

	MeshModel* mm = AssetManager::Get().CreateAsset<MeshModel>();
	ProcessObjNode(scene->mRootNode, scene, path, mm);
	mm->assetPath = path;

	return mm;
}

void AssetLoader::ProcessObjNode(aiNode* node, const aiScene* scene, const std::string& path, MeshModel* meshModel) {
	for (uint32_t i = 0; i < node->mNumMeshes; i++) {
		aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];
		AssetId mesh = ProcessMesh(aimesh, scene, path);
		meshModel->meshes.push_back(mesh);
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++) {
		ProcessObjNode(node->mChildren[i], scene, path, meshModel);
	}
}

AssetId AssetLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene, const std::string& path) {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;

		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;

		vertex.Position = vector;

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;

		vertex.Normal = vector;

		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	};

	for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	};

	StaticMesh* newMesh = AssetManager::Get().CreateAsset<StaticMesh>();
	newMesh->assetPath = path;
	newMesh->vertices = vertices;
	newMesh->indices = indices;

	Material* material = AssetManager::Get().CreateAsset<Material>();

	// copy material data from file
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<AssetId> diffuseTextures = LoadTexturesFromMaterial(aiMat, aiTextureType_DIFFUSE, path);
		std::vector<AssetId> specularTextures = LoadTexturesFromMaterial(aiMat, aiTextureType_SPECULAR, path);

		// create material assigned to this mesh
		if (!diffuseTextures.empty())
			material->diffuseTexture = diffuseTextures[0];
		if (!specularTextures.empty())
			material->specularTexture = specularTextures[0];
	}

	newMesh->material = material->assetId;

	return newMesh->assetId;
}

std::vector<AssetId> AssetLoader::LoadTexturesFromMaterial(aiMaterial* mat, aiTextureType aiType, const std::string& path) {
	std::vector<AssetId> textures;

	for (uint32_t i = 0; i < mat->GetTextureCount(aiType); i++) {
		aiString fileName;
		mat->GetTexture(aiType, i, &fileName);
		std::filesystem::path pathObj(path);
		std::filesystem::path texturePathObj = pathObj.parent_path() / std::string(fileName.C_Str());

		Texture* texture = LoadTextureFromFile(texturePathObj.string());
		textures.push_back(texture->assetId);
	}

	return textures;
}

Texture* AssetLoader::LoadTextureFromFile(const std::string& path) {
	if (Texture* texture = AssetManager::Get().GetAssetByPath<Texture>(path)) {
		return texture;
	}

	//std::string path = directory + "/" + std::string(filename);
	std::cout << "Loading texture at " << path << std::endl;

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.data(), &width, &height, &nrChannels, 0);

	if (!data) {
		std::cout << "Error: Failed to load texture from file " << path << std::endl;
		stbi_image_free(data);
		return 0;
	}

	Texture* newTexture = AssetManager::Get().CreateAsset<Texture>();
	newTexture->assetPath = path;

	if (newTexture) {
		newTexture->data = data;
		newTexture->height = height;
		newTexture->width = width;
		newTexture->nrChannels = nrChannels;
		stbi_image_free(data);
		return newTexture;
	}
	else {
		std::cout << "Error: Failed to load texture from file " << path << std::endl;
		stbi_image_free(data);
		return nullptr;
	}
}