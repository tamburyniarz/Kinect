#include "System_Client.h"
#include "demain.h"
#pragma comment( lib, "opengl32.lib" ) 
#pragma comment( lib, "glu32.lib" )
#pragma comment(lib, "ws2_32.lib") //Winsock Librar



int main(int argc, char* argv[])
{

	ThorClientSystem *stream_visualiser = new ThorClientSystem(PUTV4_YUV_PREVIEW_WINDOW, "Client");
	stream_visualiser->RunSystem();
	stream_visualiser->Show();
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}