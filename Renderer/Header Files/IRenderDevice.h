#ifndef IRENDERDEVICE_H
#define IRENDERDEVICE_H

#include <windows.h>
#include <vector>
#include <memory>

#include "RendererAttributes.h"     
#include <DirectXMath.h>

enum class DEVICESTATUS :int;



class IRenderDevice
{
public:

	IRenderDevice(void) {};
	virtual ~IRenderDevice(void) {};

	// INIT/RELEASE:
	virtual HRESULT Direct3DInit(const HWND& hWnd, const std::vector<HWND>& hWnd3D, int nMinDepth, int nMinStencil, bool bSaveLog) = 0;
	virtual void ShutDown() = 0; //release
	virtual bool IsRunning() = 0;

	//Rendering
	virtual HRESULT UseWindow(UINT nHwnd) = 0;
	virtual HRESULT Tick() = 0;
	virtual HRESULT BeginRendering(void) = 0;
	virtual void EndRendering() = 0;
	//virtual HRESULT Clear(int i) = 0;
	virtual void SetClearColor(float fRed, float fBlue, float FGreen) = 0;
	virtual HRESULT OnResize(LONG width, LONG height, bool minimized) = 0;
	virtual HRESULT OnUpdate(float time,float  keyboard, POINT position) = 0;
	//virtual HRESULT OnOrthogonalView(float) = 0;
	//virtual void UpdateCamera(POINT pos)=0;
	virtual void UpdateCameraCB(const DirectX::XMMATRIX& view3D, const DirectX::XMMATRIX& ProjectionPerspective, const DirectX::XMVECTOR& position, int index)=0;

	virtual void BuildMeshBuffer(struct MeshData& meshData) = 0;
	virtual void BuildRenderItem(class std::shared_ptr<struct CRenderItem>& renderItem, const std::string& primitiveName) = 0;
	virtual void InitializeScene() = 0;
	

private:
 

};// End of Class

//typedef class std::unique_ptr<IRenderDevice>& LPIRenderDevice;
extern "C"
{
	HRESULT CreateRenderDevice(HINSTANCE hDLL, std::unique_ptr<IRenderDevice> *pDevice);
	typedef HRESULT(*CREATERENDERDEVICE)(HINSTANCE dllHandle, std::unique_ptr<IRenderDevice>*pointerInterface);

	HRESULT ShutdownRenderDevice(std::unique_ptr<IRenderDevice>*pDevice);
	typedef HRESULT(*SHUTDOWNRENDERDEVICE)(std::unique_ptr<IRenderDevice>*pointerInterface);

}


#endif