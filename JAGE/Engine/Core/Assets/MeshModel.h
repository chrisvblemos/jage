#pragma once

#include "GameAsset.h"

struct MeshModel : public GameAsset {
	std::vector<Asset> meshes;

	MeshModel() = default;
	MeshModel(const std::string& assetName) : GameAsset(assetName) {};
	MeshModel(const Asset id) : GameAsset(id) {};
};
