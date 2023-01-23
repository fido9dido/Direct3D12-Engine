#ifndef RENDERERATTRIBUTES_H
#define RENDERERATTRIBUTES_H


#define MAX_3DHWND 8
#define FRAMEBUFFCOUNT 3
#define THREADCOUNT 1

class RendererAttributes
{
	protected:
		HWND				HandleMainWindow;          // application main window
		std::vector<HWND>	HandleWindowList;//[MAX_3DHWND];  // 3D render window handle
		UINT				NumHandleWindow;          // number of hwnds in array
		UINT				ActiveWindow;       // which one is active (0=main hWnd)
		HINSTANCE			HandleDLL;              // dll module handle
		DWORD				Width;           // screen width
		DWORD				Height;          // screen height
		bool				bWindowed;         // windowed mode?
		std::vector<char>	Adapter;//           Adapter[256];    // graphics adapter name
		bool				bRunning;          // after succesful initialization
		FILE*				Log;              // log file

	private:

		

		
public:
	
	
};
#endif
/*#ifndef RENDERERATTRIBUTES_H
#define RENDERERATTRIBUTES_H


#define MAX_3DHWND 8
#define FRAMEBUFFCOUNT 3
#define THREADCOUNT 1

class RendererAttributes
{
	protected:
		HWND           m_hWndMain;          // application main window
		std::vector<HWND>           m_hWnd;//[MAX_3DHWND];  // 3D render window handle
		UINT           m_nNumhWnd;          // number of hwnds in array
		UINT           m_nActivehWnd;       // which one is active (0=main hWnd)
		HINSTANCE      m_hDLL;              // dll module handle
		DWORD          m_dwWidth;           // screen width
		DWORD          m_dwHeight;          // screen height
		bool           m_bWindowed;         // windowed mode?
		std::vector<char>m_chAdapter;//           m_chAdapter[256];    // graphics adapter name
		bool           m_bRunning;          // after succesful initialization
		FILE          *m_pLog;              // log file

	private:

		

		
public:
	
	
};
#endif
*/