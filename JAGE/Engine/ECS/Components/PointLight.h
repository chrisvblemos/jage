#pragma once

#include <Core/Core.h>

struct PointLight {
	PointLight() = default;
	PointLight(const Vec3& color, const float intensity)
		: color(color), intensity(intensity) {};

	Vec3 color{ 1.0f };
	float intensity{ 1.0f };
	int glShadowMapIndex{ -1 };
	float shadowMapNearPlane{ 0.1f };
	float shadowMapFarPlane{ 1000.0f };
	Mat4 cubemapMatrices[6];
	Mat4 projMatrix;
};