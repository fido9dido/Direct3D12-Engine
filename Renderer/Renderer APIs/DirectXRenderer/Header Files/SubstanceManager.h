//***************************************************************************************
// SubstanceManager.h by Mohamed Ali(C) 2021 All Rights Reserved.
//  
// Defines a  class for Managing Textures and Material.
// 
// Substance Is Material with physical characteristics - Texture and Material
// 
// Substance Manager Handle The Creation of Texture and Materials and create Substance 
//***************************************************************************************

#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include "SubstanceManagerBase.h"

#define MAX_TEXTURES 131072
//TODO  add alpha blending
class SubstanceManager : public SubstanceManagerBase
{
private:

private:
	struct ID3D12Device* DeviceRef;
public:

	SubstanceManager(struct ID3D12Device* pDevice, FILE* pLog);
	SubstanceManager() = default;
	~SubstanceManager() = default;

	HRESULT AddSubstance(const std::wstring& name, unique_ptr<Material>& material);
	HRESULT AddMaterial(const Material& material);

	Material CreateMaterial(
		std::string name,
		float Roughness, 
		DirectX::XMVECTOR FresnelR0,
		DirectX::XMVECTOR DiffuseAlbedo = { 1.f, 1.f, 1.f, 1.0f },
		DirectX::XMVECTOR ambient = { 1.0f, 1.0f, 1.0f, 1.0f },
		DirectX::XMVECTOR specular = { 1.0f, 1.0f, 1.0f, 1.0f },
		DirectX::XMVECTOR emissive = { 1.0f, 1.0f, 1.0f, 1.0f });

	HRESULT AddTexture(uint32_t SubstanceID, const std::wstring& name, ID3D12GraphicsCommandList* commandList, TEXTURE_TYPE type, bool bAlpha);
	HRESULT CreateTexture(const std::wstring& name, unique_ptr<Texture>& texture, ID3D12GraphicsCommandList* commandList);
	UINT GetNumSubstances(void) { return SubstanceCount; }

	bool MaterialEqual(const Material* pMat0, const Material* pMat1);
	void BuildTextureSRVDescriptors(ID3D12DescriptorHeap* cbvSrvDescriptorHeapInOut, UINT cbvSrvDescriptorSize);

protected:
	void Log(char*, ...);

};
#endif