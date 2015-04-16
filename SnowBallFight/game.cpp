#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "WindowControl.h"

#pragma comment( lib, "opengl32.lib" )					
#pragma comment( lib, "glu32.lib" )	
//#pragma warning(disable: 4996)

#define PI 3.14159265

#ifndef		CDS_FULLSCREEN	
#define		CDS_FULLSCREEN 4								
#endif

GLfloat findangle(GLfloat, GLfloat, GLfloat, GLfloat);

GL_Window*	g_window;
Keys*		g_keys;

BOOL isprefight;
GLfloat g = 0.98f;
GLfloat ground = 100.0f;
GLfloat x, y;
GLfloat CentreX;
GLfloat CentreY;
GLfloat dltx;
struct man
{
	bool hit;
	GLfloat x;
	GLfloat y;
	GLfloat speed;
	GLfloat jump_speed;
	bool isjump;
	GLfloat t;
	GLuint count;
	BOOL isdirR;
	BOOL isStrike;
	BOOL ismove;
};
man me;
man enemy;
typedef struct
{
	GLfloat x;
	GLfloat y;
	GLfloat angle;
	GLfloat tx,ty;
	GLfloat speedX,speedY;
	BOOL isStrike;
	GLuint side;
}snowball;

snowball mysb,enemsb;


typedef struct													
{
	GLubyte	*imageData;											// Image Data (Up To 32 Bits)
	GLuint	bpp;												// Image Color Depth In Bits Per Pixel.
	GLuint	width;												// Image Width
	GLuint	height;												// Image Height
	GLuint	texID;												// Texture ID Used To Select A Texture
} TGATexture;

TGATexture textures[4];

bool LoadTGA(TGATexture *texture, char *filename)				// Loads A TGA File Into Memory
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

BOOL Init(GL_Window* window, Keys* keys)	
{
	g_window = window;
	g_keys = keys;

	if ((!LoadTGA(&textures[0], "Data/sprites.tga")) ||
		(!LoadTGA(&textures[1], "Data/cursor.tga")) ||
		(!LoadTGA(&textures[2], "Data/snowball.tga")) ||			
		(!LoadTGA(&textures[3], "Data/level2.tga")))						
	{
		return FALSE;											
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);						
	glClearDepth(1.0f);											
	glDepthFunc(GL_LEQUAL);									
	glEnable(GL_DEPTH_TEST);									
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Enable Alpha Blending (disable alpha testing)
	glEnable(GL_BLEND);										// Enable Blending       (disable alpha testing)
	//glAlphaFunc(GL_GREATER, 0.1f);								// Set Alpha Testing     (disable blending)
	//glEnable(GL_ALPHA_TEST);									// Enable Alpha Testing  (disable blending)
	glEnable(GL_TEXTURE_2D);									
	me.isdirR = TRUE;
	RECT wnd;
	GetClientRect(window->hWnd, &wnd);
	CentreX = wnd.right / 2;
	CentreY = wnd.bottom / 2;
	me.x = CentreX;
	me.y = CentreY;
	me.speed = 2.0f;
	me.count = 0;
	return TRUE;											
}
void Update(float intpl)
{
	if (g_keys->keyDown[VK_ESCAPE])								
	{
		TerminateApp(g_window);						
	}

	if (g_keys->keyDown['A'] == FALSE || g_keys->keyDown['D'] == FALSE) 
		me.ismove = FALSE;

	if (g_keys->keyDown['D'])							
	{
		if (isprefight == FALSE)
		{
			me.x = me.x + me.speed;
			me.ismove = TRUE;
			me.count++;
		}
		else if(me.isjump==FALSE) g_keys->keyDown['D'] = FALSE;
	}
	if (g_keys->keyDown['A'])							
	{
		if (isprefight == FALSE)
		{
			me.x = me.x - me.speed;
			me.ismove = TRUE;
			me.count++;
		}
		else if (me.isjump == FALSE) g_keys->keyDown['A'] = FALSE;
	}
	
	if (g_keys->keyDown['W'])
	{
		if (me.isjump == FALSE)
		{
				me.isjump = TRUE;
				me.t = 0.0f;
				me.jump_speed = 70.0f;
				me.count = 0;
		}
		g_keys->keyDown['W'] = FALSE;
	}

//кинул снежок
	if (mysb.isStrike)
	{
		mysb.x = ((mysb.speedX) * mysb.tx * cos(mysb.angle * PI / 180)) + x;
		mysb.y = ((mysb.speedY) * mysb.ty * sin(mysb.angle * PI / 180) - g*mysb.ty*mysb.ty / 2) + y + 20;
		if (mysb.x > 2 * CentreX + 20 || mysb.x < -20 || mysb.y < -20)
		{
			mysb.isStrike = FALSE;
			me.ismove = TRUE;
		}
		if(mysb.side == 1) mysb.tx += 0.2;
		else mysb.tx -= 0.2;
		mysb.ty += 0.2;
		if (mysb.ty > 10) me.isStrike = FALSE;
	}

//смотрит влево или вправо
	if (mouse_x > me.x) me.isdirR = TRUE;
	else me.isdirR = FALSE;

//если в прыжке
	if (me.isjump)
	{
		prefight(0);
		me.jump_speed -= g*(me.t/7) ;
		//if (me.isdirR)	me.x += 0.3f;
		//else me.x -= 0.3f;
		me.y = me.jump_speed*me.t - g*me.t*me.t / 2 + ground;
		me.t += 0.1;
		if (me.y < ground)
		{
			me.isjump = FALSE;
			me.y = ground;
		}
		else if (me.y > 295 + ground) me.count = 1;

	}
	else me.y = ground;
}

void fight(DWORD *power)
{
	if (mysb.isStrike) return;
	if (me.ismove) return;
	if (me.isjump) return;
	prefight(0);
	int q = 2 * CentreY;
	if (*power > 500) *power = 500; 
	if (*power < 70) *power = 70;
	mysb.speedX = mysb.speedY = *power / 10;
	mysb.angle = findangle(mouse_x, 2 * CentreY - mouse_y, me.x, me.y);
	//mysb.speedX *= cos(mysb.angle * PI / 180);
	//mysb.speedY *= sin(mysb.angle * PI / 180);
	if (me.isdirR)
	{
		mysb.side = 1;
	}
	else
	{
		mysb.angle = 180 + mysb.angle;
		mysb.side = -1;
	}

	mysb.speedX *= cos(mysb.angle * PI / 180);
	mysb.speedY *= sin(mysb.angle * PI / 180);
	mysb.x = x = me.x;
	y = me.y;
	mysb.tx = mysb.ty = 0.0f;
	mysb.isStrike = TRUE;
	me.isStrike = TRUE;
}

void draw_boy()
{
	int dir;
	GLfloat texx = 0.0f, texy = 1.0f;
	if (me.isdirR) dir = -1;
	else dir = 1;
	if (me.isjump)
	{
		if (me.count < 1) texx = 0.125f;
		else texx = 0.25f;
	}else
		if (me.ismove)
		{
			texy = 0.5f;
			if (me.count < 5) texx = 0.0f;
			else 
			{
				if (me.count < 15) texx = 0.125f;
				else
				{
					if (me.count < 25) texx = 0.25f;
					else
					{
						if (me.count < 35) texx = 0.375f;
						else
						{
							if (me.count < 45) texx = 0.5f;
							else
							{
								if (me.count < 55) texx = 0.625f;
								else
								{
									if (me.count < 65) texx = 0.75f;
									else me.count = 0;
								}
							}
						}
					}
				}
			}
		}

	if (me.isStrike) texx = 0.5f;

	if (isprefight) texx = 0.375f;

	glBindTexture(GL_TEXTURE_2D, textures[0].texID);
	glBegin(GL_QUADS);
	glTexCoord2f(texx, texy);						glVertex3f(17*dir, 34, 0.0f);
	glTexCoord2f(texx + 0.125f, texy);				glVertex3f(-17*dir, 34, 0.0f);
	glTexCoord2f(texx + 0.125f, texy - 0.5f);		glVertex3f(-17*dir, -34, 0.0f);
	glTexCoord2f(texx, texy - 0.5f);				glVertex3f(17*dir, -34, 0.0f);
	glEnd();
}

void draw_obj(GLfloat w, GLfloat h, GLuint tex)
{
	int dir;
	if (me.isdirR) dir = -1;
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

void DrawScene()
{
	RECT window;
	GetClientRect(g_window->hWnd, &window);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glLoadIdentity();
	glPushMatrix();											//push modelview
	glMatrixMode(GL_PROJECTION);							
	glPushMatrix();											//push projection
	glLoadIdentity();
	gluOrtho2D(0, window.right, 0, window.bottom);
	glMatrixMode(GL_MODELVIEW);
	glBindTexture(GL_TEXTURE_2D, textures[3].texID);
	glTranslatef(CentreX,CentreY,0.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1366/2, 768/2, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1366/2, 768/2, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1366/2, -768/2, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1366/2, -768/2, 0.0f);
	glEnd();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
													
/////////////////
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();											//push projection
	glLoadIdentity();
	gluOrtho2D(0, window.right, 0, window.bottom);

	glMatrixMode(GL_MODELVIEW);
	//man
	glLoadIdentity();
	glTranslatef(me.x, me.y , 0.0f);
	draw_boy();
	//snowball
	glLoadIdentity();
	glTranslatef(mysb.x, mysb.y, 0.0f);
	draw_obj(7,7,2);							
	//Crosshair														
	glLoadIdentity();														
	glMatrixMode(GL_MODELVIEW);									
	glTranslated(mouse_x, window.bottom - mouse_y, 0.0f);			
	draw_obj(16,16,1);

	glMatrixMode(GL_PROJECTION);								
	glPopMatrix();												
	glMatrixMode(GL_MODELVIEW);									

	glFlush();
}

void prefight(BOOL fl)
{
	if (mysb.isStrike) return;
	isprefight = fl;
}