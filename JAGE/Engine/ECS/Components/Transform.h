#pragma once

#include <Core/Core.h>
#include <Core/Utils.h>

using namespace Utils;

struct Transform {
	Transform(const Vec3& position) : position(position) {};

	Transform(
		const Vec3& position = Vec3(0.0f),
		const Vec3& rotation = Vec3(0.0f),
		const Vec3& scale = Vec3(1.0f))
		: position(position), scale(scale) {

		this->rotation = glm::quat(ToRad(rotation));
	};

	Vec3 position = Vec3(0.0f);
	Quat rotation = Quat(Vec3(0.0f));
	Vec3 scale = Vec3(1.0f);
	Vec3 forward = WForward;
	Vec3 right = WRight;
	Vec3 up = WUp;

	/* Gets transform rotation in euler angles (pitch, yaw, roll). */
	Vec3 GetEulerAngles(const bool radians = false) const {
		Mat4 m = glm::mat4_cast(rotation);
		float p, y, r;
		glm::extractEulerAngleXYZ(m, p, y, r);

		if (!radians)
			return ToDegrees(Vec3(p, y, r));
		else
			return Vec3(p, y, r);
	}

	/* Rotates transform around a axis. */
	void RotateAround(const Vec3 axis, const float angle) {
		rotation = glm::angleAxis(ToRad(angle), axis) * rotation;
	}

	/* Rotates transform by euler angles (pitch, yaw, roll). */
	void Rotate(const Vec3 euler) {
		rotation = glm::quat(ToRad(euler)) * rotation;
	}
};