///
//https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics
//
//cbuffer cbPerObject: register(b0)
//{
//	float4x4 gWorldViewProj;
//
//}
struct ObjectConstants
{
	float4x4 gWorldViewProj;
	uint matIndex;
};
ConstantBuffer<ObjectConstants> gObjConstants: register(b0);
struct VertexIn
{
	float3 PosL:POSITION;
	float4 Color:COLOR;
};
struct VertexOut
{
	//SV = system value PREFEX
	float4 PosH:SV_POSITION;
	float4 Color:COLOR;
};
cbuffer cbCamera : register(b1)
{
	float4x4 gView;              //
	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;          //
	float4x4 gInvViewProj;
	float3 gEyePosW;
	float gNearZ;
	float gFarZ;
	float gTotalTime;
	float gDeltaTime;

	float4x4		 
		m_mView2D,           // view matrix orthogonal
		//m_mView3D,           // view matrix perspective
		m_mProj2D,           // orthogonal projection (whole screen)
		m_mProjP[4],         // perspective projection
		m_mProjO[4],         // orthogonal projection
		m_mWorld,            // active world matrix
		//m_mViewProj,         // combo matrix for 3D
		m_mWorldViewProj;    // combo matrix for 3D
};
//void VS(VertexIn vInput)
//{
//	VertexOut vOutput;
//	//Transform to homogeneous clip space
//	vOutput.PosH = mul(float4(vInput.PosL, 1.f), gWorldViewProj);
//	// there's nothing to do for da color
//	vOutput.Color = vInput.Color;
//
//}
// vertex shader
VertexOut mainVS(VertexIn vInput/*float4 pos : POSITION*/ ) //: SV_POSITION
{
	VertexOut vOutput;
	//Transform to homogeneous clip space
	vOutput.PosH = mul(float4(vInput.PosL, 1.f), gObjConstants.gWorldViewProj);
	// there's nothing to do for da color
	vOutput.Color = vInput.Color;
	return vOutput;
}
//pixel shader
float4 PS(VertexOut vertexShader) : SV__Target
{
	return vertexShader.Color;
}