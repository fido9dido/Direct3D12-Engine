#ifndef CSWAPCHAIN_H
#define CSWAPCHAIN_H

#include <wrl.h>
#include <dxgi1_5.h>

class CSwapChain
{
	bool Options = true;
	bool bAllowTearing = true;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain;
	HWND WindowHandle;
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
public:
	IDXGISwapChain3* const * GetAddressOf() const { return SwapChain.GetAddressOf(); }
	IDXGISwapChain3* Get() { return SwapChain.Get(); }
	CSwapChain(
		const HWND& windowHandle,
		const bool options,
		const bool allowTearing);

	HRESULT CreateSwapChain(
		struct IDXGIFactory7* dxgiFactory,
		struct ID3D12Device* d3dDevice,
		struct ID3D12CommandQueue* commandQueue,
		const DXGI_FORMAT& bufferFormat,
		const UINT& bufferCount,
		const UINT& bufferWidth,
		const UINT& bufferHeight);

	HRESULT CSwapChain::CreateSwapChain(
		IDXGIFactory7* dxgiFactory,
		ID3D12Device* d3dDevice,
		ID3D12CommandQueue* commandQueue,
		const HWND& windowHandle,
		const DXGI_FORMAT& bufferFormat,
		const UINT& bufferCount,
		const UINT& bufferWidth,
		const UINT& bufferHeight);

	HRESULT Resize(const UINT& BackBufferCount, const UINT& backBufferHeight, const UINT& backBufferWidth);
};
#endif
