#include <windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <process.h>
#include "game.h"

#pragma comment( lib, "opengl32.lib" )					
#pragma comment( lib, "glu32.lib" )	
//#pragma warning(disable: 4996)

#define PI 3.14159265

#ifndef		CDS_FULLSCREEN	
#define		CDS_FULLSCREEN 4								
#endif

HANDLE  hSendMutex;
bool		isConnected;
bool		mainmenu;
GL_Window*	g_window;
Keys*		g_keys;

BOOL		MENU = TRUE;
GLbyte		StateMenu;
GLint		type;

GLuint		base;
GLfloat		g = 0.98f;
GLfloat		ground = 100.0f;
GLfloat		x, y;
GLfloat		CentreX;
GLfloat		CentreY;
GLubyte		colors[5][3];

man *me, *enemy;

TGATexture textures[4];

BOOL LoadTGA(TGATexture *texture, char *filename)				// Loads A TGA File Into Memory
{
	GLubyte		TGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };		
	GLubyte		TGAcompare[12];	
	GLubyte		header[6];	
	GLuint		bytesPerPixel;
	GLuint		imageSize;	
	GLuint		temp;	
	GLuint		type = GL_RGBA;		

	FILE *file = fopen(filename, "rb");		

	if (file == NULL ||										
		fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||	
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||	
		fread(header, 1, sizeof(header), file) != sizeof(header))				
	{
		if (file == NULL)										
			return FALSE;										
		else													
		{
			fclose(file);										
			return FALSE;										
		}
	}

	texture->width = header[1] * 256 + header[0];				
	texture->height = header[3] * 256 + header[2];				

	if (texture->width <= 0 ||									
		texture->height <= 0 ||									
		(header[4] != 24 && header[4] != 32))						
	{
		fclose(file);											
		return FALSE;											
	}

	texture->bpp = header[4];								
	bytesPerPixel = texture->bpp / 8;							
	imageSize = texture->width*texture->height*bytesPerPixel;	

	texture->imageData = (GLubyte *)malloc(imageSize);			

	if (texture->imageData == NULL ||								
		fread(texture->imageData, 1, imageSize, file) != imageSize)	
	{
		if (texture->imageData != NULL)							
			free(texture->imageData);	

		fclose(file);	
		return FALSE;	
	}

	for (GLuint i = 0; i<int(imageSize); i += bytesPerPixel)			
	{															
		temp = texture->imageData[i];							
		texture->imageData[i] = texture->imageData[i + 2];		
		texture->imageData[i + 2] = temp;
	}

	fclose(file);

	// Build A Texture From The Data
	glGenTextures(1, &texture[0].texID);

	glBindTexture(GL_TEXTURE_2D, texture[0].texID);	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (texture[0].bpp == 24)
	{
		type = GL_RGB;	
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);

	return true;	
}

GLvoid BuildFont(HDC hDC)								// Build Our Bitmap Font
{
	HFONT	font;										// Windows Font ID
	HFONT	oldfont;									// Used For Good House Keeping

	base = glGenLists(96);								// Storage For 96 Characters

	font = CreateFont( -40,							// Height Of Font
		0,								// Width Of Font
		0,								// Angle Of Escapement
		0,								// Orientation Angle
		FW_NORMAL,						// Font Weight
		FALSE,							// Italic
		FALSE,							// Underline
		FALSE,							// Strikeout
		ANSI_CHARSET,					// Character Set Identifier
		OUT_TT_PRECIS,					// Output Precision
		CLIP_DEFAULT_PRECIS,			// Clipping Precision
		ANTIALIASED_QUALITY,			// Output Quality
		FF_DONTCARE | DEFAULT_PITCH,		// Family And Pitch
		"Algerian");					// Font Name

	oldfont = (HFONT)SelectObject(hDC, font);           // Selects The Font We Want
	wglUseFontBitmaps(hDC, 32, 96, base);				// Builds 96 Characters Starting At Character 32
	SelectObject(hDC, oldfont);							// Selects The Font We Want
	DeleteObject(font);									// Delete The Font
	StateMenu = 1;
}

GLvoid glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
{
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base - 32);								// Sets The Base Character to 32
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}

GLvoid refreshColors(GLbyte State)
{
	colors[0][0] = 20;
	colors[0][1] = 35;
	colors[0][2] = 170;

	colors[1][0] = 20;
	colors[1][1] = 35;
	colors[1][2] = 170;

	colors[2][0] = 20;
	colors[2][1] = 35;
	colors[2][2] = 170;

	colors[3][0] = 20;
	colors[3][1] = 35;
	colors[3][2] = 170;

	colors[4][0] = 20;
	colors[4][1] = 35;
	colors[4][2] = 170;
	switch (State)
	{
	case 1: 
		{
			colors[0][0] = 70;
			colors[0][1] = 165;
			colors[0][2] = 230;
		} break;
	case 2:
		{
			colors[1][0] = 70;
			colors[1][1] = 165;
			colors[1][2] = 230;
		} break;
	case 3:
		{
			colors[2][0] = 70;
			colors[2][1] = 165;
			colors[2][2] = 230;
		} break;
	case 4:
		{
			colors[3][0] = 70;
			colors[3][1] = 165;
			colors[3][2] = 230;
		} break;
	case 5:
		{
			colors[4][0] = 70;
			colors[4][1] = 165;
			colors[4][2] = 230;
		} break;

	default: break;
	}

}

BOOL Init(GL_Window* window, Keys* keys)	
{
	g_window = window;
	g_keys = keys;
	isConnected = false;
	hSendMutex = CreateMutex(NULL, TRUE, NULL);
	RECT wnd;
	me = (man*)malloc(sizeof(struct man));
	enemy = (man*)malloc(sizeof(struct man));
	if ((!LoadTGA(&textures[0], "Data/sprites.tga")) ||
		(!LoadTGA(&textures[1], "Data/cursor.tga")) ||
		(!LoadTGA(&textures[2], "Data/snowball.tga")) ||			
		(!LoadTGA(&textures[3], "Data/level2.tga")))						
	{
		MessageBox(HWND_DESKTOP, "Error in loading textures", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											
	}
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);						
	glClearDepth(1.0f);											
	glDepthFunc(GL_LEQUAL);									
	glEnable(GL_DEPTH_TEST);									
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Enable Alpha Blending (disable alpha testing)
	//glEnable(GL_BLEND);										// Enable Blending       (disable alpha testing)
	glAlphaFunc(GL_GREATER, 0.1f);							// Set Alpha Testing     (disable blending)
	glEnable(GL_ALPHA_TEST);									// Enable Alpha Testing  (disable blending)
	glEnable(GL_TEXTURE_2D);									
	me->isdirR = TRUE;
	me->isprefight = FALSE;
	GetClientRect(window->hWnd, &wnd);
	CentreX = wnd.right / 2;
	CentreY = wnd.bottom / 2;
	me->x = CentreX;
	me->y = CentreY;
	//me->sb.x = -10.0;
	//me->sb.y = -10.0;
	me->speed = 2.0f* SPEED;
	me->count = 0;
	me->isStrike = 0;
	BuildFont(window->hDC);
	refreshColors(StateMenu);
	mainmenu = true;
	return TRUE;											
}

GLvoid Update(float intpl)
{
	if (g_keys->keyDown[VK_RETURN])								
	{
		if (MENU) 
		{
			if (mainmenu)
			{
				switch (StateMenu)
				{
				case 1: MENU = FALSE; glEnable(GL_ALPHA_TEST); type = 2; mainmenu = false; break;

				case 2:
					if (Init_Networking(SERVER) == TRUE)
					{
						MENU = FALSE;
						type = SERVER;
						glEnable(GL_ALPHA_TEST);
					}
					mainmenu = false;
					break;
				case 3:
					if (Init_Networking(CLIENT) == TRUE)
					{
						MENU = FALSE;
						type = CLIENT;
						glEnable(GL_ALPHA_TEST);
					}
					else
					{
						MessageBox(HWND_DESKTOP, "NO SERVER", "Error", MB_OK | MB_ICONEXCLAMATION);
					}
					mainmenu = false;
					break;
				
				case 4: ToggleFullscreen(g_window); break;
				case 5: TerminateApp(g_window); break;

				default: break;
				}
			}
			else
			{
				switch (StateMenu)
				{
				case 1: MENU = FALSE; glEnable(GL_ALPHA_TEST); type = 2;  break;

				case 2:	isConnected = false; mainmenu = true; break;

				case 3: TerminateApp(g_window); break;

				default: break;
				}
			}
			if (type == SERVER) me->x = CentreX - 500;
			else if (type == CLIENT) me->x = CentreX + 500;
		}
		g_keys->keyDown[VK_RETURN] = FALSE;
	}
	if (g_keys->keyDown[VK_ESCAPE])								
	{
		if (MENU == FALSE)
		{
			MENU = TRUE;
			glDisable(GL_ALPHA_TEST);
		}
		//TerminateApp(g_window);
		g_keys->keyDown[VK_ESCAPE] = FALSE;
	}

	if (g_keys->keyDown['A'] == FALSE || g_keys->keyDown['D'] == FALSE) 
		me->ismove = FALSE;

	if (g_keys->keyDown['D'])							
	{
		if (me->isprefight == FALSE)
		{
			if (me->x < 1360)
				me->x = me->x + me->speed;
			me->ismove = TRUE;
			if (!me->isjump) me->count++;
		}
		else if(me->isjump==FALSE) g_keys->keyDown['D'] = FALSE;
	}
	if (g_keys->keyDown['A'])							
	{
		if (me->isprefight == FALSE)
		{
			if(me->x > 6 )me->x = me->x - me->speed;
			me->ismove = TRUE;
			if(!me->isjump) me->count++;
		}
		else if (me->isjump == FALSE) g_keys->keyDown['A'] = FALSE;
	}
	
	if (g_keys->keyDown['W'] || g_keys->keyDown[VK_UP])
	{
		if (MENU)
		{
			if (StateMenu > 1) {
				StateMenu--; 
				refreshColors(StateMenu);
			}
		}else
		{
			if (me->isjump == FALSE)
			{
				me->isjump = TRUE;
				me->t = 0.0f;
				me->jump_speed = 70.0f;
				me->count = 0;
			}
		}
		g_keys->keyDown['W'] = FALSE;
		g_keys->keyDown[VK_UP] = FALSE;
	}
	if (g_keys->keyDown['S'] || g_keys->keyDown[VK_DOWN])
	{
		int max;
		if (mainmenu) max = 5;
		else max = 3;
		if (MENU)
		{
			if (StateMenu < max)
			{
				StateMenu++;
				refreshColors(StateMenu);
			}
		}
		g_keys->keyDown['S'] = FALSE;
		g_keys->keyDown[VK_DOWN] = FALSE;
	}

//кинул снежок
	if (me->sb.isStrike)
	{
		me->sb.x = ((me->sb.speedX) * me->sb.tx * cos(me->sb.angle * PI / 180)) + x;
		me->sb.y = ((me->sb.speedY) * me->sb.ty * sin(me->sb.angle * PI / 180) - g*me->sb.ty*me->sb.ty / 2) + y + 20;
		if (me->sb.x > 2 * CentreX + 20 || me->sb.x < -20 || me->sb.y < -20)
		{
			me->sb.isStrike = FALSE;
			me->ismove = TRUE;
		}
		if(me->sb.side == 1) me->sb.tx += (0.2 *SPEED);
		else me->sb.tx -= (0.2 *SPEED);
		me->sb.ty += 0.2;
		if (me->sb.ty > 10) me->isStrike = FALSE;
	}

//смотрит влево или вправо
	if (mouse_x > me->x) me->isdirR = TRUE;
	else me->isdirR = FALSE;

//если в прыжке
	if (me->isjump)
	{
		GLfloat temp;
		prefight(0);
		me->jump_speed -= g*(me->t/12) ;
		//if (me->isdirR)	me->x += 0.3f;
		//else me->x -= 0.3f;
		temp = me->y;
		me->y = me->jump_speed*me->t - g*me->t*me->t / 2 + ground;
		me->t += (0.07 * SPEED);
		if (me->y < ground)
		{
			me->isjump = FALSE;
			me->y = ground;
		}
		else if (temp > me->y) me->count = 1;
		//me->t *= SPEED;
	}
	else me->y = ground;
}

BOOL Init_Networking(int type)
{
	if (type == SERVER)
	{
		if (initSocket(SERVER,me) == 2) 	_beginthread(Networking, 0, enemy);
		else return FALSE;
	}
		
	if (type == CLIENT)
	{
		if (initSocket(CLIENT,me) == 2) 	_beginthread(Networking, 0, enemy);
		else return FALSE;
	}
	return TRUE;
} 

GLvoid fight(DWORD *power)
{
	if (me->sb.isStrike) return;
	if (me->ismove) return;
	if (me->isjump) return;
	prefight(0);
	int q = 2 * CentreY;
	if (*power > 600) *power = 600; 
	if (*power < 70) *power = 70;
	me->sb.speedX = me->sb.speedY = *power / 10;
	me->sb.angle = findangle(mouse_x, 2 * CentreY - mouse_y, me->x, me->y);
	//me->sb.speedX *= cos(me->sb.angle * PI / 180);
	//me->sb.speedY *= sin(me->sb.angle * PI / 180);
	if (me->isdirR)
	{
		me->sb.side = 1;
	}
	else
	{
		me->sb.angle = 180 + me->sb.angle;
		me->sb.side = -1;
	}

	me->sb.speedX *= cos(me->sb.angle * PI / 180);
	me->sb.speedY *= sin(me->sb.angle * PI / 180);
	me->sb.x = x = me->x;
	y = me->y;
	me->sb.tx = me->sb.ty = 0.0f;
	me->sb.isStrike = TRUE;
	me->isStrike = TRUE;
}

GLvoid showMenu(RECT wnd)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();											//push projection
	glLoadIdentity();
	gluOrtho2D(0, wnd.right, 0, wnd.bottom);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Pulsing Colors Based On Text Position
	if (mainmenu)
	{
		glColor3ubv(colors[0]);
		glRasterPos2d(100.0, 400.0);
		glPrint("Start(test)");
		glColor3ubv(colors[1]);
		glRasterPos2d(100.0, 360.0);
		glPrint("Create game");
		glColor3ubv(colors[2]);
		glRasterPos2d(100.0, 320.0);
		glPrint("Join game");
		glColor3ubv(colors[3]);
		glRasterPos2d(100.0, 280.0);
		glPrint("Switch screen mode");
		glColor3ubv(colors[4]);
		glRasterPos2d(100.0, 240.0);
		glPrint("Quit");
		
	}
	else
	{
		glColor3ubv(colors[0]);
		glRasterPos2d(100.0, 400.0);
		glPrint("Continue");
		glColor3ubv(colors[1]);
		glRasterPos2d(100.0, 360.0);
		glPrint("Disconnect");
		glColor3ubv(colors[2]);
		glRasterPos2d(100.0, 320.0);
		glPrint("Quit");
	}
	
}

GLvoid draw_boy(man* boy)
{
	int dir;
	GLfloat texx = 0.0f, texy = 1.0f;
	if (boy->isdirR) dir = -1;
	else dir = 1;
	if (boy->isjump)
	{
		if (boy->count < 1) texx = 0.125f;
		else texx = 0.25f;
	}else
		if (boy->ismove)
		{
			texy = 0.5f;
			if (boy->count < 5) texx = 0.0f;
			else 
			{
				if (boy->count < 15) texx = 0.125f;
				else
				{
					if (boy->count < 25) texx = 0.25f;
					else
					{
						if (boy->count < 35) texx = 0.375f;
						else
						{
							if (boy->count < 45) texx = 0.5f;
							else
							{
								if (boy->count < 55) texx = 0.625f;
								else
								{
									if (boy->count < 65) texx = 0.75f;
									else boy->count = 0;
								}
							}
						}
					}
				}
			}
		}

	if (boy->isStrike) texx = 0.5f;

	if (boy->isprefight) 
		texx = 0.375f;

	glBindTexture(GL_TEXTURE_2D, textures[0].texID);
	glBegin(GL_QUADS);
	glTexCoord2f(texx, texy);						glVertex3f(17*dir, 34, 0.0f);
	glTexCoord2f(texx + 0.125f, texy);				glVertex3f(-17*dir, 34, 0.0f);
	glTexCoord2f(texx + 0.125f, texy - 0.5f);		glVertex3f(-17*dir, -34, 0.0f);
	glTexCoord2f(texx, texy - 0.5f);				glVertex3f(17*dir, -34, 0.0f);
	glEnd();
}

GLvoid draw_obj(GLfloat w, GLfloat h, GLuint tex)
{
	int dir;
	if (me->isdirR) dir = -1;
	else dir = 1;
	glBindTexture(GL_TEXTURE_2D, textures[tex].texID);		
	glBegin(GL_QUADS);											
	glTexCoord2f(0.0f, 1.0f); glVertex3f(w*dir, h, 0.0f);	
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-w*dir, h, 0.0f);	
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-w*dir, -h, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(w*dir, -h, 0.0f);
	glEnd();
}

GLfloat findangle(GLfloat x2, GLfloat y2, GLfloat x1, GLfloat y1)
{
	return atan((y2 - y1) / (x2 - x1))* 180.0 / PI;
}

GLvoid DrawScene()
{
	RECT window;
	GetClientRect(g_window->hWnd, &window);
	if (MENU) showMenu(window);
	else
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor3ub(255,255,255);
		glLoadIdentity();
		glPushMatrix();											//push modelview
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();											//push projection
		glLoadIdentity();
		gluOrtho2D(0, window.right, 0, window.bottom);
		glMatrixMode(GL_MODELVIEW);
		glBindTexture(GL_TEXTURE_2D, textures[3].texID);
		glTranslatef(CentreX, CentreY, 0.0f);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(1366 / 2, 768 / 2, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1366 / 2, 768 / 2, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1366 / 2, -768 / 2, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1366 / 2, -768 / 2, 0.0f);
		glEnd();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		/////////////////
		glLoadIdentity();
		gluOrtho2D(0, window.right, 0, window.bottom);
		glMatrixMode(GL_MODELVIEW);
		//man
		glLoadIdentity();
		glTranslatef(me->x, me->y, 0.0f);
		draw_boy(me);
		glDisable(GL_ALPHA_TEST);
		glLoadIdentity();
		glColor3ub(255, 0, 0);
		glRasterPos2d(20.0, 50.0);
		glPrint("%.0f - %.0f", me->x, me->y);
		glEnable(GL_ALPHA_TEST);
		glColor3ub(255, 255, 255);
		//snowball
		glLoadIdentity();
		glTranslatef(me->sb.x, me->sb.y, 0.0f);
		draw_obj(7, 7, 2);
		glDisable(GL_ALPHA_TEST);
		glLoadIdentity();
		glColor3ub(255, 0, 0);
		glRasterPos2d(20.0, 20.0);
		glPrint("%.0f - %.0f", me->sb.x, me->sb.y);
		glEnable(GL_ALPHA_TEST);
		glColor3ub(255, 255, 255);
		
		if (isConnected)
		{
			glLoadIdentity();
			glTranslatef(enemy->x, enemy->y, 0.0f);
			draw_boy(enemy);
			glDisable(GL_ALPHA_TEST);
			glLoadIdentity(); 
			glColor3ub(255, 0, 0);
			glRasterPos2d(20.0, 620.0);
			glPrint("%.0f - %.0f", enemy->x, enemy->y);
			glEnable(GL_ALPHA_TEST);
			glColor3ub(255, 255, 255);

			glLoadIdentity();
			glTranslatef(enemy->sb.x, enemy->sb.y, 0.0f);
			draw_obj(7, 7, 2);
			glDisable(GL_ALPHA_TEST);
			glLoadIdentity();
			glColor3ub(255, 0, 0);
			glRasterPos2d(20.0, 590.0);
			glPrint("%.0f - %.0f", enemy->sb.x, enemy->sb.y);
			glEnable(GL_ALPHA_TEST);
			glColor3ub(255, 255, 255);
			//glLoadIdentity();
			//glTranslatef(enemy->sb.x, enemy->sb.y, 0.0f);
			//draw_obj(7, 7, 2);
		}
		//Crosshair														
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glTranslated(mouse_x, window.bottom - mouse_y, 0.0f);
		draw_obj(16, 16, 1);
		
	}
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glFlush();
}

/*==============================================================================*/

GLvoid prefight(BOOL fl)
{
	if (me->sb.isStrike) return;
	me->isprefight = fl;
}

GLvoid killFont()
{
	glDeleteLists(base, 96);
}