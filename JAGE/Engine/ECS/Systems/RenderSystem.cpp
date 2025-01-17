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
			for (AssetId assetID : staticMeshRenderer.meshes) {
				auto it_0 = cachedStaticMeshes.find(assetID);
				if (it_0 == cachedStaticMeshes.end()) {
					cachedStaticMeshes.insert(assetID);
					hasNewStaticMesh = true;
					StaticMesh* sm = AssetManager::Get().GetAssetById<StaticMesh>(assetID);
					mRenderApi->RegisterStaticMesh(sm);
				}

				auto it_1 = entityToTransformID.find(entity);
				if (it_1 == entityToTransformID.end()) {
					entityToTransformID[entity] = transforms.size();
					transforms.push_back(transform);

					mRenderApi->RegisterStaticMeshInstance(assetID, entity, &transform);
					hasTransformsChanged = true;
				}

				uint32_t cachedTransformIndex = entityToTransformID[entity];
				Transform& cachedTransform = transforms[cachedTransformIndex];
				if (transform.position != cachedTransform.position ||
					transform.rotation != cachedTransform.rotation ||
					transform.scale != cachedTransform.scale) {

					transforms[cachedTransformIndex] = transform;
					mRenderApi->RegisterStaticMeshInstance(assetID, entity, &transform);
					hasTransformsChanged = true;
				}
					
			}
		}

		if ((entitySignature & directionalLightSignature) == directionalLightSignature) {
			DirectionalLight& dirLight = World::Get().GetComponent<DirectionalLight>(entity);
			mRenderApi->RegisterDirectionalLight(&dirLight);
		}
	}

	mRenderApi->UploadCameraData();

	if (hasNewStaticMesh) {
		mRenderApi->BatchUploadStaticMeshData();
		hasNewStaticMesh = false;
	}

	if (hasTransformsChanged) {
		mRenderApi->BatchUploadStaticMeshInstanceData();
		hasTransformsChanged = false;
	}

	mRenderApi->GeometryPass();
	mRenderApi->ShadowMapPass();
	mRenderApi->UploadSceneLightData();
	mRenderApi->LightingPass();
	//mRenderApi->DebugGbuffer(0);
}