#ifndef CPHYSICSCOMPONENT_H
#define CPHYSICSCOMPONENT_H

#include "IComponent.h"

#include <PxShape.h>
#include <PxPhysics.h>
#include <PxScene.h>
#include <PxMaterial.h>
#include <PxRigidStatic.h>
#include <DirectXMath.h>
#include <CEvent.h>

#include "MemoryUtil.h"
#include <cooking/PxCooking.h>
#include <CShapeConfiguration.h>


template<typename T>
struct PxDeleter {
	void operator()	(T* ptr) {
		ptr->release();
		ptr = nullptr;
	}
};

struct SPhysicsDescriptor
{
	ECShapeType ShapeType;
	ECActorType ActorType;
};

struct SPhysicsActor
{
	virtual physx::PxRigidActor* Get() {return Actor.get();}
	unique_ptr<physx::PxRigidActor, PxDeleter<physx::PxRigidActor>> Actor;
};
struct SDynamicActor : public SPhysicsActor
{
	CEvent<const DirectX::XMMATRIX&> OnTransformUpdate;
};

class CPhysicsComponent : public IComponent
{
	unique_ptr <physx::PxMaterial, PxDeleter<physx::PxMaterial>> Material;

	unique_ptr<physx::PxShape, PxDeleter<physx::PxShape>> shape;
	physx::PxPhysics* PhysicsRef;
	physx::PxCooking* CookRef;
	physx::PxScene* SceneRef;
	SPhysicsDescriptor Description;
	std::vector<physx::PxVec3> pxVerts;

	using OnDynamicActorCreatedEvent = CEvent<SDynamicActor*>;
	using OnStaticActorCreatedEvent = CEvent<SPhysicsActor*>;
	OnDynamicActorCreatedEvent OnDynamicActorCreated;
	OnStaticActorCreatedEvent OnStaticActorCreated;

	SPhysicsActor* ActorRef;
	unique_ptr<CShapeConfiguration> ShapeConfiguration;
public:
	CPhysicsComponent() = default;
	virtual ~CPhysicsComponent() {}
	void InitializeComponent(class CPhysicsSimulation* physics);
	void CreateShapeFromMeshComponent();
	void BuildActorFromConfiguration();
	void SetDescription(const SPhysicsDescriptor& desc);
	// Inherited via IComponent
	virtual IComponent* BuildComponent() override;
	virtual void RegisterEvents() override;
	virtual void OnUpdate() override;
};

#endif