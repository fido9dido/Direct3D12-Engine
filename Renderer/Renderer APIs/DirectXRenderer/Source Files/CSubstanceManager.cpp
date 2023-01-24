#include "CSubstanceManager.h"
#include <CustomReturnValues.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include "Utility.h"
#include <d3dx12.h>

CSubstanceManager::CSubstanceManager(ID3D12Device* pDevice, FILE* pLog)
	:DeviceRef(pDevice)
{
}

HRESULT CSubstanceManager::AddSubstance(const std::wstring& name, unique_ptr<Material>& mat)
{
	bool bMatExists = false;
	std::uint32_t index;

	Substance substance;
	for (index = 0; index < MaterialCount; ++index) {

		if (MaterialEqual(mat.get(), MaterialList[index].get())) {
			bMatExists = true;
			break;
		}
	}

	substance.SubstanceTransform = MathHelper::Identity4x4();

	if (bMatExists)
	{
		substance.MaterialIndex = index;
	}
	else
	{
		substance.MaterialIndex = MaterialCount;
		mat->MatCBIndex = MaterialCount++;
		MaterialList.push_back(std::move(mat));
	}

	for (size_t i = 0; i < 8; i++)
	{
		substance.TextureList[i] = MAX_TEXTURES;
	}

	substance.bAlpha = false;
	substance.SubstanceCBIndex = SubstanceCount++;
	SubstanceList.push_back(make_unique<Substance>(substance));

	return DXR_OK;
}
HRESULT CSubstanceManager::AddMaterial(const Material& material)
{

	return E_NOTIMPL;
}

Material CSubstanceManager::CreateMaterial(
	std::string name,
	float roughness,
	DirectX::XMVECTOR fresnelR0,
	DirectX::XMVECTOR diffuseAlbedo,
	DirectX::XMVECTOR ambient,
	DirectX::XMVECTOR specular,
	DirectX::XMVECTOR emissive)
{
	Material matertial;
	matertial.Name = name;
	matertial.Roughness = roughness;
	matertial.FresnelR0 = fresnelR0,
	matertial.DiffuseAlbedo = diffuseAlbedo;
	matertial.ambient = ambient;
	matertial.specular = specular;
	matertial.emissive = emissive;
	return matertial;
}

HRESULT CSubstanceManager::AddTexture(
	std::uint32_t substanceID,
	const std::wstring& name,
	ID3D12GraphicsCommandList* commandList, TEXTURE_TYPE type,
	bool bAlpha)
{
	unique_ptr<Texture> texture = make_unique<Texture>();
	//check if ID exists
	if (substanceID >= SubstanceCount)
	{
		return DXR_INVALIDID;
	}
	//TO DO ADD POPUP WANNE REPLACE
	// Check if SKIN HAS TEXTURE 
	if (SubstanceList[substanceID]->TextureList[(int)type] != MAX_TEXTURES)
	{
		Log("error: AddTexture() failed, Texture Exists");
		return DXR_BUFFERSIZE;
	}
	std::uint32_t  texturesCount;
	bool bTexExists = false;
	// Check if texture exists
	for (texturesCount = 0; texturesCount < TextureCount; texturesCount++)
	{
		if (name.compare(TextureList[texturesCount]->Name) == 0) {
			bTexExists = true;
			break;
		}
	}
	//todo pop up do you want to replace
	if (!bTexExists)
	{
		if (bAlpha)
		{
			SubstanceList[substanceID]->bAlpha = true;
		}
		else
		{
			//texture->alpha = 1.0f;
		}

		texture->Name = name;
		CreateTexture(name, texture, commandList);

		// save ID and add to count
		texturesCount = TextureCount;
		TextureCount++;
		//TODO
	//ADD ALPHA BLENDING
		if (texture->bAlpha)
		{
		}
		TextureList.push_back(std::move(texture));
	}

	// put texture ID to skin ID
	SubstanceList[substanceID]->TextureList[(int)type] = texturesCount;

	return DXR_OK;
}

HRESULT CSubstanceManager::CreateTexture(const std::wstring& name, unique_ptr<Texture>& texture, ID3D12GraphicsCommandList* commandList)
{
	if (!DeviceRef) { return false; }
	std::wstring ext = GetFileExtensionToLower(name);
	if (ext == L"dds")
	{
		ThrowIfFailed(DirectX::LoadDDSTextureFromFile(DeviceRef, name.c_str(), texture->TextureHeap.ReleaseAndGetAddressOf(),
			texture->TextureDecodedData, texture->SubResourceList));
	}
	else
	{
		D3D12_SUBRESOURCE_DATA x;
		texture->SubResourceList.push_back(x);
		ThrowIfFailed(DirectX::LoadWICTextureFromFile(DeviceRef, name.c_str(), texture->TextureHeap.ReleaseAndGetAddressOf(),
			texture->TextureDecodedData, texture->SubResourceList.back()));
	}
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture->TextureHeap.Get(), 0, static_cast<UINT>(texture->SubResourceList.size()));
	ThrowIfFailed(DeviceRef->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(ID3D12Resource), &texture->TextureUploadHeap));
	UpdateSubresources(commandList, texture->TextureHeap.Get(), texture->TextureUploadHeap.Get(), 0, 0, static_cast<UINT>(texture->SubResourceList.size()), texture->SubResourceList.data());
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture->TextureHeap.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	return DXR_OK;
}

bool CSubstanceManager::MaterialEqual(const Material* pMat0, const Material* pMat1)
{
	if (!DirectX::XMVectorEqual(pMat0->ambient, pMat1->ambient).m128_f32[0] ||
		!DirectX::XMVectorEqual(pMat0->DiffuseAlbedo, pMat1->DiffuseAlbedo).m128_f32[0] ||
		!DirectX::XMVectorEqual(pMat0->emissive, pMat1->emissive).m128_f32[0] ||
		!DirectX::XMVectorEqual(pMat0->specular, pMat1->specular).m128_f32[0] ||
		!DirectX::XMVectorEqual(pMat0->FresnelR0, pMat1->FresnelR0).m128_f32[0] ||

		(pMat0->power != pMat1->power) ||
		(pMat0->roughness != pMat1->roughness))
	{
		return false;
	}
	return true;
}

void CSubstanceManager::BuildTextureSRVDescriptors(ID3D12DescriptorHeap* cbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize)
{
	
	// Fill out the heap with actual descriptors
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	D3D12_RESOURCE_DESC textureDesc;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(cbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	Microsoft::WRL::ComPtr<ID3D12Resource> texture;

	for (size_t i = 0; i < TextureCount; i++)
	{
		texture = TextureList[i]->TextureHeap;
		textureDesc = texture->GetDesc();
		srvDesc.Format = texture->GetDesc().Format;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
		DeviceRef->CreateShaderResourceView(texture.Get(), &srvDesc, hDescriptor);
		hDescriptor.Offset(1, cbvSrvDescriptorSize);
	}
}

void CSubstanceManager::Log(char*, ...)
{
}
