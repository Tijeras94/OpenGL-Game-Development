/*****************************************************************************************************************
**
**	OpenGL Game Development
**	EX6_1.CPP (Example 6.1)
**
**	Copyright:	2003, Chris Seddon. 
**	Written by: Chris Seddon
**	Date:		July 12, 2003
**
******************************************************************************************************************/

/* Headers *******************************************************************************************************/
#include <windows.h>
#include <winbase.h>
#include <stdio.h>

#include "resource.h"

#include "raster.h"
#include "map.h"


/* Constants *****************************************************************************************************/
#define DEFAULT_BUTTON_WIDTH	100
#define DEFAULT_BUTTON_HEIGHT	20


/* Enumerated Type ****************************************************************************************************/
enum { CREATE_MODE_NULL = 0, CREATE_MODE_START, CREATE_MODE_SIZE, CREATE_MODE_FINISH };
enum { DRAW_MODE_WIREFRAME = 0, DRAW_MODE_SOLID };


/* Structures ****************************************************************************************************/
typedef struct
{
	long	mouse_x;
	long	mouse_y;

	double	world_x;
	double	world_y;
	double	world_z;
} COORDS;


typedef struct
{
	long	mode;
	long	type;

	COORDS	start;
	COORDS	finish;
} CREATION_COORDS;


typedef struct 
{
	long	draw_mode;
} CONFIG;


typedef struct
{
	bool draw_floor;
	bool draw_ceiling;
	bool draw_wall;
} LAYER;


/* Global Variables **********************************************************************************************/
HINSTANCE		GlobalInstance;
HMENU 			Menu;
HMENU			PopupMenu;
HWND			Window;
HWND			RenderWindow;
HWND			bCreateWall;
HWND			bCreateFloor;
HWND			bCreateCeiling;
RASTER			raster;

CREATION_COORDS	creation_coords;	
MAP				*map = new MAP;

CONFIG			config;
LAYER			layer;


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


/* DrawWireframe *****************************************************************************************************/
void DrawWireframe()
{
	if (map->header.max_objects > 0)
	{			
		for (long i = 0; i < map->header.max_objects; i++)
		{
			if ((layer.draw_floor && map->object[ i ].type == OBJECTTYPE_FLOOR) ||
				(layer.draw_ceiling && map->object[ i ].type == OBJECTTYPE_CEILING) ||
				(layer.draw_wall && map->object[ i ].type == OBJECTTYPE_WALL))				
			{
				glBegin (GL_LINE_LOOP);					
					for (long i2 = 0; i2 < map->object[i].max_vertices; i2++) glVertex2d (map->object[i].vertex[i2].xyz[0], map->object[i].vertex[i2].xyz[2]);											
				glEnd();
			}
		}
	}
}


/* DrawSolid *****************************************************************************************************/
void DrawSolid()
{	
	for (long i = 0; i < map->header.max_objects; i++)
	{				
		glColor3ubv (map->object[i].select_rgb);

		if ((layer.draw_floor && map->object[ i ].type == OBJECTTYPE_FLOOR) ||
			(layer.draw_ceiling && map->object[ i ].type == OBJECTTYPE_CEILING))				
		{
			glBegin (GL_TRIANGLES);		
			for (long i2 = 0; i2 < map->object[ i ].max_triangles; i2++)
			{
				long vertex_0 = map->object[ i ].triangle[ i2 ].point[0];
				long vertex_1 = map->object[ i ].triangle[ i2 ].point[1];
				long vertex_2 = map->object[ i ].triangle[ i2 ].point[2];	

				glVertex2d (map->object[i].vertex[ vertex_0 ].xyz[0],map->object[i].vertex[ vertex_0 ].xyz[2]);
				glVertex2d (map->object[i].vertex[ vertex_1 ].xyz[0],map->object[i].vertex[ vertex_1 ].xyz[2]);
				glVertex2d (map->object[i].vertex[ vertex_2 ].xyz[0],map->object[i].vertex[ vertex_2 ].xyz[2]);
			}
			glEnd();
		}
		else if (layer.draw_wall && map->object[ i ].type == OBJECTTYPE_WALL)
		{
			for (long i2 = 0; i2 < map->object[ i ].max_triangles; i2++)
			{
				long vertex_0 = map->object[ i ].triangle[ i2 ].point[0];
				long vertex_1 = map->object[ i ].triangle[ i2 ].point[1];
				long vertex_2 = map->object[ i ].triangle[ i2 ].point[2];	

				glBegin (GL_LINE_LOOP);		
					glVertex2d (map->object[i].vertex[ vertex_0 ].xyz[0],map->object[i].vertex[ vertex_0 ].xyz[2]);
					glVertex2d (map->object[i].vertex[ vertex_1 ].xyz[0],map->object[i].vertex[ vertex_1 ].xyz[2]);
					glVertex2d (map->object[i].vertex[ vertex_2 ].xyz[0],map->object[i].vertex[ vertex_2 ].xyz[2]);
				glEnd();
			}			
		}
	}
}


/* Render *****************************************************************************************************/
void Render()
{	
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glLoadIdentity();
	glPushMatrix();
		glTranslatef (0.0f, 0.0f, 0.0f);
				
		if (config.draw_mode == DRAW_MODE_WIREFRAME) DrawWireframe();
		else if (config.draw_mode == DRAW_MODE_SOLID) DrawSolid();

		if (creation_coords.type == OBJECTTYPE_WALL)
		{
			glBegin (GL_LINES);
				glVertex2d (creation_coords.start.world_x, creation_coords.start.world_z);
				glVertex2d (creation_coords.finish.world_x, creation_coords.finish.world_z);			
			glEnd();
		}
		else if (creation_coords.type == OBJECTTYPE_FLOOR || creation_coords.type == OBJECTTYPE_CEILING)
		{
			glBegin (GL_LINE_LOOP);
				glVertex2d (creation_coords.start.world_x, creation_coords.start.world_z);
				glVertex2d (creation_coords.finish.world_x, creation_coords.start.world_z);
				glVertex2d (creation_coords.finish.world_x, creation_coords.finish.world_z);			
				glVertex2d (creation_coords.start.world_x, creation_coords.finish.world_z);
			glEnd();
		}

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
	if (lParam == (LPARAM)bCreateWall) 
	{
		creation_coords.type = OBJECTTYPE_WALL;
		SetWindowText (bCreateWall, "*Wall*");
		SetWindowText (bCreateCeiling, "Create Ceiling");
		SetWindowText (bCreateFloor, "Create Floor");
	}
	else if (lParam == (LPARAM)bCreateFloor) 
	{
		creation_coords.type = OBJECTTYPE_FLOOR;
		SetWindowText (bCreateFloor, "*Floor*");
		SetWindowText (bCreateCeiling, "Create Ceiling");
		SetWindowText (bCreateWall, "Create Wall");
	}
	else if (lParam == (LPARAM)bCreateCeiling) 
	{
		creation_coords.type = OBJECTTYPE_CEILING;
		SetWindowText (bCreateCeiling, "*Ceiling*");		
		SetWindowText (bCreateFloor, "Create Floor");
		SetWindowText (bCreateWall, "Create Wall");
	}
	else if (wParam == ID_FILE_EXIT) PostQuitMessage(0);
	else if (wParam == ID_DRAWING_WIREFRAME) 
	{
		CheckMenuItem (Menu, ID_DRAWING_WIREFRAME, MF_CHECKED);
		CheckMenuItem (Menu, ID_DRAWING_SOLID, MF_UNCHECKED);

		config.draw_mode = DRAW_MODE_WIREFRAME;
	}
	else if (wParam == ID_DRAWING_SOLID) 
	{
		CheckMenuItem (Menu, ID_DRAWING_SOLID, MF_CHECKED);
		CheckMenuItem (Menu, ID_DRAWING_WIREFRAME, MF_UNCHECKED);		

		config.draw_mode = DRAW_MODE_SOLID;
	}
	else if (wParam == ID_MAP_DETAILS) DialogBox (GlobalInstance, MAKEINTRESOURCE(IDD_MAP_DETAILS), NULL, (DLGPROC)MapDetailsDlgProc);
	else if (wParam == ID_LAYERS_FLOOR) 
	{
		if (layer.draw_floor) CheckMenuItem (Menu, ID_LAYERS_FLOOR, MF_UNCHECKED);		
		else CheckMenuItem (Menu, ID_LAYERS_FLOOR, MF_CHECKED);
		layer.draw_floor = !layer.draw_floor;		
	}
	else if (wParam == ID_LAYERS_CEILING) 
	{
		if (layer.draw_ceiling) CheckMenuItem (Menu, ID_LAYERS_CEILING, MF_UNCHECKED);		
		else CheckMenuItem (Menu, ID_LAYERS_CEILING, MF_CHECKED);
		layer.draw_ceiling = !layer.draw_ceiling;		
	}
	else if (wParam == ID_LAYERS_WALL) 
	{
		if (layer.draw_wall) CheckMenuItem (Menu, ID_LAYERS_WALL, MF_UNCHECKED);		
		else CheckMenuItem (Menu, ID_LAYERS_WALL, MF_CHECKED);
		layer.draw_wall = !layer.draw_wall;		
	}

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


/* ComputeMouseCoords *******************************************************************************************************/
COORDS	ComputeMouseCoords(long xPos, long yPos)
{
	COORDS	coords;
	RECT	rect;

	float	window_width;
	float	window_height;
	float	window_start_x;
	float	window_start_y;


	coords.mouse_x	= xPos;
	coords.mouse_y	= yPos;


	GetWindowRect (RenderWindow, &rect);
	window_width	= (float)(rect.right - rect.left);
	window_height	= (float)(rect.bottom - rect.top);
	window_start_x	= (float)(coords.mouse_x - rect.left);
	window_start_y	= (float)coords.mouse_y;


	coords.world_x	= (window_start_x / window_width) * 2.0 - 1.0;		
	coords.world_z	= -((window_start_y / window_height) * 2.0 - 1.0);	

	return (coords);
}


/* WMLButtonDown *******************************************************************************************************/
void WMLButtonDown(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	creation_coords.mode	= CREATE_MODE_START;
	creation_coords.start	= ComputeMouseCoords(LOWORD(lParam), HIWORD(lParam));
	creation_coords.finish	= creation_coords.start;
}


/* WMLButtonUp **********************************************************************************************************/
void WMLButtonUp(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (creation_coords.mode != CREATE_MODE_NULL)
	{
		creation_coords.mode	= CREATE_MODE_NULL;
		creation_coords.finish	= ComputeMouseCoords(LOWORD(lParam), HIWORD(lParam));

		if (creation_coords.type == OBJECTTYPE_WALL)
		{
			map->InsertObject ("Wall", creation_coords.type);		
			map->InsertVertex (map->header.max_objects-1, creation_coords.start.world_x, 1, creation_coords.start.world_z);	
			map->InsertVertex (map->header.max_objects-1, creation_coords.finish.world_x, 1, creation_coords.finish.world_z);
			map->InsertVertex (map->header.max_objects-1, creation_coords.finish.world_x, 0, creation_coords.finish.world_z);
			map->InsertVertex (map->header.max_objects-1, creation_coords.start.world_x, 0, creation_coords.start.world_z);								
			map->InsertTriangle (map->header.max_objects-1, 0, 1, 2, 0.0f,0.0f, 1.0f,0.0f, 1.0f,1.0f);
			map->InsertTriangle (map->header.max_objects-1, 2, 3, 0, 1.0f,1.0f, 0.0f,1.0f, 0.0f,0.0f);
		}
		else if (creation_coords.type == OBJECTTYPE_FLOOR)
		{
			map->InsertObject ("Floor", creation_coords.type);
			map->InsertVertex (map->header.max_objects-1, creation_coords.start.world_x, 0, creation_coords.start.world_z);	
			map->InsertVertex (map->header.max_objects-1, creation_coords.finish.world_x, 0, creation_coords.start.world_z);				
			map->InsertVertex (map->header.max_objects-1, creation_coords.finish.world_x, 0, creation_coords.finish.world_z);
			map->InsertVertex (map->header.max_objects-1, creation_coords.start.world_x, 0, creation_coords.finish.world_z);			
			map->InsertTriangle (map->header.max_objects-1, 0, 1, 2, 0.0f,0.0f, 1.0f,0.0f, 1.0f,1.0f);
			map->InsertTriangle (map->header.max_objects-1, 2, 3, 0, 1.0f,1.0f, 0.0f,1.0f, 0.0f,0.0f);
		}		
		else if (creation_coords.type == OBJECTTYPE_CEILING)
		{
			map->InsertObject ("Ceiling", creation_coords.type);
			map->InsertVertex (map->header.max_objects-1, creation_coords.start.world_x, 1, creation_coords.start.world_z);	
			map->InsertVertex (map->header.max_objects-1, creation_coords.finish.world_x, 1, creation_coords.start.world_z);				
			map->InsertVertex (map->header.max_objects-1, creation_coords.finish.world_x, 1, creation_coords.finish.world_z);
			map->InsertVertex (map->header.max_objects-1, creation_coords.start.world_x, 1, creation_coords.finish.world_z);			
			map->InsertTriangle (map->header.max_objects-1, 0, 1, 2, 0.0f,0.0f, 1.0f,0.0f, 1.0f,1.0f);
			map->InsertTriangle (map->header.max_objects-1, 2, 3, 0, 1.0f,1.0f, 0.0f,1.0f, 0.0f,0.0f);
		}			
		
		memset (&creation_coords.start, 0, sizeof(creation_coords.start));
		memset (&creation_coords.finish, 0, sizeof(creation_coords.finish));
	}
}


/* WMMouseMove ***************************************************************************************************/
void WMMouseMove(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char temp[500];

	if (creation_coords.mode != CREATE_MODE_NULL)
	{
		creation_coords.mode	= CREATE_MODE_SIZE;
		creation_coords.finish	= ComputeMouseCoords(LOWORD(lParam), HIWORD(lParam));
		sprintf (temp,"Map Editor, Mx=%i My=%i, X=%0.4f Z=%0.4f", creation_coords.finish.mouse_x, creation_coords.finish.mouse_y, creation_coords.finish.world_x, creation_coords.finish.world_z);
	}	
	else sprintf (temp,"Map Editor, Mx=%i My=%i", LOWORD(lParam), HIWORD(lParam));

	SetWindowText (Window, temp);
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
		case WM_LBUTTONDOWN: WMLButtonDown (hWnd, msg, wParam, lParam); break;
		case WM_LBUTTONUP: WMLButtonUp (hWnd, msg, wParam, lParam); break;
		case WM_MOUSEMOVE: WMMouseMove (hWnd, msg, wParam, lParam); break;
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

	bCreateCeiling = CreateWindow(	"BUTTON", "Create Ceiling", WS_CHILD | WS_VISIBLE,
							0, 100+(DEFAULT_BUTTON_HEIGHT*2), DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
							Window, NULL, hInstance, NULL);

	bCreateFloor = CreateWindow( "BUTTON", "Create Floor", WS_CHILD | WS_VISIBLE,
							0, 100+(DEFAULT_BUTTON_HEIGHT*4), DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
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


	memset (&creation_coords, 0, sizeof(creation_coords));	
	memset (&layer, 1, sizeof(layer));
	memset (&config, 0, sizeof(config));



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
	delete map;


	return (1);
}
