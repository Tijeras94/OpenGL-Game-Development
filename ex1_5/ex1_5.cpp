/*****************************************************************************************************************
**
**	OpenGL Game Development
**	EX1_5.CPP (Example 1.5)
**
**	Copyright:	2003, Chris Seddon. 
**	Written by: Chris Seddon
**	Date:		Mar 1, 2003
**
******************************************************************************************************************/

/* Headers *******************************************************************************************************/
#include <windows.h>
#include <winbase.h>
#include <stdio.h>

#include "resource.h"

/* Constants *****************************************************************************************************/
#define DEFAULT_BUTTON_WIDTH	100
#define DEFAULT_BUTTON_HEIGHT	20

/* Global Variables **********************************************************************************************/
HINSTANCE	GlobalInstance;
HMENU 		Menu;
HMENU		PopupMenu;
HWND		Window;
HWND		bCreateWall;

/* MapDetailsDlgProc *****************************************************************************************************/
LRESULT CALLBACK MapDetailsDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			SetDlgItemText (hWnd, IDC_MAP_DETAILS_NAME, "Map Name");

			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_RULES, LB_ADDSTRING, 0, (LPARAM)"Erase Me");
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_RULES, LB_RESETCONTENT, 0, 0);
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_RULES, LB_ADDSTRING, 0, (LPARAM)"Exit");
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_RULES, LB_ADDSTRING, 0, (LPARAM)"Get Fragged");
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_RULES, LB_SETCURSEL, 0, 1);

			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_TYPE, CB_ADDSTRING, 0, (LPARAM)"Erase Me");
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_TYPE, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_TYPE, CB_ADDSTRING, 0, (LPARAM)"Single Player");
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_TYPE, CB_ADDSTRING, 0, (LPARAM)"Multi Player");
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_TYPE, CB_SETCURSEL, 0, 1);
		} break;

		case WM_COMMAND:
		{
			if (wParam == IDOK)
			{
				long level_rule = SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_RULES, LB_GETCURSEL, 0, 0);
				long level_type = SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_TYPE, CB_GETCURSEL, 0, 0);

				char temp[500];

				sprintf (temp,"Level Type: %i\r\nLevel Rule: %i\r\nOk Button!", level_type, level_rule);
				MessageBox (hWnd, temp, "Ok", MB_OK);

				EndDialog (hWnd, 0);
			}
			else if (wParam == IDCANCEL)
			{
				MessageBox (hWnd, "Cancel Button!", "Cancel", MB_OK);
				EndDialog (hWnd, 0);
			}
		} break;
	}
	return (0);
}

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
	else if (wParam == ID_MAP_DETAILS) DialogBox (GlobalInstance, MAKEINTRESOURCE(IDD_MAP_DETAILS), NULL, (DLGPROC)MapDetailsDlgProc);


	// Popup Menu Items
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

	GlobalInstance		= hInstance;

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