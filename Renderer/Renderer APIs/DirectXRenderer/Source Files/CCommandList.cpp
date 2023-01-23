#include "CCommandList.h"
#include "CCommandListPool.h"
#include "d3d12.h"

CCommandList::CCommandList(CCommandListPool& pool, ID3D12Device* const deviceRef) :
    PoolRef(pool),
    DeviceRef(deviceRef)
{
}

CCommandList::CCommandList(CCommandList&& rhs) :
    CommandList(std::move(rhs.CommandList)),
    CommandAllocator(std::move(rhs.CommandAllocator)),
    DeviceRef(std::move(rhs.DeviceRef)),
    PoolRef(std::move(rhs.PoolRef))
{
}

void CCommandList::Reset()
{
    CommandList->Reset(CommandAllocator.get(), nullptr);
}

HRESULT CCommandList::Reset(ID3D12PipelineState* pso)
{

    return CommandList->Reset(CommandAllocator.get(), pso);
}

HRESULT CCommandList::ResetAllocator()
{

    return CommandAllocator->Reset();
}

void CCommandList::CreateCommandList(D3D12_COMMAND_LIST_TYPE type)
{
    if (!CommandAllocator.get())
    {
        if (S_OK != DeviceRef->CreateCommandAllocator(type, IID_PPV_ARGS (&CommandAllocator)))
        {

            return;
        }
    }

    ThrowIfFailed(DeviceRef->CreateCommandList(0, type, CommandAllocator.get(), nullptr, IID_PPV_ARGS(&CommandList)));

    CommandList->SetName(TEXT("CommandList %d", PoolRef.Size()));
    CommandList->Close();
}
