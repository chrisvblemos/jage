#pragma once

#include "GameAsset.h"
#include "Shader.h"
#include "Texture.h"
#include <string>
#include <glm/glm.hpp>

struct Material : public GameAsset {
	glm::vec3 mBaseColor{ 1.0f };
	float mBaseSpecular{ 0.5f };

	Texture* diffuseTexture;
	Texture* specularTexture;
	Texture* roughnessTexture;
	Texture* normalTexture;
	Shader* shader;

	Material() = default;
	Material(const std::string& assetName) : GameAsset(assetName) {};

	bool HasDiffuseTexture() const {
		return (diffuseTexture != nullptr);
	}

	bool HasSpecularTexture() const {
		return (specularTexture != nullptr);
	}

	bool HasRoughnessTexture() const {
		return (roughnessTexture != nullptr);
	}

	bool HasNormalTexture() const {
		return (normalTexture != nullptr);
	}

	bool HasCustomShader() const {
		return (shader != nullptr);
	}

};