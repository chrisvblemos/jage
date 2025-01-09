#pragma once

#include "GameAsset.h"

struct Texture : public GameAsset {
	unsigned char* data = nullptr;
	int width = 0;
	int height = 0;
	int nrChannels = 0;

	Texture() = default;
	Texture(const std::string& assetName) : GameAsset(assetName) {};
};