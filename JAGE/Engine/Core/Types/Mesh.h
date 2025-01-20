#pragma once

#include <Core/Core.h>
#include "GameAsset.h"

struct Vertex {
	Vertex() = default;
	glm::vec3 Position = glm::vec3(0.0f);
	glm::vec3 Normal = glm::vec3(0.0f);
	glm::vec2 TexCoords = glm::vec2(0.0f);
};

struct Mesh : public GameAsset {
	GLuint glID = 0;				// open gl id when buffered
	bool mGPU = false;				// is this asset data on the GPU

	uint32_t mGlInstanceCount = 0;
	uint32_t mGlBaseInstance = 0;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	AssetId diffuseTexture = -1;
	AssetId specularTexture = -1;
	AssetId normalTexture = -1;

	Mesh() = default;
	Mesh(const std::string& assetName) : GameAsset(assetName) {};
	Mesh(const AssetId id) : GameAsset(id) {};
};
