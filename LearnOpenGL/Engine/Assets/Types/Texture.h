#pragma once

#include <Engine/Core.h>
#include "GameAsset.h"

struct Texture : public GameAsset {
	unsigned char* data = nullptr;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t nrChannels = 0;

	Texture() = default;
	Texture(const std::string& assetName) : GameAsset(assetName) {};
};