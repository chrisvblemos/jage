#pragma once

#include <Core/Core.h>

struct StaticMeshRenderer {
	std::vector<Asset> meshes;

	StaticMeshRenderer() = default;
	StaticMeshRenderer(const std::vector<Asset>& meshes) : meshes(meshes) {};
};