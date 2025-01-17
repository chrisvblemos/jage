#pragma once

#include <World/World.h>
#include <ECS/Components/Transform.h>
#include "ComponentWidget.h"

class TransformWidget : public ComponentWidget {
public:
	void Render(const Entity entity) override {
		auto& transform = World::Get().GetComponent<Transform>(entity);

		if (ImGui::DragFloat3("Position", &transform.position.x, 0.1f)) {

		}

		if (ImGui::DragFloat4("Rotation", &transform.rotation.x, 0.1f)) {

		}

		if (ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f)) {

		}
		
	}
};