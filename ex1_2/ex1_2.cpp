/*****************************************************************************************************************
**
**	OpenGL Game Development
**	Ex1_2.CPP (Example 1.2)
**
**	Copyright:	2003, Chris Seddon. 
**	Written by: Chris Seddon
**	Date:		Feb 4, 2003
**
******************************************************************************************************************/

/* Headers *******************************************************************************************************/
#include <windows.h>

/* Constants *****************************************************************************************************/
#define DEFAULT_BUTTON_WIDTH	100
#define DEFAULT_BUTTON_HEIGHT	20

/* Global Variables **********************************************************************************************/
HWND Window;
HWND bCreateWall;

/* WMCommand *****************************************************************************************************/
void WMCommand(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (lParam == (LPARAM)bCreateWall) MessageBox (Window,"You Pressed bCreateWall","Congrats!",MB_OK);
}

/* WndProc *******************************************************************************************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_DESTROY: PostQuitMessage(0); break;
		case WM_COMMAND: WMCommand (hWnd, msg, wParam, lParam); break;
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
	
	bCreateWall = CreateWindow(	"BUTTON", "Create Wall", WS_CHILD | WS_VISIBLE,
							0, 100, DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
							Window, NULL, hInstance, NULL);

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

	return (1);
}