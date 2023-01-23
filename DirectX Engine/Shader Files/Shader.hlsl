
//***************************************************************************************
// Shader.hlsl by Frank Luna (C) 2015 All Rights Reserved.
// Edit Mohamed Ali
// Transforms and colors geometry.
//***************************************************************************************

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

#include "LightingShader.hlsl"


struct SubstanceData
{
	float4x4 subTransform;
	uint hMaterial; 
	uint gObjPad2;
	uint gObjPad3;
	uint gObjPad4;
	uint hTextures[8];
};

struct MaterialData
{
	float4 diffuseAlbedo;
	float3 fresnelR0;
	float roughness;
};

Texture2D gDiffuseMap[]: register(t0);
StructuredBuffer<SubstanceData> gSubstanceData : register(t0, space1);
StructuredBuffer<MaterialData> gMaterialData : register(t0, space5);

SamplerState gPointWrap        : register(s0);
SamplerState gPointClamp       : register(s1);
SamplerState gLinearWrap       : register(s2);
SamplerState gLinearClamp      : register(s3);
SamplerState gAnisotropicWrap  : register(s4);
SamplerState gAnisotropicClamp : register(s5);
SamplerState gShadow		   : register(s6);



cbuffer cbPass : register(b0)
{
	float4x4 gViewProj;
	float4 gAmbientLight;
	float3 gEyePosW;//camera position
	float cbPerObjectPad1;
	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Light gLights[MaxLights];
}
cbuffer cbPerObject : register(b1)
{
	float4x4 gWorld;
	float4x4 gTexTransform;//to transform the input texture coordinates:
	uint  gSubstanceIndex;
	uint hTextures;
	uint gObjPad1;
	uint gObjPad2;
};


struct VertexIn
{
	float3 posL  : POSITION;
	//local
	float3 normalL : NORMAL;
	float2 texCoord : TEXCOORD;
};

struct VertexOut
{
	float4 posH		: SV_POSITION;
	float3 posW		: POSITION;
	float3 normalW	: NORMAL;
	float2 texCoord : TEXCOORD;
	float3 normalL : NORMAL2;
	float3 posL  : POSITIONLOCAL;
};

VertexOut mainVS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	SubstanceData substanceData = gSubstanceData[gSubstanceIndex];
	// Transform to world space.
	float4 posW = mul(float4(vin.posL, 1.0f), gWorld);
	vout.posW = posW.xyz;
	vout.posL = vin.posL;

	// Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
	vout.normalW = mul(vin.normalL, (float3x3)gWorld);
	vout.normalL = vin.normalL;
	// Transform to homogeneous clip space.
	vout.posH = mul(posW, gViewProj);

	// Output vertex attributes for interpolation across triangle.
	float4 texC = mul(float4(vin.texCoord, 0.0f, 1.0f), gTexTransform);
	vout.texCoord = mul(texC, substanceData.subTransform).xy;
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	
	// Interpolating normal can unnormalize it, so renormalize it.
	pin.normalW = normalize(pin.normalW);
	SubstanceData substanceData = gSubstanceData[gSubstanceIndex];
	MaterialData materialData = gMaterialData[substanceData.hMaterial];
   
	float3 blend_weights = abs(pin.normalW.xyz);
	blend_weights = (blend_weights - 0.2) * 7;
	blend_weights = max(blend_weights, 0);
	blend_weights = blend_weights / (blend_weights.x + blend_weights.y + blend_weights.z).xxx;
	
	float scale = 1;
	//// Triplanar uvs
	float2 uvX = (pin.posL.yz) * scale; // x facing plane
	float2 uvY = (pin.posL.zx) * scale; // y facing plane
	float2 uvZ = (pin.posL.xy) * scale; // z facing plane

	uint albedo= substanceData.hTextures[0];

	uint nNormalMap= substanceData.hTextures[2];
	float3 BumpNormal;
	if(nNormalMap == 131072)
	{
		  BumpNormal =pin.normalW;
	}
	else {

		float2 bumpFetch1 = 2 * gDiffuseMap[nNormalMap].Sample(gAnisotropicWrap, uvX).xy - 1;
		float2 bumpFetch2 = 2 * gDiffuseMap[nNormalMap].Sample(gAnisotropicWrap, uvY).xy - 1;
		float2 bumpFetch3 = 2 * gDiffuseMap[nNormalMap].Sample(gAnisotropicWrap, uvZ).xy - 1;
		float3 bump1 = float3(0, bumpFetch1.x, bumpFetch1.y);
		float3 bump2 = float3(bumpFetch2.y, 0, bumpFetch2.x);
		float3 bump3 = float3(bumpFetch3.x, bumpFetch3.y, 0);
		float3 blended_bump;
		blended_bump = bump1.xyz * blend_weights.xxx + bump2.xyz * blend_weights.yyy + bump3.xyz * blend_weights.zzz;
		 BumpNormal = normalize(pin.normalW + blended_bump);
		
	}
	
	float4 col1 = gDiffuseMap[albedo].Sample(gAnisotropicWrap, uvX);
	float4 col2 = gDiffuseMap[albedo].Sample(gAnisotropicWrap, uvY);
	float4 col3 = gDiffuseMap[albedo].Sample(gAnisotropicWrap, uvZ);
	float4 blended_color = (col1.xyzw * blend_weights.xxxx) + (col2.xyzw * blend_weights.yyyy) + (col3.xyzw * blend_weights.zzzz);

	 float4 diffuseAlbedo = blended_color * materialData.diffuseAlbedo;
	 // Vector from point being lit to eye. 
	 float3 toEyeW = normalize(gEyePosW - pin.posW);
	
	 // Light terms.
	 float4 ambient = gAmbientLight * blended_color;

	 const float shininess = 1.0f - materialData.roughness;
	 Material mat = { diffuseAlbedo, materialData.fresnelR0, shininess };
	 float3 shadowFactor = 1.0f;
	 float4 directLight = ComputeLighting(gLights, mat, pin.posW,
			BumpNormal, toEyeW, shadowFactor);

	 float4 litColor = ambient + directLight;
	 // Common convention to take alpha from diffuse albedo. todo delete xs
	 litColor.a = diffuseAlbedo.a;
	 return litColor;

}













///////
//////https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics
//////
