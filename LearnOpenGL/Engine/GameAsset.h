#pragma once

#include <string>

typedef uint32_t ASSET_ID;

struct GameAsset {
public:
	ASSET_ID assetId;
	std::string assetName;
	std::string assetPath;

	GameAsset() = default;
	GameAsset(const std::string& assetName) : assetName(assetName) {};
};