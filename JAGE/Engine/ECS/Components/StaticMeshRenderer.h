#pragma once

#include <Core/Core.h>

struct StaticMeshRenderer {
	std::vector<Asset> meshes;
	Mat4 modelMatrix = Id4;
	Mat4 inverseModelMatrix = Id4;
};