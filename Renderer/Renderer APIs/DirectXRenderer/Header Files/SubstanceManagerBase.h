//***************************************************************************************
// SubstanceManagerBase.h by Mohamed Ali(C) 2021 All Rights Reserved.
//  
// Defines a base class for Managing Textures and Material.
// 
// Substance Is material with physical characteristics - Texture and Material
// 
// Substance Manager Handle The Creation of Texture and Materials and create Substance 
//***************************************************************************************

#ifndef TEXTUREMANAGERBASE_H
#define TEXTUREMANAGERBASE_H

#include "Texture.h"
#include "Material.h"
#include <array>
#include <unordered_map>
#include "MemoryUtil.h"

enum class TEXTURE_TYPE:int
{
	DiffuseAlbedo, HeightMap, NormalMap, Ambient, Emissive, Glossiness, Opacity, Specular,Count
};

 struct SubstanceData
{
	DirectX::XMFLOAT4X4 SubstanceTransform = MathHelper::Identity4x4();
	UINT hMaterial; // handle to material
	UINT gObjPad2;
	UINT gObjPad3;
	UINT gObjPad4;
	UINT hTextures[8];// handle to texture
};

struct Substance
{
	std::string Name;
	int SubstanceCBIndex;
	std::array<uint32_t, (int)TEXTURE_TYPE::Count> TextureList;
	DirectX::XMFLOAT4X4 SubstanceTransform;
	std::uint32_t MaterialIndex; 
	bool bAlpha;

	// Dirty flag indicating the substance has changed and we need to update the constant buffer.
	// Because we have a material constant buffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify a material we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

};
//TextureManager Optimize 
class SubstanceManagerBase
{
public:

	SubstanceManagerBase() = default;
	virtual ~SubstanceManagerBase() = default;
	virtual HRESULT AddSubstance(const std::wstring& name, unique_ptr<Material>& material) = 0;
	virtual HRESULT AddTexture(uint32_t SubstanceID, const std::wstring& name, ID3D12GraphicsCommandList* commandList, TEXTURE_TYPE type, bool bAlpha) = 0;
	virtual bool MaterialEqual(const Material* pMat0, const Material* pMat1) = 0;
	virtual UINT GetNumSubstances(void) = 0;
	virtual const Substance* GetSubstance(const UINT& SubstanceID) { return (SubstanceList[SubstanceID]) ? SubstanceList[SubstanceID].get() : nullptr; }
	virtual const Material* GetMaterial(const UINT& substance_ID) { return (SubstanceCount > substance_ID) ? MaterialList[SubstanceList[substance_ID]->MaterialIndex].get() : nullptr; }
	const std::vector<unique_ptr<Substance>>& GetAllSubstances() { return SubstanceList; };
	const std::vector<unique_ptr<Material>>& GetAllMaterial() { return MaterialList; };
	const UINT GetTextureCount() const { return TextureCount; };
	const std::vector<unique_ptr<Texture>>& GetAllTextures(void) const { return  TextureList; }

protected:
	UINT SubstanceCount; // Number of Substances
	UINT MaterialCount;// Number of Materials
	UINT TextureCount; // Number of Textures
	std::vector<unique_ptr<Substance>> SubstanceList;//Substance
	std::vector<unique_ptr<Material>> MaterialList;//Materials
	std::vector<unique_ptr<Texture>> TextureList;// Textures

};
#endif