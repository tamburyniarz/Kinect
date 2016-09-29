#ifndef __CWINDOW_H__
#define __CWINDOW_H__

#include <stdio.h>
#include <Windows.h>


class cWindow {
protected:
    virtual INT_PTR CALLBACK xWindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	HWND m_hWnd;
public:
    static INT_PTR CALLBACK WindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	cWindow(int iResourceID, char* sWindowName);
	virtual ~cWindow() {};
    void Show();
	void ShowMaximized();
};

#endif