#include"ImprovedNoise.hlsl"

cbuffer cbSettings
{
	int vertexCount;
};

struct TerrainPatchSize
{
	uint width;
	uint row;
	uint depth;
};


struct TerrainData
{
	float3 position;
	float3 normal;
};

struct Vertex
{
	float3 pos;
	float density;
	float3 normal;

	int  signedEdges[12];
	int edgenormal[3];
	int isVertexCreated;
};

inline void ForceNormalToPointToAir(Vertex vert, inout float3 vertexNormal)
{

	if (vert.edgenormal[0] == 1)
	{
		vertexNormal.z = (sign(vertexNormal.z) < 0) ? vertexNormal.z * 1.f : vertexNormal.z * -1.f;
	}
	else
	{
		vertexNormal.z = (sign(vertexNormal.z) < 0) ? vertexNormal.z * -1.f : vertexNormal.z * 1.f;
	}
	if (vert.edgenormal[1] == 1)
	{
		vertexNormal.x = (sign(vertexNormal.x) < 0) ? vertexNormal.x * 1.f : vertexNormal.x * -1.f;
	}
	else
	{
		vertexNormal.x = (sign(vertexNormal.x) < 0) ? vertexNormal.x * -1.f : vertexNormal.x * 1.f;
	}
	if (vert.edgenormal[2] == 1)
	{
		vertexNormal.y = (sign(vertexNormal.y) < 0) ? vertexNormal.y * 1.f : vertexNormal.y * -1.f;
	}
	else
	{
		vertexNormal.y = (sign(vertexNormal.y) < 0) ? vertexNormal.y * -1.f : vertexNormal.y * 1.f;
	}


}

inline Vertex InitVertex()
{
	Vertex vert;
	[unroll]
	for (int i = 0; i < 12; ++i)
	{
		vert.signedEdges[i] = 0;
	}
	vert.pos = float3(0.f, 0.f, 0.f);
	vert.normal = float3(0.f, 0.f, 0.f);
	vert.density = 0.f;
	vert.isVertexCreated = 0;
	vert.edgenormal[0] = 0;
	vert.edgenormal[1] = 0;
	vert.edgenormal[2] = 0;
	return vert;


}
#define X 32 
#define Y 32 
#define Z 32 
RWStructuredBuffer <Vertex> tempVertexBuffer:register(u0);
RWStructuredBuffer<TerrainData> VertexBufferOutput:register(u1);
AppendStructuredBuffer<uint3> IndexBufferOutput:register(u2);

inline void GetPositionOnEdge(float density1, float density2, int index1, int index2, inout float3  output)
{
	float marg =
		abs(density1)
		/ (abs(density1)
			+ abs(density2));

	float3 IntersectPoint = abs(tempVertexBuffer[index2].pos - tempVertexBuffer[index1].pos);
	IntersectPoint = IntersectPoint * marg;
	IntersectPoint = IntersectPoint + tempVertexBuffer[index1].pos;
	output += IntersectPoint;
}
[numthreads(X, Y, 1)]
void GenerateTempBuffer(uint3 DTid : SV_DispatchThreadID)
{
	uint nDepths = Z, nRows = Y, nCols = X;
	float row = DTid.y;
	float column = DTid.x;
	float depth = DTid.z;
	float3 halfDim = 0.5f * float3(nCols, nRows, nDepths);;
	int scale = 15;
	float3 delta = float3(.5, .5, .5);
	float y = -halfDim.y + (row * delta.y);
	float x = -halfDim.x + (column * delta.x);
	float z = -halfDim.z + (depth * delta.z);

	int currentIndex = (row * nDepths * nCols) + (column * nDepths) + depth;
	
	float density = .25f * Noise(float3(4.03f * x / scale, 4.03f * z / scale, 4.03f * y / scale));
	density += .5f * Noise(float3(1.96f * x / scale, 1.96f * z / scale, 1.96f * y / scale));
	density += Noise(float3(1.01f * x / scale, 1.01f * z / scale, 1.01f * y / scale));
	density = ((float)(y / 4.f)) - floor(density);

	Vertex v = InitVertex();;
	v.density = density;
	v.pos = float3(x, y, z);
	v.normal = float3(column, row, depth);
	tempVertexBuffer[currentIndex] = v;
	tempVertexBuffer[currentIndex].density = density;
	float temp = 0;
	VertexBufferOutput[currentIndex].position = float3(x, y, z);
}


//

[numthreads(X-1, Y - 1, 1)]
void GenerateFinalVertices(uint3 DTid : SV_DispatchThreadID)
{
	uint nDepths = Z, nRows = Y, nCols = X;

	float row = DTid.y;
	float column = DTid.x;
	float depth = DTid.z;
	float3 delta = float3(.5, .5, .5);
	int currentIndex = (row * nDepths * nCols) + (column * nDepths) + depth;

	int index1 = (row * nDepths * nCols) + (column * nDepths) + (depth + 1);
	int index2 = (row * nDepths * nCols) + (column * nDepths) + depth + nDepths;
	int index3 = (row * nDepths * nCols) + (column * nDepths) + depth + 1 + nDepths;
	int index4 = ((row + 1) * nDepths * nCols) + (column * nDepths) + depth;
	int index5 = ((row + 1) * nDepths * nCols) + (column * nDepths) + depth + 1;
	int index6 = ((row + 1) * nDepths * nCols) + ((column + 1) * nDepths) + depth;
	int index7 = ((row + 1) * nDepths * nCols) + ((column + 1) * nDepths) + depth + 1;
	Vertex vert = InitVertex();

	const float density = tempVertexBuffer[currentIndex].density;
	const float density1 = tempVertexBuffer[index1].density;
	const float density2 = tempVertexBuffer[index2].density;
	const float density3 = tempVertexBuffer[index3].density;
	const float density4 = tempVertexBuffer[index4].density;
	const float density5 = tempVertexBuffer[index5].density;
	const float density6 = tempVertexBuffer[index6].density;
	const float density7 = tempVertexBuffer[index7].density;

	int4 bin;
	int4  bin2;
	// if  sign negative then it is 1(1) else false
	bin[0] = (sign(density) >= 0) ? 0 : 1;
	bin[1] = (sign(density1) >= 0) ? 0 : 1;
	bin[2] = (sign(density2) >= 0) ? 0 : 1;
	bin[3] = (sign(density3) >= 0) ? 0 : 1;
	bin2[0] = (sign(density4) >= 0) ? 0 : 1;
	bin2[1] = (sign(density5) >= 0) ? 0 : 1;
	bin2[2] = (sign(density6) >= 0) ? 0 : 1;
	bin2[3] = (sign(density7) >= 0) ? 0 : 1;



	if (all(bin) && all(bin2))
	{
	}
	else if (!any(bin) && !any(bin2))
	{
	}
	else
	{

		float3 v0 = tempVertexBuffer[currentIndex].pos;
		float3 v1 = tempVertexBuffer[index1].pos;
		float3 v2 = tempVertexBuffer[index2].pos;
		float3 v3 = tempVertexBuffer[index3].pos;
		float3 v4 = tempVertexBuffer[index4].pos;
		float3 v5 = tempVertexBuffer[index5].pos;
		float3 v6 = tempVertexBuffer[index6].pos;
		float3 v7 = tempVertexBuffer[index7].pos;


		float3 center = v0 + v1;
		center = center + v2;
		center = center + v3;
		center = center + v4;
		center = center + v5;
		center = center + v6;
		center = center + v7;
		center = center * .125f;
		vert.pos = center;
		vert.density = tempVertexBuffer[currentIndex].density;
		if (bin2[2] == bin2[3])
		{
		}
		else
		{
			tempVertexBuffer[index6].pos = center + float3(delta.x, delta.y, 0);
			tempVertexBuffer[index6].isVertexCreated = 1;
			tempVertexBuffer[index2].pos = center + float3(delta.x, 0, 0);
			tempVertexBuffer[index2].isVertexCreated = 1;
			tempVertexBuffer[index4].pos = center + float3(0, delta.y, 0);
			tempVertexBuffer[index4].isVertexCreated = 1;
			vert.signedEdges[10] = 1;
			vert.isVertexCreated = 1;
			if (bin2[2] == 1)
			{
				vert.edgenormal[0] = 1;
			}
			tempVertexBuffer[currentIndex] = vert;
		}
		if (bin2[1] == bin2[3])
		{
		}
		else
		{
			tempVertexBuffer[index1].pos = center + float3(0, 0, delta.z);
			tempVertexBuffer[index1].isVertexCreated = 1;
			tempVertexBuffer[index5].pos = center + float3(0, delta.y, delta.z);
			tempVertexBuffer[index5].isVertexCreated = 1;
			tempVertexBuffer[index4].pos = center + float3(0, delta.y, 0);
			tempVertexBuffer[index4].isVertexCreated = 1;
			vert.signedEdges[1] = 1;
			vert.isVertexCreated = 1;
			if (bin2[1] == 1) // if bin is negative 
			{
				vert.edgenormal[1] = 1;
			}
			tempVertexBuffer[currentIndex] = vert;
		}
		if (bin[3] == bin2[3])
		{
		}
		else
		{
			tempVertexBuffer[index1].pos = center + float3(0, 0, delta.z);
			tempVertexBuffer[index1].isVertexCreated = 1;
			tempVertexBuffer[index2].pos = center + float3(delta.x, 0, 0);
			tempVertexBuffer[index2].isVertexCreated = 1;
			tempVertexBuffer[index3].pos = center + float3(delta.x, 0, delta.z);
			tempVertexBuffer[index3].isVertexCreated = 1;
			vert.signedEdges[2] = 1;
			vert.isVertexCreated = 1;
			if (bin[3] == 1)
			{
				vert.edgenormal[2] = 1;
			}
			tempVertexBuffer[currentIndex] = vert;
		}
	}

	int count;

	float3 pos = 0;
	if (vert.isVertexCreated)
	{

		int count = 0;
		if (bin2[3] == bin2[2])
		{
		}
		else
		{
			GetPositionOnEdge(density6, density7, index6, index7, pos);
			count += 1;
		}
		if (bin2[3] == bin2[1])
		{
		}
		else
		{
			GetPositionOnEdge(density5, density7, index5, index7, pos);
			count += 1;
		}
		if (bin2[3] == bin[3])
		{
		}
		else
		{
			GetPositionOnEdge(density3, density7, index3, index7, pos);
			count += 1;
		}
		if (bin[0] == bin[1])
		{
		}
		else
		{
			GetPositionOnEdge(density, density1, currentIndex, index1, pos);
			count += 1;
		}
		if (bin[0] == bin[2])
		{
		}
		else
		{
			GetPositionOnEdge(density, density2, currentIndex, index2, pos);
			count += 1;
		}
		if (bin[0] == bin2[0])
		{
		}
		else
		{
			GetPositionOnEdge(density, density4, currentIndex, index4, pos);
			count += 1;
		}
		if (bin[2] == bin[3])
		{
		}
		else
		{
			GetPositionOnEdge(density2, density3, index2, index3, pos);
			count += 1;
		}
		if (bin[2] == bin2[2])
		{
		}
		else
		{
			GetPositionOnEdge(density2, density6, index2, index6, pos);
			count += 1;
		}
		if (bin[1] == bin[3])
		{
		}
		else
		{
			GetPositionOnEdge(density1, density3, index1, index3, pos);
			count += 1;
		}
		if (bin[1] == bin2[1])
		{
		}
		else
		{
			GetPositionOnEdge(density1, density5, index1, index5, pos);
			count += 1;
		}
		if (bin2[0] == bin2[1])
		{
		}
		else
		{
			GetPositionOnEdge(density4, density5, index4, index5, pos);
			count += 1;
		}
		if (bin2[0] == bin2[2])
		{
		}
		else
		{
			GetPositionOnEdge(density4, density6, index4, index6, pos);
			count += 1;
		}
		float avg = (float)1 / count;
		pos = pos * avg;
		tempVertexBuffer[currentIndex].pos = pos;
	}


}

[numthreads(X - 1, Y - 1, 1)]
void SmoothenTerrain(uint3 DTid : SV_DispatchThreadID)
{

	uint nDepths = Z, nRows = Y, nCols = X;
	float row = DTid.y;
	float column = DTid.x;
	float depth = DTid.z;
	int currentIndex = (row * nDepths * nCols) + (column * nDepths) + depth;
	int index1 = (row * nDepths * nCols) + (column * nDepths) + (depth + 1);
	int index2 = (row * nDepths * nCols) + (column * nDepths) + depth + nDepths;
	int index3 = (row * nDepths * nCols) + (column * nDepths) + depth + 1 + nDepths;
	int index4 = ((row + 1) * nDepths * nCols) + (column * nDepths) + depth;
	int index5 = ((row + 1) * nDepths * nCols) + (column * nDepths) + depth + 1;
	int index6 = ((row + 1) * nDepths * nCols) + ((column + 1) * nDepths) + depth;
	int index7 = ((row + 1) * nDepths * nCols) + ((column + 1) * nDepths) + depth + 1;
	Vertex vert = tempVertexBuffer[currentIndex];




	float3 v0 = tempVertexBuffer[currentIndex].pos;
	float3 v1 = tempVertexBuffer[index1].pos;
	float3 v2 = tempVertexBuffer[index2].pos;
	float3 v3 = tempVertexBuffer[index3].pos;
	float3 v4 = tempVertexBuffer[index4].pos;
	float3 v5 = tempVertexBuffer[index5].pos;
	float3 v6 = tempVertexBuffer[index6].pos;
	float3 v7 = tempVertexBuffer[index7].pos;

	int counter = 0;

	if (vert.isVertexCreated == 1)
	{
		int count = 0;
		if (vert.signedEdges[1] == 1)
		{
			float3 N = cross(v0 - v1, v0 - v4);
			ForceNormalToPointToAir(vert, N);
			float3 N1 = cross(v1 - v0, v1 - v5);
			ForceNormalToPointToAir(vert, N1);
			float3 N5 = cross(v5 - v4, v5 - v1);
			ForceNormalToPointToAir(vert, N5);
			float3 N4 = cross(v4 - v0, v4 - v5);
			ForceNormalToPointToAir(vert, N4);

			VertexBufferOutput[currentIndex].normal += N;
			VertexBufferOutput[index1].normal += N;
			VertexBufferOutput[index4].normal += N;
			VertexBufferOutput[index5].normal += N5;
			VertexBufferOutput[index4].normal += N5;
			VertexBufferOutput[index1].normal += N5;
			if (sign(N.x) > 0)
			{


				IndexBufferOutput.Append(uint3(index1, index5, index4));
				IndexBufferOutput.Append(uint3(index1, index4, currentIndex));



			}
			else
			{

				IndexBufferOutput.Append(uint3(currentIndex, index4, index5));
				IndexBufferOutput.Append(uint3(currentIndex, index5, index1));

			
			}
		}
		if (vert.signedEdges[10] == 1)
		{
			float3 N = cross(v0 - v4, v0 - v2);
			ForceNormalToPointToAir(vert, N);
			float3 N2 = cross(v2 - v0, v2 - v6);
			ForceNormalToPointToAir(vert, N2);
			float3 N6 = cross(v6 - v4, v6 - v2);
			ForceNormalToPointToAir(vert, N6);
			float3 N4 = cross(v4 - v0, v4 - v6);
			ForceNormalToPointToAir(vert, N4);

			{

				VertexBufferOutput[currentIndex].normal += N;
				VertexBufferOutput[index4].normal += N;
				VertexBufferOutput[index2].normal += N;
				VertexBufferOutput[index6].normal += N6;
				VertexBufferOutput[index4].normal += N6;
				VertexBufferOutput[index2].normal += N6;
			}
			if (sign(N.z) > 0)
			{
				IndexBufferOutput.Append(uint3(index2, currentIndex, index4));
				IndexBufferOutput.Append(uint3(index2, index4, index6));

			
			}
			else
			{
				IndexBufferOutput.Append(uint3(index4, currentIndex, index2));
				IndexBufferOutput.Append(uint3(index4, index2, index6));

		
			}
		}
		if (vert.signedEdges[2] == 1)
		{
			float3 N = cross(v0 - v1, v0 - v2);
			ForceNormalToPointToAir(vert, N);
			float3 N1 = cross(v1 - v0, v1 - v3);
			ForceNormalToPointToAir(vert, N1);
			float3 N2 = cross(v2 - v0, v2 - v3);
			ForceNormalToPointToAir(vert, N2);
			float3 N3 = cross(v3 - v1, v3 - v2);
			ForceNormalToPointToAir(vert, N3);
			{
				VertexBufferOutput[index1].normal += N1;
				VertexBufferOutput[currentIndex].normal += N1;
				VertexBufferOutput[index3].normal += N1;
				VertexBufferOutput[index2].normal += N2;
				VertexBufferOutput[currentIndex].normal += N2;
				VertexBufferOutput[index3].normal += N2;
			}
			if (sign(N.y) > 0)
			{

				IndexBufferOutput.Append(uint3(currentIndex, index2, index3));
				IndexBufferOutput.Append(uint3(currentIndex, index3, index1));


			}
			else
			{
				IndexBufferOutput.Append(uint3(currentIndex, index1, index3));
				IndexBufferOutput.Append(uint3(currentIndex, index3, index2));
			
			}
		}
	}


	VertexBufferOutput[currentIndex].position = tempVertexBuffer[currentIndex].pos;
}







