#pragma once

#include <Renderer/OpenGlApi.h>
#include <ECS/Components/Camera.h>
#include "System.h"

class RenderSystem : public System {
private:
	Entity camera;
	OpenGlApi* renderAPI;
public:
	RenderSystem() {
		name = "RenderSystem";
	}

	void SetActiveCamera(const Entity camera);
	void SetRenderApi(OpenGlApi* r);

	void HandlePointLights(const Entity entity);
	void HandleDirectionalLights(const Entity entity);
	void HandleStaticMeshes(const Entity entity);
	void HandleCameras(const Entity entity);
	void OffloadToGPU();
	void CallRenderPass();

	void Update(float dt) override;
};