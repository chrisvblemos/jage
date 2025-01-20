#pragma once

#include <Core/Core.h>
#include "Texture.h"
#include "GameAsset.h"

struct Material : public GameAsset {
	glm::vec3 mBaseColor = glm::vec3(1.0f);
	float mBaseSpecular = 0.5f;

	AssetId diffuseTexture = -1;
	AssetId specularTexture = -1;
	AssetId roughnessTexture = -1;
	AssetId normalTexture = -1;

	Material() = default;
	Material(const std::string& assetName) : GameAsset(assetName) {};
	Material(const AssetId id) : GameAsset(id) {};
};