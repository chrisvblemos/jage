#pragma once

#include "Texture.h"
#include "GameAsset.h"

struct Material : public GameAsset {
	glm::vec3 mBaseColor = glm::vec3(1.0f);
	float mBaseSpecular = 0.5f;

	Asset diffuseTexture = -1;
	Asset specularTexture = -1;
	Asset roughnessTexture = -1;
	Asset normalTexture = -1;

	Material() = default;
	Material(const std::string& assetName) : GameAsset(assetName) {};
	Material(const Asset id) : GameAsset(id) {};
};