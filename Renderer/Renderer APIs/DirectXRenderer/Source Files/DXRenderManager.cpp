#include "DXRenderManager.h"
#include "CRenderUtil.h"
#include "CSwapChain.h"
#include "CCommandlist.h"
#include "MeshGeometry.h"
#include "Texture.h"
#include "CTerrain.h"
#include <future>
#include <d3dx12.h>
#include <dxgidebug.h>
#include <DirectXColors.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <RenderTargetState.h>
#include <CRenderItem.h>

HRESULT CreateRenderDevice(HINSTANCE hDLL, std::unique_ptr<IRenderDevice>* pDevice)
{
	if (pDevice && !*pDevice)
	{
		*pDevice = std::make_unique<DXRenderManager>(hDLL);
		return DXR_OK;
	}
	return DXR_FAIL;
}

HRESULT ShutdownRenderDevice(std::unique_ptr<IRenderDevice>* pDevice)
{
	if (!*pDevice)
	{
		return DXR_FAIL;
	}
	pDevice->reset();
	return DXR_OK;
}

DXRenderManager::DXRenderManager(HINSTANCE dllHandle,
	DXGI_FORMAT backBufferFormat,
	DXGI_FORMAT depthBufferFormat,
	D3D_FEATURE_LEVEL minFeatureLevel,
	unsigned int flags) :
	DLLHandle(dllHandle),
	BackBufferFormat(backBufferFormat),
	DepthBufferFormat(depthBufferFormat),
	d3dMinFeatureLevel(minFeatureLevel),
	d3dFeatureLevel(D3D_FEATURE_LEVEL_11_0),
	dxgiFactoryFlags(0),
	OutputScreenSize{ 0, 0, 1, 1 },
	ColorSpaceType(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709),
	Options(flags),
	CurrentFrameResourcesIndex(0),
	CurrentFrameResources(std::make_shared<FrameResource>()),
	RenderPool(bDone), DCTerrain(make_unique<CTerrain>()),
	MeshManager(make_unique<CMeshManager>())
{
	DirectoryPath = std::filesystem::current_path();

}

HRESULT DXRenderManager::Direct3DInit(const HWND& hWndMain, const std::vector<HWND>& hWnd3D,
	int width, int height, bool bSaveLog)
{
	bIsRunning = true;
	Width = width;
	Height = height;
	WindowHandleList = hWnd3D;
	d3dMinFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	CreateDevice();

	CommandListPool = make_unique<CCommandListPool>(Device.get(), CommandQueue.get());
	MeshManager->Init(Device.get(), CommandListPool.get());

	CreateSubstance();
	BuildRootSignature();
	ConfigureRenderThreads();
	CreateShaderAndInputLayout();
	CreatePipelineStateObject();
	CommandListPool->WaitforGPU();

	return DXR_OK;
}

void DXRenderManager::InitializeScene()
{
	CreateFrameResource();
	CreateResources();
	
	//Wait until initialization is complete.
	CommandListPool->WaitforGPU();
}

HRESULT DXRenderManager::UseWindow(UINT nHwnd)
{
	return E_NOTIMPL;
}

HRESULT DXRenderManager::BeginRendering()
{
	RenderPool.BeginRender();

	WaitForMultipleObjects(4, RenderPool.GetWorkerFinished().data(), true, INFINITE);
	HRESULT hr;
	for (UINT i = 0; i < WindowCount; i++)
	{
		if (SwapChainList.size() < i) { return false; }
		IDXGISwapChain3* currentSwapChain = SwapChainList[i]->Get();
		if (Options & bAllowTearing)
		{
			// Recommended to always use tearing if supported when using a sync interval of 0.
			// Note this will fail if in true 'fullscreen' mode.
			hr = currentSwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
		}
		else
		{
			// The first argument instructs DXGI to block until VSync, putting the application
			// to sleep until the next VSync. This ensures we don't waste any cycles rendering
			// frames that will never be displayed to the screen.
			hr = currentSwapChain->Present(1, 0);
		}
		// If the device was reset we must completely reinitialize the renderer.
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
#ifdef _DEBUG
			char buff[64] = {};
			sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n",
				static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? Device->GetDeviceRemovedReason() : hr));
			OutputDebugStringA(buff);
#endif
			OnDeviceLost();
		}
		else
		{
			ThrowIfFailed(hr);
			MoveToNextFrame(i);
			if (!DXGIFactory->IsCurrent())
			{
				IDXGIFactory7* dxgiFactory;
				// Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
				ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));
				DXGIFactory = unique_ptr<IDXGIFactory7>(dxgiFactory);
			}
		}
	}
	return DXR_OK;
}

void DXRenderManager::EndRendering()
{
}

HRESULT DXRenderManager::Clear(ID3D12GraphicsCommandList* commandList, int threadIndex)
{
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocators = CurrentFrameResources->CommandAllocators[threadIndex];
	ThrowIfFailed(commandAllocators->Reset());

	commandList->Reset(commandAllocators.Get(), PipelineStateMap["opaque"].Get());

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(RenderTargetList[threadIndex][BackBufferIndexList[threadIndex]].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	// Clear the views. current back buffer
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(
		RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		static_cast<INT>(BackBufferIndexList[threadIndex]), RtvDescriptorSize);

	rtvDescriptor.Offset(threadIndex * BackBufferCount, RtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvDescriptor(DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Specify the buffers we are going to render to.
	commandList->OMSetRenderTargets(1, &rtvDescriptor, true, &dsvDescriptor);
	// Clear the back buffer and depth buffer.
	// todo add clear color here
	commandList->ClearRenderTargetView(rtvDescriptor, DirectX::Colors::SkyBlue, 0, nullptr);
	commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList->RSSetViewports(1, &ScreenViewportList[threadIndex]);
	commandList->RSSetScissorRects(1, &ScissorRect);

	return DXR_FAIL;
}

void DXRenderManager::SetClearColor(float fRed, float fBlue, float FGreen)
{
}

void DXRenderManager::ShutDown()
{
}

bool DXRenderManager::IsRunning()
{
	return bIsRunning;
}

void DXRenderManager::UpdateSelectedDynamicActor(const DirectX::XMMATRIX& worldMatrix, int index)
{
	//temporarily Assumes each physics item contains a single actor
	if (index < RenderItemLayerList[(int)RenderLayer::OpaquePhysics].size() - 1) { return; }
	std::shared_ptr<CRenderItem>& selectedActor = RenderItemLayerList[(int)RenderLayer::OpaquePhysics][index];

	selectedActor->WorldMatrix = worldMatrix;
	selectedActor->FramesDirtyCount += 3;

}

HRESULT DXRenderManager::LogAdapterOutputs(const Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter)
{
	int i = 0;
	Microsoft::WRL::ComPtr<IDXGIOutput> pAdapterOutput;
	while (pAdapter->EnumOutputs(i, &pAdapterOutput) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		pAdapterOutput->GetDesc(&desc);
		std::wstring text = L"Output :";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());
		LogOutputDisplayModes(pAdapterOutput, DXGI_FORMAT_B8G8R8A8_UNORM);
		++i;
	}
	return DXR_OK;

}
HRESULT DXRenderManager::LogOutputDisplayModes(const Microsoft::WRL::ComPtr<IDXGIOutput> pAdapterOutput, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;
	// Call with nullptr to get list count.
	pAdapterOutput->GetDisplayModeList(format, flags, &count, nullptr);
	std::vector<DXGI_MODE_DESC> modeList(count);
	pAdapterOutput->GetDisplayModeList(format, flags, &count, &modeList[0]);
	for (DXGI_MODE_DESC& mode : modeList)
	{
		int n = mode.RefreshRate.Numerator;
		int d = mode.RefreshRate.Denominator;
		std::wstring text = L"Width = " + std::to_wstring(mode.Width) + L" "
			+
			L"Height = " + std::to_wstring(mode.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" +
			std::to_wstring(d) + L"\n";
		::OutputDebugStringW(text.c_str());
	}
	return DXR_OK;
}
HRESULT DXRenderManager::OnUpdate(float deltaTime, float keyboard, POINT mousePosition)
{
	// Cycle through the circular frame resource array.
	CurrentFrameResourcesIndex = (CurrentFrameResourcesIndex + 1) % FrameResourcesCount;
	CurrentFrameResources = FrameResourceList[CurrentFrameResourcesIndex];
	
	// Convert Spherical to Cartesian coordinates.
	float x = Radius * sinf(DirectX::XM_PI) * cosf(Theta);
	float z = Radius * sinf(DirectX::XM_PI) * sinf(Theta);
	float y = Radius * cosf(DirectX::XM_PI);

	UpdateObjectCB();
	UpdateMaterialCB();
	UpdateSubstanceCB();

	return DXR_OK;
}
void DXRenderManager::UpdateObjectCB()
{
	std::shared_ptr<UploadBuffer<ObjectConstants>> currentObjectBuffer = CurrentFrameResources->ObjectCB;
	for (std::shared_ptr<CRenderItem>& item : RenderitemList)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.*/
		if ((*item).FramesDirtyCount > 0)
		{
			const DirectX::XMMATRIX& worldMatrix = (*item).WorldMatrix;;
			const DirectX::XMMATRIX& textureTransform = (*item).TextureTransform;

			// Update the constant buffer with the latest worldViewProj matrix.
			ObjectConstants objConstants;
			// HLSL is column based
			XMStoreFloat4x4(&objConstants.WorldMatrix, DirectX::XMMatrixTranspose(worldMatrix));
			XMStoreFloat4x4(&objConstants.TextureTransform, DirectX::XMMatrixTranspose(textureTransform));
			objConstants.Textures = SubstanceManagerPtr->GetTextureCount();
			objConstants.SubstanceCBIndex = (*item).SubstanceCBIndex;
			currentObjectBuffer->CopyData((*item).ObjectCBIndex, objConstants);
			(*item).FramesDirtyCount--;
		}
	}
}
// camera index in frame resource
void DXRenderManager::UpdateCameraCB(const DirectX::XMMATRIX& view3D, const DirectX::XMMATRIX& ProjectionPerspective, const DirectX::XMVECTOR& position, int index)
{
	UploadBuffer<FrameConstants>* currentPassCB = CurrentFrameResources->FrameCB.get();
	if (!currentPassCB) { return; }
	// HLSL is column based
	FrameConstantBuffer.ViewProjection = DirectX::XMMatrixTranspose(view3D * ProjectionPerspective);
	DirectX::XMStoreFloat3(&FrameConstantBuffer.CameraPosition, position);

	//if prespective camera(index0) else orthogonal light
	FrameConstantBuffer.AmbientLight = (index == 0) ? DirectX::XMFLOAT4(0.75f, 0.75f, 0.85f, 1.0f) : DirectX::XMFLOAT4(0.25f, 0.25f, 0.35f, 1.0f);
	FrameConstantBuffer.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	FrameConstantBuffer.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
	FrameConstantBuffer.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	FrameConstantBuffer.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
	FrameConstantBuffer.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	FrameConstantBuffer.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	DirectX::XMVECTOR lightDir = DirectX::XMVectorScale(MathHelper::SphericalToCartesian(1.0f, mSunTheta, mSunPhi), 1);

	XMStoreFloat3(&FrameConstantBuffer.Lights[0].Direction, lightDir);
	FrameConstantBuffer.Lights[0].Strength = { 1.0f, 1.0f, 0.9f };
	currentPassCB->CopyData(index, FrameConstantBuffer);
}
void DXRenderManager::UpdateSubstanceCB()
{
	UploadBuffer<SubstanceData>* currentSubstanceCB = CurrentFrameResources->SubstanceCB.get();
	const std::vector<unique_ptr<Substance>>& substances = SubstanceManagerPtr->GetAllSubstances();
	for (const unique_ptr<Substance>& item : substances)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		if ((*item).NumFramesDirty > 0)
		{
			SubstanceData substanceData;
			substanceData.hMaterial = (*item).MaterialIndex;
			for (size_t i = 0; i < 8; i++)
			{
				substanceData.hTextures[i] = (*item).TextureList[i];
			}
			substanceData.gObjPad3 = (*item).TextureList[0];
			substanceData.gObjPad4 = (*item).TextureList[2];

			DirectX::XMMATRIX substanceTransform = DirectX::XMLoadFloat4x4(&(*item).SubstanceTransform);
			XMStoreFloat4x4(&substanceData.SubstanceTransform, XMMatrixTranspose(substanceTransform));

			currentSubstanceCB->CopyData((*item).SubstanceCBIndex, substanceData);

			(*item).NumFramesDirty--;
		}
	}
}
void DXRenderManager::UpdateMaterialCB()
{
	UploadBuffer<MaterialData>* currentMaterialCB = CurrentFrameResources->MaterialCB.get();
	const std::vector<unique_ptr<Material>>& materials = SubstanceManagerPtr->GetAllMaterial();
	for (const unique_ptr<Material>& currentMaterial : materials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		if (currentMaterial->NumFramesDirty > 0)
		{
			MaterialData materialConstants;
			DirectX::XMStoreFloat4(&materialConstants.diffuseAlbedo, currentMaterial->DiffuseAlbedo);
			DirectX::XMStoreFloat3(&materialConstants.fresnelR0, currentMaterial->FresnelR0);
			materialConstants.roughness = currentMaterial->Roughness;

			currentMaterialCB->CopyData(currentMaterial->MatCBIndex, materialConstants);

			currentMaterial->NumFramesDirty--;
		}
	}
}


void DXRenderManager::UpdateForResize(UINT clientWidth, UINT clientHeight)
{
	Width = clientWidth;
	Height = clientHeight;
	AspectRatio = static_cast<float>(clientWidth) / static_cast<float>(clientHeight);
}

bool DXRenderManager::CreateResources()
{
	if (!WindowHandleList[1])
	{
		throw std::exception("Call SetWindow with a valid Win32 window handle");
	}
	// Wait until all previous GPU work is complete.

	CommandListPool->WaitforGPU();

	const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	const DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;
	const UINT backBufferWidth = std::max<UINT>(static_cast<UINT>(OutputScreenSize.right - OutputScreenSize.left), 1u);
	const UINT backBufferHeight = std::max<UINT>(static_cast<UINT>(OutputScreenSize.bottom - OutputScreenSize.top), 1u);
	for (UINT i = 0; i < WindowCount; i++)
	{
		if (SwapChainList.size() < i) { return false; }
		if (!SwapChainList[i])
		{
			SwapChainList[i] = make_unique<CSwapChain>(WindowHandleList[i], Options, bAllowTearing);
			
		}
		CSwapChain* currentSwapChain = SwapChainList[i].get();
		// Release resources that are tied to the swap chain.
		for (UINT n = 0; n < BackBufferCount; n++)
		{
			RenderTargetList[i][n].Reset();
		}

		// If the swap chain already exists, resize it, otherwise create one.
		if (currentSwapChain->Get())
		{
			HRESULT hr = currentSwapChain->Resize(BackBufferCount, backBufferHeight, backBufferWidth);

			if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
			{
				// If the device was removed for any reason, a new device and swap chain will need to be created.
				OnDeviceLost();
				// Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method
				// and correctly set up the new device.
				return false;
			}
			else {
				ThrowIfFailed(hr);
			}
		}
		else
		{

			// Create a swap chain for the window.
			currentSwapChain->CreateSwapChain(
				DXGIFactory.get(),
				Device.get(),
				CommandQueue.get(),
				WindowHandleList[i],
				BackBufferFormat,
				BackBufferCount,
				backBufferWidth,
				backBufferHeight
			);

			// This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut
			ThrowIfFailed(DXGIFactory->MakeWindowAssociation(WindowHandleList[i], DXGI_MWA_NO_ALT_ENTER));
		}

		// Handle color space settings for HDR
		UpdateColorSpace(i);

		// Obtain the back buffers for this window which will be the final render targets
		// and create render target views for each of them.
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
		wchar_t name[25] = {};
		IDXGISwapChain3* dxgiSwapChain = currentSwapChain->Get();
		for (UINT n = 0; n < BackBufferCount; n++)
		{
			ThrowIfFailed(dxgiSwapChain->GetBuffer(n, __uuidof(ID3D12Resource), &RenderTargetList[i][n]));
			memset(name, 0, sizeof(name));
			swprintf_s(name, L"Render target %d %u", i, n);
			RenderTargetList[i][n]->SetName(name);

			rtvDesc = {};
			rtvDesc.Format = BackBufferFormat;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptorHeapHandle(
				RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
				static_cast<INT>(n + (i * BackBufferCount)), RtvDescriptorSize);
			Device->CreateRenderTargetView(RenderTargetList[i][n].Get(), &rtvDesc, rtvDescriptorHeapHandle);
		}

		// Reset the index to the current back buffer.
		BackBufferIndexList[i] = dxgiSwapChain->GetCurrentBackBufferIndex();
	}
	if (DepthBufferFormat != DXGI_FORMAT_UNKNOWN)
	{
		// Allocate a 2-D surface as the depth/stencil buffer and create a depth/stencil view
		// on this surface.
		CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

		D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			DepthBufferFormat,
			backBufferWidth,
			backBufferHeight,
			1, // This depth stencil view has only one texture.
			1  // Use a single mipmap level.
		);
		depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DepthBufferFormat;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;
		if (DepthStencil.get())
		{ 
			DepthStencil.reset(); 
		}
		ThrowIfFailed(Device->CreateCommittedResource(
			&depthHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&DepthStencil)

		));

		DepthStencil->SetName(L"Depth stencil");
		// Create descriptor to mip level 0 of entire resource using the format of the resource.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.Format = DepthBufferFormat;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		Device->CreateDepthStencilView(DepthStencil.get(), &dsvDesc, DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}
	for (size_t i = 0; i < WindowCount; i++)
	{
		// Set the 3D rendering viewport and scissor rectangle to target the entire window.
		ScreenViewportList[i].TopLeftX = ScreenViewportList[i].TopLeftY = 0.f;
		ScreenViewportList[i].Width = static_cast<float>(backBufferWidth);
		ScreenViewportList[i].Height = static_cast<float>(backBufferHeight);
		ScreenViewportList[i].MinDepth = D3D12_MIN_DEPTH;
		ScreenViewportList[i].MaxDepth = D3D12_MAX_DEPTH;
	}

	ScissorRect.left = ScissorRect.top = 0;
	ScissorRect.right = static_cast<LONG>(backBufferWidth);
	ScissorRect.bottom = static_cast<LONG>(backBufferHeight);
	return DXR_OK;
}

void DXRenderManager::CreateFrameResource()
{
	for (int nFrameResources = 0; nFrameResources < FrameResourcesCount; ++nFrameResources)
	{
		FrameResourceList[nFrameResources] = make_unique<FrameResource>(Device.get(), FrameConstBufferCount,
			(UINT)RenderitemList.size(), 0, (UINT)SubstanceManagerPtr->GetAllSubstances().size(), (UINT)SubstanceManagerPtr->GetAllMaterial().size(), 0/* m_pWaves->VertexCount()*/);
	}
}

HRESULT DXRenderManager::CreatePipelineStateObject()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { InputLayoutList.data(), (UINT)InputLayoutList.size() };
	psoDesc.pRootSignature = RootSignatureMap["MainRootSig"].Get();
	psoDesc.VS.pShaderBytecode = reinterpret_cast<UINT8*>(ShaderObjectMap["standardVS"]->GetBufferPointer());
	psoDesc.VS.BytecodeLength = ShaderObjectMap["standardVS"]->GetBufferSize();
	psoDesc.PS.pShaderBytecode = reinterpret_cast<UINT8*>(ShaderObjectMap["opaquePS"]->GetBufferPointer());
	psoDesc.PS.BytecodeLength = ShaderObjectMap["opaquePS"]->GetBufferSize();
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = BackBufferFormat;
	psoDesc.SampleDesc.Count = bIsMsaaEnabled ? 4 : 1;
	psoDesc.SampleDesc.Quality = bIsMsaaEnabled ? (MsaaQualityX4 - 1) : 0;
	psoDesc.DSVFormat = DepthBufferFormat;
	ThrowIfFailed(Device->CreateGraphicsPipelineState(&psoDesc, _uuidof(ID3D12PipelineState), &PipelineStateMap["opaque"]));
	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueline = psoDesc;
	opaqueline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	ThrowIfFailed(Device->CreateGraphicsPipelineState(&opaqueline, _uuidof(ID3D12PipelineState), &PipelineStateMap["opaqueLine"]));
	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC wireframePsoDesc = opaqueline;
	wireframePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	wireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(Device->CreateGraphicsPipelineState(&wireframePsoDesc, _uuidof(ID3D12PipelineState), &PipelineStateMap["wireframeLine"]));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC wireframeTriPsoDesc = psoDesc;
	wireframeTriPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	wireframeTriPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(Device->CreateGraphicsPipelineState(&wireframeTriPsoDesc, _uuidof(ID3D12PipelineState), &PipelineStateMap["wireframeTri"]));
	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc = psoDesc;
	// The camera is inside the sky sphere, so just turn off culling.
	skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	// Make sure the depth function is LESS_EQUAL and not just LESS.  
	// Otherwise, the normalized depth values at z = 1 (NDC) will 
	// fail the depth test if the depth buffer was cleared to 1.
	skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	ThrowIfFailed(Device->CreateGraphicsPipelineState(&skyPsoDesc, _uuidof(ID3D12PipelineState), &PipelineStateMap["sky"]));

	//ComputeShader PSO
	D3D12_COMPUTE_PIPELINE_STATE_DESC tempBufferPSODesc = {};
	tempBufferPSODesc.pRootSignature = RootSignatureMap["RootSigCS"].Get();
	tempBufferPSODesc.CS = {
		reinterpret_cast<BYTE*>(ShaderObjectMap["GenerateTempBufferCS"]->GetBufferPointer()),
		ShaderObjectMap["GenerateTempBufferCS"]->GetBufferSize()
	};
	tempBufferPSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(Device->CreateComputePipelineState(&tempBufferPSODesc,
		__uuidof(ID3D12PipelineState), &PipelineStateMap["TempBufferCSPSO"]));

	D3D12_COMPUTE_PIPELINE_STATE_DESC vbPSODesc = {};
	vbPSODesc.pRootSignature = RootSignatureMap["RootSigCS"].Get();
	vbPSODesc.CS = {
		reinterpret_cast<BYTE*>(ShaderObjectMap["GenerateFinalVertices"]->GetBufferPointer()),
		ShaderObjectMap["GenerateFinalVertices"]->GetBufferSize()
	};
	vbPSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(Device->CreateComputePipelineState(&vbPSODesc,
		__uuidof(ID3D12PipelineState), &PipelineStateMap["VBCSPSO"]));

	D3D12_COMPUTE_PIPELINE_STATE_DESC smoothenTerrainPSODesc = {};
	smoothenTerrainPSODesc.pRootSignature = RootSignatureMap["RootSigCS"].Get();
	smoothenTerrainPSODesc.CS = {
		reinterpret_cast<BYTE*>(ShaderObjectMap["SmoothenTerrain"]->GetBufferPointer()),
		ShaderObjectMap["SmoothenTerrain"]->GetBufferSize()
	};
	smoothenTerrainPSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(Device->CreateComputePipelineState(&smoothenTerrainPSODesc,
		__uuidof(ID3D12PipelineState), &PipelineStateMap["SmoothenTerrainCSPSO"]));

		return DXR_OK;
}

HRESULT DXRenderManager::Tick()
{
	return BeginRendering();
}

void DXRenderManager::CreateShaderAndInputLayout()
{
	std::wstring path = DirectoryPath.wstring();
	ShaderObjectMap["standardVS"] = CompileShader((path + L"\\Shader Files\\Shader.hlsl").c_str(), nullptr,
		"mainVS", "vs_5_1");
	ShaderObjectMap["opaquePS"] = CompileShader((path + L"\\Shader Files\\Shader.hlsl").c_str(), nullptr,
		"PS", "ps_5_1");

	ShaderObjectMap["GenerateTempBufferCS"] = CompileShader(
		(path + L"\\Shader Files\\ComputeShader.hlsl").c_str(), nullptr,
		"GenerateTempBuffer", "cs_5_1");
	ShaderObjectMap["GenerateFinalVertices"] = CompileShader(
		(path + L"\\Shader Files\\ComputeShader.hlsl").c_str(), nullptr,
		"GenerateFinalVertices", "cs_5_1");
	ShaderObjectMap["SmoothenTerrain"] = CompileShader(
		(path + L"\\Shader Files\\ComputeShader.hlsl").c_str(), nullptr,
		"SmoothenTerrain", "cs_5_1");

	InputLayoutList = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"TEXCOORD",0,DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

HRESULT DXRenderManager::CreateDevice(void)
{
#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
		Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
		{
			dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
			DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
			{
				80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
			};
			DXGI_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
		}
	}
#endif

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&DXGIFactory)));
	// Determines whether tearing support is available for fullscreen borderless windows.
	if (Options & bAllowTearing)
	{
		BOOL allowTearing = FALSE;

		HRESULT hr = DXGIFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));

		if (FAILED(hr) || !allowTearing)
		{
			hr = DXGIFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
		}
		if (FAILED(hr) || !allowTearing)
		{
			Options &= ~bAllowTearing;
#ifdef _DEBUG
			OutputDebugStringA("WARNING: Variable refresh rate displays not supported");
#endif
		}
	}
	GetAdapter(Adapter.GetAddressOf());
	LogAdapterOutputs(Adapter);

	ThrowIfFailed(D3D12CreateDevice(
		Adapter.Get(), // when null return the default 
		// also you can use GetHardwareAdapter here TODO: add setting to change that
		d3dMinFeatureLevel,
		IID_PPV_ARGS(&Device)));
	
	Device->SetName(L"MainDevice");
	

#ifndef NDEBUG
	// Configure debug device (if active).
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> d3dInfoQueue;
	;
	if (SUCCEEDED(Device->QueryInterface(__uuidof(ID3D12InfoQueue), &d3dInfoQueue)))
	{
#ifdef _DEBUG
		d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif
		D3D12_MESSAGE_ID hide[] =
		{
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
		  D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
		  D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE
		};
		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = _countof(hide);
		filter.DenyList.pIDList = hide;
		d3dInfoQueue->AddStorageFilterEntries(&filter);
	}
#endif

	// Determine maximum supported feature level for this device
	static const D3D_FEATURE_LEVEL s_featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels =
	{
		_countof(s_featureLevels), s_featureLevels, D3D_FEATURE_LEVEL_11_0
	};

	HRESULT hr = Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));
	if (SUCCEEDED(hr))
	{
		d3dFeatureLevel = featLevels.MaxSupportedFeatureLevel;
	}
	else
	{
		d3dFeatureLevel = d3dMinFeatureLevel;
	}
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = BackBufferFormat;
	msQualityLevels.SampleCount = 8;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(Device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));
	MsaaQualityX4 = msQualityLevels.NumQualityLevels;
	assert(MsaaQualityX4 > 0 && "Unexpected MSAA quality level.");
	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue)));
	CommandQueue->SetName(L"CommandQueue");

	// Describe and create a render target view (RTV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = BackBufferCount * WindowCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;;
	// Create the render target view heap for the back buffers.

	ThrowIfFailed(Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&RtvDescriptorHeap)));;

	RtvDescriptorHeap->SetName(L"rtv MainDevice");

	{
		D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
		dsvDescriptorHeapDesc.NumDescriptors = 1;;
		dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		ThrowIfFailed(Device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&DsvDescriptorHeap)));
		DsvDescriptorHeap->SetName(L"DSV DEsc MainDevice");
		DsvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CbvSrvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	if (bIsTearingSupport)
	{
		// When tearing support is enabled we will handle ALT+Enter key presses in the
		// window message loop rather than let DXGI handle it by calling SetFullscreenState.
		DXGIFactory->MakeWindowAssociation(WindowHandleList[1], DXGI_MWA_NO_ALT_ENTER);
	}
	//fill heap with descriptors
	bIsRunning = true;
	SubstanceManagerPtr = make_unique<SubstanceManager>(Device.get(), nullptr);

	return DXR_OK;
}

void DXRenderManager::CreateSubstance(void)
{
	std::wstring path = DirectoryPath.wstring();

	unique_ptr<Material> material = make_unique<Material>(
		SubstanceManagerPtr->CreateMaterial("rock", .2f, DirectX::XMVectorSet(0.05f, 0.05f, 0.05f, 0.f)));

	SubstanceManagerPtr->AddSubstance(L"terrain", material);

	CCommandList& commandList = CommandListPool->Get();
	commandList.Reset();

	SubstanceManagerPtr->AddTexture(0, path + L"\\Textures\\SnowCoveredRock_PbrGenericSpecular2kBitmap\\SnowCoveredRock_albedo.png",
		commandList.Get(), TEXTURE_TYPE::DiffuseAlbedo, false);
	SubstanceManagerPtr->AddTexture(0, path + L"\\Textures\\SnowCoveredRock_PbrGenericSpecular2kBitmap\\SnowCoveredRock_normal.png",
		commandList.Get(), TEXTURE_TYPE::NormalMap, false);

	material = make_unique<Material>(
		SubstanceManagerPtr->CreateMaterial("woodCrate", .2f, DirectX::XMVectorSet(0.15f, 0.15f, 0.15f, 0.f)));

	SubstanceManagerPtr->AddSubstance(L"woodCrate", material);
	SubstanceManagerPtr->AddTexture(1, path + L"\\Textures\\WoodCrate01.dds",
		commandList.Get(), TEXTURE_TYPE::DiffuseAlbedo, false);

	material = make_unique<Material>(
		SubstanceManagerPtr->CreateMaterial("lava", .1f, DirectX::XMVectorSet(0.01f, 0.01f, 0.01f, 0.f)));

	SubstanceManagerPtr->AddSubstance(L"lava", material);
	SubstanceManagerPtr->AddTexture(2, path + L"\\Textures\\HardeningLava_PbrGenericSpecular2kBitmap\\HardeningLava_albedo.png",
		commandList.Get(), TEXTURE_TYPE::DiffuseAlbedo, false);

	SubstanceManagerPtr->AddTexture(2, path + L"\\Textures\\HardeningLava_PbrGenericSpecular2kBitmap\\HardeningLava_normal.png",
		commandList.Get(), TEXTURE_TYPE::NormalMap, false);

	//// Describe and create a constant buffer view (CBV) and shader resource view (SRV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC cbvSRVHeapDesc = {};

	////number of texture resource 
	cbvSRVHeapDesc.NumDescriptors = SubstanceManagerPtr->GetTextureCount() + 3; //3 for compute shader
	cbvSRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvSRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvSRVHeapDesc.NodeMask = 0;

	ThrowIfFailed(Device->CreateDescriptorHeap(&cbvSRVHeapDesc, IID_PPV_ARGS(&CbvSrvDescriptorHeap)));

	CbvSrvDescriptorHeap->SetName(L" cbvSrv MainDevice");

	SubstanceManagerPtr->BuildTextureSRVDescriptors(CbvSrvDescriptorHeap.get(), CbvSrvDescriptorSize);

	CommandListPool->Execute(commandList);
	CommandListPool->ReturnCommandList(commandList);
}

void DXRenderManager::CreateShaderResourceView(std::vector<unique_ptr<Texture>>& mTextures, bool isCubeMap)
{
	auto length = mTextures.size();
	if (length <= 0)
	{
		ThrowIfFailed(DXR_FAIL);
	}
	else
	{
		D3D12_RESOURCE_DESC desc;
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(CbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		hDescriptor.Offset(-1, this->CbvSrvDescriptorSize);
		for (size_t i = 0; i < length; i++)
		{
			srvDesc = {};
			desc = {};
			hDescriptor.Offset(1, this->CbvSrvDescriptorSize);
			desc = mTextures[i]->TextureHeap->GetDesc();
			srvDesc.Format = desc.Format;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			switch (desc.Dimension)
			{
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				if (desc.DepthOrArraySize > 1)
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
					srvDesc.Texture1DArray.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;
					srvDesc.Texture1DArray.ArraySize = static_cast<UINT>(desc.DepthOrArraySize);
				}
				else
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
					srvDesc.Texture1D.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;
				}
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				if (isCubeMap)
				{
					if (desc.DepthOrArraySize > 6)
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
						srvDesc.TextureCubeArray.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;
						srvDesc.TextureCubeArray.NumCubes = static_cast<UINT>(desc.DepthOrArraySize / 6);
					}
					else
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
						srvDesc.TextureCube.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;
					}
				}
				else if (desc.DepthOrArraySize > 1)
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
					srvDesc.Texture2DArray.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;
					srvDesc.Texture2DArray.ArraySize = static_cast<UINT>(desc.DepthOrArraySize);
				}
				else
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Texture2D.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;
				}
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				srvDesc.Texture3D.MipLevels = (!desc.MipLevels) ? UINT(-1) : desc.MipLevels;
				break;

			case D3D12_RESOURCE_DIMENSION_BUFFER:
				throw std::invalid_argument("buffer resources not supported");
			case D3D12_RESOURCE_DIMENSION_UNKNOWN:
			default:
				throw std::invalid_argument("unknown resource dimension");
			}
			Device->CreateShaderResourceView(mTextures[i]->TextureHeap.Get(), &srvDesc, hDescriptor);
		}
	}
}

//swap chain windowed and one windows TODO CHANGE THIS
HRESULT DXRenderManager::LoadSwapChain(DXGI_SWAP_CHAIN_DESC& outDesc, int nMsaa)
{
	// 0 match windows size and desktop refresh rate
	outDesc.BufferDesc.Width = 0;
	outDesc.BufferDesc.Height = 0;

	outDesc.BufferDesc.RefreshRate.Numerator = 0;
	outDesc.BufferDesc.RefreshRate.Denominator = 1;

	outDesc.BufferDesc.Format = BackBufferFormat;
	//use defaults
	outDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	outDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	outDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//todo fix this  make one global variable through out the app
	outDesc.SampleDesc.Count = bIsMsaaEnabled ? nMsaa : 1;
	outDesc.SampleDesc.Quality = bIsMsaaEnabled ? (MsaaQualityX4 - 1) : 0;
	outDesc.BufferCount = BackBufferCount;
	outDesc.Windowed = TRUE;
	outDesc.OutputWindow = WindowHandleList[1];
	//metro apps use flip sequential or hardware select  swapeffect with da best performance  
	//ASSUME backbuffer not initialized after swap
	outDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	outDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	return DXR_OK;
}

void DXRenderManager::BuildMeshBuffer(MeshData& meshData)
{
	MeshManager->CreateMeshBuffer(meshData);

}

void DXRenderManager::RemoveRenderItem(std::string name)
{
}
void DXRenderManager::OnDeviceLost()
{
	// TODO: Perform Direct3D resource cleanup.
	for (size_t i = 0; i < WindowCount; i++)
	{
		for (UINT n = 0; n < BackBufferIndexList[i]; n++)
		{
			//CommandAllocators[i][n].Reset();
			RenderTargetList[i][n].Reset();
		}
		SwapChainList[i]->Get()->Release();
	}
	DepthStencil.reset();
	RtvDescriptorHeap.reset();
	DsvDescriptorHeap.reset();
	CommandQueue.reset();
	Device.reset();
	DXGIFactory.reset();
#ifdef _DEBUG
	{
		Microsoft::WRL::ComPtr<IDXGIDebug1> dxgiDebug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
		{
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}
	}
#endif
	CreateDevice();
	CreateResources();
}

// Prepare to render the next frame.
bool DXRenderManager::MoveToNextFrame(int i)
{
	if (SwapChainList.size() < i) { return false; }
	const UINT64 currentFenceValue = CommandListPool->GetFenceValue();
	IDXGISwapChain3* currentSwapChain = SwapChainList[i]->Get();
	// Schedule a Signal command in the queue.
	CommandListPool->WaitforGPU();
	
	// Update the back buffer index.
	BackBufferIndexList[i] = currentSwapChain->GetCurrentBackBufferIndex();
	
	// Advance the fence value to mark commands up to this fence point.
	CurrentFrameResources->FenceCount = currentFenceValue;
	return DXR_OK;
}

HRESULT DXRenderManager::OnResize(LONG width, LONG height, bool minimized)
{
	//main width and height
	Width = width;
	Height = height;
	RECT newRc;
	newRc.left = newRc.top = 0;
	newRc.right = width;
	newRc.bottom = height;
	if (newRc.left == OutputScreenSize.left
		&& newRc.top == OutputScreenSize.top
		&& newRc.right == OutputScreenSize.right
		&& newRc.bottom == OutputScreenSize.bottom)
	{
		return DXR_FAIL;
	}
	for (UINT i = 0; i < WindowCount; i++)
	{
		// Handle color space settings for HDR
		UpdateColorSpace(i);
	}
	OutputScreenSize = newRc;
	CreateResources();

	return DXR_OK;

}

bool DXRenderManager::UpdateColorSpace(int i)
{
	if (SwapChainList.size() < i) { return false; }
	IDXGISwapChain3* currentSwapChain = SwapChainList[i]->Get();
	DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

	bool isDisplayHDR10 = false;

#if defined(NTDDI_WIN10_RS2)
	if (currentSwapChain)
	{
		Microsoft::WRL::ComPtr<IDXGIOutput> output;
		if (SUCCEEDED(currentSwapChain->GetContainingOutput(output.GetAddressOf())))
		{
			Microsoft::WRL::ComPtr<IDXGIOutput6> output6;
			if (SUCCEEDED(output.As(&output6)))
			{
				DXGI_OUTPUT_DESC1 desc;
				ThrowIfFailed(output6->GetDesc1(&desc));

				if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
				{
					// Display output is HDR10.
					isDisplayHDR10 = true;
				}
			}
		}
	}
#endif

	if ((Options & EnableHDR) && isDisplayHDR10)
	{
		switch (BackBufferFormat)
		{
		case DXGI_FORMAT_R10G10B10A2_UNORM:
			// The application creates the HDR10 signal.
			colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
			break;

		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			// The system creates the HDR10 signal; application uses linear values.
			colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
			break;

		default:
			break;
		}
	}

	ColorSpaceType = colorSpace;

	UINT colorSpaceSupport = 0;
	if (SUCCEEDED(currentSwapChain->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport))
		&& (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
	{
		ThrowIfFailed(currentSwapChain->SetColorSpace1(colorSpace));
	}
	return true;
}

void DXRenderManager::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		SubstanceManagerPtr->GetTextureCount(),
		0// register t0
	);

	// Root parameter can be a table, root descriptor or root constants.
	std::array<CD3DX12_ROOT_PARAMETER, 5> slotRootParameter;

	// Create a single descriptor table of CBVs.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsShaderResourceView(0, 1);
	slotRootParameter[4].InitAsShaderResourceView(0, 5);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7U> staticSampler = GetStaticSamplers();
	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(static_cast<UINT>(slotRootParameter.size()), slotRootParameter.data(), static_cast<UINT>(staticSampler.size()), staticSampler.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(Device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&RootSignatureMap["MainRootSig"])));

	//CS
	std::array<CD3DX12_DESCRIPTOR_RANGE, 3> uavTable;
	uavTable[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		1,// range 2 u0,u1
		0 // start from U0, if its 1 then it will start from U1
	);
	uavTable[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		1,// range 2 u0,u1
		1 // start from U0, if its 1 then it will start from U1
	);
	uavTable[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		1,// range 2 u0,u1
		2 // start from U0, if its 1 then it will start from U1
	);

	std::array<CD3DX12_ROOT_PARAMETER, 3> slotRootParameterCS;
	//slotRootParameter[0].InitAsDescriptorTable(1, &uavTable, D3D12_SHADER_VISIBILITY_ALL);
	slotRootParameterCS[0].InitAsDescriptorTable(1, &uavTable[0]);
	slotRootParameterCS[1].InitAsDescriptorTable(1, &uavTable[1]);
	slotRootParameterCS[2].InitAsDescriptorTable(1, &uavTable[2]);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDescCS(static_cast<UINT>(slotRootParameterCS.size()),
		slotRootParameterCS.data(),
		0,//no sampler
		nullptr,//no sampler
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSigCS = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errBlob = nullptr;
	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	hr = D3D12SerializeRootSignature(&rootSigDescCS,
		D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSigCS.GetAddressOf(),
		errBlob.GetAddressOf());
	if (errBlob != nullptr)
	{

		::OutputDebugStringA((char*)errBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);
	ThrowIfFailed(Device->CreateRootSignature(0,
		serializedRootSigCS->GetBufferPointer(),
		serializedRootSigCS->GetBufferSize(),
		__uuidof(ID3D12RootSignature), &RootSignatureMap["RootSigCS"]));
}

void DXRenderManager::BuildSamplerDesc()
{
	//TODO USE it to create sampler
	D3D12_DESCRIPTOR_HEAP_DESC samplerDescHeap = {};
	samplerDescHeap.NumDescriptors = 6;
	samplerDescHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	samplerDescHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ID3D12DescriptorHeap* samplerHeap;
	ThrowIfFailed(Device->CreateDescriptorHeap(&samplerDescHeap, IID_PPV_ARGS(&samplerHeap)));
	SamplerDescrHeap = unique_ptr<ID3D12DescriptorHeap>(samplerHeap);

	D3D12_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
}

void DXRenderManager::BuildTextureSRVDescriptors()
{
	//// Describe and create a constant buffer view (CBV) and shader resource view (SRV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC cbvSRVHeapDesc = {};
	////number of texture resource 
	cbvSRVHeapDesc.NumDescriptors = 1;
	cbvSRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvSRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvSRVHeapDesc.NodeMask = 0;
	ID3D12DescriptorHeap* cbvSrvHeap;
	ThrowIfFailed(Device->CreateDescriptorHeap(&cbvSRVHeapDesc, IID_PPV_ARGS(&cbvSrvHeap)));
	CbvSrvDescriptorHeap = unique_ptr<ID3D12DescriptorHeap>(cbvSrvHeap);

	CbvSrvDescriptorHeap->SetName(L" cbvSrv MainDevice");
	UINT textureCount = static_cast<UINT>(TextureList.size());
	// Fill out the heap with actual descriptors
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	D3D12_RESOURCE_DESC textureDesc;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(CbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	Microsoft::WRL::ComPtr<ID3D12Resource> texture;
	for (UINT i = 0; i < textureCount; i++)
	{
		hDescriptor.Offset(i, CbvSrvDescriptorSize);
		texture = TextureList[i]->TextureHeap;
		textureDesc = texture->GetDesc();
		srvDesc.Format = texture->GetDesc().Format;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
		Device->CreateShaderResourceView(texture.Get(), &srvDesc, hDescriptor);
	}
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> DXRenderManager::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy
	const CD3DX12_STATIC_SAMPLER_DESC shadow(
		6, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

	return {
		pointWrap,
		pointClamp,
		linearWrap,
		linearClamp,
		anisotropicWrap,
		anisotropicClamp,
		shadow
	};
}

int DXRenderManager::ExecuteRenderThread(int threadIndex)
{
	while (true)
	{
		std::atomic_bool stop_thread_1 = false;
		WaitForSingleObject(RenderPool.GetWorkerBeginHandle(threadIndex), INFINITE);
		if (bDone) { return 0; }
		CCommandList& commandListWrapper = CommandListPool->Get();
		ID3D12GraphicsCommandList* commandList = commandListWrapper.Get();
		Clear(commandList, threadIndex);

		commandList->SetGraphicsRootSignature(RootSignatureMap["MainRootSig"].Get());
		commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// first 4 for cameras
		UINT passCBByteSize = CalcConstantBufferByteSize(sizeof(FrameConstants));
		const Microsoft::WRL::ComPtr<ID3D12Resource>& buffer = CurrentFrameResources->FrameCB->GetResource();
		D3D12_GPU_VIRTUAL_ADDRESS FrameCBAddress = buffer->GetGPUVirtualAddress() + (threadIndex * passCBByteSize);
		commandList->SetGraphicsRootConstantBufferView(1, FrameCBAddress);

		ID3D12DescriptorHeap* descriptorHeaps[] = { CbvSrvDescriptorHeap.get() };
		commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		//all material in scene
		const Microsoft::WRL::ComPtr<ID3D12Resource>&  MaterialResource = CurrentFrameResources->MaterialCB->GetResource();
		commandList->SetGraphicsRootShaderResourceView(4, MaterialResource->GetGPUVirtualAddress());

		//all Substances in scene
		const Microsoft::WRL::ComPtr<ID3D12Resource>&  SubstanceResource = CurrentFrameResources->SubstanceCB->GetResource();
		commandList->SetGraphicsRootShaderResourceView(3, SubstanceResource->GetGPUVirtualAddress());

		//all textures in the scene
		commandList->SetGraphicsRootDescriptorTable(0, CbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		commandList->SetPipelineState(PipelineStateMap["sky"].Get());
		DrawRenderItems(commandList, RenderItemLayerList[(int)RenderLayer::Sky]);
		commandList->SetPipelineState(PipelineStateMap["opaque"].Get());
		DrawRenderItems(commandList, RenderItemLayerList[(int)RenderLayer::OpaquePhysics]);
		commandList->SetPipelineState(PipelineStateMap["opaque"].Get());
		DrawRenderItems(commandList, RenderItemLayerList[(int)RenderLayer::Opaque]);
		commandList->SetPipelineState(PipelineStateMap["opaqueLine"].Get());
		DrawRenderItems(commandList, RenderItemLayerList[(int)RenderLayer::OpaqueLine]);
		commandList->SetPipelineState(PipelineStateMap["wireframeLine"].Get());
		DrawRenderItems(commandList, RenderItemLayerList[(int)RenderLayer::WireframeLine]);
		commandList->SetPipelineState(PipelineStateMap["wireframeTri"].Get());
		DrawRenderItems(commandList, RenderItemLayerList[(int)RenderLayer::WireframeTri]);

		// Transition the render target to the state that allows it to be presented to the display.
		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(RenderTargetList[threadIndex][BackBufferIndexList[threadIndex]].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		commandList->ResourceBarrier(1, &barrier);
		wchar_t name[25] = {};
		UINT size = sizeof(name);
		barrier.Transition.pResource->GetPrivateData(WKPDID_D3DDebugObjectNameW, &size, name);

		CommandListPool->Execute(commandListWrapper);

		SetEvent(RenderPool.GetWorkerFinished(threadIndex)); 
	
		CommandListPool->ReturnCommandList(commandListWrapper);
	}
	return 0;
}

void DXRenderManager::DrawRenderItems(ID3D12GraphicsCommandList* commandList, const std::vector<std::shared_ptr<CRenderItem>> renderItemLayer)
{
	UINT objCBByteSize = CalcConstantBufferByteSize(sizeof(ObjectConstants));
	const Microsoft::WRL::ComPtr<ID3D12Resource>& objectCB = CurrentFrameResources->ObjectCB->GetResource();
	for (size_t i = 0; i < renderItemLayer.size(); ++i)
	{
		CRenderItem* renderItem = renderItemLayer[i].get();
		commandList->IASetVertexBuffers(0, 1, &renderItem->GeometryPrimitive->VertexBufferView());
		commandList->IASetIndexBuffer(&renderItem->GeometryPrimitive->IndexBufferView());
		commandList->IASetPrimitiveTopology(renderItem->PrimitiveType);

		// geometry
		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + (D3D12_GPU_VIRTUAL_ADDRESS)renderItem->ObjectCBIndex * objCBByteSize;
		commandList->SetGraphicsRootConstantBufferView(2, objCBAddress);
		commandList->DrawIndexedInstanced(renderItem->IndexCount, 1, renderItem->StartIndexLocation, renderItem->BaseVertexLocation, 0);
	}
}

void DXRenderManager::ConfigureRenderThreads()
{
#if !SINGLETHREADED

	for (int i = 0; i < 4; i++)
	{
		RenderPool.Submit(&DXRenderManager::ExecuteRenderThread, this, i);;
	}
#endif
}

void DXRenderManager::GetAdapter(IDXGIAdapter1** ppAdapter)
{
	*ppAdapter = nullptr;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
	if (DXGIFactory)
	{
		for (UINT adapterIndex = 0;
			SUCCEEDED(DXGIFactory->EnumAdapterByGpuPreference(
				adapterIndex,
				DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
				IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())));
			adapterIndex++)
		{
			DXGI_ADAPTER_DESC1 desc;
			ThrowIfFailed(adapter->GetDesc1(&desc));

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				continue;
			}
			// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), d3dMinFeatureLevel, _uuidof(ID3D12Device), nullptr)))
			{
#ifdef _DEBUG
				wchar_t buff[256] = {};
				swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
				OutputDebugStringW(buff);
#endif
				break;
			}
		}
	}
#endif
	if (!adapter)
	{
		for (UINT adapterIndex = 0;
			SUCCEEDED(DXGIFactory->EnumAdapters1(
				adapterIndex,
				adapter.ReleaseAndGetAddressOf()));
			++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			ThrowIfFailed(adapter->GetDesc1(&desc));
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				continue;
			}
			// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), d3dMinFeatureLevel, _uuidof(ID3D12Device), nullptr)))
			{
#ifdef _DEBUG
				wchar_t buff[256] = {};
				swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
				OutputDebugStringW(buff);
#endif
				break;
			}
		}
	}

#if !defined(NDEBUG)
	if (!adapter)
	{
		// Try WARP12 instead
		if (FAILED(DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()))))
		{
			throw std::exception("WARP12 not available. Enable the 'Graphics Tools' optional feature");
		}
		OutputDebugStringA("Direct3D Adapter - WARP12\n");
	}
#endif
	if (!adapter)
	{
		throw std::exception("No Direct3D 12 device found");
	}
	*ppAdapter = adapter.Detach();
}

void DXRenderManager::BuildRenderItem(std::shared_ptr<CRenderItem>& renderItem, const std::string& primitiveName)
{
	if (const std::shared_ptr<MeshGeometry>& meshGeometry = MeshManager->GetMeshGeometry(primitiveName))
	{

		renderItem->TextureTransform = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
		renderItem->ObjectCBIndex = RenderitemList.size();
		renderItem->GeometryPrimitive = meshGeometry;
		renderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		renderItem->IndexCount = meshGeometry->DrawArgs[primitiveName].IndexCount;
		renderItem->StartIndexLocation = meshGeometry->DrawArgs[primitiveName].StartIndexLocation;
		renderItem->BaseVertexLocation = meshGeometry->DrawArgs[primitiveName].BaseVertexLocation;
		RenderItemLayerList[(int)renderItem->Layer].push_back(renderItem);
		RenderitemList.emplace_back(renderItem);
	}
}
