#include <windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include "WindowControl.h"

#define WM_TOGGLEFULLSCREEN (WM_USER+1)	

int	mouse_x, mouse_y;
BOOL isfullscreen;
BOOL isplaying;

#pragma region Windows Operations

void TerminateApp(GL_Window* window)							// Terminate The Application
{
	PostMessage(window->hWnd, WM_QUIT, 0, 0);							// Send A WM_QUIT Message
	isplaying = FALSE;											// Stop Looping Of The Program
}

BOOL Create_Window(GL_Window* window)									// This Code Creates Our OpenGL Window
{
	DWORD windowStyle = WS_TILED | WS_SYSMENU;							// Define Our Window Style
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;						// Define The Window's Extended Style
	GLuint PixelFormat;
	RECT windowRect;
	PIXELFORMATDESCRIPTOR pfd;
	ShowCursor(FALSE);
	pfd =											// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),									// Size Of This Pixel Format Descriptor
		1,																// Version Number
		PFD_DRAW_TO_WINDOW |											// Format Must Support Window
		PFD_SUPPORT_OPENGL |											// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,												// Must Support Double Buffering
		PFD_TYPE_RGBA,													// Request An RGBA Format
		window->init.bitsPerPixel,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,												// Color Bits Ignored
		0,																// No Alpha Buffer
		0,																// Shift Bit Ignored
		0,																// No Accumulation Buffer
		0, 0, 0, 0,														// Accumulation Bits Ignored
		16,																// 16Bit Z-Buffer (Depth Buffer)  
		0,																// No Stencil Buffer
		0,																// No Auxiliary Buffer
		PFD_MAIN_PLANE,													// Main Drawing Layer
		0,																// Reserved
		0, 0, 0															// Layer Masks Ignored
	};

	windowRect = { 0, 0, window->init.width, window->init.height };	// Define Our Window Coordinates

													// Will Hold The Selected Pixel Format

	if (window->init.isFullScreen == TRUE)								//fullscreen
	{
		if (ChangeResolution(window->init.width, window->init.height, window->init.bitsPerPixel) == FALSE)
		{
			// Fullscreen Mode Failed.  Run In Windowed Mode Instead
			MessageBox(HWND_DESKTOP, "Mode Switch Failed.\nRunning In Windowed Mode.", "Error", MB_OK | MB_ICONEXCLAMATION);
			window->init.isFullScreen = FALSE;							//if fullscreen mode failed - back to windowed
		}
		else
		{
			windowStyle = WS_POPUP;
			windowExtendedStyle |= WS_EX_TOPMOST;
		}
	}
	else																//not fullscreen
	{
		AdjustWindowRectEx(&windowRect, windowStyle, 0, windowExtendedStyle);
	}
	window->hWnd = CreateWindowEx(windowExtendedStyle, 
		window->init.application->className,
		window->init.title,
		windowStyle,							
		0, 0,								
		windowRect.right - windowRect.left,	
		windowRect.bottom - windowRect.top,	
		HWND_DESKTOP,						
		0,									
		window->init.application->hInstance, 
		window);

	if (window->hWnd == 0)												
	{
		return FALSE;													
	}

	window->hDC = GetDC(window->hWnd);									
	if (window->hDC == 0)												
	{
		DestroyWindow(window->hWnd);									
		window->hWnd = 0;												
		return FALSE;													
	}

	PixelFormat = ChoosePixelFormat(window->hDC, &pfd);				
	if (PixelFormat == 0)												
	{
		ReleaseDC(window->hWnd, window->hDC);							
		window->hDC = 0;												
		DestroyWindow(window->hWnd);				
		window->hWnd = 0;												
		return FALSE;								
	}

	if (SetPixelFormat(window->hDC, PixelFormat, &pfd) == FALSE)
	{
		ReleaseDC(window->hWnd, window->hDC);
		window->hDC = 0;
		DestroyWindow(window->hWnd);			
		window->hWnd = 0;	
		return FALSE;
	}

	window->hRC = wglCreateContext(window->hDC);						
	if (window->hRC == 0)												
	{
		ReleaseDC(window->hWnd, window->hDC);							
		window->hDC = 0;												
		DestroyWindow(window->hWnd);									
		window->hWnd = 0;												
	}

	//make current rendering context
	if (wglMakeCurrent(window->hDC, window->hRC) == FALSE)
	{
		wglDeleteContext(window->hRC);					
		window->hRC = 0;					
		ReleaseDC(window->hWnd, window->hDC);				
		window->hDC = 0;						
		DestroyWindow(window->hWnd);						
		window->hWnd = 0;												
		return FALSE;	
	}

	ShowWindow(window->hWnd, SW_NORMAL);								
	window->isVisible = TRUE;											

	ReshapeGL(window->init.width, window->init.height);				

	ZeroMemory(window->keys, sizeof(Keys));							

	window->lastTickCount = GetTickCount();							

	return TRUE;														
	
}

BOOL Destroy_Window(GL_Window* window)								// Destroy The OpenGL Window & Release Resources
{
	if (window->hWnd != 0)												// Does The Window Have A Handle?
	{
		if (window->hDC != 0)											// Does The Window Have A Device Context?
		{
			wglMakeCurrent(window->hDC, 0);							// Set The Current Active Rendering Context To Zero
			if (window->hRC != 0)										// Does The Window Have A Rendering Context?
			{
				wglDeleteContext(window->hRC);							// Release The Rendering Context
				window->hRC = 0;										// Zero The Rendering Context

			}
			ReleaseDC(window->hWnd, window->hDC);						// Release The Device Context
			window->hDC = 0;											// Zero The Device Context
		}
		DestroyWindow(window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
	}

	if (window->init.isFullScreen)										// Is Window In Fullscreen Mode
	{
		ChangeDisplaySettings(NULL, 0);									// Switch Back To Desktop Resolution
	}
	killFont();
	ShowCursor(TRUE);
	return TRUE;														// Return True
}

BOOL ChangeResolution(int width, int height, int bitsPerPixel)	// Change The Screen Resolution
{
	DEVMODE dmScreenSettings;											// Device Mode
	ZeroMemory(&dmScreenSettings, sizeof(DEVMODE));					// Make Sure Memory Is Cleared
	dmScreenSettings.dmSize = sizeof(DEVMODE);				// Size Of The Devmode Structure
	dmScreenSettings.dmPelsWidth = width;						// Select Screen Width
	dmScreenSettings.dmPelsHeight = height;						// Select Screen Height
	dmScreenSettings.dmBitsPerPel = bitsPerPixel;					// Select Bits Per Pixel
	dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return FALSE;													// Display Change Failed, Return False
	}

	return TRUE;														// Display Change Was Successful, Return True
}

void ReshapeGL(int width, int height)									
{
	glViewport(0, 0, (GLsizei)(width), (GLsizei)(height));				
	glMatrixMode(GL_PROJECTION);										
	glLoadIdentity();												
	gluPerspective(45.0f, (GLfloat)(width) / (GLfloat)(height),	1.0f, 100.0f);
	glMatrixMode(GL_MODELVIEW);	
	glLoadIdentity();	
}

BOOL RegisterWindowClass(Application* app)						//register window
{
	WNDCLASSEX windowClass;
	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = (WNDPROC)(WindowProc);
	windowClass.hInstance = app->hInstance;
	windowClass.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = app->className;
	if (RegisterClassEx(&windowClass) == 0)
	{

		MessageBox(HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	return TRUE;
}

void ToggleFullscreen(GL_Window* window)								// Toggle Fullscreen/Windowed
{
	PostMessage(window->hWnd, WM_TOGGLEFULLSCREEN, 0, 0);				// Send A WM_TOGGLEFULLSCREEN Message
}

#pragma endregion

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	GL_Window* window = (GL_Window*)(GetWindowLong(hWnd, GWL_USERDATA));
	switch (uMsg)
	{
	case WM_SYSCOMMAND:												
	{
		switch (wParam)												
		{
		case SC_SCREENSAVE:										
		case SC_MONITORPOWER:									
			return 0;												
		}
		break;														
	}
	return 0;														

	case WM_CREATE:													
	{
		CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);			
		window = (GL_Window*)(creation->lpCreateParams);
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)(window));
	}
	return 0;														

	case WM_CLOSE:													
		TerminateApp(window);								
		return 0;														

	case WM_SIZE:													
		switch (wParam)												
		{
		case SIZE_MINIMIZED:									
			window->isVisible = FALSE;							
			return 0;												

		case SIZE_MAXIMIZED:									
			window->isVisible = TRUE;							
			ReshapeGL(LOWORD(lParam), HIWORD(lParam));		
			return 0;												

		case SIZE_RESTORED:										
			window->isVisible = TRUE;							
			ReshapeGL(LOWORD(lParam), HIWORD(lParam));		
			return 0;												
		}
		break;															

	case WM_KEYDOWN:												
		if ((wParam >= 0) && (wParam <= 255))						
		{
			window->keys->keyDown[wParam] = TRUE;					
			return 0;												
		}
		break;															

	case WM_KEYUP:													
		if ((wParam >= 0) && (wParam <= 255))						
		{
			window->keys->keyDown[wParam] = FALSE;					
			return 0;												
		}
		break;															

	case WM_TOGGLEFULLSCREEN:										
		isfullscreen = (isfullscreen == TRUE) ? FALSE : TRUE;
		PostMessage(hWnd, WM_QUIT, 0, 0);
		break;															

	case WM_LBUTTONDOWN:
	{
		//mouse_x = LOWORD(lParam);
		//mouse_y = HIWORD(lParam);
		prefight(TRUE);
		window->lastTickCount = GetTickCount();
	}
	break;
	case WM_LBUTTONUP:
	{
		DWORD		*tickCount = new DWORD();
		*tickCount = GetTickCount();
		*tickCount -= window->lastTickCount;
		mouse_x = LOWORD(lParam);
		mouse_y = HIWORD(lParam);
		fight(tickCount);
		delete tickCount;
	}
	break;
	case WM_MOUSEMOVE:
	{
		mouse_x = LOWORD(lParam);
		mouse_y = HIWORD(lParam);
	}
	break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);					
}


//prog entry
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	const int TICKS_PER_SECOND = 200/SPEED;
	const int SKIP_TICKS = 1000 / TICKS_PER_SECOND;
	const int MAX_FRAMESKIP = 10;

	int loops;

	Application			application;									
	GL_Window			window;											
	Keys				keys;											
	BOOL				isMessagePumpActive;							
	MSG					msg;																					
	application.className = "OpenGL";							
	application.hInstance = hInstance;	

	ZeroMemory(&window, sizeof(GL_Window));							
	window.keys = &keys;								
	window.init.application = &application;							
	window.init.title = "*** SnowBallFight ***";			
	window.init.width = 1366;									
	window.init.height = 768;									
	window.init.bitsPerPixel = 32;									
	window.init.isFullScreen = FALSE;									

	ZeroMemory(&keys, sizeof(Keys));									

	window.init.isFullScreen = FALSE;		

	if (RegisterWindowClass(&application) == FALSE)					
	{
		MessageBox(HWND_DESKTOP, "Error Registering Window Class!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return -1;											
	}

	isfullscreen = window.init.isFullScreen;
	isplaying = TRUE;
	while (isplaying)											
	{
		window.init.isFullScreen = isfullscreen;					
		if (Create_Window(&window) == TRUE)	
		{
			if (Init(&window, &keys) == FALSE)					
			{
				MessageBox(HWND_DESKTOP, "Error in INIT", "Error", MB_OK | MB_ICONEXCLAMATION);
				TerminateApp(&window);							
			}
			else														
			{	// Initialize was a success
				isMessagePumpActive = TRUE;
				DWORD next_game_tick = GetTickCount();
				while (isMessagePumpActive == TRUE)						
				{
					if (PeekMessage(&msg, window.hWnd, 0, 0, PM_REMOVE) != 0)
					{
						if (msg.message != WM_QUIT)						
						{
							DispatchMessage(&msg);						
						}
						else											
						{
							isMessagePumpActive = FALSE;				
						}
					}
					else												
					{
						if (window.isVisible == FALSE)					
						{
							WaitMessage();								
						}
//============================================================================
						else											
						{
							SwapBuffers(window.hDC);
							loops = 0;
							while (GetTickCount() > next_game_tick && loops < MAX_FRAMESKIP) {
								Update(1);

								next_game_tick += SKIP_TICKS;
								loops++;
							}
							ReleaseMutex(hSendMutex);
							DrawScene();						
						}
//=============================================================================
					}
				}														
			}
			
			Destroy_Window(&window);									
		}
		else															
		{
			//error creating window
			MessageBox(HWND_DESKTOP, "Error Creating OpenGL Window", "Error", MB_OK | MB_ICONEXCLAMATION);
			isplaying = FALSE;									
		}
	}																	

	UnregisterClass(application.className, application.hInstance);
	
	return 0;
}

/*void drawtime()
{
	float framesPerSecond = 0.0f;  
	static float lastTime = 0.0f;    

	float currentTime = (float)GetTickCount() * 0.001f;

	++framesPerSecond;


	if (currentTime - lastTime > 1.0f)
	{
		lastTime = currentTime;
		framesPerSecond;
		framesPerSecond = 0;
	}
}*/