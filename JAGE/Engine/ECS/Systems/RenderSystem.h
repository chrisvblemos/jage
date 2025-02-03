#pragma once

#include <Renderer/API.h>
#include <ECS/Components/Camera.h>
#include "System.h"

using TransformID = uint32_t;

class RenderSystem : public System {
private:
	Camera* mActiveCamera;
	API* mRenderApi;
public:
	RenderSystem() {
		name = "RenderSystem";
	}

	void SetActiveCamera(Camera* camera);
	void SetRenderApi(API* r);
	void Update(float dt);
};