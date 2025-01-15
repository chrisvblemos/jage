#pragma once

#include <Engine/Core.h>
#include "GameAsset.h"

struct MeshModel : public GameAsset {
	std::vector<AssetId> meshes;

	MeshModel() = default;
	MeshModel(const std::string& assetName) : GameAsset(assetName) {};
};
