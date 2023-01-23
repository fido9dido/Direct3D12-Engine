#ifndef TERRAIN_H
#define TERRAIN_H

#include <d3d12.h>
#include <DirectXMath.h>

struct TerrainData
{
	DirectX::XMFLOAT3	Position;
	float				Density;
	DirectX::XMFLOAT3	Normal;
	int					SignedEdges[12];
	int					Edgenormal[3];
	int					b_IsVertexCreated;
};

struct CTerrain
{
	unique_ptr<ID3D12Resource> IndexBuffer;
	unique_ptr<ID3D12Resource> VertexBuffer;
	unique_ptr<ID3D12Resource> CounterReadbackBuffer; // UAV Counter
	unique_ptr<ID3D12Resource> IndexReadbackBuffer = nullptr;
	unique_ptr<ID3D12Resource> VertexReadbackBuffer = nullptr;
};
#endif