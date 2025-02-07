#pragma once

#include "Common.h"

struct GameAsset {
public:
	Asset assetId = -1;
	std::string assetName;
	std::string assetPath;

	GameAsset() = default;
	GameAsset(const std::string& assetName) : assetName(assetName) {};
	GameAsset(const Asset id) : assetId(id) {};
};