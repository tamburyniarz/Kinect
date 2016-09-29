//libraries needed to read a PCL Point Cloud
#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>

#pragma comment( lib, "opengl32.lib" ) 
#pragma comment( lib, "glu32.lib" ) 
//#pragma comment( lib, "glaux.lib" ) 

#include <windows.h>										      // Header File For Windows
#include <gl\gl.h>											      // Header File For The OpenGL32 Library
#include <gl\glu.h>											      // Header File For The GLu32 Library
//#include <gl\glaux.h>										      // Header File For The GLaux Library



typedef pcl::PointXYZRGB PointType;

HGLRC           hRC = NULL;									      // Permanent Rendering Context
HDC             hDC = NULL;								          // Private GDI Device Context
HWND            hWnd = NULL;						              // Holds Our Window Handle
HINSTANCE       hInstance;						                  // Holds The Instance Of The Application

bool keys[256];													  // Stan klawiszy
bool active = TRUE;							              		  // flaga zminimalizowania, domy�lnie na true
bool fullscreen = TRUE;					                 		  // tryb pe�noekranowy. domy�lnie na true 

GLfloat rtri = 45.0f;                                                     // K�t obrotu tr�jk�ta ( NOWE )
GLfloat rquad = 30.0f;                                                    // K�t obroty czworok�ta ( NOWE )

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

typedef struct Wierzcholek
{
	float x, y, z;
};

Wierzcholek ZwrocWierzchoilek(float x, float y, float z)
{
	Wierzcholek temp;
	temp.x = x;
	temp.y = y;
	temp.z = z;
	return temp;
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
	if (height == 0)					                		  // zapobiegnij dzieleniu przez zero...
	{
		height = 1;						                		  // ...ustawiaj�c liczb� 1
	}
	glViewport(0, 0, width, height);		                	  // zresetuj pole widzenia
	glMatrixMode(GL_PROJECTION);			                  	  // wybierz macierz projekcji
	glLoadIdentity();						                 	  // zresetuj j�
																  // oblicz perspektyw� dla okna
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);				                	  // wybierz macier modeli
	glLoadIdentity();						                	  // zresetuj j�
}

int InitGL(GLvoid)
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	glClearDepth(1.0f);							                  // ustawienie bufora g�ebi
	glEnable(GL_DEPTH_TEST);					                  // w��czenie testowania g��bi
	glDepthFunc(GL_LEQUAL);					                      // ustawienie typu testowania
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	return TRUE;
}

int DrawGLScene(pcl::PointCloud<PointType>::Ptr &cloud)					                 		  // funkcja rysuj�ca
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);           // czy�� ekran
	glLoadIdentity();                                             // zresetuj macierz modeli

	glTranslatef(0.0f, 0.0f, -7.0f);                             // Porusz 1.5 w lewo i 6.0 wg��b

	glRotatef(rtri, 0.0f, 1.0f, .0f);
	//glRotatef(rquad, 1.0f, 0.0f, .0f);

	// Zresetuj macierz modelowania
	// g�rna �ciana kostki 

	glBegin(GL_POINTS);

	for (size_t i = 0; i < cloud->points.size(); ++i)
	{
		glColor4f(cloud->points[i].r/255.0, cloud->points[i].g/255.0, cloud->points[i].b/255.0,cloud->points[i].a/255.0);
		glVertex3f(cloud->points[i].x, cloud->points[i].y, cloud->points[i].z);
	}

	glEnd();

	rtri += 0.05f;
	rquad -= 0.05f;
	return TRUE;         // Wszytko posz�o ok
}

GLvoid KillGLWindow(GLvoid)                                       // zamknij okno
{
	if (fullscreen)
	{
		ChangeDisplaySettings(NULL, 0);							  // przywr�c rozdzielczo��
		ShowCursor(TRUE);                                         // poka� kursor
	}
	if (hRC)                                                      // mamy kontekst renderu?
	{
		MessageBox(NULL, TEXT("Nie mo�na zwolni� hDC lub hRC!"), TEXT("B��D ZAMYKANIA"), MB_OK | MB_ICONINFORMATION);
	}
	if (!wglDeleteContext(hRC))                                   // Czy mo�emy usun�� kontekst renderu?
	{
		MessageBox(NULL, TEXT("Nie mo�na usun�� kontekstu renderowania"), TEXT("B��D ZAMYKANIA"), MB_OK | MB_ICONINFORMATION);
	}
	hRC = NULL;

	if (hDC && !ReleaseDC(hWnd, hDC))                             // Czy mo�emy zwolni� hDC
	{
		MessageBox(NULL, TEXT("Nie mo�na zwolni� kontekstu urz�dzenia (DC)"), TEXT("B��D ZAMYKANIA"), MB_OK | MB_ICONINFORMATION);
		hDC = NULL;                                               // Ustawiamy DC na NULL
	}

	if (hWnd && !DestroyWindow(hWnd))                             // Czy mo�emy zwolni� uchwyt okna?
	{
		MessageBox(NULL, TEXT("Nie mo�na zwolni� hWnd"), TEXT("B��D ZAMYKANIA"), MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;                                              // Ustaw hWnd na Null
	}

	if (!UnregisterClass(TEXT("OpenGL"), hInstance))              // Czy mo�emy wyrejstrowa� klas� okna?
	{
		MessageBox(NULL, TEXT("Nie mo�na wyrejstrowa� klasy okna"), TEXT("B��D ZAMYKANIA"), MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;                                         // Ustawiamy instancj� na NULL
	}
}

bool CreateGLWindow(LPCSTR title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint PixelFormat;                                           // B�dzie przechowywa� znaleziony format piksela 
	WNDCLASS wc;                                                  // Struktura klasy okna 
	DWORD dwExStyle;                                              // Rozszerzony styl okna
	DWORD dwStyle;                                                // Normalny styl okna
	RECT WindowRect;                                              // Tu b�d� rozmiary okna ;)
	WindowRect.left = (long)0;                                    // Pocz�tek szeroko�ci (od lewej) ma 0
	WindowRect.right = (long)width;                               // Szeroko�c bierzemy z parametru naszej funkcji
	WindowRect.top = (long)0;                                     // Wysoko�� te� zaczynamy od 0 (od g�ry)
	WindowRect.bottom = (long)height;                             // Ustawiamy wysoko�� z parametru naszej funkcji
	fullscreen = fullscreenflag;                                  // Ustawiamy globaln� zmienn� 
	hInstance = GetModuleHandle(NULL);                            // Pobieramy instancj� dla okna
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;                // Ustawiamy odmalowywanie
	wc.lpfnWndProc = (WNDPROC)WndProc;                            // WndProc b�dzie obs�ugiwa� komunikaty
	wc.cbClsExtra = 0;                                            // nie wa�ne
	wc.cbWndExtra = 0;                                            // nie wa�ne
	wc.hInstance = hInstance;                                     // Ustawiamy instancj�
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);                       // Ikona domy�lna
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);                     // Kursor - strza�ka
	wc.hbrBackground = NULL;                                      // T�o nie jest wa�ne w OpenGL
	wc.lpszMenuName = NULL;                                       // Nie chcemy menu
	wc.lpszClassName = TEXT("OpenGL");                            // Nazwa klasy okna

	if (!RegisterClass(&wc))                                      // Spr�buj zarejstrowa� klas� okna
	{
		MessageBox(NULL, TEXT("Nie uda�o si� zarejstrowa� klasy okna"), TEXT("B��D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zako�cz i zwr�� fa�sz.
	}

	if (fullscreen)                                               // Czy ma by� pe�ny ekran ?
	{
		DEVMODE dmScreenSettings;                                 // Tryb karty graficznej
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));   // Wyczy�� pami��
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);       // Ustaw rozmiar tej struktury
		dmScreenSettings.dmPelsWidth = width;                     // Wybie� ��dan� szeroko��
		dmScreenSettings.dmPelsHeight = height;                   // Wybierz ��dan� wysoko��
		dmScreenSettings.dmBitsPerPel = bits;                     // Wybierz g��bie kolor�w
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		// Spr�buj ustawi� pe�ny ekran. CDS_FULLSCREEN usuwa pasek start.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// Je�li si� nie uda, przejd� do okna lub zako�cz program
			if (MessageBox(NULL, TEXT("Tryb graficzny nie jest obs�ugiwany przez twoj� kart� graf. Czy u�y� zamiast niego okna?"), TEXT("Okno OpenGL"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;                               // Tryb okienkowy. fullscreen na false.
			}
			else
			{
				// Uwaga o zamykaniu
				MessageBox(NULL, TEXT("Program teraz si� zamknie."), TEXT("B��D"), MB_OK | MB_ICONSTOP);
				return FALSE;                                     // Zako�cz i zwr�� false
			}
		}
	}

	if (fullscreen)                                               // Wci�� jeste�my w trybie pe�no ekranowym
	{
		dwExStyle = WS_EX_APPWINDOW;                              // Rozszerzony styl okna
		dwStyle = WS_POPUP;                                       // Styl okna
		ShowCursor(FALSE);                                        // Ukryj kursor myszy
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;           // Rozszerzony styl okna
		dwStyle = WS_OVERLAPPEDWINDOW;                            // Styl okna
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);   // przy fullsreen nie robi nic



	if (!(hWnd = CreateWindowEx(dwExStyle,                        // Rozszerzony styl dla okna
		TEXT("OpenGL"),                                           // Nazwa klasy
		title,                                                    // Tytu� okna
		WS_CLIPSIBLINGS |                                         // Wymagane style okna
		WS_CLIPCHILDREN |                                         // Wymagane style okna
		dwStyle,                                                  // Wybrane style okna
		0, 0,                                                     // Pozycja okna
		WindowRect.right - WindowRect.left,                       // Szeroko��
		WindowRect.bottom - WindowRect.top,                       // Wysoko��
		NULL,                                                     // Nie u�ywamy okna potomnego
		NULL,                                                     // �adnego menu
		hInstance,                                                // Instancja
		NULL)))                                                   // Nie dawaj nic do WM_CREATE
	{
		KillGLWindow();                                           // Zresetuj tryb ekranu
		MessageBox(NULL, TEXT("Nie mo�na stworzy� okna."), TEXT("B��D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zwr�� false
	}

	static PIXELFORMATDESCRIPTOR pfd =                            // pfd m�wi oknu co chcemy
	{
		sizeof(PIXELFORMATDESCRIPTOR),                            // Rozmiar opisu piksela
		1,                                                        // Numer wersji
		PFD_DRAW_TO_WINDOW |                                      // Format musi obs�ugiwa� okno
		PFD_SUPPORT_OPENGL |                                      // Format musi obs�ugiwa� OpenGL
		PFD_DOUBLEBUFFER,                                         // Musi obs�ugiwa� Podw�jne buforowanie
		PFD_TYPE_RGBA,                                            // i format RGBA
		bits,                                                     // Wybieramy g��bie kolor�w
		0, 0, 0, 0, 0, 0,                                         // ignorujemy
		0,                                                        // Bez bufora alpha
		0,                                                        // Bit ignorujemy
		0,                                                        // ignorujemy
		0, 0, 0, 0,                                               // ignorujemy
		16,                                                       // 16 bitowy bufor Z
		0,                                                        // ignorujemy
		0,                                                        // ignorujemy
		PFD_MAIN_PLANE,                                           // G��wna warstwa rysowania
		0,                                                        // zarezerwowane
		0, 0, 0                                                   // ignorujemy maski warstw
	};

	if (!(hDC = GetDC(hWnd)))                                     // Mamy kontekst urz�dzenia?
	{
		KillGLWindow();                                           // Resetujemy ekran
		MessageBox(NULL, TEXT("Nie mo�na stworzy� kontekstu urz�dzenia."), TEXT("B��D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zwracamy false
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))            // Czy windows znajdzie taki format pixela?
	{
		KillGLWindow();                                           // Resetujemy ekran
		MessageBox(NULL, TEXT("Nie mo�na znale�� ��danego formatu piksela."), TEXT("B��D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zwracamy false
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))                  // Czy mo�emy ustawi� taki format
	{
		KillGLWindow();                                           // Resetujemy ekran
		MessageBox(NULL, TEXT("Nie mo�na ustawi� ��danego formatu piksela."), TEXT("B��D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zwracamy false
	}

	if (!(hRC = wglCreateContext(hDC)))                           // Czy mo�emy pobra� hRC
	{
		KillGLWindow();                                           // Resetujemy ekran
		MessageBox(NULL, TEXT("Nie mo�na stworzy� kontekstu renderowania."), TEXT("B��D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zwracamy false
	}

	if (!wglMakeCurrent(hDC, hRC))                                // Czy mo�emy aktywowa� kontekst renderowania?
	{
		KillGLWindow();                                           // Resetujemy ekran
		MessageBox(NULL, TEXT("Nie mo�na aktywowa� kontekstu renderowania."), TEXT("B��D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zwracamy false
	}

	ShowWindow(hWnd, SW_SHOW);                                    // Pokazujemy okno
	SetForegroundWindow(hWnd);                                    // Ustawiamy wy�szy priorytet
	SetFocus(hWnd);                                               // Dzia�anie klawiatury skierowujemy na okno
	ReSizeGLScene(width, height);                                 // Ustawiamy perspektyw�

	if (!InitGL())                                                // Czy GL zanicjowa� si� ?
	{
		KillGLWindow();                                           // Resetujemy ekran
		MessageBox(NULL, TEXT("Inicjacja niepomy�lna."), TEXT("B��D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zwracamy false
	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)                                                // Sprawd� komunikaty okna
	{
	case WM_ACTIVATE:                                        // Czy to wiadomo�� aktywowania?
	{
		if (!HIWORD(wParam))                                 // Czy program jest aktywowany
		{
			active = TRUE;                                   // Program jest aktywny
		}
		else
		{
			active = FALSE;                                  // Program nie jest aktywny
		}
		return 0;                                            // Powr�� do p�tli wiadomo�ci
	}
	case WM_SYSCOMMAND:                                      // Czy to komenda systemowa?
	{
		switch (wParam)                                      // Sprawdzimy typ
		{
		case SC_SCREENSAVE:                                  // Zgaszacz ekranu chce si� w��czy�
		case SC_MONITORPOWER:                                // Monitor chce si� wy��czy�
			return 0;                                        // Anulujemy wygaszacze itp.
		}
		break;                                               // koniec
	}
	case WM_CLOSE:                                           // Czy to rozkaz zamkni�cia?
	{
		PostQuitMessage(0);                                  // Wy�lij wiadomo�� zamkni�cia
		return 0;                                            // skocz dalej
	}
	case WM_KEYDOWN:                                         // Czy klawisz zosta� wci�ni�ty
	{
		keys[wParam] = TRUE;                                 // Odpowiednie pole zostaje ustawione
		return 0;                                            // skocz dalej
	}
	case WM_KEYUP:                                           // Czy klawisz zosta� wci�ni�ty
	{
		keys[wParam] = FALSE;                                // Odpowiednie pole zostaje ustawione na false
		return 0;                                            // skocz dalej
	}
	case WM_SIZE:                                            // Czy okno si� zmieni�o ?
	{
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));       // Zmieniamy scene OpenGL
		return 0;                                            // skocz dalej
	}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;                                                     // Struktura przechowuj�ca komunikaty okna
	BOOL done = FALSE;                                           // Stan dzia�ania programu
																 // Zapytaj o tryb ekranu

	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGB>);
	//frame_after_decopresion
	if (pcl::io::loadPCDFile<pcl::PointXYZRGB>("act_frame_5.pcd", *cloud) == -1) //* load the file
	{
		PCL_ERROR("Couldn't read file common.pcd \n");
		getchar();
		return (-1);
	}


	if (MessageBox(NULL, TEXT("Czy chcesz by� w pe�nym ekranie?"), TEXT("Start?"), MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		fullscreen = FALSE;                                      // tryb okienkowym
	}

	// Stw�rz okno OpenGL
	if (!CreateGLWindow(TEXT("Pierwsze okno w OpenGL!"), 1366, 768, 16, fullscreen))
	{
		return 0;                                                // zako�cz program
	}

	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))            // czy s� jakie� wiadomo�ci ?
		{
			if (msg.message == WM_QUIT)                          // czy otrzymano wiadomo�� zamkni�cia ?
			{
				done = TRUE;                                     // skoro tak, to done=TRUE
			}
			else                                                 // nie otrzymano wiadomo�ci zamkni�cia ?
			{
				TranslateMessage(&msg);                          // wyt�umacz wiadomo��
				DispatchMessage(&msg);                           // wy�lij j�
			}
		}
		else                                                     // nie ma �adnych komunikat�w
		{
			// Rysuj scen� OpenGL
			if (active)                                          // program jest aktywny ?
			{
				if (keys[VK_ESCAPE])                             // czy wci�ni�ty jest ESC ?
				{
					done = TRUE;                                 // przerwanie warunku p�tli
				}
				else                                             // nie ma czasu na zamkni�cie, rysujemy scene
				{
					DrawGLScene(cloud);                               // Rysuj scen�
					SwapBuffers(hDC);                            // Zamie� bufory (ekrany)
				}
			}
			if (keys[VK_F1])                                     // czy F1 jest wci�ni�te
			{
				keys[VK_F1] = FALSE;                             // ustaw go na false, bo zosta� u�yty
				KillGLWindow();                                  // Zamknij okno
				fullscreen = !fullscreen;                        // Zamie� pe�ny ekran)
																 // Stw�rz nowe okno
				if (!CreateGLWindow(TEXT("CO SIE ODWALA?!"), 1366, 768, 16, fullscreen))
				{
					return 0;                                    // Wyst�pi� b��d
				}
			}
		}
	}
	KillGLWindow();                                              // Zamknij OpenGL
	return (msg.wParam);                                         // Koniec programu
}