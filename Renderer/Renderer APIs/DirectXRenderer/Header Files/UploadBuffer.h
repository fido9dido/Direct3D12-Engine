#ifndef UPLOADBUFFER_H
#define UPLOADBUFFER_H

#include "MemoryUtil.h"
#include <d3dx12.h>
#include "Utility.h"

template<typename T>
class UploadBuffer
{
public:
    UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) :
        bIsConstantBuffer(isConstantBuffer)
    {
        // Constant buffer elements need to be multiples of 256 bytes.
       // This is because the hardware can only view constant data 
       // at m*256 byte offsets and of n*256 byte lengths. 
       // typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
       // UINT64 OffsetInBytes; // multiple of 256
       // UINT   SizeInBytes;   // multiple of 256
       // } D3D12_CONSTANT_BUFFER_VIEW_DESC;
        if (isConstantBuffer)
        {
            ElementByteSize = CalcConstantBufferByteSize(sizeof(T));
        }
        else
        {
            ElementByteSize = sizeof(T);
        }

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(ElementByteSize * elementCount),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&Buffer)));

        ThrowIfFailed(Buffer->Map(0, nullptr, reinterpret_cast<void**>(&MappedData)));

        // We do not need to unmap until we are done with the resource.  However, we must not write to
        // the resource while it is in use by the GPU (so we must use synchronization techniques).
    }

    UploadBuffer(const UploadBuffer& rhs) = delete;
    UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
    ~UploadBuffer()
    {
        if (Buffer != nullptr)
            Buffer->Unmap(0, nullptr);

        MappedData = nullptr;
    }

    const Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() const
    {
        return Buffer.Get();
    }

    void CopyData(int elementIndex, const T& data)
    {
        memcpy(&MappedData[elementIndex * ElementByteSize], &data, sizeof(T));
    }

public:
    Microsoft::WRL::ComPtr<ID3D12Resource> Buffer;
    BYTE* MappedData = nullptr;

    UINT ElementByteSize = 0;
    bool bIsConstantBuffer = false;
};


inline void CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    UINT64 byteSize,
    unique_ptr<ID3D12Resource>& uploadBuffer,
    unique_ptr<ID3D12Resource>& defaultBuffer)
{

    // Create the actual default buffer resource.
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&defaultBuffer)));

    // In order to copy CPU memory data into our default buffer, we need to create
    // an intermediate upload heap. 
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)));


    // Describe the data we want to copy into the default buffer.
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    subResourceData.RowPitch = byteSize;
    subResourceData.SlicePitch = byteSize;

    // Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
    // will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
    // the intermediate upload heap data will be copied to mBuffer.
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
    UpdateSubresources<1>(cmdList, defaultBuffer.get(), uploadBuffer.get(), 0, 0, 1, &subResourceData);
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

}

#endif