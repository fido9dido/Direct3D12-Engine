#include <Windows.h>

void AddItem(HWND hWnd, char* ch, void* pData) {
	WPARAM nI = (WPARAM)((int)(DWORD)SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)ch));
	SendMessage(hWnd, CB_SETITEMDATA, nI, (LPARAM)pData);
}
