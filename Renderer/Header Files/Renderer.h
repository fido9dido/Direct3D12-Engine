
#ifndef RENDERER_H
#define RENDERER_H

#include <string>


#include <memory>
#include "IRenderDevice.h"

class Renderer 
{
private:
	
	std::unique_ptr<IRenderDevice> m_pDevice;
	HINSTANCE m_hInst;
	HMODULE m_hDLL;
public:

	Renderer(HINSTANCE instanceHandle);
	~Renderer(void);

	HRESULT CreateRendererDevice(std::string m_GraphicAdapterName);
	void ShutDownRendererDevice(void);
	std::unique_ptr<IRenderDevice>& GetRendererDevice(void);
	HINSTANCE GetModule(void) const { return m_hDLL; }
	LPCWSTR StringtoLPCWSTR(std::string string);
};
typedef class std::unique_ptr<Renderer> ULPRenderer;
#endif
