#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <windows.h>
#include <vector>

template<class WINDOW_TYPE>
class  BaseWindow
{
private:
protected:
	virtual PCWSTR GetWindowName(void) const = 0;
	virtual LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual void SetMultiChildWindows(std::vector<HWND> g_hWnd3D) = 0;
	HWND m_hWnd;
public:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
	{

		WINDOW_TYPE *pThis = NULL;
		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (WINDOW_TYPE*)(*pCreate).lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);

			(*pThis).m_hWnd = hWnd;
		}
		else
		{
			pThis = (WINDOW_TYPE*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		}
		if (pThis)
		{
			return (*pThis).HandleMessage( hWnd, uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}
	BaseWindow(void) :m_hWnd(nullptr){}
	BOOL Create(
		PCWSTR lpClassName,
		PCWSTR lpWindowName,
		DWORD dwStyle,
		DWORD dwExStyle = 0,
		int nWidth = CW_USEDEFAULT,
		int nHeight = CW_USEDEFAULT,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,
		
		HWND hWndParent = 0,
		HMENU hMenu = 0
	)
	{
		WNDCLASS wc = { 0 };

		wc.lpfnWndProc = WINDOW_TYPE::WindowProc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.lpszClassName = lpClassName;
		wc.hbrBackground = CreateSolidBrush(RGB(210, 105, 30));
		RegisterClass(&wc);

		m_hWnd = CreateWindowExW(
			dwExStyle, lpClassName, lpWindowName, dwStyle, x, y,
			nWidth, nHeight, hWndParent, hMenu, GetModuleHandle(NULL), this
		);

		return (m_hWnd ? TRUE : FALSE);
	}

	HWND CreateAndGetWndx(
		PCWSTR lpClassName,
		PCWSTR lpWindowName,
		DWORD dwStyle,
		DWORD dwExStyle = 0,
		int nWidth = CW_USEDEFAULT,
		int nHeight = CW_USEDEFAULT,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,

		HWND hWndParent = 0,
		HMENU hMenu = 0
	)
	{
		WNDCLASS wc = { 0 };

		wc.lpfnWndProc = WINDOW_TYPE::WindowProc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.lpszClassName = lpClassName;
		RegisterClass(&wc);

		m_hWnd = CreateWindowExW(
			dwExStyle, lpClassName, lpWindowName, dwStyle, x, y,
			nWidth, nHeight, hWndParent, hMenu, GetModuleHandle(NULL), this
		);

		return (m_hWnd ? m_hWnd : nullptr);
	}
	HWND CreateAndGetWnd(
		PCWSTR lpClassName,
		PCWSTR lpWindowName,
		DWORD dwStyle,
		DWORD dwExStyle = 0,
		int nWidth = CW_USEDEFAULT,
		int nHeight = CW_USEDEFAULT,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,

		HWND hWndParent = 0,
		HMENU hMenu = 0
	)
	{
		

		m_hWnd = CreateWindowExW(
			dwExStyle, lpClassName, lpWindowName, dwStyle, x, y,
			nWidth, nHeight, hWndParent, hMenu, GetModuleHandle(NULL), this
		);

		return (m_hWnd ? m_hWnd : nullptr);
	}
	HWND GetWindow(void) const {
		return m_hWnd;
	}
};



#endif