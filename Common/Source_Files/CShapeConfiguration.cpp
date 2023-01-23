#include "CShapeConfiguration.h"
#include <utility>
#include <CPhysicsSimulation.h>
#include <extensions/PxDefaultStreams.h>
#include <extensions/PxRigidActorExt.h>
#include <extensions/PxRigidBodyExt.h>

CShapeConfiguration::CShapeConfiguration(float scale)
    : Scale(scale)
{
}

ECShapeType CBoxConfiguration::GetShapeType() const
{
    return ECShapeType::Box;
}

physx::PxRigidActor* CBoxConfiguration::BuildShape(physx::PxPhysics* physics,  const physx::PxMaterial* material, const physx::PxCooking* cook) const
{
	//physx::PxTransform boxPos(physx::PxVec3(0.0f, 50.f, 0.0f));
	//physx::PxBoxGeometry boxGeometry(physx::PxVec3(0.5f, 0.5f, 0.5f));
	unique_ptr<physx::PxShape, PxDeleter<physx::PxShape>> shape3(
		physics->createShape(physx::PxBoxGeometry(Dimensions *Scale), *material));

	physx::PxRigidDynamic* boxRigid = physics->createRigidDynamic(Position);
	boxRigid->attachShape(*shape3);
	physx::PxRigidBodyExt::updateMassAndInertia(*boxRigid, .5f);
	return boxRigid;
}

CBoxConfiguration::CBoxConfiguration(const physx::PxVec3& dimension, const physx::PxVec3& position, float scale)
	: Dimensions(dimension)
	, Position(physx::PxTransform(position))
	, CShapeConfiguration(scale)
{
}

CTriangleConfiguration::CTriangleConfiguration(const physx::PxTriangleMeshDesc& meshDesc)
    : MeshDescription(meshDesc)
	, CShapeConfiguration()
{
}

CTriangleConfiguration::CTriangleConfiguration(CTriangleConfiguration&& other)
	: VertexData(std::move(other.VertexData))
	, TriangleData(std::move(other.TriangleData))
	, VertexCount(std::move(other.VertexCount))
	, TrianglesCount(std::move(other.TrianglesCount))
	, VertexStride(std::move(other.VertexStride))
	, TriangleStride(std::move(other.TriangleStride))
{
}

CTriangleConfiguration::~CTriangleConfiguration()
{
}

ECShapeType CTriangleConfiguration::GetShapeType() const
{
    return ECShapeType::TriangleMesh;
}

physx::PxRigidActor* CTriangleConfiguration::BuildShape(physx::PxPhysics* physics,  const physx::PxMaterial* material, const physx::PxCooking* cook) const
{
	
	physx::PxTriangleMeshDesc meshDescription;
	meshDescription.points.data = VertexData.get();
	meshDescription.points.count = VertexCount;
	meshDescription.points.stride = VertexStride;
	meshDescription.triangles.data = TriangleData.get();
	meshDescription.triangles.count = TrianglesCount;
	meshDescription.triangles.stride = TriangleStride;

	physx::PxDefaultMemoryOutputStream writeBuffer;
	physx::PxTriangleMeshCookingResult::Enum res;
	bool status = cook->cookTriangleMesh(meshDescription, writeBuffer, &res);
	if (!status) { throw(0); }

	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	unique_ptr<physx::PxTriangleMesh, PxDeleter<physx::PxTriangleMesh>> triMesh(physics->createTriangleMesh(readBuffer));
	if (triMesh)
	{
		physx::PxTriangleMeshGeometry triGeom;
		triGeom.triangleMesh = triMesh.get();
		physx::PxTransform local(physx::PxVec3(0.0f, 0.0f, 0.0f));

		physx::PxRigidActor* terrainActor = physics->createRigidStatic(physx::PxTransform(0.0f, 7.0f, 0.0f));

		physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*terrainActor, triGeom, *material);

		PX_UNUSED(shape);
		shape->setLocalPose(physx::PxTransform(LocalPosition));

		return terrainActor;
	}
	return nullptr;
}
