#ifndef MATERIAL_H
#define MATERIAL_H

#include "Utility.h"
#include <DirectXMath.h>

const int gNumFrameResources = 3;

#define MaxLights 16

enum class LIGHTTYPE :int
{
	SpotLight,
	PointLight,
	DirectionalLight
};
struct LIGHT
{
	DirectX::XMFLOAT3 diffuse;
	float range;
	DirectX::XMFLOAT3 specular;
	float falloff;
	DirectX::XMFLOAT3 ambient;
	const float attenuation0;//to calculate the way the light fades out as you get further from its position
	DirectX::XMFLOAT3 position;
	const float attenuation1;
	DirectX::XMFLOAT3 direction;
	const float attenuation2;
	LIGHTTYPE type;
	const float Theta;
	const float PI;
};

struct MATERIALDATA
{
	DirectX::XMFLOAT4 diffuseAlbedo;
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 specular;
	DirectX::XMFLOAT4 emissive;
	DirectX::XMFLOAT3 FresnelR0;
	float power;
	float roughness;
	
};
struct Light
{
	DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
	float FalloffStart = 1.0f;                          // point/spot light only
	DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
	float FalloffEnd = 10.0f;                           // point/spot light only
	DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
	float SpotPower = 64.0f;                            // spot light only
};


struct MaterialData
{// Used in texture mapping.
	DirectX::XMFLOAT4 diffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 fresnelR0  = { 0.01f, 0.01f, 0.01f };
	float roughness;// = 0.25f;
};

struct Material
{
	// Unique material name for lookup.
	std::string Name;

	// Index into constant buffer corresponding to this material.
	 int MatCBIndex;

	//// Index into SRV heap for diffuse texture. this is where texture is
	//int DiffuseSRVHeapIndex = -1;

	// Index into SRV heap for normal texture.
	//int NormalSRVHeapIndex = -1;

	// Dirty flag indicating the material has changed and we need to update the constant buffer.
	// Because we have a material constant buffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify a material we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Material constant buffer data used for shading.
	//diffuse reflection
	DirectX::XMVECTOR DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	// frensel effect R THETA
	DirectX::XMVECTOR FresnelR0 = { 0.01f, 0.01f, 0.01f,0 };

	//0 is smooth 1 is heightest roughness
	float Roughness = .25f;
	
	float power;
	float roughness;

	DirectX::XMVECTOR ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMVECTOR specular = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMVECTOR emissive = { 1.0f, 1.0f, 1.0f, 1.0f };
};

#endif