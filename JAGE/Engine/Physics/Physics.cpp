#include <thread>
#include "Physics.h"

/* These callbacks are required by PhysX sdk. */
class PxAllocatorCallback
{
public:
	virtual ~PxAllocatorCallback() {}
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) = 0;
	virtual void deallocate(void* ptr) = 0;
};

class UserErrorCallback : public PxErrorCallback
{
public:
	virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		LOG(LogPhysics, LOG_ERROR, std::format("{}", message));
	}
};

static PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	/* Enable contact generation between all collision pairs. */
	pairFlags = PxPairFlag::eCONTACT_DEFAULT;
	return PxFilterFlags();
}

/* Global variables from physx. */
#define PVD_HOST "127.0.0.1"

PxFoundation* gFoundation;
PxPvd* gPvd;
ContactReportCallback gContactReportCallback;
UserErrorCallback gErrorCallback;
PxDefaultAllocator gAllocator;
PxPhysics* gPhysics;
PxScene* gScene;
PxMaterial* gDefaultMaterial;
PxRigidStatic* gGroundPlaneDebug;
PxDefaultCpuDispatcher* gCpuDispatcher;

bool Physics::Initialize()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator,
		gErrorCallback);
	if (!gFoundation) {
		LOG(LogPhysics, LOG_ERROR, "PxCreateFoundation failed!");
		return false;
	}

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
	gDefaultMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
	gGroundPlaneDebug = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 1.5f), *gDefaultMaterial);

	PxU32 numThreads = static_cast<PxU32>(std::thread::hardware_concurrency());
	gCpuDispatcher = PxDefaultCpuDispatcherCreate(numThreads);
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = gCpuDispatcher;
	sceneDesc.filterShader = contactReportFilterShader;
	sceneDesc.simulationEventCallback = &gContactReportCallback;
	gScene = gPhysics->createScene(sceneDesc);

#ifdef _DEBUG
	gScene->addActor(*gGroundPlaneDebug);
#endif

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient) {
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	LOG(LogPhysics, LOG_INFO, "PhysX SDK initialized successfully.");
	return true;
}

PxScene* Physics::GetScene()
{
	return gScene;
}

PxPhysics* Physics::GetPhysics()
{
	return gPhysics;
}

PxShape* Physics::CreateBoxShape(float width, float height, float depth, Vec3 offset, PxMaterial* material)
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(width, height, depth), *material);
	return shape;
}

Vec3 Physics::QuatToEuler(const PxQuat& quat)
{
	/* Convert quaternion to euler angles. */
	PxMat33 mat(quat);
	float yaw, pitch, roll;

	// Check for gimbal lock (pitch close to ±90 degrees)
	if (mat[2][0] >= 0.9999f)  // Singularity at +90°
	{
		pitch = PxPi / 2.0f;
		yaw = 0.0f;
		roll = atan2(mat[1][2], mat[1][1]); // Roll derived from other axes
	}
	else if (mat[2][0] <= -0.9999f)  // Singularity at -90°
	{
		pitch = -PxPi / 2.0f;
		yaw = 0.0f;
		roll = atan2(-mat[1][2], mat[1][1]); // Roll derived similarly
	}
	else
	{
		// Regular case (no singularity)
		pitch = asin(mat[2][0]);  // sin(pitch) = mat[2][0]
		yaw = atan2(-mat[2][1], mat[2][2]);
		roll = atan2(-mat[1][0], mat[0][0]);
	}

	const float toDeg = 180.0f / PxPi;
	Vec3 euler = Vec3(pitch, yaw, roll) * toDeg;
	euler.x = fmod(euler.x + 180.0f, 360.0f) - 180.0f;
	euler.y = fmod(euler.y + 180.0f, 360.0f) - 180.0f;
	euler.z = fmod(euler.z + 180.0f, 360.0f) - 180.0f;


	return euler;
}

PxQuat Physics::EulerToQuat(const Vec3& euler)
{
	return PxQuat(euler.x * PxPi / 180.0f, PxVec3(1, 0, 0)) *
		PxQuat(euler.y * PxPi / 180.0f, PxVec3(0, 1, 0)) *
		PxQuat(euler.z * PxPi / 180.0f, PxVec3(0, 0, 1));
}

PxShape* Physics::CreateMeshShape(const std::vector<Vertex>& vertices, Vec3 offset, PxMaterial* material)
{
	// cooking step
	PxConvexMeshDesc convexDesc;
	convexDesc.points.count = vertices.size();
	convexDesc.points.stride = sizeof(Vertex);
	convexDesc.points.data = vertices.data();
	convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

	PxCookingParams cookingParams = PxCookingParams(PxTolerancesScale());
	PxDefaultMemoryOutputStream buf;
	PxConvexMeshCookingResult::Enum result;
	if (!PxCookConvexMesh(cookingParams, convexDesc, buf, &result))
		return nullptr;
	PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
	PxConvexMesh* convexMesh = gPhysics->createConvexMesh(input);
	PxShape* shape = gPhysics->createShape(PxConvexMeshGeometry(convexMesh), *material);
	return shape;
}

PxMaterial* Physics::GetDefaultMaterial()
{
	return gDefaultMaterial;
}

PxRigidStatic* Physics::GetDebugPlane()
{
	return gGroundPlaneDebug;
}

void Physics::Update(float dt)
{
	mCollisions.clear();

	gScene->simulate(dt);
	gScene->fetchResults(true);
}

void Physics::AddRigidBody(const Entity entity, const RigidBody& rb, const Transform& transform) {
	auto it = mRigidBodies.find(entity);
	if (it == mRigidBodies.end()) {
		PxQuat q = PxQuat(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
		PxTransform t(PxVec3(transform.position.x, transform.position.y, transform.position.z), q);
		PxRigidDynamic* body = gPhysics->createRigidDynamic(t);
		PxMaterial* physMaterial = gPhysics->createMaterial(rb.staticFriction, rb.dynamicFriction, rb.restitution);
		body->setMass(rb.mass);
		PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
	}
}

void Physics::AddRigidBody(const Entity entity, const RigidBody& rb, const Transform& transform, const Collider& collider)
{
	auto it = mRigidBodies.find(entity);
	if (it == mRigidBodies.end()) {
		PxQuat q = PxQuat(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
		PxTransform t(PxVec3(transform.position.x, transform.position.y, transform.position.z), q);
		PxRigidDynamic* body = gPhysics->createRigidDynamic(t);
		PxMaterial* physMaterial = gPhysics->createMaterial(rb.staticFriction, rb.dynamicFriction, rb.restitution);
		body->setMass(rb.mass);
		PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
		
		PxShape* shape = nullptr;
		
		if (collider.type == ColliderType::Mesh) {
			for (auto meshId : collider.meshes) {
				auto it = mShapes.find(meshId);
				if (it == mShapes.end()) {
					std::cout << "Generating convex hull collision for mesh " << meshId << std::endl;
					Mesh* loadedMesh = AssetManager::Get().GetAssetById<Mesh>(meshId);
					mShapes[meshId] = CreateMeshShape(loadedMesh->vertices, Vec3(0), physMaterial);
				}
				
				shape = mShapes[meshId];
				body->attachShape(*shape);
				shape->release();
			}	
		}
		else if (collider.type == ColliderType::Cube) {
			shape = CreateBoxShape(collider.width, collider.height, collider.depth, Vec3(0.0f), physMaterial);
			body->attachShape(*shape);
			shape->release();
		}
		else if (collider.type == ColliderType::Sphere) {
			shape = gPhysics->createShape(PxSphereGeometry(collider.radius), *physMaterial);
			body->attachShape(*shape);
			shape->release();
		}
		
		gScene->addActor(*body);
		mRigidBodies[entity] = body;
	}
}

Vec3 Physics::GetRigidBodyPosition(const int32_t index)
{
	auto it = mRigidBodies.find(index);
	if (it != mRigidBodies.end()) {
		PxRigidDynamic* body = it->second;
		PxTransform t = body->getGlobalPose();
		return Vec3(t.p.x, t.p.y, t.p.z);
	}

	return Vec3();
}

Vec3 Physics::GetRigidBodyVelocity(const int32_t index)
{
	auto it = mRigidBodies.find(index);
	if (it != mRigidBodies.end()) {
		PxRigidDynamic* body = it->second;
		PxVec3 v = body->getLinearVelocity();
		return Vec3(v.x, v.y, v.z);
	}

	return Vec3();
}

Quat Physics::GetRigidBodyRotation(const int32_t index)
{
	auto it = mRigidBodies.find(index);
	if (it != mRigidBodies.end()) {
		PxRigidDynamic* body = it->second;
		PxQuat q = body->getGlobalPose().q;

		Quat physxToOpengl = glm::angleAxis(glm::radians(-90.0f), Vec3(0, 1, 0));
		return Quat(q.w, q.x, q.y, q.z);
	}

	return Quat(Vec3(0.0f));
}
