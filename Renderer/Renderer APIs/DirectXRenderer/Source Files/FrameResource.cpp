#include "FrameResource.h"

FrameResource::FrameResource() : FrameCB(nullptr), ObjectCB(nullptr), SubstanceCB(nullptr), MaterialCB(nullptr)
{
}

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT skinnedObjectCount, UINT substanceCount, UINT materialCount, UINT nWaveVB)
	: FrameCB(nullptr), ObjectCB(nullptr), SubstanceCB(nullptr), MaterialCB(nullptr)
{
	for (size_t i = 0; i < 4; i++)
	{
		ThrowIfFailed(device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(CommandAllocators[i].GetAddressOf())));
	}

	FrameCB = make_unique<UploadBuffer<FrameConstants>>(device, passCount, true);
	//SsaoCB = make_unique<UploadBuffer<SsaoConstants>>(device, 1, true);
	MaterialCB = make_unique<UploadBuffer<MaterialData>>(device, materialCount, false);
	ObjectCB = make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
	SubstanceCB = make_unique<UploadBuffer<SubstanceData>>(device, substanceCount, false);
	//SkinnedCB = make_unique<UploadBuffer<SkinnedConstants>>(device, skinnedObjectCount, true);

	//pWavesVB = make_unique<UploadBuffer<WavesVertex>>(device, nWaveVB, false);
}
