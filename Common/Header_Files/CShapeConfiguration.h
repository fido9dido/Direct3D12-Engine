#ifndef CSHAPECONFIGURATION_H
#define CSHAPECONFIGURATION_H

#include <foundation/PxTransform.h>
#include <geometry/PxBoxGeometry.h>
#include <cooking/PxTriangleMeshDesc.h>
#include <PxRigidActor.h>
#include <PxMaterial.h>
#include <cooking/PxCooking.h>
#include "MemoryUtil.h"

enum class ECShapeType
{
	Box
	, TriangleMesh
	, NotImplemented
};
enum class ECActorType
{
	Static,
	Dynamic
};

class CShapeConfiguration
{
public:
	CShapeConfiguration() = default;
	CShapeConfiguration(float scale);
	virtual ECShapeType GetShapeType() const = 0;
	virtual physx::PxRigidActor* BuildShape(physx::PxPhysics* physics, const physx::PxMaterial* material, const physx::PxCooking* cook = nullptr) const = 0;
	float Scale = 0;
	physx::PxVec3 LocalPosition = physx::PxVec3(0.f, 0.f, 0.f);
};

class CBoxConfiguration : public CShapeConfiguration
{
public:
	CBoxConfiguration() = delete;
	CBoxConfiguration(const physx::PxVec3& dimension, const physx::PxVec3& position, float scale);
	// Inherited via CShapeBehaviour
	virtual ECShapeType GetShapeType() const override;
	physx::PxVec3 Dimensions = physx::PxVec3(1.0f, 1.f, 1.0f);
	physx::PxTransform Position;

	// Inherited via CShapeConfiguration
	virtual physx::PxRigidActor* BuildShape(physx::PxPhysics* physics,  const physx::PxMaterial* material, const physx::PxCooking* cook) const override;

};
template<typename T>
struct ArrayDeleter
{
	void operator()(void* data) const noexcept
	{
		T* ptr = static_cast<T*>(data);
		delete[] ptr;
	}
};
class CTriangleConfiguration : public CShapeConfiguration
{
public:
	CTriangleConfiguration() = default;
	CTriangleConfiguration(const physx::PxTriangleMeshDesc& meshDesc);
	CTriangleConfiguration(CTriangleConfiguration&& other);
	~CTriangleConfiguration();
	physx::PxTriangleMeshDesc MeshDescription;
	int VertexCount;
	int TrianglesCount ;
	int VertexStride = sizeof(physx::PxVec3);
	int TriangleStride = sizeof(physx::PxU32)*3;

	unique_ptr<void, ArrayDeleter<physx::PxVec3>> VertexData;
	unique_ptr<void, ArrayDeleter<physx::PxU32>>TriangleData;
	
	// Inherited via CShapeBehaviour
	virtual ECShapeType GetShapeType() const override;

	// Inherited via CShapeConfiguration
	virtual physx::PxRigidActor* BuildShape(physx::PxPhysics* physics,  const physx::PxMaterial* material, const physx::PxCooking* cook) const override;

};

#endif