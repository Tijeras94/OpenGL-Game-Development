/*****************************************************************************************************************
**
**	OpenGL Game Development
**	Ex1_4.CPP (Example 1.4)
**
**	Copyright:	2003, Chris Seddon. 
**	Written by: Chris Seddon
**	Date:		Feb 21, 2003
**
******************************************************************************************************************/

/* Headers *******************************************************************************************************/
#include <windows.h>
#include <winbase.h>

#include "resource.h"

/* Constants *****************************************************************************************************/
#define DEFAULT_BUTTON_WIDTH	100
#define DEFAULT_BUTTON_HEIGHT	20

/* Global Variables **********************************************************************************************/
HMENU 	Menu;
HMENU	PopupMenu;
HWND	Window;
HWND	bCreateWall;

/* WMCommand *****************************************************************************************************/
void WMCommand(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (lParam == (LPARAM)bCreateWall) MessageBox (Window,"You Pressed bCreateWall","Congrats!",MB_OK);
	else if (wParam == ID_FILE_EXIT) PostQuitMessage(0);
	else if (wParam == ID_DRAWING_WIREFRAME) 
	{
		CheckMenuItem (Menu, ID_DRAWING_WIREFRAME, MF_CHECKED);
		CheckMenuItem (Menu, ID_DRAWING_SOLID, MF_UNCHECKED);
	}
	else if (wParam == ID_DRAWING_SOLID) 
	{
		CheckMenuItem (Menu, ID_DRAWING_SOLID, MF_CHECKED);
		CheckMenuItem (Menu, ID_DRAWING_WIREFRAME, MF_UNCHECKED);		
	}

	
	else if (wParam == ID_POPUP_MOVE) MessageBox (Window, "Move", "Click", MB_OK);
	else if (wParam == ID_POPUP_DELETE) MessageBox (Window, "Delete", "Click", MB_OK);
	else if (wParam == ID_POPUP_TEXTURE) MessageBox (Window, "Texture", "Click", MB_OK);	
	else if (wParam == ID_POPUP_DUPLICATE) MessageBox (Window, "Duplicate", "Click", MB_OK);	
}

/* DisplayPopupMenu **********************************************************************************************/
void DisplayPopupMenu()
{
	HMENU temp = GetSubMenu(PopupMenu, 0);
	POINT point;
	GetCursorPos (&point);
	TrackPopupMenu(temp, TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, 0, Window, NULL);
}

/* WndProc *******************************************************************************************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_DESTROY: PostQuitMessage(0); break;
		case WM_COMMAND: WMCommand (hWnd, msg, wParam, lParam); break;	
		case WM_RBUTTONUP: DisplayPopupMenu(); break;
	}
	return (DefWindowProc(hWnd, msg, wParam, lParam));
}

/* WinMain *******************************************************************************************************/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevious, LPSTR lpCmdString, int CmdShow)
{
	WNDCLASS	wc;
	MSG			msg;

	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hbrBackground	= (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.hCursor			= LoadCursor (NULL, IDC_ARROW);
	wc.hIcon			= LoadIcon (NULL, IDI_APPLICATION);
	wc.hInstance		= hInstance;
	wc.lpfnWndProc		= WndProc;
	wc.lpszClassName	= "ME";
	wc.lpszMenuName		= NULL;
	wc.style			= CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	if (!RegisterClass(&wc))
	{
		MessageBox (NULL,"Error: Cannot Register Class","ERROR!",MB_OK);
		return (0);
	}

	Window = CreateWindow(	"ME", "Map Editor", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
							0, 0, 640, 480,
							NULL, NULL, hInstance, NULL);	
	if (Window == NULL) 
	{
		MessageBox (NULL,"Error: Failed to Create Window","ERROR!",MB_OK);
		return (0);
	}

	bCreateWall = CreateWindow(	"BUTTON", "Create Wall", WS_CHILD | WS_VISIBLE,
							0, 100, DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
							Window, NULL, hInstance, NULL);

	Menu = LoadMenu (hInstance, MAKEINTRESOURCE(IDR_MENU));
	SetMenu (Window, Menu);
	
	PopupMenu = LoadMenu (hInstance, MAKEINTRESOURCE(IDR_POPUP_MENU));

	
	while (1)
	{
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage (&msg);
		}
	}

	return (1);
}