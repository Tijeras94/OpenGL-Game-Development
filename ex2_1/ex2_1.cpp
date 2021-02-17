/*****************************************************************************************************************
**
**	OpenGL Game Development
**	EX2_1.CPP (Example 2.1)
**
**	Copyright:	2003, Chris Seddon. 
**	Written by: Chris Seddon
**	Date:		Mar 15, 2003
**
******************************************************************************************************************/

/* Headers *******************************************************************************************************/
#include <windows.h>
#include <winbase.h>
#include <stdio.h>

#include "resource.h"

#include "raster.h"

/* Constants *****************************************************************************************************/
#define DEFAULT_BUTTON_WIDTH	100
#define DEFAULT_BUTTON_HEIGHT	20

/* Global Variables **********************************************************************************************/
HINSTANCE	GlobalInstance;
HMENU 		Menu;
HMENU		PopupMenu;
HWND		Window;
HWND		RenderWindow;
HWND		bCreateWall;

RASTER		raster;


/* ResizeGLWindow *************************************************************************************************************/
void ResizeGLWindow(long width, long height)
{
	glViewport(0, 0, (GLsizei) width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-200,200, -200,-200, -2000,2000);
	glMatrixMode(GL_MODELVIEW);			
}


/* SetGLDefaults *****************************************************************************************************/
void SetGLDefaults()
{		
	glEnable (GL_DEPTH_TEST);		
	glDisable (GL_CULL_FACE);

	glClearColor (0.6f, 0.6f, 0.6f, 1.0f);
}


/* Render *****************************************************************************************************/
void Render()
{	
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glLoadIdentity();
	glPushMatrix();
		glTranslatef (0.0f, 0.0f, 0.0f);
		
		
		glBegin (GL_TRIANGLES);
			glVertex3f (0.0f, 0.0f, 0.0f);
			glVertex3f (0.0f, 1.0f, 0.0f);
			glVertex3f (1.0f, 1.0f, 0.0f);
		glEnd();


		glBegin (GL_QUADS);
			glVertex3f (0.05f, -0.05f, 0.0f);
			glVertex3f (0.95f, -0.05f, 0.0f);
			glVertex3f (0.95f, -0.95f, 0.0f);			
			glVertex3f (0.05f, -0.95f, 0.0f);
		glEnd();
		
		
		glBegin (GL_POLYGON);
			glVertex3f (-0.25f, -0.25f, 0.0f);
			glVertex3f (-0.50f, -0.125f, 0.0f);
			glVertex3f (-0.75f, -0.25f, 0.0f);
			glVertex3f (-0.875, -0.5f, 0.0f);
			glVertex3f (-0.75f, -0.75f, 0.0f);			
			glVertex3f (-0.50f, -0.875f, 0.0f);				
			glVertex3f (-0.25f, -0.75f, 0.0f);
			glVertex3f (-0.125f, -0.5f, 0.0f);
		glEnd();


		glBegin (GL_LINES);
			glVertex3f (-0.25f, 0.25f, 0.0f);
			glVertex3f (-0.75f, 0.75f, 0.0f);			
		glEnd();


	glPopMatrix();


	SwapBuffers (raster.hDC);
}


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


/* WMSize *******************************************************************************************************/
void WMSize(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rect;

	GetClientRect (Window, &rect);
	MoveWindow (RenderWindow, DEFAULT_BUTTON_WIDTH, 0, rect.right-rect.left-DEFAULT_BUTTON_WIDTH, rect.bottom-rect.top, true);
	
	GetClientRect (RenderWindow, &rect);
	ResizeGLWindow (rect.right-rect.left, rect.bottom-rect.top);
}


/* WndProc *******************************************************************************************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_DESTROY: PostQuitMessage(0); break;
		case WM_COMMAND: WMCommand (hWnd, msg, wParam, lParam); break;	
		case WM_SIZE: WMSize (hWnd, msg, wParam, lParam); break;
		case WM_RBUTTONUP: DisplayPopupMenu(); break;
	}
	return (DefWindowProc(hWnd, msg, wParam, lParam));
}


/* WinMain *******************************************************************************************************/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevious, LPSTR lpCmdString, int CmdShow)
{
	WNDCLASS	wc;
	MSG			msg;
	RECT		rect;

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
	GetClientRect (Window, &rect);


	bCreateWall = CreateWindow(	"BUTTON", "Create Wall", WS_CHILD | WS_VISIBLE,
							0, 100, DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
							Window, NULL, hInstance, NULL);


	RenderWindow= CreateWindow( "STATIC", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
							DEFAULT_BUTTON_WIDTH, 0, rect.right-rect.left-DEFAULT_BUTTON_WIDTH, rect.bottom-rect.top,
							Window, NULL, hInstance, NULL);


	Menu = LoadMenu (hInstance, MAKEINTRESOURCE(IDR_MENU));
	SetMenu (Window, Menu);
	
	PopupMenu = LoadMenu (hInstance, MAKEINTRESOURCE(IDR_POPUP_MENU));

	
	if (!raster.Init(RenderWindow)) return (0);

	GetClientRect (RenderWindow, &rect);
	ResizeGLWindow (rect.right-rect.left, rect.bottom-rect.top);

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

	raster.Release(RenderWindow);


	return (1);
}