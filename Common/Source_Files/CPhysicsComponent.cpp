#include "CPhysicsComponent.h"
#include "CEntityComponent.h"
#include <extensions/PxRigidBodyExt.h>
#include <PxRigidDynamic.h>
#include <cooking/PxTriangleMeshDesc.h>
#include <CMeshComponent.h>
#include "MeshGeometry.h"
#include "GeometryGenerator.h"
#include <extensions/PxDefaultStreams.h>
#include <extensions/PxRigidActorExt.h>
#include "CPhysicsSimulation.h"
#include <functional>
#include <CTransformComponent.h>

void CPhysicsComponent::InitializeComponent(CPhysicsSimulation* physics)
{
	PhysicsRef = physics->GetPhysics();
	CookRef = physics->GetCook();
	SceneRef = physics->GetScene();
	Material = unique_ptr<physx::PxMaterial, PxDeleter<physx::PxMaterial>>(PhysicsRef->createMaterial(.5f, .5f, .5f));
	OnStaticActorCreated.Register(std::bind(&CPhysicsSimulation::AddStaticActor, physics, std::placeholders::_1));

	OnDynamicActorCreated.Register(std::bind(&CPhysicsSimulation::AddDynamicActor, physics, std::placeholders::_1));
}

void CPhysicsComponent::CreateShapeFromMeshComponent()
{
	CMeshComponent* meshComponent = GetEntity()->GetComponentOfType<CMeshComponent>();
	if (!meshComponent) { return; }
	MeshData data = *meshComponent->GetBufferData();
	
	const std::vector<VertexData>& verticesList = data.VerticesList;
	const physx::PxU32 verticesCount = static_cast<physx::PxU32>(verticesList.size());
	physx::PxVec3* pxVerts = new physx::PxVec3[verticesCount * sizeof(physx::PxVec3)];
	if (!pxVerts) { return; }
	
	for (physx::PxU32 i = 0; i < verticesCount; ++i)
	{
		const DirectX::XMVECTOR& position = verticesList[i].Position;
		pxVerts[i] = physx::PxVec3(position.m128_f32[0], position.m128_f32[1], position.m128_f32[2]);
	}

	std::vector<std::uint32_t>& indices32List = data.Indices32List;
	const physx::PxU32 indexCount = static_cast<physx::PxU32>(indices32List.size());
	
	CTriangleConfiguration shapeConfiguration;
	shapeConfiguration.VertexData = unique_ptr<void, ArrayDeleter<physx::PxVec3>>(pxVerts);
	shapeConfiguration.VertexCount = verticesCount;
	shapeConfiguration.VertexStride = sizeof(physx::PxVec3); 

	physx::PxU32* pxTriangle = new physx::PxU32[indexCount * sizeof(physx::PxU32)];
	memcpy(pxTriangle, indices32List.data(), indexCount * sizeof(physx::PxU32));
	shapeConfiguration.TriangleData = unique_ptr<void, ArrayDeleter<physx::PxU32>>(pxTriangle);
	shapeConfiguration.TrianglesCount = static_cast<physx::PxU32>(indices32List.size() / 3);
	shapeConfiguration.TriangleStride = sizeof(physx::PxU32) * 3;
	
	ShapeConfiguration = make_unique<CTriangleConfiguration>(std::move(shapeConfiguration));
}

void CPhysicsComponent::BuildActorFromConfiguration()
{
	physx::PxRigidActor* actor = ShapeConfiguration->BuildShape(PhysicsRef, Material.get(), CookRef);
	SceneRef->addActor(*actor);
	if (Description.ActorType == ECActorType::Static)
	{
		ActorRef = new SPhysicsActor();
		ActorRef->Actor = unique_ptr<physx::PxRigidActor, PxDeleter<physx::PxRigidActor>>(actor);

		OnStaticActorCreated.Notify(ActorRef);
	}
	else if (Description.ActorType == ECActorType::Dynamic)
	{
		SDynamicActor* tempActor = new SDynamicActor();
		ActorRef = tempActor;
		tempActor->Actor = unique_ptr<physx::PxRigidActor, PxDeleter<physx::PxRigidActor>>(actor);

		OnDynamicActorCreated.Notify(tempActor);
	}
}

void CPhysicsComponent::SetDescription(const SPhysicsDescriptor& desc)
{
	Description = desc;
}

IComponent* CPhysicsComponent::BuildComponent()
{
	if (Description.ShapeType == ECShapeType::Box)
	{
		CTransformComponent* transformComponent = GetEntity()->GetComponentOfType<CTransformComponent>();
		const DirectX::XMVECTOR& initalTranslation = transformComponent->GetTranslation();
		ShapeConfiguration = make_unique<CBoxConfiguration>(
			physx::PxVec3(0.5f, 0.5f, 0.5f), 
			physx::PxVec3(initalTranslation.m128_f32[0], initalTranslation.m128_f32[1], initalTranslation.m128_f32[2]),
			1.f);

	}
	else if (Description.ShapeType == ECShapeType::TriangleMesh)
	{
		CreateShapeFromMeshComponent();
	}
	else
	{
		throw "Only a Box and Terrain are implemented for testing they are to be deleted anyway";
	}

	BuildActorFromConfiguration();
    return this;
}

void CPhysicsComponent::RegisterEvents()
{
	if (!ActorRef) { return; }
	SDynamicActor* actor = dynamic_cast<SDynamicActor*>(ActorRef);
	if (!actor) { return; }
	CTransformComponent* transformComponent = GetEntity()->GetComponentOfType<CTransformComponent>();
	actor->OnTransformUpdate.Register(std::bind(&CTransformComponent::SetTransform, transformComponent, std::placeholders::_1));
}

void CPhysicsComponent::OnUpdate()
{
}
