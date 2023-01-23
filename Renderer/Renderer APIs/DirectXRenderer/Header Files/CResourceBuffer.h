#ifndef CRESOURCEBUFFER_H
#define CRESOURCEBUFFER_H

#include <d3d12.h>
#include "MemoryUtil.h"
#include <mutex>
#include <array>
#include <d3dx12.h>
#include "Utility.h"

//WIP currently not in use
enum class ECShaderBufferType
{
	Constant,			
	TextureAndBuffer,
	Sampler,			
	UnorderedAccessView,
	Count
};

struct SBufferDescriptor
{
	UINT ElementByteSize = 0;
	UINT ElementCount = 0;
	D3D12_HEAP_TYPE HeapType = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;

	D3D12_HEAP_FLAGS HeapFlags = D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE;
	D3D12_RESOURCE_STATES ResourceState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ;
	D3D12_CLEAR_VALUE* OptimizedClearValue = nullptr;
};

class CBuffer	
{
private:
	unique_ptr<ID3D12Resource> Buffer;
	unique_ptr<BYTE> MappedData;
	SBufferDescriptor BufferDescription;
public:

	CBuffer() = default;

	~CBuffer()
	{
		Unmap();
	}

	ID3D12Resource* GetResource() const
	{
		return Buffer.get();
	}

	void Unmap()
	{
		if (Buffer.get() != nullptr)
		{
			Buffer->Unmap(0, nullptr);
		}
	}

	void Create(ID3D12Device* device, const SBufferDescriptor& bufferDesc)
	{
		BufferDescription = bufferDesc;

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(bufferDesc.HeapType),
			bufferDesc.HeapFlags,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferDesc.ElementByteSize * bufferDesc.ElementCount),
			bufferDesc.ResourceState,
			bufferDesc.OptimizedClearValue,
			IID_PPV_ARGS(&Buffer)));

		ThrowIfFailed(Buffer->Map(0, nullptr, reinterpret_cast<void**>(&MappedData)));

	}
};

class CBufferPool
{
private:
	ID3D12Device* DeviceRef;
public:

	CBufferPool(){}

	~CBufferPool() {}
	CBuffer* GetOrCreateBuffer(const SBufferDescriptor& desc);
	void ReturnBuffer(const CBuffer*& buffer);
	
	CBufferPool(const CBufferPool& other) = delete;
	CBufferPool& operator=(const CBufferPool& other) = delete;
};

template<typename T>
struct SConstantBufferDescription : SBufferDescriptor
{
	SConstantBufferDescription()
	{
		ElementByteSize = CalculateConstantBuffer(sizeof(T));
	}
	SConstantBufferDescription(UINT elementCount)
	{
		ElementByteSize = CalculateConstantBuffer(sizeof(T));
		ElementCount = elementCount;

	}
	virtual ~SConstantBufferDescription() {}

	UINT CalculateConstantBuffer(UINT byteSize)
	{ 
		// Constant buffers must be a multiple of the minimum hardware
	// allocation size (usually 256 bytes).  So round up to nearest
	// multiple of 256.  We do this by adding 255 and then masking off
	// the lower 2 bytes which store all bits < 256.
	// Example: Suppose byteSize = 300.
	// (300 + 255) & ~255
	// 555 & ~255
	// 0x022B & ~0x00ff
	// 0x022B & 0xff00
	// 0x0200
	// 512
		return (byteSize + 255) & ~255;
	}
	SConstantBufferDescription(const SConstantBufferDescription& other) = delete;
	SConstantBufferDescription& operator=(const SConstantBufferDescription& other) = delete;
};

enum class EBufferType
{
	IndexBuffer,
	VertexBuffer,
	ConstantBuffer,
	Count,
};
enum class EBufferUsage
{
	Static, //  resources that barely update
	Dynamic, // resources that Update frequently 
	Count,
};
class CBufferManager
{
private:
	std::mutex Mutex;
	std::array<std::array<SConstantBufferDescription<int>, (int)EBufferUsage::Count>, (int)EBufferType::Count> ConstantBufferPool;
public:
	CBufferManager();
	CBuffer CreateConstantBuffer();

};
#endif