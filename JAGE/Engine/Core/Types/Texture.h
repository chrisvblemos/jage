#pragma once

#include <Core/Core.h>
#include "GameAsset.h"

enum TextureAssetType : uint8_t {
	Albedo,
	Normal,
	Specular,
	Roughness,
	AO,
	Metallic,
	Linear,
	Count
};

struct Texture : public GameAsset {
	GLuint glID = 0;
	GLuint64 glHandle = 0;
	GLint handleIndex = -1;
	TextureAssetType type = TextureAssetType::Albedo;
	bool hasAlphaChannel = false;
	bool inLinearSpace = false;

	unsigned char* data = nullptr;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t nrChannels = 0;

	Texture() = default;
	Texture(const std::string& assetName) : GameAsset(assetName) {};
	Texture(const AssetId id) : GameAsset(id) {};
};