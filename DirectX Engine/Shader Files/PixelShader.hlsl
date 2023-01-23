//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
};

struct VertexIn
{
	float3 PosL  : POSITION;
	float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float4 Color : COLOR;
};

VertexOut mainVS(VertexIn vin)
{
	VertexOut vout;

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	// Just pass vertex color into the pixel shader.
	vout.Color = vin.Color;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return pin.Color;
}













///////
//////https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics
//////
//////cbuffer cbPerObject: register(b0)
//////{
//////	float4x4 gWorldViewProj;
//////
//////}
////Texture2D    gDiffuseMap : register(t0);
////SamplerState gsamLinear  : register(s0);
////
////struct ObjectConstants
////{
////	float4x4 gWorldViewProj;
////	uint matIndex;
////};
////ConstantBuffer<ObjectConstants> gObjConstants: register(b0);
////struct VertexIn
////{
////	float3 PosL    : POSITION;
////	float3 NormalL : NORMAL;
////	float2 TexC    : TEXCOORD;
////};
////struct VertexOut
////{
////	//SV = system value PREFEX
////	float4 PosH    : SV_POSITION;
////	float3 PosW    : POSITION;
////	float3 NormalW : NORMAL;
////	float2 TexC    : TEXCOORD;
////};
////cbuffer cbCamera : register(b1)
////{
////	float4x4 gView;              //
////	float4x4 gInvView;
////	float4x4 gProj;
////	float4x4 gInvProj;
////	float4x4 gViewProj;          //
////	float4x4 gInvViewProj;
////	float3 gEyePosW;
////	float gNearZ;
////	float gFarZ;
////	float gTotalTime;
////	float gDeltaTime;
////
////	float4x4		 
////		m_mView2D,           // view matrix orthogonal
////		//m_mView3D,           // view matrix perspective
////		m_mProj2D,           // orthogonal projection (whole screen)
////		m_mProjP[4],         // perspective projection
////		m_mProjO[4],         // orthogonal projection
////		m_mWorld,            // active world matrix
////		//m_mViewProj,         // combo matrix for 3D
////		m_mWorldViewProj;    // combo matrix for 3D
////};
//////void VS(VertexIn vInput)
//////{
//////	VertexOut vOutput;
//////	//Transform to homogeneous clip space
//////	vOutput.PosH = mul(float4(vInput.PosL, 1.f), gWorldViewProj);
//////	// there's nothing to do for da color
//////	vOutput.Color = vInput.Color;
//////
//////}
////// vertex shader
////VertexOut mainVS(VertexIn vInput/*float4 pos : POSITION*/ ) //: SV_POSITION
////{
////	VertexOut vOutput;
////	// Transform to world space.
////	float4 posW = mul(float4(vInput.PosL, 1.0f), gObjConstants.gWorldViewProj);
////	vOutput.PosW = posW.xyz;
////	
////	vOutput.PosH = posW;
////	//// Transform to homogeneous clip space.
////	//vOutput.PosH = mul(posW, gViewProj);
////
////	// Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
////	vOutput.NormalW = mul(vInput.NormalL, (float3x3) gObjConstants.gWorldViewProj);
////
////	
////	return vOutput;
////}
//////pixel shader
////float4 PS(VertexOut PSInput) : SV_Target
////{
////	    float4 diffuseAlbedo = gDiffuseMap.Sample(gsamLinear, PSInput.TexC);// * gDiffuseAlbedo;
////		// Light terms.
////	    float4 ambient = diffuseAlbedo;//gAmbientLight;// *diffuseAlbedo;
////	    float4 litColor = ambient;// + directLight;
////	    litColor.a = diffuseAlbedo.a;
////	return litColor;
////}