
#ifndef GEOMETRYGENERATOR_H
#define GEOMETRYGENERATOR_H

//***************************************************************************************
// GeometryGenerator.h by Frank Luna (C) 2011 All Rights Reserved.
//  
// Defines a static class for procedurally generating the geometry of 
// common mathematical objects.
//
// All triangles are generated "outward" facing.  If you want "inward" 
// facing triangles (for example, if you want to place the camera inside
// a sphere to simulate a sky), you will need to:
//   1. Change the Direct3D cull mode or manually reverse the winding order.
//   2. Invert the normal.
//   3. Update the texture coordinates and tangent vectors.
//***************************************************************************************
//Edited by Mohamed Ali  (C) 2021 All Rights Reserved.
//***************************************************************************************

#include <cstdint>
#include <DirectXMath.h>
#include <vector>
#include <array>
#include <bitset>

struct VertexData
{
	VertexData() = default;
	VertexData(
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& normal,
		const DirectX::XMFLOAT3& tangent,
		const DirectX::XMFLOAT2& uv) :
		Position(position),
		Normal(normal),
		TangentU(tangent),
		TextureCoord(uv) {}
	VertexData(
		float posx, float posy, float posz,
		float normx, float normy, float normz,
		float tangx, float tangy, float tangz,
		float u, float v) :

		Position(DirectX::XMVectorSet(posx, posy, posz, 0)),
		Normal(DirectX::XMVectorSet(normx, normy, normz, 0)),
		TangentU(tangx, tangy, tangz),
		TextureCoord(u, v) {}

	DirectX::XMVECTOR Position;
	std::array<int, 3> VoxelIndex; //height column  row  
	DirectX::XMVECTOR Gradientnormal;
	DirectX::XMVECTOR Normal;
	DirectX::XMFLOAT3 TangentU;
	DirectX::XMFLOAT2 TextureCoord;
	DirectX::XMFLOAT3 TextureCoord3D;
	//front back top bottom right left
	std::bitset<12> SignedEdges;
	std::bitset<3> EdgeNormal;

	bool bIsVertexCreated;
	// 1top,2 right,3 bottom ,4left,5front,6back, 7topright,8topleft,9downright,10downleft
	bool bWinding;

};
struct MeshData
{
	std::vector<VertexData>		VerticesList;
	std::vector<std::uint32_t>	Indices32List;
	std::vector<float>		DensityList;
	std::string Name;
	std::vector<uint16_t>& GetIndices16()
	{
		if (Indices16List.empty())
		{
			Indices16List.resize(Indices32List.size());
			for (size_t i = 0; i < Indices32List.size(); ++i)
				Indices16List[i] = static_cast<uint16_t>(Indices32List[i]);
		}

		return Indices16List;
	}

private:
	std::vector<uint16_t> Indices16List;
};
class GeometryGenerator
{
	
public:
	float du;
	float dv;
	float dw;
	
	DirectX::XMVECTOR GetPositionOnEdge(int CurrentIndex, int index1, int index2, MeshData& meshData);
	DirectX::XMVECTOR QEF(DirectX::XMVECTOR vertexPos, DirectX::XMVECTOR normal, DirectX::XMVECTOR intersectionPoint);
	DirectX::XMVECTOR ReCalcPosition(int CurrentIndex, int v1, int v2, MeshData& meshData);

	MeshData CreateBox(float width, float height, float depth, uint32_t numSubdivisions);
	MeshData CreateCubeGrid(float width, float height, float depth);
    MeshData CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount);
    MeshData CreateGeosphere(float radius, uint32_t numSubdivisions);
    MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount);
    MeshData CreateGrid(float width, float depth, uint32_t m, uint32_t n);
	MeshData CreateGridLine(float width, float depth, size_t divisions = 20, float scale=1);
    MeshData CreateQuad(float x, float y, float w, float h, float depth);
	void CreateVoxelGrid();

private:
	float ColCube;
	float RowCube;
	float HeighCube;
	void Subdivide(MeshData& meshData);
	VertexData MidPoint(const VertexData& v0, const VertexData& v1);
    void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, MeshData& meshData);
    void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, MeshData& meshData);
};


#endif