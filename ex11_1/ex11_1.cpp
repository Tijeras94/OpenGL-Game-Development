/*****************************************************************************************************************
**
**	OpenGL Game Development
**	Ex11_1.CPP (Example 11.1)
**
**	Copyright:	2003-2004, Chris Seddon. 
**	Written by: Chris Seddon
**	Date:		Jan 6, 2004
**
*****************************************************************************************************************/

/* Headers *******************************************************************************************************/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "raster.h"
#include "texture.h"

#include "resource.h"


/* Constants *****************************************************************************************************/
#define MAX_TEXTURE_NAMES	5
#define MAX_TEXTURE_FILES	1


/* Structures ****************************************************************************************************/
typedef struct
{
	char			filename[ 100 ];	
	bool			is_transparent;
	long			filter_type;
	long			wrap_type;
} TEXTURE_FILE;


/* Global Variables **********************************************************************************************/
HWND	Window;
DEVMODE	old_resolution;
DEVMODE new_resolution;

RASTER	raster;

GLuint	TextureName[ MAX_TEXTURE_NAMES ];
TEXTURE_FILE texture_file[] = 
	{	
		{"hud.bmp", true, GL_NEAREST, GL_REPEAT},
	};

	

/* glBitBlt ******************************************************************************************************/
void glBitBlt(long texture_id, long start_x, long start_y, long width, long height)
{
	glBindTexture (GL_TEXTURE_2D, TextureName[ texture_id ]);

	glMatrixMode (GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix();
	glOrtho (0,639, 479,0, 0,1);	
	
	glColor3f (1.0f, 1.0f, 1.0f);
	glBegin (GL_QUADS);		
		glTexCoord2f (0.0f, 1.0f);
		glVertex2i (start_x, start_y);
		
		glTexCoord2f (1.0f, 1.0f);
		glVertex2i (start_x+width, start_y);

		glTexCoord2f (1.0f, 0.0f);
		glVertex2i (start_x+width, start_y+height);

		glTexCoord2f (0.0f, 0.0f);
		glVertex2i (start_x, start_y+height);		
	glEnd();

	glPopMatrix();
	glMatrixMode (GL_PROJECTION);
		glPopMatrix();		
	glMatrixMode (GL_MODELVIEW);	
}


/* Render ********************************************************************************************************/
void Render()
{
	glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);	

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBitBlt(1, 0,240,640,240);

	SwapBuffers(raster.hDC);
}


/* LoadTexture *******************************************************************************************************/
void LoadTexture(char *filename, long texture_id, long mag_filter, long min_filter, long wrap_type)
{
	TEXTURE texture;
	
	if (!texture.Load (filename)) return;
		glBindTexture (GL_TEXTURE_2D, TextureName[ texture_id ]);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, texture.info_header.biWidth, texture.info_header.biHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, texture.data);					
		gluBuild2DMipmaps (GL_TEXTURE_2D, GL_RGB, texture.info_header.biWidth, texture.info_header.biHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, texture.data);    

		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_type);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_type);				
		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	texture.Release();
}


/* LoadTransparentTexture ********************************************************************************************/
void LoadTransparentTexture(char *filename, long texture_id, long mag_filter, long min_filter, long wrap_type, unsigned char trans_r, unsigned char trans_g, unsigned char trans_b)
{
	TEXTURE			texture;
	unsigned char	*alpha_texture=NULL;
	long			texel;

	if (!texture.Load (filename)) return;				
		
		alpha_texture = new unsigned char[ texture.info_header.biWidth*texture.info_header.biHeight*4+1 ];
		for (texel = 0; texel < texture.info_header.biWidth*texture.info_header.biHeight; texel++)
		{
			alpha_texture[ texel*4 ] = texture.data[ texel*3 ];
			alpha_texture[ texel*4+1]= texture.data[ texel*3+1 ];
			alpha_texture[ texel*4+2]= texture.data[ texel*3+2 ];

			if (texture.data[ texel*3 ] == trans_b &&
				texture.data[ texel*3+1]== trans_g &&
				texture.data[ texel*3+2]== trans_r) alpha_texture[ texel*4+3] = 0;
			else alpha_texture[ texel*4+3] = 255;			
		}

		glBindTexture (GL_TEXTURE_2D, TextureName[ texture_id ]);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, texture.info_header.biWidth, texture.info_header.biHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, alpha_texture);							
		gluBuild2DMipmaps (GL_TEXTURE_2D, GL_RGBA, texture.info_header.biWidth, texture.info_header.biHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, alpha_texture);    

		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_type);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_type);
		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		delete [] alpha_texture;
		alpha_texture = NULL;
	texture.Release();
}


/* SetGLDefaults *****************************************************************************************************/
void SetGLDefaults()
{	
	long tex_id;
	char texture_path[] = "../media/";
	char filename[50];

	glClearColor (0.0f, 0.0f, 0.0f, 1.0f);	
	glEnable (GL_DEPTH_TEST);	
	glDisable (GL_CULL_FACE);
	glEnable (GL_TEXTURE_2D);	
	glEnable(GL_BLEND);

	glGenTextures (MAX_TEXTURE_NAMES, TextureName);	
	for (tex_id = 0; tex_id < MAX_TEXTURE_FILES; tex_id++)
	{
		sprintf (filename,"%s%s", texture_path, texture_file[ tex_id ].filename);

		if (texture_file[ tex_id ].is_transparent) LoadTransparentTexture (filename, tex_id+1, texture_file[ tex_id ].filter_type, texture_file[ tex_id ].filter_type, texture_file[ tex_id ].wrap_type, 0,0,255);
		else LoadTexture (filename, tex_id+1, texture_file[ tex_id ].filter_type, texture_file[ tex_id ].filter_type, texture_file[ tex_id ].wrap_type);
	}
}


/* WndProc *******************************************************************************************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{		
		case WM_DESTROY: PostQuitMessage(0); break;
	}
	return (DefWindowProc(hWnd, msg, wParam, lParam));
}


/* Config Dialog Box *********************************************************************************************/
LRESULT CALLBACK ConfigDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			DEVMODE dm;
			long	mode=0;

			EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &old_resolution);			
			while (EnumDisplaySettings (NULL, mode, &dm) != false)
			{
				char temp[500];

				sprintf (temp, "%ix%ix%i", dm.dmPelsWidth, dm.dmPelsHeight, dm.dmBitsPerPel);
				if (dm.dmBitsPerPel > 15) SendDlgItemMessage (hWnd, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)temp);

				mode++;
			}
			SendDlgItemMessage (hWnd, IDC_RESOLUTION, CB_SETCURSEL, 0, 1);			
		} break;
		case WM_COMMAND:
		{
			if (wParam == IDEXIT) EndDialog (hWnd, 0);
			else if (wParam == IDOK) 
			{				
				long	cursel = SendDlgItemMessage (hWnd, IDC_RESOLUTION, CB_GETCURSEL, 0, 0);
				char	temp[500];


				SendDlgItemMessage (hWnd, IDC_RESOLUTION, CB_GETLBTEXT, cursel, (LPARAM)(char *)temp);
				new_resolution = old_resolution;
				sscanf (temp, "%ix%ix%i", &new_resolution.dmPelsWidth, &new_resolution.dmPelsHeight, &new_resolution.dmBitsPerPel);

				ChangeDisplaySettings (&new_resolution, 0);
				EndDialog (hWnd, 1);
			}
		} break;	
	}
	return (0);
}


/* WinMain *******************************************************************************************************/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevious, LPSTR lpCmdString, int CmdShow)
{
	WNDCLASS	wc;
	MSG			msg;
	
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hCursor			= LoadCursor (NULL, IDC_ARROW);
	wc.hIcon			= LoadIcon (NULL, IDI_APPLICATION);
	wc.hInstance		= hInstance;
	wc.lpfnWndProc		= WndProc;
	wc.lpszClassName	= "GE";
	wc.lpszMenuName		= NULL;
	wc.style			= CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	if (!RegisterClass(&wc))
	{
		MessageBox (NULL,"Error: Cannot Register Class","ERROR!",MB_OK);
		return (0);
	}

	if (!DialogBox (hInstance, MAKEINTRESOURCE(IDD_CONFIG), NULL, (DLGPROC)ConfigDlgProc)) 
	{
		MessageBox (NULL, "Thanks for running the game engine", NULL, MB_OK);
		return (0);
	}


	Window = CreateWindow(	"GE", "Game Engine", WS_POPUP | WS_VISIBLE,
							0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
							NULL, NULL, hInstance, NULL);
	if (Window == NULL) 
	{
		MessageBox (NULL,"Error: Failed to Create Window","ERROR!",MB_OK);
		return (0);
	}	


	if (!raster.Init (Window, (unsigned char)new_resolution.dmBitsPerPel))
	{
		MessageBox (NULL, "Error: Failed to Initialize OpenGL", "ERROR!", MB_OK);
		return (0);
	}

	SetGLDefaults();
	

	while (1)
	{
		Render();

		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage (&msg);
		}
	}


	raster.Release(Window);

	ChangeDisplaySettings (&old_resolution, 0);

	return (1);
}