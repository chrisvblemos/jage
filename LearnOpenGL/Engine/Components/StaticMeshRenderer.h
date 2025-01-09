#pragma once

#include "../StaticMesh.h"
#include <vector>

struct StaticMeshRenderer {
	std::vector<StaticMesh*> meshes{};

	StaticMeshRenderer() = default;
	StaticMeshRenderer(const std::vector<StaticMesh*>& meshes) : meshes(meshes) {};
};