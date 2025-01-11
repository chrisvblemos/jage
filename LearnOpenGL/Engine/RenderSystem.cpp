#include "Renderer/OpenGL.h"
#include "Components/StaticMeshRenderer.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/DirectionalLight.h"
#include "Components/PointLight.h"
#include "World.h"
#include "RenderSystem.h"

#include <cassert>

void RenderSystem::SetRenderApi(OpenGL* renderApi) {
	assert(renderApi != nullptr && "RenderSystem: Failed to set render api.");
	mRenderApi = renderApi;
}

void RenderSystem::SetActiveCamera(Camera* camera) {
	assert(camera != nullptr && "RenderSystem: Failed to set active camera.");
	mActiveCamera = camera;
}

void RenderSystem::Update(float dt) {
	assert(mActiveCamera != nullptr && "RenderSystem: No active camera detected.");
	assert(mRenderApi != nullptr && "RenderSystem: No renderer set.");

	Signature staticMeshSignature = World::Get().MakeSignature<StaticMeshRenderer>();
	Signature directionalLightSignature = World::Get().MakeSignature<DirectionalLight>();
	Signature pointLightSignature = World::Get().MakeSignature<PointLight>();

	mRenderApi->UploadCameraUniforms(mActiveCamera);

	// buffer mesh data
	for (const Entity& entity : mEntities) {
		Transform& transform = World::Get().GetComponent<Transform>(entity);

		Signature entitySignature = World::Get().GetEntitySignature(entity);

		if ((entitySignature & staticMeshSignature) == staticMeshSignature) {
			StaticMeshRenderer& staticMeshRenderer = World::Get().GetComponent<StaticMeshRenderer>(entity);

 			// load static mesh into gpu buffer
			for (StaticMesh* sm : staticMeshRenderer.meshes) {
				mRenderApi->BufferStaticMesh(entity, sm, &transform);
			}
		}

		// register all lights to renderer api
		if ((entitySignature & directionalLightSignature) == directionalLightSignature) {
			DirectionalLight& light = World::Get().GetComponent<DirectionalLight>(entity);
			mRenderApi->RegisterDirectionalLight(&light);
		}

		if ((entitySignature & pointLightSignature) == pointLightSignature) {
			PointLight& light = World::Get().GetComponent<PointLight>(entity);
			mRenderApi->RegisterPointLight(&light);
		}
	}

	// execute render passes
	mRenderApi->GeometryPass();
	mRenderApi->ShadowMapPass();
	mRenderApi->LightPass();
	//mRenderApi->DebugGbuffer(3);
}