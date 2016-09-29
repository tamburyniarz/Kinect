#include "cOpenGLWindow.h"

cSaveRestoreRC::cSaveRestoreRC(HDC hDC, HGLRC hRC) {
	//ASSERT( hDC );
	//ASSERT( hRC );

	m_oldDC = wglGetCurrentDC();
	m_oldRC = wglGetCurrentContext();

	BOOL result = wglMakeCurrent(hDC, hRC);
	//ASSERT( result );
}

cSaveRestoreRC::~cSaveRestoreRC(void)
{
	if (!m_oldRC) return;
	//ASSERT( oldDC );
	BOOL result = wglMakeCurrent(m_oldDC, m_oldRC);
	//ASSERT( result );    
}

INT_PTR CALLBACK cOpenGLWindow::xWindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_SIZE:
		xResizeGLScene(LOWORD(lParam), HIWORD(lParam));
		return xDrawGLScene();
	case WM_PAINT:
		return xDrawGLScene();	
	}
	return cWindow::xWindowProc(hDlg, message, wParam, lParam);
}

bool cOpenGLWindow::xResizeGLScene(int iWidth, int iHeight) {
	cSaveRestoreRC c(m_hDC, m_hRC);

	m_iWidth = iWidth;
	m_iHeight = iHeight;

	if (iHeight <= 0) iHeight = 1;
	glViewport(0, 0, iWidth, iHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(0, iWidth, iHeight, 0, -10, 10);
	gluPerspective(90.0f, (GLfloat)iWidth / (GLfloat)iHeight, 0.1f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	return TRUE;
}

bool cOpenGLWindow::xDrawGLScene() {
	cSaveRestoreRC c(m_hDC, m_hRC);
//	SwapBuffers(m_hDC);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
//	glEnable(GL_TEXTURE_2D);
//	glShadeModel(GL_FLAT);

//	glBindTexture(GL_TEXTURE_2D, m_uiTexture);

	glBegin(GL_QUADS);
	glColor3d(0,1,0);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3d(0, 0.0, -10.0);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3d(1.0, 0.0, -10.0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3d(1.0, 1.0, -4.0);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3d(0, 1.0, -4.0);
	glEnd();

	SwapBuffers(m_hDC);
	return TRUE;
}

HGLRC cOpenGLWindow::xCreateOpenGLContext(HDC hDC) {
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), // size of this pfd 
		1, // version number 
		PFD_DRAW_TO_WINDOW | // support window 
		PFD_SUPPORT_OPENGL | // support OpenGL 
		PFD_DOUBLEBUFFER, // double buffered 
		PFD_TYPE_RGBA, // RGBA type 
		24, // 24-bit color depth 
		0, 0, 0, 0, 0, 0, // color bits ignored 
		0, // no alpha buffer 
		0, // shift bit ignored 
		0, // no accumulation buffer 
		0, 0, 0, 0, // accum bits ignored 
		32, // 32-bit z-buffer 
		0, // no stencil buffer 
		0, // no auxiliary buffer 
		PFD_MAIN_PLANE, // main layer 
		0, // reserved 
		0, 0, 0 // layer masks ignored 
	};

	int iPixelFormat;

	if ((iPixelFormat = ChoosePixelFormat(hDC, &pfd)) == 0) {
		printf("Error: Cannot Choose Right Pixel Type For OpenGL");
		return 0;
	}

	if (SetPixelFormat(hDC, iPixelFormat, &pfd) == FALSE) {
		printf("Error: Cannot Set Right Pixel Type For OpenGL");
		return 0;
	}

	return wglCreateContext(hDC);
}

cOpenGLWindow::cOpenGLWindow(int iResourceID, char* sWindowName) : cWindow(iResourceID, sWindowName) {

	m_hDC = GetDC(m_hWnd);
	m_hRC = xCreateOpenGLContext(m_hDC);

	xInitGLScene();
}

cOpenGLWindow::~cOpenGLWindow() {
	xDestroyGLScene();

	wglDeleteContext(m_hRC); //?
	ReleaseDC(m_hWnd, m_hDC); //?
}

bool cOpenGLWindow::xInitGLScene() {
	cSaveRestoreRC c(m_hDC, m_hRC);

	return TRUE;
}

bool cOpenGLWindow::xDestroyGLScene() {
	cSaveRestoreRC c(m_hDC, m_hRC);

	return TRUE;
}

