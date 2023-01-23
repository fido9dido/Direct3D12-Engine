#ifndef Texture_H
#define Texture_H

#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <memory>
#include "Utility.h"

struct Texture
{
	bool bAlpha;

	// Unique material name for lookup.
	std::wstring Name;
	std::vector<D3D12_SUBRESOURCE_DATA> SubResourceList;
	
	std::unique_ptr<uint8_t[]> TextureDecodedData;

	Microsoft::WRL::ComPtr<ID3D12Resource> TextureHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> TextureUploadHeap = nullptr;
	//LoadDDSTextureFromFile takes std pointer

};

#endif