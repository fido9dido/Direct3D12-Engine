#include "Renderer.h"
#include "Header Files/MainApp.h"
#include "Header Files/Exception.h"
#include "Header Files/CustomReturnValues.h"

#include <Windows.h>
#include <memory>

HINSTANCE g_hInst=nullptr;

HWND InitWindows(const HINSTANCE& hInst, MainApp& mainApp);
HRESULT InitDevice(void);

int WINAPI WinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lCmdLine, _In_ int nShowCmd )
{ 
	g_hInst = hInst;
	MainApp mainApp;
	HWND hWnd = InitWindows(hInst, mainApp);

	
	std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>(g_hInst);
	 //load DirectX DLL
	renderer->CreateRendererDevice("Direct3D");
	std::unique_ptr<IRenderDevice>& pDevice = renderer->GetRendererDevice();
	mainApp.SetRenderDevice(pDevice);

	 if (pDevice == nullptr)
	 {
		 return E_FAIL;
	 }
	 RECT rect;

	 GetClientRect(hWnd, &rect);
	 std::unique_ptr<MainApp> mywin = std::make_unique<MainApp>();
	 std::vector<HWND> hWnd3D; 
	 for (size_t i = 0; i < 4; i++)
	 {
		 mainApp.SetHWnd3D(mywin->CreateAndGetWnd(TEXT("static"), TEXT(""), WS_CHILD | SS_BLACKRECT | WS_VISIBLE  /*  |SS_INMOTION | SS_ERASE*/,
			 WS_EX_CLIENTEDGE, 300, 150, 0, 0, hWnd, nullptr));
		 hWnd3D.push_back(mainApp.GetHWnd3D()[i]);
	 }
	 HRESULT hr; 
	 if (IsFail(hr = pDevice->Direct3DInit(hWnd,hWnd3D, 300, 150, false)))
	 {
		 return 0;
	 }
	 else if (hr == DX_CANCELED)
	 {

	 }
	 mainApp.Initialize(300, 150);
	 SendMessage(hWnd, WM_SIZE, 0, MAKELONG((WORD)rect.right, (WORD)rect.bottom));
	 pDevice->SetClearColor(255.f, 99.f, 71.f);
	 mainApp.TranslateMessages();
	return DX_OK;
}
HRESULT InitDevice(void)
{	
	return DX_OK;
}

HWND InitWindows(const HINSTANCE& hInst, MainApp& mainApp)
{
	HMENU hMenu = LoadMenu(hInst, TEXT(" Menu"));
	if (!mainApp.Create(L"DX Engine",L"DX Engine", WS_OVERLAPPEDWINDOW, 0, 628, 371
	))
	{
		return 0;
	}
	ShowWindow(mainApp.GetWindow(), SW_SHOW);
	UpdateWindow(mainApp.GetWindow());

	return  mainApp.GetWindow();
}