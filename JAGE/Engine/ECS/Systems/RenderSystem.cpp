#include <Renderer/OpenGL.h>
#include <ECS/Components/Camera.h>
#include <ECS/Components/StaticMeshRenderer.h>
#include <ECS/Components/Transform.h>
#include <ECS/Components/DirectionalLight.h>
#include <ECS/Components/PointLight.h>
#include <Core/Core.h>
#include <World/World.h>
#include <Core/AssetManager.h>
#include "RenderSystem.h"

void RenderSystem::SetRenderApi(OpenGL* renderApi) {
	assert(renderApi != nullptr && "RenderSystem: Failed to set render API.");
	mRenderApi = renderApi;
}

void RenderSystem::SetActiveCamera(Camera* camera) {
	assert(camera != nullptr && "RenderSystem: Failed to set active camera.");
	mActiveCamera = camera;
	mRenderApi->RegisterCamera(camera);
}

void RenderSystem::Update(float dt) {
	assert(mActiveCamera != nullptr && "RenderSystem: No active camera detected.");
	assert(mRenderApi != nullptr && "RenderSystem: No renderer set.");

	Signature staticMeshSignature = World::Get().MakeSignature<StaticMeshRenderer>();
	Signature directionalLightSignature = World::Get().MakeSignature<DirectionalLight>();
	Signature pointLightSignature = World::Get().MakeSignature<PointLight>();

	// buffer mesh data
	for (const Entity& entity : mEntities) {
		Transform& transform = World::Get().GetComponent<Transform>(entity);

		Signature entitySignature = World::Get().GetEntitySignature(entity);

		if ((entitySignature & staticMeshSignature) == staticMeshSignature) {
			StaticMeshRenderer& staticMeshRenderer = World::Get().GetComponent<StaticMeshRenderer>(entity);

 			// load static mesh into GPU buffer
			for (AssetId smId : staticMeshRenderer.meshes) {
				StaticMesh* sm = AssetManager::Get().GetAssetById<StaticMesh>(smId);
				mRenderApi->BufferStaticMesh(entity, sm, &transform);
			}
		}

		// register all lights to renderer API
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
	//mRenderApi->ShadowMapPass();
	//mRenderApi->LightingPass();
	mRenderApi->DebugGbuffer(2);
}