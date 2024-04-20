#include "CMeshManager.h"
#include "CCommandListPool.h"
#include "MeshGeometry.h"
#include "UploadBuffer.h"
#include "CRenderUtil.h"
#include "Utility.h"
#include <d3d12.h>

void CMeshManager::Init(ID3D12Device* device, CCommandListPool* cmdPool)
{
	DeviceRef = device;
	CommandListPoolRef = cmdPool;
}

std::shared_ptr<MeshGeometry> CMeshManager::GetMeshGeometry(const std::string& name)
{
	if (name.empty()) { return nullptr; }
	return MeshGeometryMap[name];
}

void CMeshManager::CreateMeshBuffer(const MeshData& meshData)
{
	if (!DeviceRef || !CommandListPoolRef) { return; }
	

	size_t verticesCount = meshData.VerticesList.size();
	const std::vector<VertexData>& verticesList = meshData.VerticesList;

	std::vector<Vertex> vertices(verticesCount);

	for (size_t i = 0; i < verticesCount; ++i)
	{
		const DirectX::XMVECTOR& p = verticesList[i].Position;
		DirectX::XMStoreFloat3(&vertices[i].Position, p);
		DirectX::XMStoreFloat3(&vertices[i].Normal, DirectX::XMVector3Normalize(verticesList[i].Normal));
		vertices[i].TextureCoord = verticesList[i].TextureCoord;

	}

	const std::vector<uint32_t>& indices32List = meshData.Indices32List;

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT indexCount = static_cast<UINT>(indices32List.size());
	UINT vbByteSize = static_cast<UINT>(verticesCount * sizeof(Vertex));
	UINT ibByteSize = indexCount * sizeof(std::uint32_t);
	std::shared_ptr<MeshGeometry> shapeGeometry = make_unique<MeshGeometry>();
	shapeGeometry->Name = meshData.Name;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &shapeGeometry->VertexBufferCPU));
	CopyMemory(shapeGeometry->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &shapeGeometry->IndexBufferCPU));
	CopyMemory(shapeGeometry->IndexBufferCPU->GetBufferPointer(), indices32List.data(), ibByteSize);

	CCommandList commandList = CommandListPoolRef->GetAndReset(nullptr);

	CreateDefaultBuffer(DeviceRef, commandList.Get(), vertices.data(), vbByteSize, shapeGeometry->VertexBufferUpload, shapeGeometry->VertexBufferDefault);
	shapeGeometry->VertexBufferDefault->SetName(TEXT("ShaderGeom VertexBufferGPU"));

	CreateDefaultBuffer(DeviceRef, commandList.Get(), indices32List.data(), ibByteSize, shapeGeometry->IndexBufferUpload, shapeGeometry->IndexBufferDefault);
	shapeGeometry->IndexBufferDefault->SetName(TEXT("ShaderGeom IndexBufferGPU"));
	CommandListPoolRef->Execute(commandList);
	CommandListPoolRef->ReturnCommandList(commandList);

	shapeGeometry->VertexByteStride = sizeof(Vertex);
	shapeGeometry->VertexBufferByteSize = vbByteSize;
	shapeGeometry->IndexFormat = DXGI_FORMAT_R32_UINT;
	shapeGeometry->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry shapeSubmesh;
	shapeSubmesh.IndexCount = indexCount;
	shapeSubmesh.StartIndexLocation = 0;
	shapeSubmesh.BaseVertexLocation = 0;
	shapeGeometry->DrawArgs[meshData.Name] = shapeSubmesh;

	MeshDataMap[meshData.Name] = make_unique<MeshData>(std::move(meshData));
	
	shapeGeometry->MeshDataRef = MeshDataMap[meshData.Name].get();
	
	MeshGeometryMap[meshData.Name] = shapeGeometry;
}
