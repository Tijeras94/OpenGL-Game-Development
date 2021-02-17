/*****************************************************************************************************************
**
**	OpenGL Game Development
**	Ex10_1.CPP (Example 10.1)
**
**	Copyright:	2003-2004, Chris Seddon. 
**	Written by: Chris Seddon
**	Date:		Dec 4, 2004
**
*****************************************************************************************************************/

/* Headers *******************************************************************************************************/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "raster.h"

#include "resource.h"


/* Global Variables **********************************************************************************************/
HWND	Window;
DEVMODE	old_resolution;
DEVMODE new_resolution;

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

	while (1)
	{
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage (&msg);
		}
	}

	ChangeDisplaySettings (&old_resolution, 0);

	return (1);
}