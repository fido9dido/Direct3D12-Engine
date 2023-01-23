#ifndef CMESHMANAGER_H
#define CMESHMANAGER_H

#include <unordered_map>
#include <string>
#include <memory>
#include "GeometryGenerator.h"

class CMeshManager
{
	std::unordered_map<std::string, std::shared_ptr<struct MeshGeometry>> MeshGeometryMap;
	std::unordered_map<std::string, std::unique_ptr<MeshData>> MeshDataMap;
	struct ID3D12Device* DeviceRef;
	class CCommandListPool* CommandListPoolRef;
public:
	CMeshManager() = default;
	void Init(struct ID3D12Device* device,class CCommandListPool* cmdPool);
	std::shared_ptr<struct MeshGeometry> GetMeshGeometry(const std::string& name);
	void CreateMeshBuffer(const MeshData& meshData);
};

#endif