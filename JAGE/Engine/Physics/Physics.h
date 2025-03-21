#pragma once

#include <physx/PxPhysicsAPI.h>
#include <physx/cooking/PxCooking.h>
#include <Core/Log/Logging.h>
#include <ECS/Components/Transform.h>
#include <ECS/Components/Collider.h>
#include <ECS/Components/RigidBody.h>
#include <Core/Assets/AssetManager.h>
#include "Common.h"

using namespace physx;
constexpr const char* LogPhysics = "Physics";

struct CollisionData {
	PxActor* actorA;
	PxActor* actorB;
};

namespace Physics {
	bool Initialize();

	/* Aux vars. */
	inline std::unordered_map<int, PxConvexMesh*> mConvexMeshes;
	inline std::unordered_map<int, PxTriangleMesh*> mTriangleMeshes;
	inline std::vector<CollisionData> mCollisions;
	inline std::unordered_map<Entity, PxRigidDynamic*> mRigidBodies;
	inline std::unordered_map<Asset, PxShape*> mShapes;

	/* Global getters. */
	PxScene* GetScene();
	PxPhysics* GetPhysics();
	PxMaterial* GetDefaultMaterial();
	PxRigidStatic* GetDebugPlane();

	/* General functions. */
	void Update(float dt);
	void AddRigidBody(const Entity entity, const RigidBody& rb, const Transform& transform, const Collider& collider);
	void AddRigidBody(const Entity entity, const RigidBody& rb, const Transform& transform);
	Vec3 GetRigidBodyPosition(const int32_t index);
	Vec3 GetRigidBodyVelocity(const int32_t index);
	Quat GetRigidBodyRotation(const int32_t index);
	PxShape* CreateBoxShape(float width, float height, float depth, Vec3 offset, PxMaterial* material = nullptr);
	Vec3 QuatToEuler(const PxQuat& quat);
	PxQuat EulerToQuat(const Vec3& euler);
	PxShape* CreateMeshShape(const std::vector<Vertex>& vertices, Vec3 offset, PxMaterial* material = nullptr);
};

class ContactReportCallback : public PxSimulationEventCallback {
	void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) { PX_UNUSED(constraints); PX_UNUSED(count); }
	void onWake(PxActor** actors, PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
	void onSleep(PxActor** actors, PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
	void onTrigger(PxTriggerPair* pairs, PxU32 count) { PX_UNUSED(pairs); PX_UNUSED(count); }
	void onAdvance(const PxRigidBody* const*, const PxTransform*, const PxU32) {}
	void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
	{
		PX_UNUSED(pairs);
		PX_UNUSED(nbPairs);

		if (!pairHeader.actors[0] || !pairHeader.actors[1])
			return;

		CollisionData colData;
		colData.actorA = pairHeader.actors[0];
		colData.actorB = pairHeader.actors[1];

		Physics::mCollisions.push_back(colData);
	}
};

