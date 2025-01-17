#pragma once

#include <Core/Core.h>
#include <imgui/imgui.h>
#include <World/World.h>
#include "../Widgets/Components/TransformWidget.h"

class EntityViewerWindow {
private:
	Entity selectedEntity = -1;
	TransformWidget transformWidget;

public:
	EntityViewerWindow() = default;

	void Render() {
		ImGui::Begin("Entity Viewer");

		RenderEntityList();
		//ImGui::SameLine();

		ImGui::SameLine();

		ImGui::BeginChild("Entity Details", ImVec2(0, 0), true);

		if (selectedEntity >= 0) {

			ImGui::Text("ID: %d", selectedEntity);
			ImGui::Text("Name: %s", World::Get().GetEntityName(selectedEntity).c_str());
		}

		ImGui::EndChild();

		ImGui::End();
	}

	void RenderEntityList() {
		ImGui::BeginChild("Entities", ImVec2(150, 0), true);
		
		for (const auto& entity : World::Get().GetEntities()) {
			const std::string& entityName = World::Get().GetEntityName(entity);
			const std::string formattedName = std::format("{}[{}]", entityName, entity);
			if (ImGui::Selectable(formattedName.c_str(), selectedEntity == entity)) {
				selectedEntity = entity;
			}
		}

		ImGui::EndChild();
	}
};