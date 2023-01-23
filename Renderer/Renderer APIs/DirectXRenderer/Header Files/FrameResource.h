#ifndef FRAMERESOURCES_H
#define FRAMERESOURCES_H

#include <memory>
#include <array>
#include "UploadBuffer.h"
#include "Material.h"
#include "SubstanceManagerBase.h"

//per object constant
struct ObjectConstants
{
	DirectX::XMFLOAT4X4	WorldMatrix;
	DirectX::XMFLOAT4X4 TextureTransform;
	UINT				SubstanceCBIndex;
	UINT				Textures;
	UINT				ObjPad1;
	UINT				ObjPad2;
};

struct GridEdges
{
	DirectX::XMFLOAT3 Position1;
	DirectX::XMFLOAT3 Position2;
};

struct WavesVertex {
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;
};

//per frame constant
struct FrameConstants
{
	DirectX::XMMATRIX	ViewProjection = DirectX::XMMatrixIdentity();
	DirectX::XMFLOAT4	AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3	CameraPosition;
	float				PerObjectPad1 = 0.f;
	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Light Lights[MaxLights];
};

#// Stores the resources needed for the CPU to build the command lists for a frame.  
struct FrameResource
{
	FrameResource();
	FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT skinnedObjectCount, UINT substanceCount, UINT materialCount, UINT nWaveVB);

	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	
	//ID3D12CommandList* m_batchSubmit[NumContexts * 2 + CommandListCount];

	// We cannot reset the allocator until the GPU is done processing the commands.
	 // So each frame needs their own allocator.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocators[4];

	// We cannot update a cbuffer until the GPU is done processing the commands
	// that reference it.  So each frame needs their own cbuffers.

	//per frame
	std::shared_ptr<UploadBuffer<FrameConstants>> FrameCB;

	//per object
	std::shared_ptr<UploadBuffer<ObjectConstants>> ObjectCB;
	std::shared_ptr<UploadBuffer<SubstanceData>> SubstanceCB;
	std::shared_ptr<UploadBuffer<MaterialData>> MaterialCB;

	// Fence value to mark commands up to this fence point.  This lets us
	// check if these frame resources are still in use by the GPU.

	// We cannot update a dynamic vertex buffer until the GPU is done processing
	// the commands that reference it.  So each frame needs their own.
	unique_ptr<UploadBuffer<WavesVertex>> WavesVB;
	UINT64 FenceCount = 0;
};
#endif