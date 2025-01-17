#pragma once

#include <Renderer/OpenGL.h>
#include <ECS/Components/Camera.h>
#include "System.h"

using TransformID = uint32_t;

class RenderSystem : public System {
private:
	Camera* mActiveCamera;
	OpenGL* mRenderApi;
public:
	RenderSystem() {
		name = "RenderSystem";
	}

	bool hasNewStaticMesh = false;
	bool hasTransformsChanged = false;
	std::set<AssetId> cachedStaticMeshes;
	std::unordered_map<Entity, TransformID> entityToTransformID;
	std::vector<Transform> transforms;

	void SetActiveCamera(Camera* camera);
	void SetRenderApi(OpenGL* r);
	void Update(float dt);
};