#pragma once

#include <Core/Core.h>
#include <Core/Assets/AssetManager.h>

enum class ColliderType {
	Cube,
	Sphere,
	Mesh,
	COUNT
};

struct Collider {
	ColliderType type = ColliderType::Cube;
	bool isTrigger = false;

	float radius = 1.0f;
	float width = 1.0f;
	float height = 1.0f;
	float depth = 1.0f;
	std::vector<Vec3> vertices;

	Collider() = default;
	Collider(ColliderType type) : type(type) {};

	void SetVertices(const std::vector<Asset>& meshes) {
		vertices.clear();
		for (const Asset meshId : meshes) {
			Mesh* mesh = AssetManager::Get().GetAssetById<Mesh>(meshId);
			for (const Vertex& vertex : mesh->vertices) {
				vertices.push_back(vertex.Position);
			}
		}
	}
};