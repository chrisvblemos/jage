#pragma once

#include <Core/Core.h>
#include "GameAsset.h"

struct Vertex {
	glm::vec3 Position = glm::vec3(0.0f);
	glm::vec3 Normal = glm::vec3(0.0f);
	glm::vec2 TexCoords = glm::vec2(0.0f);
};

struct StaticMesh : public GameAsset {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	AssetId material = -1;

	StaticMesh() = default;
	StaticMesh(const std::string& assetName) : GameAsset(assetName) {};
};
