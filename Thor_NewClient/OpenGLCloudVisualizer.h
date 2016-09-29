#pragma once
#ifndef _OpenGLCloudVisualizerH_
#define _OpenGLCloudVisualizerH_

//needed to read a PCL Point Cloud
#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>

//needet to run openGL
#include <windows.h>										      // Header File For Windows
#include <gl\gl.h>											      // Header File For The OpenGL32 Library
#include <gl\glu.h>											      // Header File For The GLu32 Library

#define FULLSCREEN_MODE true


class OpenGLCloudVisualizer
{
public:
	OpenGLCloudVisualizer();
	~OpenGLCloudVisualizer();

	typedef struct Vertex
	{
		float x, y, z;
	};

private:
	HGLRC           hRC;									    // Permanent Rendering Context
	HDC             hDC;								        // Private GDI Device Context
	HWND            hWnd;										// Holds Our Window Handle
	HINSTANCE       hInstance;						            // Holds The Instance Of The Application

	bool keys[256];													  // Stan klawiszy
	bool active;							              		  // flaga zminimalizowania, domyœlnie na true
	bool fullscreen ;					                 		  // tryb pe³noekranowy. domyœlnie na true 

	//GLfloat rtri = 45.0f;                                             
	//GLfloat rquad = 30.0f;                                            


};

OpenGLCloudVisualizer::OpenGLCloudVisualizer()
{
	hRC = NULL;
	hDC = NULL;
	hWnd = NULL;
	active = true;
	fullscreen = FULLSCREEN_MODE;
}

OpenGLCloudVisualizer::~OpenGLCloudVisualizer()
{
}



#endif // !_OpenGLCloudVisualizerH_
