#include "MainApp.h"
#include <strsafe.h>
#include <DirectXMath.h>
#include <iterator>
#include <foundation/Px.h>
#include <PxRigidDynamic.h>
#include <GeometryGenerator.h>
#include <CRenderUtil.h>
#include <CEntityComponent.h>
#include <CTransformComponent.h>
#include <CMeshComponent.h>
#include <string>
#include <CPhysicsComponent.h>

PCWSTR MainApp::GetWindowName(void) const
{
	return L"opard Engine";
}

void MainApp::CreateOldMenus(void)
{
	std::vector<BYTE> menuListX;
	BYTE milist2[500];
	memset(milist2, 0, 500); 
	// Fill up the MENUITEMTEMPLATEHEADER structure.
	MENUITEMTEMPLATEHEADER* mheader =   new MENUITEMTEMPLATEHEADER;
	(*mheader).versionNumber =0;
	(*mheader).offset =0;

	menuListX.push_back((BYTE)mheader->versionNumber);
	menuListX.push_back(0);
	menuListX.push_back((BYTE)mheader->offset);
	menuListX.push_back(0);
	int bytes_used = sizeof(MENUITEMTEMPLATEHEADER);
		
	AddMenuItem(&menuListX,WideToBuffer(L"&File"), 0, true, false); // pop up
	AddMenuItem(&menuListX, WideToBuffer(L"E&xit"), IDM_EXIT, false, true);
	AddMenuItem(&menuListX, WideToBuffer(L"&View"), IDM_HELP, false, false);//default menu
	AddMenuItem(&menuListX, WideToBuffer(L"&Settings"), 0, true, false); // pop up
	AddMenuItem(&menuListX, WideToBuffer(L"&Options"), IDM_OPTIONS, false, true);
	AddMenuItem(&menuListX, WideToBuffer(L"&Unicode"), IDM_HELP, true, false);
	AddMenuItem(&menuListX, WideToBuffer(L"اللغة العربية&"), 0, false, true);
	AddMenuItem(&menuListX, WideToBuffer(L"&Help"), IDM_HELP, false, true);
	mtest = LoadMenuIndirectW(menuListX.data());
												
	//deprecated
	//std::wstring_convert<std::codecvt_utf16<wchar_t>, wchar_t> converter;
	//AddMenuItem(&menuListX2, converter.to_bytes(L"&File"), 0, true, false); // pop up
	//AddMenuItem(&menuListX2, converter.to_bytes(L"E&xit"), IDM_EXIT, false, true);
	//AddMenuItem(&menuListX2, converter.to_bytes(L"&View"), IDM_HELP, false, false);//default menu
	//AddMenuItem(&menuListX2, converter.to_bytes(L"&Settings"), 0, true, false); // pop up
	//AddMenuItem(&menuListX2, converter.to_bytes(L"&Options"), IDM_OPTIONS, false, true);
	//AddMenuItem(&menuListX2, converter.to_bytes(L"&Help"), IDM_HELP, false, true);
	//mtest = LoadMenuIndirectW(menuListX2.data());
	
	SetMenu(m_hWnd, mtest);
}

void MainApp::SetMultiChildWindows(std::vector<HWND> g_hWnd3D)
{

}

std::vector<HWND> MainApp::GetHWnd3D(void)
{

	return g_hWnd3D;
}

void MainApp::OnViewOrthogonalWindow(int i)
{
	if (bIsDirtyWindowList[i])
	{
		DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
	}
}

void MainApp::OnViewPerspectiveWindow(int i)
{
	if (bIsDirtyWindowList[i])
	{

	}
}

void MainApp::Update()
{
	Timer.Tick();
}

void MainApp::SetHWnd3D(HWND hWnd)
{
	g_hWnd3D.push_back(hWnd);
}

BOOL MainApp::GetbIsActive(void)
{
	return bIsActive;
}

void MainApp::SetRenderDevice(const std::unique_ptr<IRenderDevice>& pDevice)
{
	RenderDevice = pDevice.get();
}

MainApp::MainApp(void):
	bIsDirtyWindowList{false},
	mtest(nullptr)
{ 
	Timer.Reset();

}

void MainApp::Initialize(float width, float height)
{
	NvidiaPhysics = std::make_unique<CPhysicsSimulation>();
	CameraList = { CCamera() };
	CameraList[0].InitCamera(width, height, DirectX::XMVectorSet(0.f, 20.f, -10.f, 0.f), DirectX::XMVectorSet(0.f, -1.f, 1.f, 0));
	CameraList[1].InitOrthogonalCamera(width, height, 12.5f, (int)CameraViews::OrthFront);
	CameraList[2].InitOrthogonalCamera(width, height, 12.5f, (int)CameraViews::OrthSide);
	CameraList[3].InitOrthogonalCamera(width, height, 12.5f, (int)CameraViews::OrthTop);

	BuildPrimitiveBox();
	BuildLandscapeGeometry();
	BuildTestEntities();
	RenderDevice->InitializeScene();
}

void MainApp::BuildTestEntity(const SEntityDescriptor& desc, const DirectX::XMMATRIX& transform)
{
	CEntityComponent testEntity;
	CTransformComponent* testTransform = new CTransformComponent();
	CMeshComponent* testMeshComponent = new CMeshComponent(
		&IRenderDevice::BuildRenderItem, RenderDevice, std::placeholders::_1, std::placeholders::_2
	);
	
	testMeshComponent->SetDescription(desc.meshDesc);

	testEntity.AddComponent(testTransform);
	testEntity.AddComponent(testMeshComponent);
	if (desc.physicsDesc.ShapeType != ECShapeType::NotImplemented)
	{
		CPhysicsComponent* testPhysics = new CPhysicsComponent();
		testPhysics->InitializeComponent(NvidiaPhysics.get());
		testPhysics->SetDescription(desc.physicsDesc);
		
		testEntity.AddComponent(testPhysics);
	}
	testTransform->OnTransformComponentUpdate.Notify(transform);
	TestEntites.emplace_back(make_unique<CEntityComponent>(testEntity));
}

void MainApp::BuildTestEntities()
{
	SEntityDescriptor boxEntity;

	boxEntity.meshDesc.Name = "Box";
	boxEntity.meshDesc.Layer = static_cast<uint8_t>(RenderLayer::OpaquePhysics);
	boxEntity.meshDesc.SubstanceCBIndex = 1;
	boxEntity.physicsDesc.ShapeType = ECShapeType::Box;
	boxEntity.physicsDesc.ActorType = ECActorType::Dynamic;
	BuildTestEntity(boxEntity, DirectX::XMMatrixScaling(2.0f, 2.0f, 2.0f) * DirectX::XMMatrixTranslation(15, 10, 0));
	
	SEntityDescriptor TerrainDesc;
	TerrainDesc.meshDesc.Name = "Terrain";
	TerrainDesc.meshDesc.Layer = static_cast<uint8_t>(RenderLayer::Opaque); 
	TerrainDesc.meshDesc.SubstanceCBIndex = 2;
	TerrainDesc.physicsDesc.ShapeType = ECShapeType::TriangleMesh;
	TerrainDesc.physicsDesc.ActorType = ECActorType::Static;
	BuildTestEntity(TerrainDesc, DirectX::XMMatrixTranslation(0, 7, 0));
	
}

// once per frame
void MainApp::OnMouseMove(WPARAM btnState, UINT x, UINT y)
{
}

float MainApp::DegreeToRadians(float degree)
{
	return degree * 0.01745329251994329576923690768489f;
}

void MainApp::BuildLandscapeGeometry(void)
{
	GeometryGenerator geoGenerator;
	MeshData landscape = geoGenerator.CreateCubeGrid(32, 32, 32);
	landscape.Name = "Terrain";
	RenderDevice->BuildMeshBuffer(landscape);
}

void MainApp::BuildPrimitiveBox()
{
	GeometryGenerator geoGenerator;
	MeshData box = geoGenerator.CreateBox(1.0f, 1.0f, 1.0f, 3);
	int verticesCount = static_cast<int>(box.VerticesList.size());
	std::vector<Vertex> vertices(verticesCount);
	for (size_t i = 0; i < verticesCount; ++i)
	{
		DirectX::XMVECTOR& position = box.VerticesList[i].Position;
		DirectX::XMStoreFloat3(&vertices[i].Position, box.VerticesList[i].Position);
		DirectX::XMStoreFloat3(&vertices[i].Normal, box.VerticesList[i].Normal);
		
	}
	box.Name = "Box";
	RenderDevice->BuildMeshBuffer(box);

}

void MainApp::AddMenuItem(std::vector<BYTE>* MenuTemplate, std::string MenuString,
	WORD MenuID, BOOL IsPopup, BOOL LastItem)
{
	MENUITEMTEMPLATE* mitem = new MENUITEMTEMPLATE;

	if (IsPopup)         // for popup menu
	{
		if (LastItem)
		{
			mitem->mtOption = MF_POPUP | MF_END;
		}
		else
		{
			mitem->mtOption = MF_POPUP;
		}
		// a popup doesn't have mtID!!!
		(*MenuTemplate).push_back((BYTE)mitem->mtOption);

		std::copy(MenuString.begin(), MenuString.end(), std::back_inserter(*MenuTemplate));
		(*MenuTemplate).push_back(0); // include '\0'   

		(*MenuTemplate).push_back(0);
		(*MenuTemplate).push_back(0);

	}
	else      // for command item
	{

		mitem->mtOption = LastItem ? MF_END : 0;
		mitem->mtID = MenuID;

		(*MenuTemplate).push_back((BYTE)mitem->mtOption);
		(*MenuTemplate).push_back(0);
		(*MenuTemplate).push_back((BYTE)mitem->mtID);

		std::copy(MenuString.begin(), MenuString.end(), std::back_inserter(*MenuTemplate));
		(*MenuTemplate).push_back(0); // include '\0'   

		(*MenuTemplate).push_back(0);
		(*MenuTemplate).push_back(0);
	}

}

UINT MainApp::AddMenuItem(LPVOID MenuTemplate, int TemplateBytes, const WCHAR* MenuString,
	WORD MenuID, BOOL IsPopup, BOOL LastItem)
{
	MENUITEMTEMPLATE* mitem = (MENUITEMTEMPLATE*)MenuTemplate;

	UINT  bytes_used = 0;
	if (IsPopup)         // for popup menu
	{
		if (LastItem)
			mitem->mtOption = MF_POPUP | MF_END;
		else
			mitem->mtOption = MF_POPUP;
		bytes_used += sizeof(mitem->mtOption);

		mitem = (MENUITEMTEMPLATE*)((BYTE*)MenuTemplate + bytes_used);
		// a popup doesn't have mtID!!!

		TemplateBytes -= bytes_used;
		wcscpy_s((WCHAR*)mitem, TemplateBytes / sizeof(WCHAR), MenuString);
		bytes_used += (UINT)(sizeof(WCHAR) * (wcslen(MenuString) + 1)); // include '\0'   
	}
	else      // for command item
	{
		mitem->mtOption = LastItem ? MF_END : 0;
		mitem->mtID = MenuID;
		TemplateBytes -= bytes_used;
		wcscpy_s(mitem->mtString, TemplateBytes / sizeof(WCHAR), MenuString);
		bytes_used += (UINT)(sizeof(mitem->mtOption) + sizeof(mitem->mtID) +
			sizeof(WCHAR) * (wcslen(MenuString) + 1));   // include '\0'
	}

	return bytes_used;
}

void  MainApp::OnLoadMenuIndirect(void)
{

	// For simplicity, allocate 500 bytes from stack. May use 
	// GlobalAlloc(void) to allocate memory bytes from heap.
	BYTE milist[500];
	memset(milist, 0, 500);
	int bytes_left = sizeof(milist);

	// Fill up the MENUITEMTEMPLATEHEADER structure.
	MENUITEMTEMPLATEHEADER* mheader = (MENUITEMTEMPLATEHEADER*)milist;
	(*mheader).versionNumber = 0;
	(*mheader).offset = 0;

	int bytes_used = sizeof(MENUITEMTEMPLATEHEADER);
	bytes_left -= bytes_used;

	bytes_used += AddMenuItem(milist + bytes_used, bytes_left, L"&File", 0,
		TRUE, FALSE);
	bytes_left -= bytes_used;
	bytes_used += AddMenuItem(milist + bytes_used, bytes_left, L"E&xit",
		IDC_ADAPTER, FALSE, TRUE);
	bytes_left -= bytes_used;

	bytes_used += AddMenuItem(milist + bytes_used, bytes_left, L"&Paste",
		IDC_ADAPTER, FALSE, TRUE);
	bytes_left -= bytes_used;

	// Load resource from a menu template in memory.
	mtest = LoadMenuIndirectW(milist);

	// Add new menu.
	SetMenu(m_hWnd, mtest);

}

std::string  MainApp::WideToBuffer(std::wstring str) 
{
	BYTE const* p = reinterpret_cast<BYTE const*>(&str[0]);
	std::size_t size = str.size() * sizeof(str.front());
	std::string buffer = " ";
	buffer[0] = '\0';
	buffer.insert(buffer.end(), p, p + size - 1);;
	return buffer;
}

void MainApp::TranslateMessages()
{
	MSG myMsg = {};

	while (myMsg.message != WM_QUIT)
	{
		if (PeekMessage(&myMsg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&myMsg);
			DispatchMessage(&myMsg);
			continue;
		}

		if (!GetbIsActive())
		{

			while (!m_Mouse.IsEmptyRawEventBuffer())
			{
				//ROTATE CAMERA
				POINT pos = m_Mouse.ExecuteRawEvent().GetPosition();
				CameraList[0].SetRotationSpeedX((float)pos.y);
				CameraList[0].SetRotationSpeedY((float)pos.x);
				//(*m_pRenderDevice).UpdateCamera(m_Mouse.ExecuteRawEvent().GetPosition());
			}
			Timer.Tick();
			//todo make function for update camera an tick it before rendering
			float deltaTime = Timer.GetDeltaTime();
			if (GetAsyncKeyState('W') & 0x8000)
				CameraList[0].Move(10.0f * deltaTime);

			if (GetAsyncKeyState('S') & 0x8000)
				CameraList[0].Move(-10.0f * deltaTime);

			if (GetAsyncKeyState('A') & 0x8000)
				CameraList[0].Strafe(-10.0f * deltaTime);

			if (GetAsyncKeyState('D') & 0x8000)
				CameraList[0].Strafe(10.0f * deltaTime);

			CameraList[0].Update(deltaTime);
			for (int i = 0; i < 4; i++)
			{
				CameraList[i].UpdateCamera(i);
				RenderDevice->UpdateCameraCB(CameraList[i].GetView3D(),
					//ZOOM 0 
					(i == 0) ? CameraList[i].GetProjectionPerspective()[0] : CameraList[i].GetProjectionOrthogonal()[0],
					CameraList[i].GetPosition(),
					i);
			}

			NvidiaPhysics->UpdatePhysics(deltaTime);

			NvidiaPhysics->UpdateSimulationResult();
			RenderDevice->OnUpdate(deltaTime, 0, { 0 });

			RenderDevice->BeginRendering();
		}
		else
		{
			Sleep(100);
		}
	}
}

enum class DEVICESTATUS : int;
LRESULT  MainApp::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static bool s_in_sizemove = false;
	IRenderDevice* game = reinterpret_cast<IRenderDevice*>(GetWindowLongPtr(m_hWnd, GWLP_USERDATA));
	switch (uMsg)
	{
	case WM_CREATE:
	{
		CreateOldMenus();
	}
	return 0;
	//mouse 
	/*case WM_MOUSEMOVE:
	{
		p.x = LOWORD(lParam);
		p.y = HIWORD(lParam);

		m_Mouse.SetMouse(DXIMouseEvent::DEVICESTATUS::Move, p);
	}*/
	return 0;

	case WM_LBUTTONDOWN:
	{
		// Capture mouse input. 
		SetCapture(hWnd);

		//mouse class
		p.x = LOWORD(lParam);
		p.y = HIWORD(lParam);

		m_Mouse.OnMainButtons(DXIMouseEvent::DEVICESTATUS::LPressed, p, true, DXIMouseEvent::MOUSEBUTTONS::LeftB);
	}
	return 0;

	case WM_LBUTTONUP:
	{
		// The user has finished clicking left button. Reset the 
		// previous line flag, release the mouse cursor, and 
		// release the mouse capture. 
		ReleaseCapture();

		//mouse class
		p.x = LOWORD(lParam);
		p.y = HIWORD(lParam);

		m_Mouse.OnMainButtons(DXIMouseEvent::DEVICESTATUS::LReleased, p, false, DXIMouseEvent::MOUSEBUTTONS::LeftB);
	}
	return 0;
	case WM_RBUTTONDOWN:
	{
		p.x = LOWORD(lParam);
		p.y = HIWORD(lParam);
		m_Mouse.OnMainButtons(DXIMouseEvent::DEVICESTATUS::RPressed, p, true, DXIMouseEvent::MOUSEBUTTONS::RightB);
	}
	return 0;
	case WM_RBUTTONUP:
	{
		p.x = LOWORD(lParam);
		p.y = HIWORD(lParam);
		m_Mouse.OnMainButtons(DXIMouseEvent::DEVICESTATUS::RReleased, p, false, DXIMouseEvent::MOUSEBUTTONS::RightB);
	}
	return 0;
	case WM_MBUTTONDOWN:
	{
		p.x = LOWORD(lParam);
		p.y = HIWORD(lParam);
		m_Mouse.OnMainButtons(DXIMouseEvent::DEVICESTATUS::MPressed, p, true, DXIMouseEvent::MOUSEBUTTONS::MiddleB);
	}
	return 0;
	case WM_MBUTTONUP:
	{
		p.x = LOWORD(lParam);
		p.y = HIWORD(lParam);
		m_Mouse.OnMainButtons(DXIMouseEvent::DEVICESTATUS::MPressed, p, false, DXIMouseEvent::MOUSEBUTTONS::MiddleB);
	}
	return 0;
	case WM_MOUSEHWHEEL:
	{
		p.x = LOWORD(lParam);
		p.y = HIWORD(lParam);

		if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
		{
			m_Mouse.SetMouse(DXIMouseEvent::DEVICESTATUS::WheelUp, p);
		}
		else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
		{
			m_Mouse.SetMouse(DXIMouseEvent::DEVICESTATUS::WheelDown, p);
		}
	}
	return 0;
	case WM_INPUT:
	{
		TCHAR    szTempOutput[256];
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &szData, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[szData];
		if (lpb == NULL)
		{
			return 0;
		}

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &szData, sizeof(RAWINPUTHEADER)) == szData)
		{

			RAWINPUT* raw = (RAWINPUT*)lpb;
			if ((*raw).header.dwType == RIM_TYPEMOUSE)
			{
				p.x = (*raw).data.mouse.lLastX;
				p.y = (*raw).data.mouse.lLastY;
				if (p.x != 0 || p.y != 0)
				{
					auto ds = m_Mouse.IsMiddleDown();
					m_Mouse.SetRawMouse(DXIMouseEvent::DEVICESTATUS::RawMove, p);

					HRESULT  hResult = StringCchPrintfW(szTempOutput, STRSAFE_MAX_CCH,
						TEXT("Mouse: usFlags=%04x ulButtons=%04x usButtonFlags=%04x usButtonData=%04x ulRawButtons=%04x lLastX=%04x lLastY=%04x ulExtraInformation=%04x\r\n"),
						(*raw).data.mouse.usFlags,
						(*raw).data.mouse.ulButtons,
						(*raw).data.mouse.usButtonFlags,
						(*raw).data.mouse.usButtonData,
						(*raw).data.mouse.ulRawButtons,
						(*raw).data.mouse.lLastX,
						(*raw).data.mouse.lLastY,
						(*raw).data.mouse.ulExtraInformation);
					if (FAILED(hResult))
					{
						// TODO: write error handler
					}
					OutputDebugStringW(szTempOutput);
				}
			}

		}
		else
		{
			OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));
		}
		delete lpb;

	}
	return DefWindowProc(m_hWnd, uMsg, wParam, lParam);

	return 0;

	case WM_PAINT:
		if (s_in_sizemove && game)
		{
			(*game).Tick();
		}
		else
		{
			PAINTSTRUCT ps;
			(void)BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
	{
		PostQuitMessage(0);
	}
	return 0;
	case WM_SIZE:
	{
		if (RenderDevice != NULL)
		{
			m_Width = (int)LOWORD(lParam);
			m_Height = (int)HIWORD(lParam);
			if (g_hWnd3D.size() == 1) {
				SetWindowPos(g_hWnd3D[0], NULL,
					4,
					4,
					(m_Width)-4,
					(m_Height)-4,
					SWP_SHOWWINDOW);
				break;
			}
			if (g_hWnd3D.size() == 4) {

				SetWindowPos(g_hWnd3D[0], NULL,
					4,
					4,
					(m_Width - 12) / 2,
					(m_Height - 12) / 2,
					SWP_SHOWWINDOW);

				SetWindowPos(g_hWnd3D[1], NULL,
					(m_Width + 4) / 2,
					4,
					(m_Width - 12) / 2,
					(m_Height - 12) / 2,
					SWP_SHOWWINDOW);
				SetWindowPos(g_hWnd3D[2], NULL,
					4,
					(m_Height + 4) / 2,
					(m_Width - 12) / 2,
					(m_Height - 12) / 2, //3*4
					SWP_SHOWWINDOW);

				SetWindowPos(g_hWnd3D[3], NULL,
					(m_Width + 4) / 2,
					(m_Height + 4) / 2,
					(m_Width - (12)) / 2, // padding *3 
					(m_Height - (12)) / 2,
					SWP_SHOWWINDOW);

			}
			RECT rc;
			GetClientRect(m_hWnd, &rc);
			RenderDevice->OnResize(rc.right - rc.left, rc.bottom - rc.top, true);
		}
		return 0;
	}
	return 0;
	case WM_ACTIVATE:
	{
		//mIsActive = (BOOL)wParam;
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			bIsActive = true;
			//m_Timer.Stop();
		}
		else
		{
			bIsActive = false;
			//m_Timer.Start();
		}
	}
	return 0;

	default:
	{
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}
	return TRUE;
	}
	return FALSE;
}