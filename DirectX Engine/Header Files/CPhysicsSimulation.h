#ifndef PHYSICSSIMULATION_H
#define PHYSICSSIMULATION_H

#include <vector>
#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultErrorCallback.h>
#include <extensions/PxDefaultCpuDispatcher.h>
#include <foundation/PxFoundation.h>
#include <pvd/PxPvd.h>
#include <PxPhysics.h>
#include <PxScene.h>
#include <cooking/PxCooking.h>

#include <DirectXMath.h>
#include <PxMaterial.h>

#include <PxRigidStatic.h>
#include <PxRigidDynamic.h>
#include <CPhysicsComponent.h>

#include "MemoryUtil.h"

class CPhysicsSimulation
{
public:
	CPhysicsSimulation();
	~CPhysicsSimulation();

	physx::PxTransform getGlobalPose(const physx::PxShape& shape, const physx::PxRigidActor& actor);

	bool Init();
	void AddDynamicActor(SDynamicActor* dynamicActor);
	void AddStaticActor(SPhysicsActor* rigidActor);
	void UpdateSimulationResult();
	void UpdatePhysics(float f);
	const std::vector<unique_ptr<SPhysicsActor>>& GetStaticActors();
	const std::vector<unique_ptr<SDynamicActor>>& GetDynamicActors();
	void CreateTestMesh(const size_t nVerts, const std::vector<physx::PxVec3>& verts, const size_t nIndices, const std::vector<uint32_t>& indexBuffer);
	
	physx::PxScene* GetScene() { return Scene.get(); }
	physx::PxPhysics* GetPhysics() { return Physics.get(); }
	physx::PxCooking* GetCook() { return Cook.get(); }
private:
	unique_ptr<physx::PxFoundation, PxDeleter<physx::PxFoundation>> Foundation;
	unique_ptr<physx::PxPhysics, PxDeleter<physx::PxPhysics>> Physics;
	unique_ptr<physx::PxCooking, PxDeleter<physx::PxCooking>> Cook;
	physx::PxDefaultAllocator		Allocator;
	physx::PxDefaultErrorCallback	ErrorCallback;
	std::shared_ptr<physx::PxPvdTransport> Transport;
	unique_ptr <physx::PxDefaultCpuDispatcher> Dispatcher;
	unique_ptr <physx::PxScene, PxDeleter<physx::PxScene>> Scene;
	unique_ptr <physx::PxMaterial, PxDeleter<physx::PxMaterial>> Material;
	std::shared_ptr <physx::PxPvd> Pvd;
	physx::PxReal Scale;
	std::vector<unique_ptr<SPhysicsActor>> StaticActors;
	std::vector<unique_ptr<SDynamicActor>> DynamicActors;
};

#endif