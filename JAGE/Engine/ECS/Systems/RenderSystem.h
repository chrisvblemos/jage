#pragma once

#include <Renderer/OpenGlApi.h>
#include <ECS/Components/Camera.h>
#include "System.h"

using TransformID = uint32_t;

class RenderSystem : public System {
private:
	Camera* mActiveCamera;
	OpenGlApi* mRenderApi;
public:
	RenderSystem() {
		name = "RenderSystem";
	}

	void SetActiveCamera(Camera* camera);
	void SetRenderApi(OpenGlApi* r);
	void Update(float dt) override;
};