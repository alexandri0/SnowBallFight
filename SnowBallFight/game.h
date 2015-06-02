#include <windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include "WindowControl.h"

#define SERVER 1
#define CLIENT 2

GLfloat findangle(GLfloat, GLfloat, GLfloat, GLfloat);
void Networking(void*);
int initSocket(int, struct man*);
BOOL Init_Networking(int);
GLvoid glPrint(const char *fmt, ...);
extern bool		isConnected;


struct snowball
{
	GLfloat x;
	GLfloat y;
	GLfloat angle;
	GLfloat tx, ty;
	GLfloat speedX, speedY;
	BOOL isStrike;
	GLuint side;
};

struct man
{
	BOOL hit;
	GLfloat x;
	GLfloat y;
	GLfloat speed;
	GLfloat jump_speed;
	BOOL isjump;
	GLfloat t;
	GLuint count;
	BOOL isdirR;
	BOOL isStrike;
	BOOL ismove;
	BOOL isprefight;
	snowball sb;
};


typedef struct
{
	GLubyte	*imageData;											// Image Data (Up To 32 Bits)
	GLuint	bpp;												// Image Color Depth In Bits Per Pixel.
	GLuint	width;												// Image Width
	GLuint	height;												// Image Height
	GLuint	texID;												// Texture ID Used To Select A Texture
} TGATexture;