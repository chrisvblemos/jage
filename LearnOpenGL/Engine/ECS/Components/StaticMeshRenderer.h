#pragma once

#include <Engine/Core.h>

struct StaticMeshRenderer {
	std::vector<AssetId> meshes{};

	StaticMeshRenderer() = default;
	StaticMeshRenderer(const std::vector<AssetId>& meshes) : meshes(meshes) {};
};