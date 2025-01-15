#pragma once

#include <Engine/Core.h>
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

	bool HasDiffuseTexture() const {
		return (diffuseTexture != -1);
	}

	bool HasSpecularTexture() const {
		return (specularTexture != -1);
	}

	bool HasRoughnessTexture() const {
		return (roughnessTexture != -1);
	}

	bool HasNormalTexture() const {
		return (normalTexture != -1);
	}
};