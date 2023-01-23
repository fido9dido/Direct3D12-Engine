#include "CPhysicsSimulation.h"
#include <pvd/PxPvdTransport.h>
#include <pvd/PxPvdSceneClient.h>
#include <common/PxTolerancesScale.h>

#include <cooking/PxTriangleMeshDesc.h>
#include <cooking/PxCooking.h>
#include <extensions/PxDefaultStreams.h>
#include <extensions/PxRigidActorExt.h>
#include <extensions/PxDefaultSimulationFilterShader.h>
#include <PxSceneDesc.h>
#include <foundation/PxPhysicsVersion.h>
#include <extensions/PxRigidBodyExt.h>

CPhysicsSimulation::CPhysicsSimulation()
{
	Init();
}

CPhysicsSimulation::~CPhysicsSimulation()
{
	for (auto& i : StaticActors)
	{
		i.reset();
	}
	for (auto& i : DynamicActors)
	{
		i.reset();
	}
	StaticActors.clear();
	DynamicActors.clear();
	Material.reset();
	Cook.reset();
	Scene.reset();
	Dispatcher.reset();
	Physics.reset();
	Pvd.reset();
	Transport.reset();

	Foundation.reset();
}
void CPhysicsSimulation::UpdatePhysics(float f)
{
	Scene->simulate((float)1 / 60);
	Scene->fetchResults(true);
}

const std::vector<unique_ptr<SPhysicsActor>>& CPhysicsSimulation::GetStaticActors()
{
	return StaticActors;
}
const std::vector<unique_ptr<SDynamicActor>>& CPhysicsSimulation::GetDynamicActors()
{
	return DynamicActors;
}
physx::PxTransform CPhysicsSimulation::getGlobalPose(const physx::PxShape& shape, const physx::PxRigidActor& actor)
{
	return actor.getGlobalPose() * shape.getLocalPose();
}
bool CPhysicsSimulation::Init()
{
	Foundation = unique_ptr<physx::PxFoundation, PxDeleter<physx::PxFoundation>>(PxCreateFoundation(PX_PHYSICS_VERSION, Allocator, ErrorCallback));

	Pvd = std::shared_ptr<physx::PxPvd>(PxCreatePvd(*Foundation), [=](physx::PxPvd* del)
		{
			del->release();
		});

	Transport = std::shared_ptr<physx::PxPvdTransport>(physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10), [=](physx::PxPvdTransport* del)
		{
			del->release();
		});

	Pvd->connect(*Transport, physx::PxPvdInstrumentationFlag::eALL);
	Physics = unique_ptr<physx::PxPhysics, PxDeleter<physx::PxPhysics>>(PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, physx::PxTolerancesScale(), true, Pvd.get()));
	physx::PxSceneDesc sceneDesc(Physics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.f, -9.81f, 0.f);
	Dispatcher = unique_ptr<physx::PxDefaultCpuDispatcher>(physx::PxDefaultCpuDispatcherCreate(2));
	sceneDesc.cpuDispatcher = Dispatcher.get();
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	Scene = unique_ptr<physx::PxScene, PxDeleter<physx::PxScene>>(Physics->createScene(sceneDesc));
	physx::PxPvdSceneClient* pvdClient = Scene->getScenePvdClient();

	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	Material = unique_ptr<physx::PxMaterial, PxDeleter<physx::PxMaterial>>(Physics->createMaterial(.5f, .5f, .5f));

	Cook = unique_ptr<physx::PxCooking, PxDeleter<physx::PxCooking>>(PxCreateCooking(PX_PHYSICS_VERSION, *Foundation, physx::PxCookingParams(physx::PxTolerancesScale())));

	return true;
}
void CPhysicsSimulation::AddDynamicActor(SDynamicActor* dynamicActor)
{
	DynamicActors.push_back(unique_ptr<SDynamicActor>(dynamicActor));
}
void CPhysicsSimulation::AddStaticActor(SPhysicsActor* rigidActor)
{
	StaticActors.push_back(unique_ptr<SPhysicsActor>(rigidActor));
}
void CPhysicsSimulation::UpdateSimulationResult()
{
	physx::PxU32 nbShapes;

	int actorsCount = static_cast<int>(DynamicActors.size());

	for (const unique_ptr<SDynamicActor>& dynamicActor : DynamicActors)
	{
		nbShapes = 0;
		physx::PxRigidDynamic* actor = dynamicActor->Get()->is<physx::PxRigidDynamic>();
		nbShapes = actor->getNbShapes();
		const physx::PxTransform& actorGPose = actor->getGlobalPose();
		if (nbShapes > 0)
		{
			for (physx::PxU32 i = 0; i < nbShapes; i++)
			{
				physx::PxShape* shape;
				physx::PxU32 n = actor->getShapes(&shape, 1, i);
				PX_ASSERT(n == 1);
				PX_UNUSED(n);

				auto global = getGlobalPose(*shape, *actor);
				DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(global.q.x, global.q.y, global.q.z, global.q.w));

				worldMatrix = worldMatrix * DirectX::XMMatrixTranslation(global.p.x, global.p.y, global.p.z);

				if (!actor->isSleeping())
				{
					dynamicActor->OnTransformUpdate.Notify(worldMatrix);
				}
			}
		}
	}
}