#include "CSwapChain.h"
#include <dxgi1_6.h>
#include <d3dx12.h>
#include "Utility.h"

CSwapChain::CSwapChain(
	const HWND& windowHandle,
	const bool options,
	const bool allowTearing):
	WindowHandle(windowHandle), Options(options), bAllowTearing(allowTearing)
{
}

HRESULT CSwapChain::CreateSwapChain(
	IDXGIFactory7* dxgiFactory,
	ID3D12Device* d3dDevice,
	ID3D12CommandQueue* commandQueue,
	const DXGI_FORMAT& bufferFormat,
	const UINT& bufferCount,
	const UINT& bufferWidth,
	const UINT& bufferHeight)
{
	if (!d3dDevice || !dxgiFactory) { return false; }
	SwapChainDesc.Width = bufferWidth;
	SwapChainDesc.Height = bufferHeight;
	SwapChainDesc.Format = bufferFormat;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = bufferCount;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	SwapChainDesc.Flags = (Options && bAllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
	fsSwapChainDesc.Windowed = TRUE;
	// Create a swap chain for the window.
	ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
		commandQueue,
		WindowHandle,
		&SwapChainDesc,
		&fsSwapChainDesc,
		nullptr,
		(IDXGISwapChain1**)SwapChain.GetAddressOf()
	));
	// This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut
	ThrowIfFailed(dxgiFactory->MakeWindowAssociation(WindowHandle, DXGI_MWA_NO_ALT_ENTER));
	return true;
}

HRESULT CSwapChain::CreateSwapChain(
	IDXGIFactory7* dxgiFactory,
	ID3D12Device* d3dDevice,
	ID3D12CommandQueue* commandQueue,
	const HWND& windowHandle,
	const DXGI_FORMAT& bufferFormat,
	const UINT& bufferCount,
	const UINT& bufferWidth,
	const UINT& bufferHeight)
{
	if (!d3dDevice || !dxgiFactory) { return false; }
	WindowHandle = windowHandle;
	SwapChainDesc.Width = bufferWidth;
	SwapChainDesc.Height = bufferHeight;
	SwapChainDesc.Format = bufferFormat;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = bufferCount;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	SwapChainDesc.Flags = (Options && bAllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
	fsSwapChainDesc.Windowed = TRUE;
	// Create a swap chain for the window.
	ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
		commandQueue,
		WindowHandle,
		&SwapChainDesc,
		&fsSwapChainDesc,
		nullptr,
		(IDXGISwapChain1**)SwapChain.GetAddressOf()
	));
	// This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut
	ThrowIfFailed(dxgiFactory->MakeWindowAssociation(WindowHandle, DXGI_MWA_NO_ALT_ENTER));
	return true;
}

HRESULT CSwapChain::Resize(const UINT& BackBufferCount, const UINT& backBufferHeight, const UINT& backBufferWidth)
{
	return SwapChain->ResizeBuffers(BackBufferCount, backBufferWidth, backBufferHeight, SwapChainDesc.Format, (Options & bAllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);;
}
