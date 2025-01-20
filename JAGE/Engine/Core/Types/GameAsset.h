#pragma once

#include <Core/Core.h>

struct GameAsset {
public:
	AssetId assetId = -1;
	std::string assetName;
	std::string assetPath;

	GameAsset() = default;
	GameAsset(const std::string& assetName) : assetName(assetName) {};
	GameAsset(const AssetId id) : assetId(id) {};
};