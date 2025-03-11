#pragma once

#include <Core/Core.h>

struct AABBRender {
	Vec3 max{ 0.0f };
	Vec3 min{ 0.0f };
};

struct StaticMeshRenderer {
	std::vector<Asset> meshes;
	Mat4 modelMatrix = Id4;
	Mat4 inverseModelMatrix = Id4;
	AABBRender aabb;
	bool isInView = false;
};