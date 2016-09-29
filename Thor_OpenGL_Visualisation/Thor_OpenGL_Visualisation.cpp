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
bool active = TRUE;							              		  // flaga zminimalizowania, domyœlnie na true
bool fullscreen = TRUE;					                 		  // tryb pe³noekranowy. domyœlnie na true 

GLfloat rtri = 45.0f;                                                     // K¹t obrotu trójk¹ta ( NOWE )
GLfloat rquad = 30.0f;                                                    // K¹t obroty czworok¹ta ( NOWE )

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
		height = 1;						                		  // ...ustawiaj¹c liczbê 1
	}
	glViewport(0, 0, width, height);		                	  // zresetuj pole widzenia
	glMatrixMode(GL_PROJECTION);			                  	  // wybierz macierz projekcji
	glLoadIdentity();						                 	  // zresetuj j¹
																  // oblicz perspektywê dla okna
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);				                	  // wybierz macier modeli
	glLoadIdentity();						                	  // zresetuj j¹
}

int InitGL(GLvoid)
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	glClearDepth(1.0f);							                  // ustawienie bufora g³ebi
	glEnable(GL_DEPTH_TEST);					                  // w³¹czenie testowania g³êbi
	glDepthFunc(GL_LEQUAL);					                      // ustawienie typu testowania
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	return TRUE;
}

int DrawGLScene(pcl::PointCloud<PointType>::Ptr &cloud)					                 		  // funkcja rysuj¹ca
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);           // czyœæ ekran
	glLoadIdentity();                                             // zresetuj macierz modeli

	glTranslatef(0.0f, 0.0f, -7.0f);                             // Porusz 1.5 w lewo i 6.0 wg³¹b

	glRotatef(rtri, 0.0f, 1.0f, .0f);
	//glRotatef(rquad, 1.0f, 0.0f, .0f);

	// Zresetuj macierz modelowania
	// górna œciana kostki 

	glBegin(GL_POINTS);

	for (size_t i = 0; i < cloud->points.size(); ++i)
	{
		glColor4f(cloud->points[i].r/255.0, cloud->points[i].g/255.0, cloud->points[i].b/255.0,cloud->points[i].a/255.0);
		glVertex3f(cloud->points[i].x, cloud->points[i].y, cloud->points[i].z);
	}

	glEnd();

	rtri += 0.05f;
	rquad -= 0.05f;
	return TRUE;         // Wszytko posz³o ok
}

GLvoid KillGLWindow(GLvoid)                                       // zamknij okno
{
	if (fullscreen)
	{
		ChangeDisplaySettings(NULL, 0);							  // przywróc rozdzielczoœæ
		ShowCursor(TRUE);                                         // poka¿ kursor
	}
	if (hRC)                                                      // mamy kontekst renderu?
	{
		MessageBox(NULL, TEXT("Nie mo¿na zwolniæ hDC lub hRC!"), TEXT("B£¥D ZAMYKANIA"), MB_OK | MB_ICONINFORMATION);
	}
	if (!wglDeleteContext(hRC))                                   // Czy mo¿emy usun¹æ kontekst renderu?
	{
		MessageBox(NULL, TEXT("Nie mo¿na usun¹æ kontekstu renderowania"), TEXT("B£¥D ZAMYKANIA"), MB_OK | MB_ICONINFORMATION);
	}
	hRC = NULL;

	if (hDC && !ReleaseDC(hWnd, hDC))                             // Czy mo¿emy zwolniæ hDC
	{
		MessageBox(NULL, TEXT("Nie mo¿na zwolniæ kontekstu urz¹dzenia (DC)"), TEXT("B£¥D ZAMYKANIA"), MB_OK | MB_ICONINFORMATION);
		hDC = NULL;                                               // Ustawiamy DC na NULL
	}

	if (hWnd && !DestroyWindow(hWnd))                             // Czy mo¿emy zwolniæ uchwyt okna?
	{
		MessageBox(NULL, TEXT("Nie mo¿na zwolniæ hWnd"), TEXT("B£¥D ZAMYKANIA"), MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;                                              // Ustaw hWnd na Null
	}

	if (!UnregisterClass(TEXT("OpenGL"), hInstance))              // Czy mo¿emy wyrejstrowaæ klasê okna?
	{
		MessageBox(NULL, TEXT("Nie mo¿na wyrejstrowaæ klasy okna"), TEXT("B£¥D ZAMYKANIA"), MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;                                         // Ustawiamy instancjê na NULL
	}
}

bool CreateGLWindow(LPCSTR title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint PixelFormat;                                           // Bêdzie przechowywaæ znaleziony format piksela 
	WNDCLASS wc;                                                  // Struktura klasy okna 
	DWORD dwExStyle;                                              // Rozszerzony styl okna
	DWORD dwStyle;                                                // Normalny styl okna
	RECT WindowRect;                                              // Tu bêd¹ rozmiary okna ;)
	WindowRect.left = (long)0;                                    // Pocz¹tek szerokoœci (od lewej) ma 0
	WindowRect.right = (long)width;                               // Szerokoœc bierzemy z parametru naszej funkcji
	WindowRect.top = (long)0;                                     // Wysokoœæ te¿ zaczynamy od 0 (od góry)
	WindowRect.bottom = (long)height;                             // Ustawiamy wysokoœæ z parametru naszej funkcji
	fullscreen = fullscreenflag;                                  // Ustawiamy globaln¹ zmienn¹ 
	hInstance = GetModuleHandle(NULL);                            // Pobieramy instancjê dla okna
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;                // Ustawiamy odmalowywanie
	wc.lpfnWndProc = (WNDPROC)WndProc;                            // WndProc bêdzie obs³ugiwaæ komunikaty
	wc.cbClsExtra = 0;                                            // nie wa¿ne
	wc.cbWndExtra = 0;                                            // nie wa¿ne
	wc.hInstance = hInstance;                                     // Ustawiamy instancjê
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);                       // Ikona domyœlna
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);                     // Kursor - strza³ka
	wc.hbrBackground = NULL;                                      // T³o nie jest wa¿ne w OpenGL
	wc.lpszMenuName = NULL;                                       // Nie chcemy menu
	wc.lpszClassName = TEXT("OpenGL");                            // Nazwa klasy okna

	if (!RegisterClass(&wc))                                      // Spróbuj zarejstrowaæ klasê okna
	{
		MessageBox(NULL, TEXT("Nie uda³o siê zarejstrowaæ klasy okna"), TEXT("B£¥D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zakoñcz i zwróæ fa³sz.
	}

	if (fullscreen)                                               // Czy ma byæ pe³ny ekran ?
	{
		DEVMODE dmScreenSettings;                                 // Tryb karty graficznej
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));   // Wyczyœæ pamiêæ
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);       // Ustaw rozmiar tej struktury
		dmScreenSettings.dmPelsWidth = width;                     // Wybie¿ ¿¹dan¹ szerokoœæ
		dmScreenSettings.dmPelsHeight = height;                   // Wybierz ¿¹dan¹ wysokoœæ
		dmScreenSettings.dmBitsPerPel = bits;                     // Wybierz g³êbie kolorów
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		// Spróbuj ustawiæ pe³ny ekran. CDS_FULLSCREEN usuwa pasek start.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// Jeœli siê nie uda, przejdŸ do okna lub zakoñcz program
			if (MessageBox(NULL, TEXT("Tryb graficzny nie jest obs³ugiwany przez twoj¹ kartê graf. Czy u¿yæ zamiast niego okna?"), TEXT("Okno OpenGL"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;                               // Tryb okienkowy. fullscreen na false.
			}
			else
			{
				// Uwaga o zamykaniu
				MessageBox(NULL, TEXT("Program teraz siê zamknie."), TEXT("B£¥D"), MB_OK | MB_ICONSTOP);
				return FALSE;                                     // Zakoñcz i zwróæ false
			}
		}
	}

	if (fullscreen)                                               // Wci¹¿ jesteœmy w trybie pe³no ekranowym
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
		title,                                                    // Tytu³ okna
		WS_CLIPSIBLINGS |                                         // Wymagane style okna
		WS_CLIPCHILDREN |                                         // Wymagane style okna
		dwStyle,                                                  // Wybrane style okna
		0, 0,                                                     // Pozycja okna
		WindowRect.right - WindowRect.left,                       // Szerokoœæ
		WindowRect.bottom - WindowRect.top,                       // Wysokoœæ
		NULL,                                                     // Nie u¿ywamy okna potomnego
		NULL,                                                     // ¯adnego menu
		hInstance,                                                // Instancja
		NULL)))                                                   // Nie dawaj nic do WM_CREATE
	{
		KillGLWindow();                                           // Zresetuj tryb ekranu
		MessageBox(NULL, TEXT("Nie mo¿na stworzyæ okna."), TEXT("B£¥D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zwróæ false
	}

	static PIXELFORMATDESCRIPTOR pfd =                            // pfd mówi oknu co chcemy
	{
		sizeof(PIXELFORMATDESCRIPTOR),                            // Rozmiar opisu piksela
		1,                                                        // Numer wersji
		PFD_DRAW_TO_WINDOW |                                      // Format musi obs³ugiwaæ okno
		PFD_SUPPORT_OPENGL |                                      // Format musi obs³ugiwaæ OpenGL
		PFD_DOUBLEBUFFER,                                         // Musi obs³ugiwaæ Podwójne buforowanie
		PFD_TYPE_RGBA,                                            // i format RGBA
		bits,                                                     // Wybieramy g³êbie kolorów
		0, 0, 0, 0, 0, 0,                                         // ignorujemy
		0,                                                        // Bez bufora alpha
		0,                                                        // Bit ignorujemy
		0,                                                        // ignorujemy
		0, 0, 0, 0,                                               // ignorujemy
		16,                                                       // 16 bitowy bufor Z
		0,                                                        // ignorujemy
		0,                                                        // ignorujemy
		PFD_MAIN_PLANE,                                           // G³ówna warstwa rysowania
		0,                                                        // zarezerwowane
		0, 0, 0                                                   // ignorujemy maski warstw
	};

	if (!(hDC = GetDC(hWnd)))                                     // Mamy kontekst urz¹dzenia?
	{
		KillGLWindow();                                           // Resetujemy ekran
		MessageBox(NULL, TEXT("Nie mo¿na stworzyæ kontekstu urz¹dzenia."), TEXT("B£¥D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zwracamy false
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))            // Czy windows znajdzie taki format pixela?
	{
		KillGLWindow();                                           // Resetujemy ekran
		MessageBox(NULL, TEXT("Nie mo¿na znaleŸæ ¿¹danego formatu piksela."), TEXT("B£¥D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zwracamy false
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))                  // Czy mo¿emy ustawiæ taki format
	{
		KillGLWindow();                                           // Resetujemy ekran
		MessageBox(NULL, TEXT("Nie mo¿na ustawiæ ¿¹danego formatu piksela."), TEXT("B£¥D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zwracamy false
	}

	if (!(hRC = wglCreateContext(hDC)))                           // Czy mo¿emy pobraæ hRC
	{
		KillGLWindow();                                           // Resetujemy ekran
		MessageBox(NULL, TEXT("Nie mo¿na stworzyæ kontekstu renderowania."), TEXT("B£¥D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zwracamy false
	}

	if (!wglMakeCurrent(hDC, hRC))                                // Czy mo¿emy aktywowaæ kontekst renderowania?
	{
		KillGLWindow();                                           // Resetujemy ekran
		MessageBox(NULL, TEXT("Nie mo¿na aktywowaæ kontekstu renderowania."), TEXT("B£¥D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zwracamy false
	}

	ShowWindow(hWnd, SW_SHOW);                                    // Pokazujemy okno
	SetForegroundWindow(hWnd);                                    // Ustawiamy wy¿szy priorytet
	SetFocus(hWnd);                                               // Dzia³anie klawiatury skierowujemy na okno
	ReSizeGLScene(width, height);                                 // Ustawiamy perspektywê

	if (!InitGL())                                                // Czy GL zanicjowa³ siê ?
	{
		KillGLWindow();                                           // Resetujemy ekran
		MessageBox(NULL, TEXT("Inicjacja niepomyœlna."), TEXT("B£¥D"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;                                             // zwracamy false
	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)                                                // SprawdŸ komunikaty okna
	{
	case WM_ACTIVATE:                                        // Czy to wiadomoœæ aktywowania?
	{
		if (!HIWORD(wParam))                                 // Czy program jest aktywowany
		{
			active = TRUE;                                   // Program jest aktywny
		}
		else
		{
			active = FALSE;                                  // Program nie jest aktywny
		}
		return 0;                                            // Powróæ do pêtli wiadomoœci
	}
	case WM_SYSCOMMAND:                                      // Czy to komenda systemowa?
	{
		switch (wParam)                                      // Sprawdzimy typ
		{
		case SC_SCREENSAVE:                                  // Zgaszacz ekranu chce siê w³¹czyæ
		case SC_MONITORPOWER:                                // Monitor chce siê wy³¹czyæ
			return 0;                                        // Anulujemy wygaszacze itp.
		}
		break;                                               // koniec
	}
	case WM_CLOSE:                                           // Czy to rozkaz zamkniêcia?
	{
		PostQuitMessage(0);                                  // Wyœlij wiadomoœæ zamkniêcia
		return 0;                                            // skocz dalej
	}
	case WM_KEYDOWN:                                         // Czy klawisz zosta³ wciœniêty
	{
		keys[wParam] = TRUE;                                 // Odpowiednie pole zostaje ustawione
		return 0;                                            // skocz dalej
	}
	case WM_KEYUP:                                           // Czy klawisz zosta³ wciœniêty
	{
		keys[wParam] = FALSE;                                // Odpowiednie pole zostaje ustawione na false
		return 0;                                            // skocz dalej
	}
	case WM_SIZE:                                            // Czy okno siê zmieni³o ?
	{
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));       // Zmieniamy scene OpenGL
		return 0;                                            // skocz dalej
	}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;                                                     // Struktura przechowuj¹ca komunikaty okna
	BOOL done = FALSE;                                           // Stan dzia³ania programu
																 // Zapytaj o tryb ekranu

	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGB>);
	//frame_after_decopresion
	if (pcl::io::loadPCDFile<pcl::PointXYZRGB>("act_frame_5.pcd", *cloud) == -1) //* load the file
	{
		PCL_ERROR("Couldn't read file common.pcd \n");
		getchar();
		return (-1);
	}


	if (MessageBox(NULL, TEXT("Czy chcesz byæ w pe³nym ekranie?"), TEXT("Start?"), MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		fullscreen = FALSE;                                      // tryb okienkowym
	}

	// Stwórz okno OpenGL
	if (!CreateGLWindow(TEXT("Pierwsze okno w OpenGL!"), 1366, 768, 16, fullscreen))
	{
		return 0;                                                // zakoñcz program
	}

	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))            // czy s¹ jakieœ wiadomoœci ?
		{
			if (msg.message == WM_QUIT)                          // czy otrzymano wiadomoœæ zamkniêcia ?
			{
				done = TRUE;                                     // skoro tak, to done=TRUE
			}
			else                                                 // nie otrzymano wiadomoœci zamkniêcia ?
			{
				TranslateMessage(&msg);                          // wyt³umacz wiadomoœæ
				DispatchMessage(&msg);                           // wyœlij j¹
			}
		}
		else                                                     // nie ma ¿adnych komunikatów
		{
			// Rysuj scenê OpenGL
			if (active)                                          // program jest aktywny ?
			{
				if (keys[VK_ESCAPE])                             // czy wciœniêty jest ESC ?
				{
					done = TRUE;                                 // przerwanie warunku pêtli
				}
				else                                             // nie ma czasu na zamkniêcie, rysujemy scene
				{
					DrawGLScene(cloud);                               // Rysuj scenê
					SwapBuffers(hDC);                            // Zamieñ bufory (ekrany)
				}
			}
			if (keys[VK_F1])                                     // czy F1 jest wciœniête
			{
				keys[VK_F1] = FALSE;                             // ustaw go na false, bo zosta³ u¿yty
				KillGLWindow();                                  // Zamknij okno
				fullscreen = !fullscreen;                        // Zamieñ pe³ny ekran)
																 // Stwórz nowe okno
				if (!CreateGLWindow(TEXT("CO SIE ODWALA?!"), 1366, 768, 16, fullscreen))
				{
					return 0;                                    // Wyst¹pi³ b³¹d
				}
			}
		}
	}
	KillGLWindow();                                              // Zamknij OpenGL
	return (msg.wParam);                                         // Koniec programu
}