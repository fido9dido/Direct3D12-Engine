
#include "../Header Files/Renderer.h"



Renderer::Renderer(HINSTANCE hInst)
{
	
	this->m_hInst = hInst;
	m_hDLL = nullptr;
	m_pDevice = nullptr;

}
Renderer::~Renderer(void)
{
	
	ShutDownRendererDevice();
	
}//Renderer
HRESULT Renderer::CreateRendererDevice(std::string api)
{
	
	 if (api.compare("Direct3D") == 0)
	{
		 m_hDLL = LoadLibrary(L"DirectXRenderer.dll");
		if (!m_hDLL)
		{
			//GetProcAddress(void);
			MessageBox(nullptr, L"Loading D3D Failed" , L"DX REngine Error", MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
	}
	else if (api.compare("OpenGL") == 0)
	{
		// IMPLEMENT OPENGL
		MessageBox(nullptr, L"OpenGL not implemented yet" , L"DXR Engine Error", MB_OK | MB_ICONERROR);

		return E_FAIL;
	}
	else
	{
		//MessageBox(nullptr, (L"API " + api.c_str + L" not supported"), L"DXR Engine Error" , MB_OK | MB_ICONERROR);
		return E_FAIL;
	}
		CREATERENDERDEVICE pCreateRenderDevice = 0;
		HRESULT hr;
		

		// function pointer to dll's 'CreateRenderDevice' function
		pCreateRenderDevice = (CREATERENDERDEVICE)
			GetProcAddress(m_hDLL, "CreateRenderDevice");
		if(!pCreateRenderDevice)
		{
			return E_FAIL;
		}
	// call DLL Function to create the device(api)
		hr = pCreateRenderDevice(m_hDLL, &m_pDevice);
	if(FAILED(hr))
	{
		MessageBox(nullptr, L"CreateRenderDevice(void) from lib failed.", L"DXR Engine Error", MB_OK | MB_ICONERROR);
		m_hDLL = nullptr;
		return E_FAIL;
	}
	return S_OK;
	
	
}//CreateRendererAPI
void Renderer::ShutDownRendererDevice(void)
{
	SHUTDOWNRENDERDEVICE  pShutDownRendererAPI = 0;
	HRESULT hr;
	if(m_hDLL)
	{
		//pointer to dll function 'ShutDownRenderAPI'ShutdownRenderDevice
		pShutDownRendererAPI = (SHUTDOWNRENDERDEVICE)GetProcAddress(m_hDLL, "ShutdownRenderDevice");
	}
	//CALL dll release function
	if(m_pDevice)
	{
		hr = pShutDownRendererAPI(&m_pDevice);
		if(FAILED(hr))
		{
			m_pDevice = nullptr;
		}
	}
}//ShutDown Release

std::unique_ptr<IRenderDevice>& Renderer::GetRendererDevice(void)
{
	return m_pDevice;
}

LPCWSTR StringtoLPCWSTR(std::string string)
{
	
	std::wstring wString = std::wstring(string.begin(), string.end());
	return wString.c_str();
	long  x = 0;
	_InterlockedIncrement(&x);
}