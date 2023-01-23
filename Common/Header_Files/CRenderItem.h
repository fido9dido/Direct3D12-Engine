#pragma once

#include <DirectXMath.h>
#include <memory>
#include <d3d12.h>

enum class RenderLayer : int
{
	Opaque = 0,		// static Objects 
	OpaqueLine,
	OpaquePhysics, // Dynamic Objects that uses physx
	WireframeLine,
	WireframeTri,
	SkinnedOpaque,
	Debug,
	Sky,
	Camera,
	Count
};
struct CRenderItem
{
	CRenderItem() = default;
	CRenderItem(const CRenderItem& rhs) = delete;

	// World matrix of the shape that describes the object's local space
	// relative to the world space, which defines the position, orientation,
	// and scale of the object in the world.
	DirectX::XMMATRIX WorldMatrix;
	DirectX::XMMATRIX TextureTransform;
	
	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify object data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int FramesDirtyCount = 3;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjectCBIndex = -1;

	UINT SubstanceCBIndex = -1;

	std::shared_ptr<struct MeshGeometry> GeometryPrimitive;
	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;

	RenderLayer Layer = RenderLayer::OpaquePhysics;
};

