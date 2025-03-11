#include <Core/Core.h>
#include <Renderer/OpenGlApi.h>
#include <filesystem>
#include "AssetManager.h"
#include "AssetLoader.h"

MeshModel& AssetLoader::LoadMeshModelFromFile(const std::string& path, bool flipUV) {
	auto& assetManager = AssetManager::Get();
	auto* existingModel = assetManager.GetAssetByPath<MeshModel>(path);

	if (existingModel) {
		return *existingModel;
	}

	std::unique_ptr<Assimp::Importer> importer = std::make_unique<Assimp::Importer>();

	uint32_t readFlags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes;
	if (flipUV)
		readFlags |= aiProcess_FlipUVs;
	const aiScene* scene = importer->ReadFile(path, readFlags);

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
		Asset meshAssetId = ProcessMesh(aimesh, scene, path);
		meshModel.meshes.push_back(meshAssetId);
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++) {
		ProcessObjNode(node->mChildren[i], scene, path, meshModel);
	}
}

Asset AssetLoader::ProcessMesh(aiMesh* aiMesh, const aiScene* scene, const std::string& path) {
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
		std::vector<Asset> diffuseTextures = LoadTexturesFromMaterial(aiMat, aiTextureType_DIFFUSE, path);
		std::vector<Asset> specularTextures = LoadTexturesFromMaterial(aiMat, aiTextureType_SPECULAR, path);
		std::vector<Asset> normalTextures = LoadTexturesFromMaterial(aiMat, aiTextureType_NORMALS, path);

		// create material assigned to this mesh
		if (!diffuseTextures.empty())
			newMesh.diffuseTexture = diffuseTextures[0];
		if (!specularTextures.empty())
			newMesh.specularTexture = specularTextures[0];
		if (!normalTextures.empty())
			newMesh.specularTexture = normalTextures[0];
	}

	OpenGlApi::Get().RegisterMesh(&newMesh);

	return newMesh.assetId;
}

std::vector<Asset> AssetLoader::LoadTexturesFromMaterial(aiMaterial* mat, const aiTextureType aiType, const std::string& path) {
	std::vector<Asset> textures;

	TextureAssetType texType;
	if (aiType == aiTextureType_BASE_COLOR || aiType == aiTextureType_DIFFUSE) {
		texType = TextureAssetType::Albedo;
	}
	else if (aiType == aiTextureType_AMBIENT_OCCLUSION) {
		texType = TextureAssetType::AO;
	}
	else if (aiType == aiTextureType_METALNESS) {
		texType = TextureAssetType::Metallic;
	}
	else if (aiType == aiTextureType_SPECULAR) {
		texType = TextureAssetType::Specular;
	}
	else if (aiType == aiTextureType_NORMALS)
	{
		texType = TextureAssetType::Normal;
	}
	else {
		LOG(LogAssetLoader, LOG_CRITICAL, std::format("Failed to infer texture type from texture at {}", path));
	}

	for (uint32_t i = 0; i < mat->GetTextureCount(aiType); i++) {
		aiString fileName;
		mat->GetTexture(aiType, i, &fileName);
		std::filesystem::path pathObj(path);
		std::filesystem::path texturePathObj = pathObj.parent_path() / std::string(fileName.C_Str());
		Texture& texture = LoadTextureFromFile(texturePathObj.string(), texType);
		textures.push_back(texture.assetId);
	}

	return textures;
}

Texture& AssetLoader::LoadTextureFromFile(const std::string& path, const uint8_t texType) {
	auto& assetManager = AssetManager::Get();
	Texture* existingTexture = assetManager.GetAssetByPath<Texture>(path);
	if (existingTexture) {
		return *existingTexture;
	}

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.data(), &width, &height, &nrChannels, 0);

	if (!data || width == 0) {
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
	texture.hasAlphaChannel = nrChannels > 3;
	texture.type = static_cast<TextureAssetType>(texType);

	OpenGlApi::Get().RegisterTexture2D(&texture);

	stbi_image_free(data);
	return texture;
}