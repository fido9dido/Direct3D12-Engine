#include "CCommandListPool.h"
#include "Utility.h"

void CFences::Init(ID3D12Device* d3dDevice)
{
    ID3D12Fence* fence;
    ThrowIfFailed(d3dDevice->CreateFence(CurrentValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    Fence = unique_ptr<ID3D12Fence>(fence);
    WaitForSingleObject(FenceHandle, INFINITE);
    FenceHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    FenceEvent.Attach(CreateEventExW(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
    if (FenceHandle == nullptr)
    {
        throw std::exception("CreateEvent");
    }
    if (!FenceEvent.IsValid())
    {
        throw std::exception("CreateEvent");
    }
}

CFences::~CFences()
{
    Fence->Release();
    CloseHandle(FenceHandle);
}

void CFences::WaitForGPU()
{
    const UINT64 fence = CurrentValue;
   
    ++CurrentValue;
    const UINT64 fencex = Fence->GetCompletedValue();
    // Wait until the fence has been processed.
   
    if (Fence->GetCompletedValue() < CurrentValue)
    {
        ThrowIfFailed(Fence->SetEventOnCompletion(fence, FenceHandle));
       
        WaitForSingleObject(FenceHandle, INFINITE);
    }

}

CCommandListPool::CCommandListPool(ID3D12Device* device, ID3D12CommandQueue* commandQueue) :
    DeviceRef(device),
    CommandQueueRef(commandQueue)
{
    std::lock_guard<std::mutex> lockGuard(m);
    Fence.Init(DeviceRef); 

}

CCommandList CCommandListPool::Get()
{
    std::lock_guard<std::mutex> lockGuard(m);
    if (CommandListPool.empty())
    {
        CreateCommandList();
    }
    CCommandList commandList = std::move(CommandListPool.front());
    CommandListPool.pop();

    return commandList;
}

CCommandList CCommandListPool::GetAndReset(ID3D12PipelineState* pso)
{
    std::lock_guard<std::mutex> lockGuard(m);
    if (CommandListPool.empty())
    {
        CreateCommandList();
    }
    CCommandList commandList = std::move(CommandListPool.front());
    CommandListPool.pop();
    WaitforGPU();
    commandList.Reset(pso);
    return commandList;
}

void CCommandListPool::ReturnCommandList(CCommandList& commandList)
{
    std::lock_guard<std::mutex> lockGuard(m);
    
    CommandListPool.emplace(std::move(commandList));
}

void CCommandListPool::WaitforGPU()
{
    UINT64 currentValue = Fence.GetCurrentValue();
    // Schedule a Signal command in the queue.
    CommandQueueRef->Signal(Fence.Get(), currentValue);
   
    Fence.WaitForGPU();
}

void CCommandListPool::Execute(CCommandList& commandList)
{
    std::lock_guard<std::mutex> lockGuard(m);

    ThrowIfFailed(commandList.Get()->Close());
    ID3D12CommandList* cmdsLists[] = { commandList.Get() };
    CommandQueueRef->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
    WaitforGPU();
}

void CCommandListPool::CreateCommandList()
{
    CCommandList commandList(*this, DeviceRef);
    commandList.CreateCommandList();
    CommandListPool.emplace(std::move(commandList));
}