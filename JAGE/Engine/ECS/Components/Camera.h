#pragma once

#include <Core/Core.h>

struct Camera {
	bool isOrthogonal = false;
	float fov = 60.0f;
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;
	float aspectRatio = 4.0f / 3.0f;
	bool isActive = false;
	Mat4 viewMatrix = Id4;
	Mat4 projMatrix = Id4;
};