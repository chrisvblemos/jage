#pragma once

#include <Core/Core.h>

struct StaticMeshRenderer {
	std::vector<AssetId> meshes;

	StaticMeshRenderer() = default;
	StaticMeshRenderer(const std::vector<AssetId>& meshes) : meshes(meshes) {};
};