#pragma once

#include "GameAsset.h"
#include "Material.h"
#include <vector>
#include <glm/glm.hpp>

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct StaticMesh : public GameAsset {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	Material* material;

	uint32_t VAO = 0, VBO = 0, EBO = 0, staticMeshInstanceVBO = 0;

	StaticMesh() = default;
	StaticMesh(const std::string& assetName) : GameAsset(assetName) {};

};