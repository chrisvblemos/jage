#pragma once

#include "GameAsset.h"
#include "Texture.h"
#include <string>
#include <glm/glm.hpp>

struct Material : public GameAsset {
	Texture* diffuseTexture;
	Texture* specularTexture;
	Texture* roughnessTexture;
	Texture* normalTexture;
	Texture* shader;

	Material() = default;
	Material(const std::string& assetName) : GameAsset(assetName) {};

};