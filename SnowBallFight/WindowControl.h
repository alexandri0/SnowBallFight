#include <windows.h>								

#pragma warning(disable: 4996)

extern int mouse_x;
extern int mouse_y;


typedef struct {									
	BOOL keyDown[256];							//keyboard mas	
} Keys;												

typedef struct {									
	HINSTANCE		hInstance;					//app instance
	const char*		className;					//app classname
} Application;										

typedef struct {									
	Application*		application;			//app structure
	char*				title;						
	int					width;						
	int					height;						
	int					bitsPerPixel;			
	BOOL				isFullScreen;			
} GL_WindowInit;								

typedef struct {								
	Keys*				keys;					// Key Structure
	HWND				hWnd;					
	HDC					hDC;					
	HGLRC				hRC;					
	GL_WindowInit		init;					
	BOOL				isVisible;				
	DWORD				lastTickCount;			//counter for timer
} GL_Window;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void TerminateApp(GL_Window *wnd);
void Update(float);
BOOL Init(GL_Window*, Keys*);
void fight(DWORD*);
BOOL ChangeResolution(int, int, int);
void ReshapeGL(int width, int height);
void DrawScene();
void prefight(BOOL);