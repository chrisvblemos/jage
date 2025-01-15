#pragma once

#include <Engine/Renderer/OpenGL.h>
#include <Engine/ECS/Components/Camera.h>
#include "System.h"

class RenderSystem : public System {
private:
	Camera* mActiveCamera;
	OpenGL* mRenderApi;
public:
	void SetActiveCamera(Camera* camera);
	void SetRenderApi(OpenGL* r);
	void Update(float dt);
};