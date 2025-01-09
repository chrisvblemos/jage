#pragma once

#include "GameAsset.h"
#include "StaticMesh.h"
#include <vector> 

struct MeshModel : public GameAsset {
	std::vector<StaticMesh*> meshes;

	MeshModel() = default;
	MeshModel(const std::string& assetName) : GameAsset(assetName) {};
};