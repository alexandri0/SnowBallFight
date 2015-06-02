#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "game.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

extern bool		isConnected;

#define PORT 6666
#define ADDRESS "127.0.0.1"
man* _me;
char buff[18];
SOCKET sock;
sockaddr_in dest_addr;
BOOL isSVR;
int initSocket(int type, man* myB)
{
	strcpy(&buff[0], "11111111111111111");
	buff[17] = 0;
	_me = myB;
	// шаг 1 - подключение библиотеки 
	if (WSAStartup(0x202, (WSADATA *)&buff[0]))
	{
		WSAGetLastError();
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		WSACleanup();
		return -1;
	}
	//SERVER==================================================
	if (type == SERVER)
	{
		isSVR = TRUE;
		sockaddr_in local_addr;
		local_addr.sin_family = AF_INET;
		local_addr.sin_addr.s_addr = INADDR_ANY;
		local_addr.sin_port = htons(PORT);

		if (bind(sock, (sockaddr *)&local_addr,
			sizeof(local_addr)))
		{
			closesocket(sock);
			WSACleanup();
			return -1;
		}
		sockaddr_in client_addr;
		int client_addr_size = sizeof(client_addr);
		MessageBox(HWND_DESKTOP, "WAITING CLIENT", "Please, wait...", MB_OK | MB_ICONINFORMATION);
		int bsize = recvfrom(sock, &buff[0], sizeof(buff) - 1, 0, (sockaddr *)&client_addr, &client_addr_size);
		if (bsize > 0)
		{
			isConnected = true;
		}
		else
		{
			isConnected = false;
			return -1;
		}
		// ќпредел€ем IP-адрес клиента и прочие атрибуты
		HOSTENT *hst;
		hst = gethostbyaddr((char *)&client_addr.sin_addr, 4, AF_INET);
		// добавление завершающего нул€
		buff[0] = 'A';
		buff[17] = 0;
		// посылка датаграммы клиенту
		sendto(sock, &buff[0], bsize, 0,
			(sockaddr *)&client_addr, sizeof(client_addr));
	}
	//CLIENT===================================================
	else if (type == CLIENT)
	{
		HOSTENT *hst;
		
		isSVR = FALSE;
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = htons(PORT);

		// определение IP-адреса узла
		if (inet_addr(ADDRESS))
			dest_addr.sin_addr.s_addr = inet_addr(ADDRESS);
		else
			{
				closesocket(sock);
				WSACleanup();
				return -1;
			}
		strcpy(&buff[0], "1111222233334444c");
		buff[17] = 0;
		sendto(sock, &buff[0], strlen(&buff[0]), 0,
			(sockaddr *)&dest_addr, sizeof(dest_addr));

		//			if (!strcmp(&buff[0], "quit\n")) break;

		// ѕрием сообщени€ с сервера
		sockaddr_in server_addr;
		int server_addr_size = sizeof(server_addr);

		int n = recvfrom(sock, &buff[0], sizeof(buff) - 1, 0,
			(sockaddr *)&server_addr, &server_addr_size);
		if (n > 0)
		{
			isConnected = true;
		}
		else
		{
			isConnected = false;
			return -1;
		}
		buff[17] = 0;
	}
	else return -1;

	return 2;
}
void Networking(void* enemy)
{
	int coord[4];
	int x;
	strcpy(&buff[0], "1111222233334444c");
	buff[17] = 0;
	man* enem = (man*)enemy;
	if (isSVR)
	{
		while (1)
		{
			sockaddr_in client_addr;
			HOSTENT *hst;
			int client_addr_size = sizeof(client_addr);

			int bsize = recvfrom(sock, &buff[0], sizeof(buff) - 1, 0, (sockaddr *)&client_addr, &client_addr_size);
			
			hst = gethostbyaddr((char *)&client_addr.sin_addr, 4, AF_INET);
			if (buff[16] == 'd')
			{
				MessageBox(HWND_DESKTOP, "Opponent disconnected", "Error", MB_OK | MB_ICONEXCLAMATION);
				break;
			}
			x = 0;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
					coord[i] += (buff[x + j]-48) * 1000 / pow((float)10, j);
				x += 4;
			}
			enem->x = (GLfloat)coord[0];
			enem->y = (GLfloat)coord[1];
			enem->sb.x = (GLfloat)coord[2];
			enem->sb.y = (GLfloat)coord[3];
			//======================================================================
			 WaitForSingleObject(hSendMutex, 500);
			coord[0] = (int)_me->x;
			coord[1] = (int)_me->y;
			coord[2] = (int)_me->sb.x;
			coord[3] = (int)_me->sb.y;
			x = 0;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 3; j >-1; j--)
				{
					buff[x + j] = (coord[i] % 10)+48;
					coord[i] /= 10;
				}
				x += 4;
			}
			if (isConnected) buff[16] = 'c';
			else buff[16] = 'd';
			// добавление завершающего нул€
			buff[17] = 0;
			// посылка датаграммы клиенту
			sendto(sock, &buff[0], bsize, 0,
				(sockaddr *)&client_addr, sizeof(client_addr));

		}
	}
	else
	{
		while (1)
		{
			WaitForSingleObject(hSendMutex, 500);
			coord[0] = (int)_me->x;
			coord[1] = (int)_me->y;
			coord[2] = (int)_me->sb.x;
			coord[3] = (int)_me->sb.y;
			x = 0;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 3; j >-1; j--)
				{
					buff[x + j] = (coord[i] % 10)+48;
					coord[i] /= 10;
				}
				x += 4;
			}
			if (isConnected) buff[16] = 'c';
			else buff[16] = 'd';
			buff[17] = 0;
			sendto(sock, &buff[0], strlen(&buff[0]), 0,
				(sockaddr *)&dest_addr, sizeof(dest_addr));

//			if (!strcmp(&buff[0], "quit\n")) break;

			// ѕрием сообщени€ с сервера
			sockaddr_in server_addr;
			int server_addr_size = sizeof(server_addr);

			int n = recvfrom(sock, &buff[0], sizeof(buff) - 1, 0,
				(sockaddr *)&server_addr, &server_addr_size);
			if (buff[16] == 'd')
			{
				MessageBox(HWND_DESKTOP, "Opponent disconnected", "Error", MB_OK | MB_ICONEXCLAMATION);
				break;
			}
			x = 0;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
					coord[i] += (buff[x + j]-48) * 1000 / pow((float)10, j);
				x += 4;
			}
			enem->x = (GLfloat)coord[0];
			enem->y = (GLfloat)coord[1];
			enem->sb.x = (GLfloat)coord[2];
			enem->sb.y = (GLfloat)coord[3];
			buff[17] = 0;
			enemy = enem;
			// ¬ывод прин€того с сервера сообщени€ на экран
		}
	}

	closesocket(sock);
	WSACleanup();
	return;
}
