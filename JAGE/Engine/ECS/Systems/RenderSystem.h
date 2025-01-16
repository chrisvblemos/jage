#pragma once

#include <Renderer/OpenGL.h>
#include <ECS/Components/Camera.h>
#include "System.h"

class RenderSystem : public System {
private:
	Camera* mActiveCamera;
	OpenGL* mRenderApi;
public:
	RenderSystem() {
		name = "RenderSystem";
	}

	void SetActiveCamera(Camera* camera);
	void SetRenderApi(OpenGL* r);
	void Update(float dt);
};