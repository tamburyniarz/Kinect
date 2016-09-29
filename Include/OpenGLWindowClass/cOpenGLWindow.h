#ifndef __COPENGLWINDOW_H__
#define __COPENGLWINDOW_H__

#include "cWindow.h"
#include <GL/GL.h>
#include <gl/GLU.h>

class cSaveRestoreRC
{
private:
    HDC   m_oldDC;            
    HGLRC m_oldRC;            
public:
    cSaveRestoreRC(HDC hDC, HGLRC hRC);
    ~cSaveRestoreRC(void);
};

class cOpenGLWindow :public cWindow {
protected:
	virtual INT_PTR CALLBACK xWindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	HGLRC xCreateOpenGLContext(HDC hDC);
	virtual bool xResizeGLScene(int iWidth, int iHeight);
	virtual bool xDrawGLScene();
	virtual bool xInitGLScene();
	virtual bool xDestroyGLScene();
	HDC m_hDC;
	HGLRC m_hRC;
	int m_iWidth;
	int m_iHeight;
public:
	cOpenGLWindow(int iResourceID, char* sWindowName);
	virtual ~cOpenGLWindow();
};


#endif