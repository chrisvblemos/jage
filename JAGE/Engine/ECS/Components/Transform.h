#pragma once

#include <Core/Core.h>

struct Transform {
	Transform(
		Vec3 position = Vec3(0.0f),
		Vec3 rotation = Vec3(0.0f),
		Vec3 scale = Vec3(1.0f))
		: position(position), rotation(rotation), scale(scale) {
	};

	Vec3 position = Vec3(0.0f);
	Vec3 rotation = Vec3(0.0f);
	Vec3 scale = Vec3(1.0f);
	Vec3 forward = WForward;
	Vec3 right = WRight;
	Vec3 up = WUp;
};