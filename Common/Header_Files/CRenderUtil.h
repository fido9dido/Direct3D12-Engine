#ifndef CRENDERUTIL_H
#define CRENDERUTIL_H

#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <d3d12.h>
#include <wrl.h>

struct Vertex {
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TextureCoord;
};

inline Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const LPCWSTR srcFile, const  D3D_SHADER_MACRO* defines, const LPCSTR entryPoint, const LPCSTR target)
{
	UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint, target, compileFlags, 0, &shaderBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}

	return shaderBlob;
}

class CRenderUtil
{

};

#endif