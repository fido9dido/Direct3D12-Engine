#ifndef CCOMMANDLISTPOOL_H
#define CCOMMANDLISTPOOL_H

#include <queue>
#include <d3d12.h>
#include <mutex>
#include "Utility.h"
#include "MemoryUtil.h"
#include "CCommandList.h"

class CFences
{
private:
	
	unique_ptr<ID3D12Fence> Fence;
	
	UINT64 CurrentValue;
	HANDLE FenceHandle;
	Microsoft::WRL::Wrappers::Event FenceEvent;
public:
	~CFences();
	//bool IsDone(); 
	void Init(ID3D12Device* d3dDevice);
	ID3D12Fence* Get() { return Fence.get(); }
	const UINT64& GetCurrentValue() { return CurrentValue; }
	const HRESULT SetEventOnCompletion(UINT64 value) { return Fence->SetEventOnCompletion(value, FenceHandle); }
	const bool IsComplete() { return CurrentValue < Fence->GetCompletedValue(); }
	void WaitForGPU();
	//void AddFence(UINT64 id);
};

class CCommandListPool
{
	unique_ptr<ID3D12CommandAllocator> CommandAllocator;

	std::queue<CCommandList> CommandListPool;
	ID3D12CommandQueue* CommandQueueRef;
	ID3D12Device* DeviceRef;
	D3D12_COMMAND_LIST_TYPE Type;
	CFences Fence;

	std::mutex m;
public:
	CCommandListPool(ID3D12Device* d3dDevice, ID3D12CommandQueue* commandQueue);
	~CCommandListPool() = default;

	CCommandList Get();
	CCommandList GetAndReset(ID3D12PipelineState* pso);
	size_t Size() { return CommandListPool.size(); }
	const UINT64 GetFenceValue() { return Fence.GetCurrentValue(); }
	void ReturnCommandList(CCommandList& commandList);
	void Execute(CCommandList& commandList);
	void WaitforGPU();
	const bool IsFenceComplete() { return Fence.IsComplete(); }

private:
	void CreateCommandList();
};


#endif

