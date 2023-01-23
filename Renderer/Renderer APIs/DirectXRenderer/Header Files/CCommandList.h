#ifndef CCOMMANDLIST_H
#define CCOMMANDLIST_H

#include <d3d12.h>
#include "MemoryUtil.h"

class CCommandList
{
private:

	unique_ptr<ID3D12GraphicsCommandList> CommandList;
	unique_ptr<ID3D12CommandAllocator> CommandAllocator;
	ID3D12Device* DeviceRef;
	class CCommandListPool& PoolRef;
public:
	CCommandList(class CCommandListPool& pPool, ID3D12Device* const deviceRef);
	CCommandList(CCommandList&& other);
	~CCommandList() {}

	ID3D12GraphicsCommandList* Get() { return CommandList.get(); }

	HRESULT Reset(ID3D12PipelineState* pso);
	HRESULT ResetAllocator();
	void Reset();
	void CreateCommandList(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);

private:
	CCommandList() = delete;
	CCommandList(CCommandList&) = delete;
};

#endif

