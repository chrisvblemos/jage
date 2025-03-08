#include <Renderer/OpenGlApi.h>
#include <ECS/Components/Camera.h>
#include <ECS/Components/StaticMeshRenderer.h>
#include <ECS/Components/Transform.h>
#include <ECS/Components/DirectionalLight.h>
#include <ECS/Components/PointLight.h>
#include <Core/Core.h>
#include <World/World.h>
#include "RenderSystem.h"

void RenderSystem::SetRenderApi(OpenGlApi* renderApi) {
	assert(renderApi != nullptr && "RenderSystem: Failed to set render API.");
	renderAPI = renderApi;
}

void RenderSystem::HandlePointLights(Entity entity)
{
	World& world = World::Get();
	PointLight& light = world.GetComponent<PointLight>(entity);
	Transform& transform = world.GetComponent<Transform>(entity);

	light.projMatrix = glm::perspective(
		glm::radians(90.0f),
		1.0f,
		light.shadowMapNearPlane,
		light.shadowMapFarPlane
	);

	Vec3 pos = transform.position;
	light.cubemapMatrices[0] = light.projMatrix * glm::lookAt(pos, pos + WRight, -WUp);
	light.cubemapMatrices[1] = light.projMatrix * glm::lookAt(pos, pos - WRight, -WUp);
	light.cubemapMatrices[2] = light.projMatrix * glm::lookAt(pos, pos + WUp, WForward);
	light.cubemapMatrices[3] = light.projMatrix * glm::lookAt(pos, pos - WUp, -WForward);
	light.cubemapMatrices[4] = light.projMatrix * glm::lookAt(pos, pos + WForward, -WUp);
	light.cubemapMatrices[5] = light.projMatrix * glm::lookAt(pos, pos - WForward, -WUp);

	renderAPI->RegisterPointLight(entity, &light, &transform);
}

void RenderSystem::HandleDirectionalLights(const Entity entity)
{
	World& world = World::Get();
	DirectionalLight& light = world.GetComponent<DirectionalLight>(entity);
	Transform& transform = world.GetComponent<Transform>(entity);
	
	renderAPI->RegisterDirectionalLight(&light, &transform);
}

void RenderSystem::HandleStaticMeshes(const Entity entity)
{
	World& world = World::Get();
	StaticMeshRenderer& smRenderer = world.GetComponent<StaticMeshRenderer>(entity);
	Transform& transform = world.GetComponent<Transform>(entity);

	Mat4 model = Id4;
	model= glm::translate(model, transform.position);
	model *= glm::mat4_cast(transform.rotation);
	model = glm::scale(model, transform.scale);
	smRenderer.modelMatrix = model;

	renderAPI->UpsertMeshEntity(entity, &smRenderer);
}

void RenderSystem::HandleCameras(const Entity entity)
{
	World& world = World::Get();
	Camera& camera = world.GetComponent<Camera>(entity);

	if (!camera.isActive) return; // ignore inactive cameras
	Transform& transform = world.GetComponent<Transform>(entity);

	Mat4 view = Id4;
	Mat4 translate = glm::translate(view, -transform.position);
	Mat4 rotation = glm::mat4_cast(glm::conjugate(transform.rotation));
	camera.viewMatrix = rotation * translate;

	if (camera.isOrthogonal)
		camera.projMatrix = Id4;
	else
		camera.projMatrix = glm::perspective(
			glm::radians(camera.fov),
			camera.aspectRatio,
			camera.nearPlane,
			camera.farPlane);
}

void RenderSystem::OffloadToGPU()
{
	World& world = World::Get();
	Camera& cam = world.GetComponent<Camera>(camera);
	Transform& camTransform = world.GetComponent<Transform>(camera);

	renderAPI->UploadCameraData(&cam, &camTransform);
	renderAPI->UploadMeshRenderData();
}

void RenderSystem::CallRenderPass()
{
	renderAPI->GeometryPass();
	renderAPI->SSAOPass();
	renderAPI->PointLightShadowMapPass();
	renderAPI->ShadowMapPass();
	renderAPI->UploadSceneLightData();
	renderAPI->LightingPass();
	renderAPI->PostFxPass();
	renderAPI->OutputToScreen(SceneTexture::ST_POST_FX);
}

void RenderSystem::SetActiveCamera(const Entity camera) {
	if (camera == NULL_ENTITY) {
		return;
	}

	World& world = World::Get();
	if (this->camera != NULL_ENTITY) {
		Camera& cam = world.GetComponent<Camera>(camera);
		cam.isActive = false;
	}
	
	this->camera = camera;
	Camera& cam = world.GetComponent<Camera>(camera);
	renderAPI->RegisterCamera(&cam);

	cam.isActive = true;
}

void RenderSystem::Update(float dt) {
	if (camera == NULL_ENTITY)
		return;

	assert(renderAPI != nullptr && "RenderSystem: No renderer set.");

	World& world = World::Get();
	Signature staticMeshSignature = world.MakeSignature<StaticMeshRenderer>();
	Signature directionalLightSignature = world.MakeSignature<DirectionalLight>();
	Signature pointLightSignature = world.MakeSignature<PointLight>();
	Signature cameraSignature = world.MakeSignature<Camera>();

	for (const Entity& entity : entities) {
		Signature entitySignature = world.GetEntitySignature(entity);

		if ((entitySignature & cameraSignature) == cameraSignature)
			HandleCameras(entity);
		if ((entitySignature & staticMeshSignature) == staticMeshSignature)
			HandleStaticMeshes(entity);
		if ((entitySignature & directionalLightSignature) == directionalLightSignature)
			HandleDirectionalLights(entity);
		if ((entitySignature & pointLightSignature) == pointLightSignature)
			HandlePointLights(entity);
	}

	OffloadToGPU();
	CallRenderPass();
}