#ifndef MAINAPP_H
#define MAINAPP_H

#include "BaseWindow.h"
#include "resource.h"

#include "Renderer.h"
#include "DXIMouse.h"
#include <vector>
#include "CTimer.h"
#include "Camera.h"
#include <CEntityComponent.h>
#include <CPhysicsSimulation.h>
#include <MemoryUtil.h>

/*
Temporary class to initialize the app and resources for now
*/
enum class DXREngine {};
class MainApp : public BaseWindow<MainApp>
{
public:
	void SetRenderDevice(const std::unique_ptr<IRenderDevice>& pDevice);
	MainApp(void);
	void Initialize(float width, float height);
	void BuildTestEntity(const struct SEntityDescriptor& desc, const DirectX::XMMATRIX& transform);
	void BuildTestEntities();
	//HWND CreateTabControl(HWND hWndParent);
	UINT AddMenuItem(LPVOID MenuTemplate, int TemplateBytes, const WCHAR* MenuString, WORD MenuID, BOOL IsPopup, BOOL LastItem);
	void AddMenuItem(std::vector<BYTE>* MenuTemplate, std::string MenuString,
		WORD MenuID, BOOL IsPopup, BOOL LastItem);

	void OnLoadMenuIndirect(void);
	void OnMouseMove(WPARAM btnState, UINT x, UINT y);
	float DegreeToRadians(float degree);
	void BuildLandscapeGeometry(void);
	void BuildPrimitiveBox();
	PCWSTR GetWindowName(void) const override;
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	void SetMultiChildWindows(std::vector<HWND> g_hWnd3D)override;
	std::vector<HWND> GetHWnd3D(void);
	void SetHWnd3D(HWND);
	BOOL IsActive(void);

	void TranslateMessages();
	
	void OnViewOrthogonalWindow(int i);
	void OnViewPerspectiveWindow(int i);

	void Update();

	DXIMouse& GetMouse() { return m_Mouse; };

protected:
	std::string WideToBuffer(std::wstring str);
	void CreateOldMenus(void);
	std::vector<HWND> g_hWnd3D;

private:
	HMENU mtest;
	BOOL bIsActive;
	int  m_Width;
	int  m_Height;
	static const int WindowCount = 4;
	IRenderDevice* RenderDevice;
	CTimer Timer;

	std::array<CCamera, WindowCount> CameraList;

	std::unique_ptr<CPhysicsSimulation> NvidiaPhysics;

	///////////////////////////////////////////////////

	std::vector<unique_ptr<CEntityComponent>> TestEntites;

	POINT m_LastMousePos;
	
	float m_Radius = 2.5f;

	//////////////////////////////////
	// Mouse 
	DXIMouse m_Mouse;
	POINT p;
	//raw input
	 UINT szData;
	
	//////////////////////////////////
	// window status
	bool      m_bIsAppPaused = false;  
	bool      m_bIsMinimized = false;  
	bool      m_bIsMaximized = false;
	//end window status
	////////////////////////////////////

	//////////////////////////////////
	// window status
	std::array<bool, 4>      bIsDirtyWindowList ;
	std::array<float, 4>     WindowScale;   // scaling for the three mini orthog views
	//end window status
	////////////////////////////////////
};
#endif
