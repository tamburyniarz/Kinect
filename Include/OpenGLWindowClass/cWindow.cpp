#include "cWindow.h"

INT_PTR CALLBACK cWindow::WindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    cWindow* pcWin = reinterpret_cast<cWindow*>(GetWindowLongPtr(hDlg, GWLP_USERDATA));
    if(pcWin) return pcWin->xWindowProc(hDlg,message,wParam,lParam);
    return FALSE;
}

INT_PTR CALLBACK cWindow::xWindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CLOSE:
		//DestroyWindow(hDlg);
		PostQuitMessage(0);
		return TRUE;
	}
    return FALSE;
}

cWindow::cWindow(int iResourceID, char* sWindowName) {

	m_hWnd = CreateDialog(GetModuleHandle(0), MAKEINTRESOURCE(iResourceID), 0, WindowProc);

	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	SetWindowTextA(m_hWnd, sWindowName);
}


void cWindow::Show() {
    ShowWindow(m_hWnd,SW_SHOW);
}

void cWindow::ShowMaximized() {
	ShowWindow(m_hWnd, SW_MAXIMIZE);
}

