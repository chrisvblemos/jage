#pragma once

#include <Core/Core.h>
#include "GameAsset.h"

struct MeshModel : public GameAsset {
	std::vector<AssetId> meshes;

	MeshModel() = default;
	MeshModel(const std::string& assetName) : GameAsset(assetName) {};
	MeshModel(const AssetId id) : GameAsset(id) {};
};
