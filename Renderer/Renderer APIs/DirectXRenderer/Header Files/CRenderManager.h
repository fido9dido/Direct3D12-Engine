#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H

#include <dxgi1_6.h>
#include <array>
#include <string>
#include <filesystem>
#include <dxgidebug.h>
#include <unordered_map>
#include <fstream>

#include "CCommandListPool.h"
#include "CSubstanceManager.h"
#include "CRenderThreads.h"
#include "SFrameResource.h"
#include "IRenderDevice.h"
#include "CMeshManager.h"
#include "CRenderItem.h"
#include "CSwapChain.h"

struct Texture;
struct MeshGeometry;
struct CTerrain;
class CRenderManager : public IRenderDevice
{
public:
private:	

	std::vector<HWND> WindowHandleList; //[MAX_3DHWND];  // 3D render window handle

	int Width;           // app width
	int Height;          // app height
	bool bIsRunning;          // after succesful initialization
	static const UINT WindowCount = 4;          // number of active windows 

	std::atomic_bool bDone = false;
	//// Set true to use 4X MSAA (§4.1.8).  The default is false.
	bool bIsMsaaEnabled;    // 4X MSAA enabled
	bool bIsTearingSupport;
	UINT MsaaQualityX4;      // quality level of 4X MSAA
	UINT CbvSrvDescriptorSize;
	UINT RtvDescriptorSize;
	UINT DsvDescriptorSize;
	uint64_t FrameIndexCount;
	float FrameTime;
	static const size_t BackBufferCount = 2;
	static const unsigned int EnableHDR = 0x2;

	HINSTANCE DLLHandle;              // dll module handle
	unique_ptr<ID3D12Device> Device;
	unique_ptr<CCommandListPool> CommandListPool;
	unique_ptr<ID3D12CommandQueue> CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Debug> spDebugController0;
	Microsoft::WRL::ComPtr<ID3D12Debug3> spDebugController1;
	CRenderThreads RenderPool;

	unique_ptr<IDXGIFactory7> DXGIFactory;

	std::array<unique_ptr<CSwapChain>, WindowCount> SwapChainList; // one per window
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayoutList;
	
	std::array<UINT, WindowCount> BackBufferIndexList; // MultiViews

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> PipelineStateMap;

	std::array<std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, BackBufferCount>, WindowCount> RenderTargetList;// 2 per window**
	std::array<D3D12_VIEWPORT, WindowCount> ScreenViewportList;
	unique_ptr<ID3D12Resource> DepthStencil;

	unique_ptr<ID3D12DescriptorHeap> RtvDescriptorHeap;
	unique_ptr<ID3D12DescriptorHeap> DsvDescriptorHeap;
	D3D12_RECT	ScissorRect;

	DXGI_FORMAT BackBufferFormat;
	DXGI_FORMAT DepthBufferFormat;
	D3D_FEATURE_LEVEL d3dMinFeatureLevel;

	D3D_FEATURE_LEVEL d3dFeatureLevel;
	DWORD dxgiFactoryFlags;
	RECT OutputScreenSize;
	//frame resource
	static const int FrameResourcesCount = 3;
	std::shared_ptr<SFrameResource> CurrentFrameResources;
	int CurrentFrameResourcesIndex;
	FrameConstants FrameConstantBuffer;  // index 0 of pass cbuffer.

	//// HDR Support
	DXGI_COLOR_SPACE_TYPE ColorSpaceType;

	std::vector<unique_ptr<Texture>> TextureList;
;
	std::unordered_map<RenderLayer, std::shared_ptr<MeshGeometry>> MeshGeometryEMap;
	std::vector<std::shared_ptr<CRenderItem>> RenderitemList;
	std::vector<std::shared_ptr<CRenderItem>> RenderItemLayerList[(int)RenderLayer::Count];
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> ShaderObjectMap;
	std::array<std::shared_ptr<SFrameResource>, FrameResourcesCount> FrameResourceList;
	
	int FrameConstBufferCount = 4;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> Adapter;
	std::vector<unique_ptr<IDXGIAdapter1>> AdapterList;
	unique_ptr<ID3D12DescriptorHeap> CbvSrvDescriptorHeap;
	unique_ptr<ID3D12DescriptorHeap> SamplerDescrHeap;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> RootSignatureMap;

	std::filesystem::path DirectoryPath;
	//Move to another Dll
	unique_ptr<CTerrain> DCTerrain;

	float AspectRatio;
	unsigned int Options;

	//constants here temporarily TODO move when making buffers
	static const unsigned int bAllowTearing = 0x1;
	float mSunTheta = 1.25f * DirectX::XM_PI;
	float mSunPhi = DirectX::XM_PIDIV4;
	float Radius = 5.0f;
	float Theta = 1.5f * DirectX::XM_PI;

	//global for now should be ()
	unique_ptr<CSubstanceManager> SubstanceManagerPtr;
	unique_ptr<CMeshManager> MeshManager;
public:
	CRenderManager(HINSTANCE hDLL,
		DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT, // depth stencil 
		D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0,
		unsigned int flags = 0);

	~CRenderManager(void) = default;
	
	virtual HRESULT Direct3DInit(const HWND& hWnd, const std::vector<HWND>& hWnd3D, int nMinDepth, int nMinStencil, bool bSaveLog);
	virtual void InitializeScene();
	virtual HRESULT UseWindow(UINT nHwnd);
	virtual HRESULT BeginRendering(void);
	virtual void EndRendering(void);// <- todo delete
	virtual HRESULT Clear(ID3D12GraphicsCommandList* commandList, int i);
	virtual void SetClearColor(float fRed, float fBlue, float FGreen);// <- todo delete
	virtual void ShutDown(void); //release // <- todo delete
	virtual bool IsRunning(void);
	virtual void UpdateSelectedDynamicActor(const DirectX::XMMATRIX& worldMatrix, int index);

	HRESULT CreateDevice(void);

	HRESULT CreatePipelineStateObject(void);
	
	void CreateSubstance(void);
	void DrawRenderItems(ID3D12GraphicsCommandList* commandList, const std::vector<std::shared_ptr<CRenderItem>> renderItemLayer);
	void OnDeviceLost(void);
	bool MoveToNextFrame(int i);
	bool CreateResources(void);
	void CreateFrameResource(void);
	virtual void BuildMeshBuffer(struct MeshData& meshData);

	void UpdateForResize(UINT clientWidth, UINT clientHeight);
	bool UpdateColorSpace(int i);
	void CreateShaderAndInputLayout(void);
	void BuildRootSignature(void);
	void BuildSamplerDesc(void);
	void BuildTextureSRVDescriptors(void);
	void CreateShaderResourceView(std::vector<unique_ptr<Texture>>& mTextures, bool isCubeMap = false);
	void UpdateObjectCB(void);
	void UpdateCameraCB(const DirectX::XMMATRIX& view3D, const DirectX::XMMATRIX& ProjectionPerspective, const DirectX::XMVECTOR& position, int index);

	void UpdateSubstanceCB(void);
	void UpdateMaterialCB(void);

	HRESULT OnResize(LONG width, LONG height, bool minimized);
	HRESULT Tick(void);

	HRESULT LoadSwapChain(DXGI_SWAP_CHAIN_DESC& outDesc, int nMsaa);
	HRESULT LogAdapterOutputs(const Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter);
	HRESULT LogOutputDisplayModes(const Microsoft::WRL::ComPtr<IDXGIOutput> pAdapterOutput, DXGI_FORMAT format);
	HRESULT OnUpdate(float deltaTime, float keyboardStat, POINT position);

	void RemoveRenderItem(std::string name);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers(void);
	int  ExecuteRenderThread(int threadIndex);
	void ConfigureRenderThreads();
	
	uint64_t GetFrameCount(void) { return FrameIndexCount; }
	float GetFrameTime(void) { return FrameTime; }
	float GetFrameRate(void) { return FrameTime == 0.0f ? 0.0f : 1.0f / FrameTime; }
	void GetAdapter(IDXGIAdapter1** ppAdapter);

	void BuildRenderItem(class std::shared_ptr<CRenderItem>& renderItem, const std::string& primitiveName);
	void OnRenderComponentRemoved(class std::shared_ptr<CRenderItem> renderItem);
protected:

};


#endif 
