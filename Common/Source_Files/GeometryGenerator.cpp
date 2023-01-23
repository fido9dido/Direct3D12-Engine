//***************************************************************************************
// GeometryGenerator.cpp by Frank Luna (C) 2011 All Rights Reserved.
// Edit by Mohamed Ali (C) All Rights Reserved.
//***************************************************************************************

#include "GeometryGenerator.h"
#include "ImprovedNoise.h"

#include <algorithm>
//#include <Bezier.h>
#include <memory>
#include <math.h> 
#include <fstream> 
#include <iostream>
#include <string> 
#include <bitset>

using namespace DirectX;

///<summary>
/// Creates a box centered at the origin with the given dimensions, where each
/// face has m rows and n columns of VerticesList.
///</summary>		
MeshData GeometryGenerator::CreateBox(float width, float height, float depth, uint32_t numSubdivisions)
{
	MeshData meshData;

	//
	// Create the vertices.
	//

	VertexData v[24];

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	// Fill in the front face vertex data.
	v[0] = VertexData(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = VertexData(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = VertexData(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = VertexData(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = VertexData(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = VertexData(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = VertexData(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = VertexData(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8] = VertexData(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9] = VertexData(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = VertexData(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = VertexData(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = VertexData(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = VertexData(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = VertexData(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = VertexData(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = VertexData(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = VertexData(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = VertexData(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = VertexData(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = VertexData(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = VertexData(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = VertexData(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = VertexData(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	meshData.VerticesList.assign(&v[0], &v[24]);

	//
	// Create the indices.
	//

	uint32_t i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData.Indices32List.assign(&i[0], &i[36]);

	// Put a cap on the number of subdivisions.
	numSubdivisions = std::min<uint32_t>(numSubdivisions, 6u);

	for (uint32_t i = 0; i < numSubdivisions; ++i)
		Subdivide(meshData);

	return meshData;
}


DirectX::XMVECTOR GeometryGenerator::QEF(DirectX::XMVECTOR VertexDataPosX, DirectX::XMVECTOR Normal, DirectX::XMVECTOR intersectionPoint)
{
	DirectX::XMVECTOR subtract = DirectX::XMVectorSubtract(VertexDataPosX, intersectionPoint);
	DirectX::XMVECTOR dot = DirectX::XMVector3Dot(Normal, subtract);
	DirectX::XMVECTOR power = DirectX::XMVectorPow(dot, DirectX::XMVectorSet(2, 2, 2, 0));
	return power;;
}

void ChangeDirection(VertexData vert, DirectX::XMVECTOR& VertexDataNormal)
{
	if (vert.EdgeNormal[0] == true)
	{
		VertexDataNormal.m128_f32[2] = signbit(VertexDataNormal.m128_f32[2]) ? VertexDataNormal.m128_f32[2] * 1.f : VertexDataNormal.m128_f32[2] * -1.f;
	}
	else
	{
		VertexDataNormal.m128_f32[2] = signbit(VertexDataNormal.m128_f32[2]) ? VertexDataNormal.m128_f32[2] * -1.f : VertexDataNormal.m128_f32[2] * 1.f;
	}
	if (vert.EdgeNormal[1] == true)
	{
		VertexDataNormal.m128_f32[0] = signbit(VertexDataNormal.m128_f32[0]) ? VertexDataNormal.m128_f32[0] * 1.f : VertexDataNormal.m128_f32[0] * -1.f;
	}
	else
	{
		VertexDataNormal.m128_f32[0] = signbit(VertexDataNormal.m128_f32[0]) ? VertexDataNormal.m128_f32[0] * -1.f : VertexDataNormal.m128_f32[0] * 1.f;
	}
	if (vert.EdgeNormal[2] == true)
	{
		VertexDataNormal.m128_f32[1] = signbit(VertexDataNormal.m128_f32[1]) ? VertexDataNormal.m128_f32[1] * 1.f : VertexDataNormal.m128_f32[1] * -1.f;
	}
	else
	{
		VertexDataNormal.m128_f32[1] = signbit(VertexDataNormal.m128_f32[1]) ? VertexDataNormal.m128_f32[1] * -1.f : VertexDataNormal.m128_f32[1] * 1.f;
	}

}
///<summary>
/// Creates a naive Dual Contouring algorithm centered at the origin with the given dimensions..
///</summary>
MeshData GeometryGenerator::CreateCubeGrid(float width, float height, float depth)
{
	ImprovedNoise::ImprovedNoise();
	MeshData meshData;
	uint32_t nRows = (depth * 2) + 1, nHeight = (height * 2) + 1, nCols = (width * 2) + 1;
	uint32_t VertexCount = nCols * nRows * nHeight;
	uint32_t faceCount = (nCols - 1) * (nRows - 1) * (nHeight - 1) * 6;
	float halfWidth = 0.5f * width;
	float halfheight = 0.5f * height;
	float halfDepth = 0.5f * depth;
	float dx = width / (nCols - 1);
	float dy = height / (nHeight - 1);
	float dz = depth / (nRows - 1);
	du = 1.0f / (nCols - 1);
	dv = 1.0f / (nRows - 1);
	dw = 1.0f / (nHeight - 1);
	int scale = 15;
	meshData.VerticesList.resize(VertexCount);
	meshData.DensityList.resize(VertexCount);
	;
	float DensityList;	std::ofstream stream;
	for (int height = 0; height < nHeight; ++height)
	{//uncomment to create ppm image
	   /* stream.open("./noise" + std::to_string(height) + ".ppm", std::ios::out | std::ios::binary);
		stream << "P6" << std::endl << nCols << " " << nRows << std::endl << "255" << std::endl;*/
		float y = -halfheight + (height * dy);
		for (int column = 0; column < nCols; ++column)
		{
			float x = -halfWidth + (column * dx);
			for (int row = 0; row < nRows; ++row)
			{
				float z = -halfDepth + (row * dz);
				int currentIndex = ((height * nRows * nCols)) + ((column * nRows)) + row;
				meshData.VerticesList[currentIndex].Position = DirectX::XMVectorSet(x, y, z, 0);
				DensityList = .25 * ImprovedNoise::Noise(4.03 * x / scale, 4.03 * z / scale, 4.03 * y / scale);
				DensityList += .5 * ImprovedNoise::Noise(1.96 * x / scale, 1.96 * z / scale, 1.96 * y / scale);
				DensityList += ImprovedNoise::Noise(1.01 * x / scale, 1.01 * z / scale, 1.01 * y / scale);
				DensityList = ((y / 4)) - floor(DensityList);
				meshData.DensityList[currentIndex] = DensityList;
				//uncomment to create ppm image
				/*unsigned char n = static_cast<unsigned char>(floor(255 * DensityList));
				stream << n << n << n;*/
			}
		}
		stream.close();
	}

	VertexData vert = {};
	std::bitset<8> bin;

	for (int height = 0; height < nHeight - 1; ++height)
	{
		for (int column = 0; column < nCols - 1; ++column)
		{
			for (int row = 0; row < nRows - 1; ++row)
			{
				bin = {}; vert = {};
				int currentIndex = (height * nRows * nCols) + (column * nRows) + row;
				int index1 = (height * nRows * nCols) + (column * nRows) + (row + 1);
				int index2 = (height * nRows * nCols) + (column * nRows) + row + nRows;
				int index3 = (height * nRows * nCols) + (column * nRows) + row + 1 + nRows;
				int index4 = ((height + 1) * nRows * nCols) + (column * nRows) + row;
				int index5 = ((height + 1) * nRows * nCols) + (column * nRows) + row + 1;
				int index6 = ((height + 1) * nRows * nCols) + ((column + 1) * nRows) + row;
				int index7 = ((height + 1) * nRows * nCols) + ((column + 1) * nRows) + row + 1;
				// my order
				bin[0] = signbit(meshData.DensityList[currentIndex]);
				bin[1] = signbit(meshData.DensityList[index1]);
				bin[2] = signbit(meshData.DensityList[index2]);
				bin[3] = signbit(meshData.DensityList[index3]);
				bin[4] = signbit(meshData.DensityList[index4]);
				bin[5] = signbit(meshData.DensityList[index5]);
				bin[6] = signbit(meshData.DensityList[index6]);
				bin[7] = signbit(meshData.DensityList[index7]);

				if (bin.none() || bin.all())
				{
				}
				else
				{
					DirectX::XMVECTOR v0 = meshData.VerticesList[currentIndex].Position;
					DirectX::XMVECTOR v1 = meshData.VerticesList[index1].Position;
					DirectX::XMVECTOR v2 = meshData.VerticesList[index2].Position;
					DirectX::XMVECTOR v3 = meshData.VerticesList[index3].Position;
					DirectX::XMVECTOR v4 = meshData.VerticesList[index4].Position;
					DirectX::XMVECTOR v5 = meshData.VerticesList[index5].Position;
					DirectX::XMVECTOR v6 = meshData.VerticesList[index6].Position;
					DirectX::XMVECTOR v7 = meshData.VerticesList[index7].Position;
					auto center = DirectX::XMVectorAdd(v0, v1);
					center = DirectX::XMVectorAdd(center, v2);
					center = DirectX::XMVectorAdd(center, v3);
					center = DirectX::XMVectorAdd(center, v4);
					center = DirectX::XMVectorAdd(center, v5);
					center = DirectX::XMVectorAdd(center, v6);
					center = DirectX::XMVectorAdd(center, v7);
					center = DirectX::XMVectorScale(center, .125f);

					vert.Position = center;

					if (bin[6] == bin[7])
					{
					}
					else
					{
						meshData.VerticesList[index6].Position = DirectX::XMVectorAdd(center, DirectX::XMVectorSet(dx, dy, 0, 0));
						meshData.VerticesList[index6].bIsVertexCreated = true;
						meshData.VerticesList[index2].Position = DirectX::XMVectorAdd(center, DirectX::XMVectorSet(dx, 0, 0, 0));
						meshData.VerticesList[index2].bIsVertexCreated = true;
						meshData.VerticesList[index4].Position = DirectX::XMVectorAdd(center, DirectX::XMVectorSet(0, dy, 0, 0));
						meshData.VerticesList[index4].bIsVertexCreated = true;
						vert.SignedEdges[10] = true;
						vert.bIsVertexCreated = true;
						if (bin[6] == true)
						{
							vert.EdgeNormal[0] = true;//z= ensure z is negative
						}
						meshData.VerticesList[currentIndex] = vert;

					}
					if (bin[5] == bin[7])
					{
					}
					else
					{
						meshData.VerticesList[index1].Position = DirectX::XMVectorAdd(center, DirectX::XMVectorSet(0, 0, dz, 0));
						meshData.VerticesList[index1].bIsVertexCreated = true;
						meshData.VerticesList[index5].Position = DirectX::XMVectorAdd(center, DirectX::XMVectorSet(0, dy, dz, 0));
						meshData.VerticesList[index5].bIsVertexCreated = true;
						meshData.VerticesList[index4].Position = DirectX::XMVectorAdd(center, DirectX::XMVectorSet(0, dy, 0, 0));
						meshData.VerticesList[index4].bIsVertexCreated = true;
						vert.SignedEdges[1] = true;
						vert.bIsVertexCreated = true;
						if (bin[5] == true)
						{
							vert.EdgeNormal[1] = true;//z= ensure x is negative
						}
						meshData.VerticesList[currentIndex] = vert;
					}
					if (bin[3] == bin[7])
					{
					}
					else
					{
						meshData.VerticesList[index1].Position = DirectX::XMVectorAdd(center, DirectX::XMVectorSet(0, 0, dz, 0));
						meshData.VerticesList[index1].bIsVertexCreated = true;
						meshData.VerticesList[index2].Position = DirectX::XMVectorAdd(center, DirectX::XMVectorSet(dx, 0, 0, 0));
						meshData.VerticesList[index2].bIsVertexCreated = true;
						meshData.VerticesList[index3].Position = DirectX::XMVectorAdd(center, DirectX::XMVectorSet(dx, 0, dz, 0));
						meshData.VerticesList[index3].bIsVertexCreated = true;
						vert.SignedEdges[2] = true;
						vert.bIsVertexCreated = true;
						if (bin[3] == true)
						{
							vert.EdgeNormal[2] = true;//z= ensure y is negative
						}
						meshData.VerticesList[currentIndex] = vert;
					}
				}
			}
		}
	}

	int count; DirectX::XMVECTOR position; float ncenter1, ncenter2, ncenter3;
	DirectX::XMVECTOR error = DirectX::XMVectorZero();
	for (int height = 0; height < nHeight - 1; ++height)
	{
		for (int column = 0; column < nCols - 1; ++column)
		{
			for (int row = 0; row < nRows - 1; ++row)
			{
				int index = (height * nRows * nCols) + (column * nRows) + row;
				int index1 = (height * nRows * nCols) + (column * nRows) + (row + 1);
				int index2 = (height * nRows * nCols) + (column * nRows) + row + nRows;
				int index3 = (height * nRows * nCols) + (column * nRows) + row + 1 + nRows;
				int index4 = ((height + 1) * nRows * nCols) + (column * nRows) + row;
				int index5 = ((height + 1) * nRows * nCols) + (column * nRows) + row + 1;
				int index6 = ((height + 1) * nRows * nCols) + ((column + 1) * nRows) + row;
				int index7 = ((height + 1) * nRows * nCols) + ((column + 1) * nRows) + row + 1;
				vert = meshData.VerticesList[index];
				if (vert.bIsVertexCreated)
				{
					bin[0] = signbit(meshData.DensityList[index]);
					bin[1] = signbit(meshData.DensityList[index1]);
					bin[2] = signbit(meshData.DensityList[index2]);
					bin[3] = signbit(meshData.DensityList[index3]);
					bin[4] = signbit(meshData.DensityList[index4]);
					bin[5] = signbit(meshData.DensityList[index5]);
					bin[6] = signbit(meshData.DensityList[index6]);
					bin[7] = signbit(meshData.DensityList[index7]);
					position = DirectX::XMVectorZero();
					count = 0;
					if (bin[7] != bin[6])
					{
						position += GetPositionOnEdge(index, index6, index7, meshData);
						count += 1;
					}
					if (bin[7] != bin[5])
					{
						position += GetPositionOnEdge(index, index5, index7, meshData);
						count += 1;
					}
					if (bin[7] != bin[3])
					{
						position += GetPositionOnEdge(index, index3, index7, meshData);
						count += 1;
					}
					if (bin[0] != bin[1])
					{
						position += GetPositionOnEdge(index, index, index1, meshData);
						count += 1;
					}
					if (bin[0] != bin[2])
					{
						position += GetPositionOnEdge(index, index, index2, meshData);
						count += 1;
					}
					if (bin[0] != bin[4])
					{
						position += GetPositionOnEdge(index, index, index4, meshData);
						count += 1;
					}
					if (bin[2] != bin[3])
					{
						position += GetPositionOnEdge(index, index2, index3, meshData);
						count += 1;
					}
					if (bin[2] != bin[6])
					{
						position += GetPositionOnEdge(index, index2, index6, meshData);
						count += 1;
					}
					if (bin[1] != bin[3])
					{
						position += GetPositionOnEdge(index, index1, index3, meshData);
						count += 1;
					}
					if (bin[1] != bin[5])
					{
						position += GetPositionOnEdge(index, index1, index5, meshData);
						count += 1;
					}
					if (bin[4] != bin[5])
					{
						position += GetPositionOnEdge(index, index4, index5, meshData);
						count += 1;
					}
					if (bin[4] != bin[6])
					{
						position += GetPositionOnEdge(index, index4, index6, meshData);
						count += 1;
					}
					float avg = (float)1 / count;
					position = DirectX::XMVectorScale(position, avg);
					meshData.VerticesList[index].Position = position;
				}
			}
		}
	}

	dx = (float)3 / nRows;
	dy = (float)3 / nHeight;
	dz = (float)3 / nCols;

	for (int height = 0; height < nHeight - 1; ++height)
	{
		for (int column = 0; column < nCols - 1; ++column)
		{
			for (int row = 0; row < nRows - 1; ++row)
			{
				int index = (height * nRows * nCols) + (column * nRows) + row;
				int index1 = (height * nRows * nCols) + (column * nRows) + (row + 1);
				int index2 = (height * nRows * nCols) + (column * nRows) + row + nRows;
				int index3 = (height * nRows * nCols) + (column * nRows) + row + 1 + nRows;
				int index4 = ((height + 1) * nRows * nCols) + (column * nRows) + row;
				int index5 = ((height + 1) * nRows * nCols) + (column * nRows) + row + 1;
				int index6 = ((height + 1) * nRows * nCols) + ((column + 1) * nRows) + row;
				int index7 = ((height + 1) * nRows * nCols) + ((column + 1) * nRows) + row + 1; // index74
				vert = meshData.VerticesList[index];
				XMVECTOR v2 = meshData.VerticesList[index2].Position;
				XMVECTOR v = meshData.VerticesList[index].Position;
				XMVECTOR v1 = meshData.VerticesList[index1].Position;
				XMVECTOR v3 = meshData.VerticesList[index3].Position;
				XMVECTOR v4 = meshData.VerticesList[index4].Position;
				XMVECTOR v5 = meshData.VerticesList[index5].Position;
				XMVECTOR v6 = meshData.VerticesList[index6].Position;
				XMVECTOR v7 = meshData.VerticesList[index7].Position;

				if (vert.bIsVertexCreated)
				{
					int count = 0;
					if (vert.SignedEdges[1])
					{
						XMVECTOR N = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v, v1), DirectX::XMVectorSubtract(v, v4));
						XMVECTOR a = DirectX::XMVector3Dot(DirectX::XMVectorSubtract(v, v1), DirectX::XMVectorSubtract(v, v4));
						ChangeDirection(vert, N);

						XMVECTOR N1 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v1, v), DirectX::XMVectorSubtract(v1, v5));
						XMVECTOR a1 = DirectX::XMVector3Dot(DirectX::XMVectorSubtract(v1, v), DirectX::XMVectorSubtract(v1, v5));
						ChangeDirection(vert, N1);
						XMVECTOR N5 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v5, v4), DirectX::XMVectorSubtract(v5, v1));
						XMVECTOR a5 = DirectX::XMVector3Dot(DirectX::XMVectorSubtract(v5, v4), DirectX::XMVectorSubtract(v5, v1));
						ChangeDirection(vert, N5);
						XMVECTOR N4 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v4, v), DirectX::XMVectorSubtract(v4, v5));
						XMVECTOR a4 = DirectX::XMVector3Dot(DirectX::XMVectorSubtract(v4, v), DirectX::XMVectorSubtract(v4, v5));
						ChangeDirection(vert, N4);
						meshData.VerticesList[index].Normal += N;
						meshData.VerticesList[index1].Normal += N;
						meshData.VerticesList[index4].Normal += N;
						meshData.VerticesList[index5].Normal += N5;
						meshData.VerticesList[index4].Normal += N5;
						meshData.VerticesList[index1].Normal += N5;

						if (signbit(N.m128_f32[0]) == false)
						{
							meshData.Indices32List.push_back(index1);
							meshData.Indices32List.push_back(index5);
							meshData.Indices32List.push_back(index4);
							meshData.Indices32List.push_back(index1);
							meshData.Indices32List.push_back(index4);
							meshData.Indices32List.push_back(index);

						}
						else
						{
							meshData.Indices32List.push_back(index);
							meshData.Indices32List.push_back(index4);
							meshData.Indices32List.push_back(index5);
							meshData.Indices32List.push_back(index);
							meshData.Indices32List.push_back(index5);
							meshData.Indices32List.push_back(index1);
						}
					}
					if (vert.SignedEdges[10])
					{
						XMVECTOR N = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v, v4), DirectX::XMVectorSubtract(v, v2));
						XMVECTOR a = DirectX::XMVector3Dot(DirectX::XMVectorSubtract(v, v4), DirectX::XMVectorSubtract(v, v2));
						ChangeDirection(vert, N);
						XMVECTOR N2 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v2, v), DirectX::XMVectorSubtract(v2, v6));
						XMVECTOR a2 = DirectX::XMVector3Dot(DirectX::XMVectorSubtract(v2, v), DirectX::XMVectorSubtract(v2, v6));
						ChangeDirection(vert, N2);
						XMVECTOR N6 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v6, v4), DirectX::XMVectorSubtract(v6, v2));
						XMVECTOR a6 = DirectX::XMVector3Dot(DirectX::XMVectorSubtract(v6, v4), DirectX::XMVectorSubtract(v6, v2));
						ChangeDirection(vert, N6);
						XMVECTOR N4 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v4, v), DirectX::XMVectorSubtract(v4, v6));
						XMVECTOR a4 = DirectX::XMVector3Dot(DirectX::XMVectorSubtract(v4, v), DirectX::XMVectorSubtract(v4, v6));
						ChangeDirection(vert, N4);

						meshData.VerticesList[index].Normal += N;
						meshData.VerticesList[index4].Normal += N;;
						meshData.VerticesList[index2].Normal += N;//*a2;
						meshData.VerticesList[index6].Normal += N6;//*a6;
						meshData.VerticesList[index4].Normal += N6;;
						meshData.VerticesList[index2].Normal += N6;//*a2;

						if (signbit(N.m128_f32[2]) == false)
						{
							meshData.Indices32List.push_back(index2);
							meshData.Indices32List.push_back(index);
							meshData.Indices32List.push_back(index4);
							meshData.Indices32List.push_back(index2);
							meshData.Indices32List.push_back(index4);
							meshData.Indices32List.push_back(index6);
						}
						else
						{
							meshData.Indices32List.push_back(index4);
							meshData.Indices32List.push_back(index);
							meshData.Indices32List.push_back(index2);
							meshData.Indices32List.push_back(index4);
							meshData.Indices32List.push_back(index2);
							meshData.Indices32List.push_back(index6);
						}
					}

					if (vert.SignedEdges[2])
					{

						XMVECTOR N = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v, v1), DirectX::XMVectorSubtract(v, v2));
						XMVECTOR A = DirectX::XMVector3Dot(DirectX::XMVectorSubtract(v, v1), DirectX::XMVectorSubtract(v, v2));
						ChangeDirection(vert, N);
						XMVECTOR N1 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v1, v), DirectX::XMVectorSubtract(v1, v3));
						XMVECTOR A1 = DirectX::XMVector3Dot(DirectX::XMVectorSubtract(v1, v), DirectX::XMVectorSubtract(v1, v3));
						ChangeDirection(vert, N1);
						XMVECTOR N2 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v2, v), DirectX::XMVectorSubtract(v2, v3));
						XMVECTOR A2 = DirectX::XMVector3Dot(DirectX::XMVectorSubtract(v2, v), DirectX::XMVectorSubtract(v2, v3));
						ChangeDirection(vert, N2);
						XMVECTOR N3 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(v3, v1), DirectX::XMVectorSubtract(v3, v2));
						XMVECTOR A3 = DirectX::XMVector3Dot(DirectX::XMVectorSubtract(v3, v1), DirectX::XMVectorSubtract(v3, v2));
						ChangeDirection(vert, N3);

						meshData.VerticesList[index1].Normal += N1;
						meshData.VerticesList[index].Normal += N1;
						meshData.VerticesList[index3].Normal += N1;
						meshData.VerticesList[index2].Normal += N2;
						meshData.VerticesList[index].Normal += N2;
						meshData.VerticesList[index3].Normal += N2;

						if (signbit(N.m128_f32[1]) == false)
						{
							meshData.Indices32List.push_back(index);
							meshData.Indices32List.push_back(index2);
							meshData.Indices32List.push_back(index3);
							meshData.Indices32List.push_back(index);
							meshData.Indices32List.push_back(index3);
							meshData.Indices32List.push_back(index1);
						}
						else
						{
							meshData.Indices32List.push_back(index);
							meshData.Indices32List.push_back(index1);
							meshData.Indices32List.push_back(index3);
							meshData.Indices32List.push_back(index);
							meshData.Indices32List.push_back(index3);
							meshData.Indices32List.push_back(index2);
						}
					}
				}
			}
		}
	}

	return meshData;
}

DirectX::XMVECTOR GeometryGenerator::GetPositionOnEdge(int CurrentIndex, int index1, int index2, MeshData& meshData)
{
	float marg =
		abs(meshData.DensityList[index1])
		/ (abs(meshData.DensityList[index1])
			+ abs(meshData.DensityList[index2]));
	DirectX::XMVECTOR v1 = meshData.VerticesList[index1].Position;
	DirectX::XMVECTOR v2 = meshData.VerticesList[index2].Position;

	DirectX::XMVECTOR IntersectPoint = DirectX::XMVectorAbs(DirectX::XMVectorSubtract(v2, v1));
	IntersectPoint = DirectX::XMVectorScale(IntersectPoint, marg);
	IntersectPoint = DirectX::XMVectorAdd(IntersectPoint, v1);
	return IntersectPoint;
}
DirectX::XMVECTOR GeometryGenerator::ReCalcPosition(int CurrentIndex, int index1, int index2, MeshData& meshData)
{
	float marg =
		abs(meshData.DensityList[index1])
		/ (abs(meshData.DensityList[index1])
			+ abs(meshData.DensityList[index2]));
	DirectX::XMVECTOR v1 = meshData.VerticesList[index1].Position;
	DirectX::XMVECTOR v2 = meshData.VerticesList[index2].Position;

	DirectX::XMVECTOR IntersectPoint = DirectX::XMVectorAbs(DirectX::XMVectorSubtract(v2, v1));
	IntersectPoint = DirectX::XMVectorScale(IntersectPoint, marg);
	IntersectPoint = DirectX::XMVectorAdd(IntersectPoint, v1);

	DirectX::XMVECTOR IntersectPointNormal = DirectX::XMVectorSet
	(
		(ImprovedNoise::Noise((double)(IntersectPoint.m128_f32[0] + dv), IntersectPoint.m128_f32[1], IntersectPoint.m128_f32[2]) - ImprovedNoise::Noise((double)(IntersectPoint.m128_f32[0] - dv), IntersectPoint.m128_f32[1], IntersectPoint.m128_f32[2])) / (2 * dv),
		(ImprovedNoise::Noise(IntersectPoint.m128_f32[0], (double)(IntersectPoint.m128_f32[1] + dw), IntersectPoint.m128_f32[2]) - ImprovedNoise::Noise(IntersectPoint.m128_f32[0], (double)(IntersectPoint.m128_f32[1] - dw), IntersectPoint.m128_f32[2])) / (2 * dw),
		(ImprovedNoise::Noise(IntersectPoint.m128_f32[0], IntersectPoint.m128_f32[1], (double)(IntersectPoint.m128_f32[2] + du)) - ImprovedNoise::Noise(IntersectPoint.m128_f32[0], IntersectPoint.m128_f32[1], (double)(IntersectPoint.m128_f32[2] - du))) / (2 * du),
		0
	);
	IntersectPointNormal = DirectX::XMVector3Normalize(IntersectPointNormal);

	return  QEF(meshData.VerticesList[CurrentIndex].Position, IntersectPointNormal, IntersectPoint);
}
///<summary>
/// Creates a sphere centered at the origin with the given radius.  The
/// slices and stacks parameters control the degree of tessellation.
///</summary>			
MeshData GeometryGenerator::CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount)
{
	MeshData meshData;

	//
	// Compute the VerticesList stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	VertexData topVertexData(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	VertexData bottomVertexData(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	meshData.VerticesList.push_back(topVertexData);

	float phiStep = XM_PI / stackCount;
	float thetaStep = 2.0f * XM_PI / sliceCount;

	// Compute VerticesList for each stack ring (do not count the poles as rings).
	for (uint32_t i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// VerticesList of ring.
		for (uint32_t j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			VertexData v;

			// spherical to cartesian
			v.Position = DirectX::XMVectorSet(radius * sinf(phi) * cosf(theta),
				radius * cosf(phi), radius * sinf(phi) * sinf(theta), 0);



			// Partial derivative of P with respect to theta
			v.TangentU.x = -radius * sinf(phi) * sinf(theta);
			v.TangentU.y = 0.0f;
			v.TangentU.z = +radius * sinf(phi) * cosf(theta);

			XMVECTOR T = XMLoadFloat3(&v.TangentU);
			XMStoreFloat3(&v.TangentU, XMVector3Normalize(T));

			XMVECTOR p = v.Position;
			v.Normal = XMVector3Normalize(p);

			v.TextureCoord.x = theta / XM_2PI;
			v.TextureCoord.y = phi / XM_PI;

			meshData.VerticesList.push_back(v);
		}
	}

	meshData.VerticesList.push_back(bottomVertexData);

	//
	// Compute indices for top stack.  The top stack was written first to the VertexData buffer
	// and connects the top pole to the first ring.
	//

	for (uint32_t i = 1; i <= sliceCount; ++i)
	{
		meshData.Indices32List.push_back(0);
		meshData.Indices32List.push_back(i + 1);
		meshData.Indices32List.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first VertexData in the first ring.
	// This is just skipping the top pole VertexData.
	uint32_t baseIndex = 1;
	uint32_t ringVertexDataCount = sliceCount + 1;
	for (uint32_t i = 0; i < stackCount - 2; ++i)
	{
		for (uint32_t j = 0; j < sliceCount; ++j)
		{
			meshData.Indices32List.push_back(baseIndex + i * ringVertexDataCount + j);
			meshData.Indices32List.push_back(baseIndex + i * ringVertexDataCount + j + 1);
			meshData.Indices32List.push_back(baseIndex + (i + 1) * ringVertexDataCount + j);

			meshData.Indices32List.push_back(baseIndex + (i + 1) * ringVertexDataCount + j);
			meshData.Indices32List.push_back(baseIndex + i * ringVertexDataCount + j + 1);
			meshData.Indices32List.push_back(baseIndex + (i + 1) * ringVertexDataCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the VertexData buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole VertexData was added last.
	uint32_t southPoleIndex = (uint32_t)meshData.VerticesList.size() - 1;

	// Offset the indices to the index of the first VertexData in the last ring.
	baseIndex = southPoleIndex - ringVertexDataCount;

	for (uint32_t i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32List.push_back(southPoleIndex);
		meshData.Indices32List.push_back(baseIndex + i);
		meshData.Indices32List.push_back(baseIndex + i + 1);
	}

	return meshData;
}

void GeometryGenerator::Subdivide(MeshData& meshData)
{
	// Save a copy of the input geometry.
	MeshData inputCopy = meshData;


	meshData.VerticesList.resize(0);
	meshData.Indices32List.resize(0);

	//       v1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// v0    m2     v2

	uint32_t numTris = (uint32_t)inputCopy.Indices32List.size() / 3;
	for (uint32_t i = 0; i < numTris; ++i)
	{
		VertexData v0 = inputCopy.VerticesList[inputCopy.Indices32List[i * 3 + 0]];
		VertexData v1 = inputCopy.VerticesList[inputCopy.Indices32List[i * 3 + 1]];
		VertexData v2 = inputCopy.VerticesList[inputCopy.Indices32List[i * 3 + 2]];

		//
		// Generate the midpoints.
		//

		VertexData m0 = MidPoint(v0, v1);
		VertexData m1 = MidPoint(v1, v2);
		VertexData m2 = MidPoint(v0, v2);

		//
		// Add new geometry.
		//

		meshData.VerticesList.push_back(v0); // 0
		meshData.VerticesList.push_back(v1); // 1
		meshData.VerticesList.push_back(v2); // 2
		meshData.VerticesList.push_back(m0); // 3
		meshData.VerticesList.push_back(m1); // 4
		meshData.VerticesList.push_back(m2); // 5

		meshData.Indices32List.push_back(i * 6 + 0);
		meshData.Indices32List.push_back(i * 6 + 3);
		meshData.Indices32List.push_back(i * 6 + 5);

		meshData.Indices32List.push_back(i * 6 + 3);
		meshData.Indices32List.push_back(i * 6 + 4);
		meshData.Indices32List.push_back(i * 6 + 5);

		meshData.Indices32List.push_back(i * 6 + 5);
		meshData.Indices32List.push_back(i * 6 + 4);
		meshData.Indices32List.push_back(i * 6 + 2);

		meshData.Indices32List.push_back(i * 6 + 3);
		meshData.Indices32List.push_back(i * 6 + 1);
		meshData.Indices32List.push_back(i * 6 + 4);
	}
}

VertexData GeometryGenerator::MidPoint(const VertexData& v0, const VertexData& v1)
{
	XMVECTOR p0 = v0.Position;
	XMVECTOR p1 = v1.Position;

	XMVECTOR n0 = v0.Normal;
	XMVECTOR n1 = v1.Normal;

	XMVECTOR tan0 = XMLoadFloat3(&v0.TangentU);
	XMVECTOR tan1 = XMLoadFloat3(&v1.TangentU);

	XMVECTOR tex0 = XMLoadFloat2(&v0.TextureCoord);
	XMVECTOR tex1 = XMLoadFloat2(&v1.TextureCoord);

	// Compute the midpoints of all the attributes.  Vectors need to be Normalized
	// since linear interpolating can make them not unit length.  
	XMVECTOR pos = 0.5f * (p0 + p1);
	XMVECTOR Normal = XMVector3Normalize(0.5f * (n0 + n1));
	XMVECTOR tangent = XMVector3Normalize(0.5f * (tan0 + tan1));
	XMVECTOR tex = 0.5f * (tex0 + tex1);

	VertexData v;
	v.Position = pos;
	v.Normal = Normal;
	XMStoreFloat3(&v.TangentU, tangent);
	XMStoreFloat2(&v.TextureCoord, tex);

	return v;
}
///<summary>
/// Creates a geosphere centered at the origin with the given radius.  The
/// depth controls the level of tessellation.
///</summary>			
MeshData GeometryGenerator::CreateGeosphere(float radius, uint32_t numSubdivisions)
{
	MeshData meshData;

	// Put a cap on the number of subdivisions.
	numSubdivisions = std::min<uint32_t>(numSubdivisions, 6u);

	// Approximate a sphere by tessellating an icosahedron.

	const float X = 0.525731f;
	const float Z = 0.850651f;

	XMFLOAT3 pos[12] =
	{
		XMFLOAT3(-X, 0.0f, Z),  XMFLOAT3(X, 0.0f, Z),
		XMFLOAT3(-X, 0.0f, -Z), XMFLOAT3(X, 0.0f, -Z),
		XMFLOAT3(0.0f, Z, X),   XMFLOAT3(0.0f, Z, -X),
		XMFLOAT3(0.0f, -Z, X),  XMFLOAT3(0.0f, -Z, -X),
		XMFLOAT3(Z, X, 0.0f),   XMFLOAT3(-Z, X, 0.0f),
		XMFLOAT3(Z, -X, 0.0f),  XMFLOAT3(-Z, -X, 0.0f)
	};

	uint32_t k[60] =
	{
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
	};

	meshData.VerticesList.resize(12);
	meshData.Indices32List.assign(&k[0], &k[60]);

	for (uint32_t i = 0; i < 12; ++i)
		meshData.VerticesList[i].Position = XMLoadFloat3(&pos[i]);

	for (uint32_t i = 0; i < numSubdivisions; ++i)
		Subdivide(meshData);

	// Project VerticesList onto sphere and scale.
	for (uint32_t i = 0; i < meshData.VerticesList.size(); ++i)
	{
		// Project onto unit sphere.
		XMVECTOR n = XMVector3Normalize(meshData.VerticesList[i].Position);

		// Project onto sphere.
		XMVECTOR p = radius * n;

		meshData.VerticesList[i].Position = p;
		meshData.VerticesList[i].Normal = n;

		// Derive texture coordinates from spherical coordinates.
		float theta = atan2f(meshData.VerticesList[i].Position.m128_f32[2], meshData.VerticesList[i].Position.m128_f32[0]);

		// Put in [0, 2pi].
		if (theta < 0.0f)
			theta += XM_2PI;

		float phi = acosf(meshData.VerticesList[i].Position.m128_f32[1] / radius);

		meshData.VerticesList[i].TextureCoord.x = theta / XM_2PI;
		meshData.VerticesList[i].TextureCoord.y = phi / XM_PI;

		// Partial derivative of P with respect to theta
		meshData.VerticesList[i].TangentU.x = -radius * sinf(phi) * sinf(theta);
		meshData.VerticesList[i].TangentU.y = 0.0f;
		meshData.VerticesList[i].TangentU.z = +radius * sinf(phi) * cosf(theta);

		XMVECTOR T = XMLoadFloat3(&meshData.VerticesList[i].TangentU);
		XMStoreFloat3(&meshData.VerticesList[i].TangentU, XMVector3Normalize(T));
	}

	return meshData;
}

///<summary>
/// Creates a cylinder parallel to the y-axis, and centered about the origin.  
/// The bottom and top radius can vary to form various cone shapes rather than true
/// cylinders.  The slices and stacks parameters control the degree of tessellation.
///</summary>
MeshData GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount)
{
	MeshData meshData;

	//
	// Build Stacks.
	// 

	float stackHeight = height / stackCount;

	// Amount to increment radius as we move up each stack level from bottom to top.
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	uint32_t ringCount = stackCount + 1;

	// Compute VerticesList for each stack ring starting at the bottom and moving up.
	for (uint32_t i = 0; i < ringCount; ++i)
	{
		float y = -0.5f * height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;

		// VerticesList of ring
		float dTheta = 2.0f * XM_PI / sliceCount;
		for (uint32_t j = 0; j <= sliceCount; ++j)
		{
			VertexData VertexData;

			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			VertexData.Position = XMVectorSet(r * c, y, r * s, 0);

			VertexData.TextureCoord.x = (float)j / sliceCount;
			VertexData.TextureCoord.y = 1.0f - (float)i / stackCount;

			// Cylinder can be parameterized as follows, where we introduce v
			// parameter that goes in the same direction as the v tex-coord
			// so that the bitangent goes in the same direction as the v tex-coord.
			//   Let r0 be the bottom radius and let r1 be the top radius.
			//   y(v) = h - hv for v in [0,1].
			//   r(v) = r1 + (r0-r1)v
			//
			//   x(t, v) = r(v)*cos(t)
			//   y(t, v) = h - hv
			//   z(t, v) = r(v)*sin(t)
			// 
			//  dx/dt = -r(v)*sin(t)
			//  dy/dt = 0
			//  dz/dt = +r(v)*cos(t)
			//
			//  dx/dv = (r0-r1)*cos(t)
			//  dy/dv = -h
			//  dz/dv = (r0-r1)*sin(t)

			// This is unit length.
			VertexData.TangentU = XMFLOAT3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			XMFLOAT3 bitangent(dr * c, -height, dr * s);

			XMVECTOR T = XMLoadFloat3(&VertexData.TangentU);
			XMVECTOR B = XMLoadFloat3(&bitangent);
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
			VertexData.Normal = N;

			meshData.VerticesList.push_back(VertexData);
		}
	}

	// Add one because we duplicate the first and last VertexData per ring
	// since the texture coordinates are different.
	uint32_t ringVertexDataCount = sliceCount + 1;

	// Compute indices for each stack.
	for (uint32_t i = 0; i < stackCount; ++i)
	{
		for (uint32_t j = 0; j < sliceCount; ++j)
		{
			meshData.Indices32List.push_back(i * ringVertexDataCount + j);
			meshData.Indices32List.push_back((i + 1) * ringVertexDataCount + j);
			meshData.Indices32List.push_back((i + 1) * ringVertexDataCount + j + 1);

			meshData.Indices32List.push_back(i * ringVertexDataCount + j);
			meshData.Indices32List.push_back((i + 1) * ringVertexDataCount + j + 1);
			meshData.Indices32List.push_back(i * ringVertexDataCount + j + 1);
		}
	}

	BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
	BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);

	return meshData;
}

void GeometryGenerator::BuildCylinderTopCap(float bottomRadius, float topRadius, float height,
	uint32_t sliceCount, uint32_t stackCount, MeshData& meshData)
{
	uint32_t baseIndex = (uint32_t)meshData.VerticesList.size();

	float y = 0.5f * height;
	float dTheta = 2.0f * XM_PI / sliceCount;

	// Duplicate cap ring VerticesList because the texture coordinates and Normals differ.
	for (uint32_t i = 0; i <= sliceCount; ++i)
	{
		float x = topRadius * cosf(i * dTheta);
		float z = topRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.VerticesList.push_back(VertexData(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center VertexData.
	meshData.VerticesList.push_back(VertexData(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Index of center VertexData.
	uint32_t centerIndex = (uint32_t)meshData.VerticesList.size() - 1;

	for (uint32_t i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32List.push_back(centerIndex);
		meshData.Indices32List.push_back(baseIndex + i + 1);
		meshData.Indices32List.push_back(baseIndex + i);
	}
}

void GeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height,
	uint32_t sliceCount, uint32_t stackCount, MeshData& meshData)
{
	// 
	// Build bottom cap.
	//

	uint32_t baseIndex = (uint32_t)meshData.VerticesList.size();
	float y = -0.5f * height;

	// VerticesList of ring
	float dTheta = 2.0f * XM_PI / sliceCount;
	for (uint32_t i = 0; i <= sliceCount; ++i)
	{
		float x = bottomRadius * cosf(i * dTheta);
		float z = bottomRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.VerticesList.push_back(VertexData(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center VertexData.
	meshData.VerticesList.push_back(VertexData(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Cache the index of center VertexData.
	uint32_t centerIndex = (uint32_t)meshData.VerticesList.size() - 1;

	for (uint32_t i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32List.push_back(centerIndex);
		meshData.Indices32List.push_back(baseIndex + i);
		meshData.Indices32List.push_back(baseIndex + i + 1);
	}
}
///<summary>
/// Creates an mxn grid in the xz-plane with m rows and n columns, centered
/// at the origin with the specified width and depth.
///</summary>
/// <param name="width"></param>
/// <param name="depth"></param>
/// <param name="m"> number of VerticesList</param>
/// <param name="n">number of VerticesList</param>
/// <returns></returns>
MeshData GeometryGenerator::CreateGrid(float width, float depth, uint32_t m, uint32_t n)
{
	MeshData meshData;

	uint32_t VertexDataCount = m * n;
	uint32_t faceCount = (m - 1) * (n - 1) * 2;

	//
	// Create the VerticesList.
	//

	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	meshData.VerticesList.resize(VertexDataCount);
	for (uint32_t i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (uint32_t j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			meshData.VerticesList[i * n + j].Position = XMVectorSet(x, 0.0f, z, 0);
			meshData.VerticesList[i * n + j].Normal = XMVectorSet(0.0f, 1.0f, 0.0f, 0);
			meshData.VerticesList[i * n + j].TangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);

			// Stretch texture over grid.
			meshData.VerticesList[i * n + j].TextureCoord.x = j * du;
			meshData.VerticesList[i * n + j].TextureCoord.y = i * dv;
		}
	}

	//
	// Create the indices.
	//

	meshData.Indices32List.resize(faceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	uint32_t k = 0;
	for (uint32_t i = 0; i < m - 1; ++i)
	{
		for (uint32_t j = 0; j < n - 1; ++j)
		{
			meshData.Indices32List[k] = i * n + j;
			meshData.Indices32List[k + 1] = i * n + j + 1;
			meshData.Indices32List[k + 2] = (i + 1) * n + j;

			meshData.Indices32List[k + 3] = (i + 1) * n + j;
			meshData.Indices32List[k + 4] = i * n + j + 1;
			meshData.Indices32List[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	return meshData;
}


MeshData GeometryGenerator::CreateGridLine(float width, float height, size_t divisions, float scale)
{
	MeshData meshData;
	DirectX::XMVECTOR xaxis = DirectX::XMVectorZero();
	DirectX::XMVECTOR yaxis = DirectX::XMVectorZero();
	DirectX::XMVECTOR origin = DirectX::XMVectorZero();

	//float numberOfVerticesList = width / divisions;
	int gridsize = (width + 1) * (height + 1);
	meshData.VerticesList.resize(gridsize);

	//size_t k = 0;
	for (uint32_t i = 0; i <= width; ++i)
	{
		for (uint32_t j = 0; j <= height; j++)
		{
			meshData.VerticesList[i * (height + 1) + j].Position = XMVectorSet(j * scale, 0.0f, i * scale, 0.0f);
			meshData.VerticesList[i * (height + 1) + j].Normal = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			meshData.VerticesList[i * (height + 1) + j].TangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);
		}
	}
	meshData.Indices32List.resize((divisions * 4) + (divisions * divisions * 4)); // 3 indices per face

	// draw edges
	uint32_t k = 0;
	uint32_t column = divisions + 1;
	for (size_t i = 1; i <= divisions; i++)
	{
		meshData.Indices32List[k] = i - 1;
		meshData.Indices32List[k + 1] = i;


		meshData.Indices32List[k + 2] = i * column;
		meshData.Indices32List[k + 3] = (i * column) - column;



		k += 4;
	}

	for (uint32_t i = 1; i <= width; ++i)
	{
		for (uint32_t j = 1; j <= height; ++j)
		{
			meshData.Indices32List[k] = (i * (height + 1)) + j;
			meshData.Indices32List[k + 1] = i * (height + 1) + j - 1;


			meshData.Indices32List[k + 2] = i * (height + 1) + j;
			meshData.Indices32List[k + 3] = i * (height + 1) + j - column;


			k += 4; // next quad
		}
	}
	int x = 5;

	return meshData;
}
///<summary>
/// Creates a quad aligned with the screen.  This is useful for postprocessing and screen effects.
///</summary>
MeshData GeometryGenerator::CreateQuad(float x, float y, float w, float h, float depth)
{
	MeshData meshData;

	meshData.VerticesList.resize(4);
	meshData.Indices32List.resize(6);

	// pos coordinates specified in NDC space.
	meshData.VerticesList[0] = VertexData(
		x, y - h, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	meshData.VerticesList[1] = VertexData(
		x, y, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	meshData.VerticesList[2] = VertexData(
		x + w, y, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	meshData.VerticesList[3] = VertexData(
		x + w, y - h, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	meshData.Indices32List[0] = 0;
	meshData.Indices32List[1] = 1;
	meshData.Indices32List[2] = 2;

	meshData.Indices32List[3] = 0;
	meshData.Indices32List[4] = 2;
	meshData.Indices32List[5] = 3;

	return meshData;
}

void GeometryGenerator::CreateVoxelGrid()
{
	//todo delete
}
