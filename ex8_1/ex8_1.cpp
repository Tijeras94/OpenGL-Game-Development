/*****************************************************************************************************************
**
**	OpenGL Game Development
**	EX8_1.CPP (Example 8.1)
**
**	Copyright:	2003, Chris Seddon. 
**	Written by: Chris Seddon
**	Date:		Sept 13, 2003
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
enum { MODE_TYPE_MOVE = 0 };

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
	bool draw_entity;
	bool draw_item;
	bool draw_sound;
	bool draw_light;
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
HWND			bPlaceStartPosition;
HWND			bPlaceDMPosition;
HWND			bInsertEntity;
HWND			bInsertItem;
HWND			bInsertSound;
HWND			bInsertLight;
HWND			bSelectObject;
RASTER			raster;

CREATION_COORDS	creation_coords;	
MAP				*map = new MAP;

CONFIG			config;
LAYER			layer;

unsigned char	select_rgb[3];


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
			if (select_rgb[0] == map->object[i].select_rgb[0] && select_rgb[1] == map->object[i].select_rgb[1] && select_rgb[2] == map->object[i].select_rgb[2]) glColor3f (1.0f, 0.0f, 0.0f);
			else glColor3f (1.0f, 1.0f, 1.0f);
	
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
		if (select_rgb[0] == map->object[i].select_rgb[0] && select_rgb[1] == map->object[i].select_rgb[1] && select_rgb[2] == map->object[i].select_rgb[2]) glColor3f (1.0f, 0.0f, 0.0f);
		else glColor3ubv (map->object[i].select_rgb);

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


/* DrawStartPosition *****************************************************************************************************/
void DrawStartPosition()
{
	glColor3f (0.0f, 0.0f, 1.0f);	
	glBegin (GL_QUADS);
		glVertex2d (map->details.single_player.xyz[0], map->details.single_player.xyz[2]-0.01);
		glVertex2d (map->details.single_player.xyz[0]+0.01, map->details.single_player.xyz[2]);
		glVertex2d (map->details.single_player.xyz[0], map->details.single_player.xyz[2]+0.01);
		glVertex2d (map->details.single_player.xyz[0]-0.01, map->details.single_player.xyz[2]);
	glEnd();
	glColor3f (1.0f, 1.0f, 1.0f);
}


/* DrawDeathMatchPositions *****************************************************************************************************/
void DrawDeathMatchPositions()
{
	glColor3f (1.0f, 0.0f, 1.0f);
	glBegin (GL_QUADS);
		glVertex2d (map->details.deathmatch[0].xyz[0], map->details.deathmatch[0].xyz[2]-0.01);
		glVertex2d (map->details.deathmatch[0].xyz[0]+0.01, map->details.deathmatch[0].xyz[2]);
		glVertex2d (map->details.deathmatch[0].xyz[0], map->details.deathmatch[0].xyz[2]+0.01);
		glVertex2d (map->details.deathmatch[0].xyz[0]-0.01, map->details.deathmatch[0].xyz[2]);
		
		glVertex2d (map->details.deathmatch[1].xyz[0], map->details.deathmatch[1].xyz[2]-0.01);
		glVertex2d (map->details.deathmatch[1].xyz[0]+0.01, map->details.deathmatch[1].xyz[2]);
		glVertex2d (map->details.deathmatch[1].xyz[0], map->details.deathmatch[1].xyz[2]+0.01);
		glVertex2d (map->details.deathmatch[1].xyz[0]-0.01, map->details.deathmatch[1].xyz[2]);
	glEnd();
	glColor3f (1.0f, 1.0f, 1.0f);
}


/* DrawEntities *****************************************************************************************************/
void DrawEntities(bool want_srgb=false)
{
	glColor3f (0.0f, 1.0f, 1.0f);
	glBegin (GL_QUADS);
		for (long i = 0; i < map->header.max_entities; i++)
		{
			if (map->entity[i].select_rgb[0] == select_rgb[0] && map->entity[i].select_rgb[1] == select_rgb[1] && map->entity[i].select_rgb[2] == select_rgb[2]) glColor3f (1.0f, 0.0f, 0.0f);
			else 
			{
				if (want_srgb) glColor3ubv (map->entity[i].select_rgb);
				else glColor3f (0.0f, 1.0f, 1.0f);
			}

			glVertex2d (map->entity[i].xyz[0], map->entity[i].xyz[2]-0.01);
			glVertex2d (map->entity[i].xyz[0]+0.01, map->entity[i].xyz[2]);
			glVertex2d (map->entity[i].xyz[0], map->entity[i].xyz[2]+0.01);
			glVertex2d (map->entity[i].xyz[0]-0.01, map->entity[i].xyz[2]);
		}
	glEnd();
	glColor3f (1.0f, 1.0f, 1.0f);
}


/* DrawItems *****************************************************************************************************/
void DrawItems(bool want_srgb=false)
{
	glColor3f (1.0f, 1.0f, 0.0f);
	glBegin (GL_QUADS);
		for (long i = 0; i < map->header.max_items; i++)
		{
			if (map->item[i].select_rgb[0] == select_rgb[0] && map->item[i].select_rgb[1] == select_rgb[1] && map->item[i].select_rgb[2] == select_rgb[2]) glColor3f (1.0f, 0.0f, 0.0f);
			else 
			{
				if (want_srgb) glColor3ubv (map->item[i].select_rgb);
				else glColor3f (1.0f, 1.0f, 0.0f);
			}

			glVertex2d (map->item[i].xyz[0], map->item[i].xyz[2]-0.01);
			glVertex2d (map->item[i].xyz[0]+0.01, map->item[i].xyz[2]);
			glVertex2d (map->item[i].xyz[0], map->item[i].xyz[2]+0.01);
			glVertex2d (map->item[i].xyz[0]-0.01, map->item[i].xyz[2]);
		}
	glEnd();
	glColor3f (1.0f, 1.0f, 1.0f);
}


/* DrawSounds *****************************************************************************************************/
void DrawSounds(bool want_srgb=false)
{
	glColor3f (0.0f, 1.0f, 0.0f);
	glBegin (GL_QUADS);
		for (long i = 0; i < map->header.max_sounds; i++)
		{
			if (map->sound[i].select_rgb[0] == select_rgb[0] && map->sound[i].select_rgb[1] == select_rgb[1] && map->sound[i].select_rgb[2] == select_rgb[2]) glColor3f (1.0f, 0.0f, 0.0f);
			else 
			{
				if (want_srgb) glColor3ubv (map->sound[i].select_rgb);
				else glColor3f (0.0f, 1.0f, 0.0f);
			}

			glVertex2d (map->sound[i].xyz[0], map->sound[i].xyz[2]-0.01);
			glVertex2d (map->sound[i].xyz[0]+0.01, map->sound[i].xyz[2]);
			glVertex2d (map->sound[i].xyz[0], map->sound[i].xyz[2]+0.01);
			glVertex2d (map->sound[i].xyz[0]-0.01, map->sound[i].xyz[2]);
		}
	glEnd();
	glColor3f (1.0f, 1.0f, 1.0f);
}


/* DrawLights *****************************************************************************************************/
void DrawLights(bool want_srgb=false)
{
	glColor3f (0.5f, 1.0f, 0.0f);
	glBegin (GL_QUADS);
		for (long i = 0; i < map->header.max_lights; i++)
		{			
			if (map->light[i].select_rgb[0] == select_rgb[0] && map->light[i].select_rgb[1] == select_rgb[1] && map->light[i].select_rgb[2] == select_rgb[2]) glColor3f (1.0f, 0.0f, 0.0f);
			else 
			{
				if (want_srgb) glColor3ubv (map->light[i].select_rgb);
				else glColor3f (0.5f, 1.0f, 0.0f);
			}			

			glVertex2d (map->light[i].xyz[0], map->light[i].xyz[2]-0.01);
			glVertex2d (map->light[i].xyz[0]+0.01, map->light[i].xyz[2]);
			glVertex2d (map->light[i].xyz[0], map->light[i].xyz[2]+0.01);
			glVertex2d (map->light[i].xyz[0]-0.01, map->light[i].xyz[2]);
		}
	glEnd();
	glColor3f (1.0f, 1.0f, 1.0f);
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
		
		DrawStartPosition();
		DrawDeathMatchPositions();
		if (layer.draw_entity) DrawEntities();
		if (layer.draw_item) DrawItems();
		if (layer.draw_sound) DrawSounds();
		if (layer.draw_light) DrawLights();


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
			SetDlgItemText (hWnd, IDC_MAP_DETAILS_NAME, map->details.map_name);

			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_RULES, LB_RESETCONTENT, 0, 0);
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_RULES, LB_ADDSTRING, 0, (LPARAM)"Exit");
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_RULES, LB_ADDSTRING, 0, (LPARAM)"Get Fragged");
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_RULES, LB_SETCURSEL, map->details.map_exit_rules, 0);
		
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_TYPE, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_TYPE, CB_ADDSTRING, 0, (LPARAM)"Single Player");
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_TYPE, CB_ADDSTRING, 0, (LPARAM)"Multi Player");
			SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_TYPE, CB_SETCURSEL, map->details.map_type, 0);
		} break;

		case WM_COMMAND:
		{
			if (wParam == IDOK)
			{
				GetDlgItemText (hWnd, IDC_MAP_DETAILS_NAME, map->details.map_name, 500);
				map->details.map_type		= SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_TYPE, CB_GETCURSEL, 0, 0);
				map->details.map_exit_rules	= SendDlgItemMessage (hWnd, IDC_MAP_DETAILS_LEVEL_RULES, LB_GETCURSEL, 0, 0);				

				EndDialog (hWnd, 0);
			}
			else if (wParam == IDCANCEL) EndDialog (hWnd, 0);
		} break;
	}
	return (0);
}


/* ShowSelectedButton *****************************************************************************************************/
void ShowSelectedButton()
{
	SetWindowText (bCreateFloor, "Create Floor");
	SetWindowText (bCreateCeiling, "Create Ceiling");
	SetWindowText (bCreateWall, "Create Wall");
	SetWindowText (bPlaceStartPosition, "Place Start");
	SetWindowText (bPlaceDMPosition, "Place DM");
	SetWindowText (bInsertEntity, "Insert Entity");
	SetWindowText (bInsertItem, "Insert Item");
	SetWindowText (bInsertSound, "Insert Sound");
	SetWindowText (bInsertLight, "Insert Light");
	
	switch (creation_coords.type)
	{
		case OBJECTTYPE_FLOOR: SetWindowText (bCreateFloor, "*Floor*"); break;
		case OBJECTTYPE_CEILING: SetWindowText (bCreateCeiling, "*Ceiling*"); break;
		case OBJECTTYPE_WALL: SetWindowText (bCreateWall, "*Wall*"); break;
		case ITEM_START_POSITION: SetWindowText (bPlaceStartPosition, "*StartPos*"); break;
		case ITEM_DM_POSITION: SetWindowText (bPlaceDMPosition, "*DMPos*"); break;
		case ITEM_ENTITY: SetWindowText (bInsertEntity, "*Entity*"); break;
		case ITEM: SetWindowText (bInsertItem, "*Item*"); break;
		case SOUND: SetWindowText (bInsertSound, "*Sound*"); break;
		case LIGHT: SetWindowText (bInsertLight, "*Light*"); break;
	}
}


/* AssignTextureDlgProc *****************************************************************************************************/
LRESULT CALLBACK AssignTextureDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	switch (msg)
	{
		case WM_INITDIALOG:
		{				
			long obj = -1;

			for (long i = 0; i < map->header.max_objects; i++)
			{
				if (map->object[i].select_rgb[0] == select_rgb[0] &&
					map->object[i].select_rgb[1] == select_rgb[1] &&
					map->object[i].select_rgb[2] == select_rgb[2]) obj = i;								
			}
			if (obj < 0) 
			{
				MessageBox (hWnd, "Selected Object cannot be assigned a texture", "Oops!", MB_OK);
				EndDialog (hWnd, 0);
			}
			else 
			{
				if (map->object[obj].max_textures == 0)
				{
					map->InsertTexture (obj, "MYBITMAP1.BMP");
					map->InsertTexture (obj, "MYBITMAP2.BMP");
				}

				SetDlgItemText (hWnd, IDC_ASSIGNTEXTURE_FILENAME1, map->object[ obj ].texture[0].filename);
				SetDlgItemText (hWnd, IDC_ASSIGNTEXTURE_FILENAME2, map->object[ obj ].texture[1].filename);

				SendDlgItemMessage (hWnd, IDC_ASSIGNTEXTURE_TYPE1, CB_RESETCONTENT, 0, 0);
				SendDlgItemMessage (hWnd, IDC_ASSIGNTEXTURE_TYPE1, CB_ADDSTRING, 0, (LPARAM)"Regular");
				SendDlgItemMessage (hWnd, IDC_ASSIGNTEXTURE_TYPE1, CB_ADDSTRING, 0, (LPARAM)"Masked");				
				SendDlgItemMessage (hWnd, IDC_ASSIGNTEXTURE_TYPE1, CB_SETCURSEL, map->object[ obj ].texture[0].style-1, 0);

				SendDlgItemMessage (hWnd, IDC_ASSIGNTEXTURE_TYPE2, CB_RESETCONTENT, 0, 0);
				SendDlgItemMessage (hWnd, IDC_ASSIGNTEXTURE_TYPE2, CB_ADDSTRING, 0, (LPARAM)"None");
				SendDlgItemMessage (hWnd, IDC_ASSIGNTEXTURE_TYPE2, CB_ADDSTRING, 0, (LPARAM)"Regular");
				SendDlgItemMessage (hWnd, IDC_ASSIGNTEXTURE_TYPE2, CB_ADDSTRING, 0, (LPARAM)"Masked");			
				SendDlgItemMessage (hWnd, IDC_ASSIGNTEXTURE_TYPE2, CB_ADDSTRING, 0, (LPARAM)"BumpMap");
				SendDlgItemMessage (hWnd, IDC_ASSIGNTEXTURE_TYPE2, CB_SETCURSEL, map->object[ obj ].texture[1].style, 0);
			}
		} break;
		case WM_COMMAND:
		{
			if (wParam == IDCANCEL) EndDialog (hWnd, 0);
			else if (wParam == IDOK) 
			{
				long obj = -1;
				for (long i = 0; i < map->header.max_objects; i++)
				{
					if (map->object[i].select_rgb[0] == select_rgb[0] &&
						map->object[i].select_rgb[1] == select_rgb[1] &&
						map->object[i].select_rgb[2] == select_rgb[2]) obj = i;								
				}

				GetDlgItemText (hWnd, IDC_ASSIGNTEXTURE_FILENAME1, map->object[obj].texture[0].filename, 500);
				GetDlgItemText (hWnd, IDC_ASSIGNTEXTURE_FILENAME2, map->object[obj].texture[1].filename, 500);
				
				map->object[obj].texture[0].style = SendDlgItemMessage (hWnd, IDC_ASSIGNTEXTURE_TYPE1, CB_GETCURSEL, 0, 0) + 1;
				map->object[obj].texture[1].style = SendDlgItemMessage (hWnd, IDC_ASSIGNTEXTURE_TYPE2, CB_GETCURSEL, 0, 0);				

				EndDialog (hWnd, 1);
			}
		} break;
	}
	return (0);
}


/* Duplicate ********************************************************************************************************/
void Duplicate(HWND hWnd)
{			

	for (long i = 0; i < map->header.max_objects; i++)
	{
		if (map->object[i].select_rgb[0] == select_rgb[0] &&
			map->object[i].select_rgb[1] == select_rgb[1] &&
			map->object[i].select_rgb[2] == select_rgb[2])
		{
			long last_entry = map->header.max_objects;

			map->InsertObject (map->object[i].name, map->object[i].type, map->object[i].special, map->object[i].is_collidable, map->object[i].is_visible);
			for (long i2 = 0; i2 < map->object[i].max_vertices; i2++) map->InsertVertex (last_entry, map->object[i].vertex[i2].xyz[0], map->object[i].vertex[i2].xyz[1], map->object[i].vertex[i2].xyz[2], map->object[i].vertex[i2].rgba[0], map->object[i].vertex[i2].rgba[1], map->object[i].vertex[i2].rgba[2], map->object[i].vertex[i2].rgba[3], map->object[i].vertex[i2].normal[0], map->object[i].vertex[i2].normal[1], map->object[i].vertex[i2].normal[2], map->object[i].vertex[i2].fog_depth);
			for (i2 = 0; i2 < map->object[i].max_triangles; i2++) map->InsertTriangle (last_entry, map->object[i].triangle[i2].point[0], map->object[i].triangle[i2].point[1], map->object[i].triangle[i2].point[2], map->object[i].triangle[i2].uv[0].uv1[0],map->object[i].triangle[i2].uv[0].uv1[1], map->object[i].triangle[i2].uv[0].uv2[0],map->object[i].triangle[i2].uv[1].uv2[1], map->object[i].triangle[i2].uv[0].uv3[0],map->object[i].triangle[i2].uv[0].uv3[1]);										
			for (i2 = 0; i2 < map->object[i].max_textures; i2++) map->InsertTexture (last_entry, map->object[i].texture[i2].filename, map->object[i].texture[i2].style);			
			return;
		}
	}

	for (i = 0; i < map->header.max_entities; i++)
	{
		if (map->entity[i].select_rgb[0] == select_rgb[0] &&
			map->entity[i].select_rgb[1] == select_rgb[1] &&
			map->entity[i].select_rgb[2] == select_rgb[2]) 
		{
			map->InsertEntity (map->entity[i].type, map->entity[i].xyz[0], map->entity[i].xyz[1], map->entity[i].xyz[2], map->entity[i].angle[0], map->entity[i].angle[1], map->entity[i].angle[2], map->entity[i].health, map->entity[i].strength, map->entity[i].armour);
			return;
		}
	}

	for (i = 0; i < map->header.max_items; i++)
	{
		if (map->item[i].select_rgb[0] == select_rgb[0] &&
			map->item[i].select_rgb[1] == select_rgb[1] &&
			map->item[i].select_rgb[2] == select_rgb[2])
		{
			map->InsertItem (map->item[i].xyz[0], map->item[i].xyz[1], map->item[i].xyz[2], map->item[i].type, map->item[i].respawn_wait, map->item[i].respawn_time);
			return;
		}
	}

	for (i = 0; i < map->header.max_lights; i++)
	{
		if (map->light[i].select_rgb[0] == select_rgb[0] &&
			map->light[i].select_rgb[1] == select_rgb[1] &&
			map->light[i].select_rgb[2] == select_rgb[2])
		{
			long last_entry = map->header.max_lights;

			map->InsertLight (map->light[i].name, map->light[i].texture_filename, map->light[i].xyz[0], map->light[i].xyz[1], map->light[i].xyz[2], map->light[i].angle[0], map->light[i].angle[1], map->light[i].angle[2], map->light[i].rgba[0], map->light[i].rgba[1], map->light[i].rgba[2]);
			if (map->light[i].max_inclusions > 0)
			{
				map->light[ last_entry ].max_inclusions = map->light[ i ].max_inclusions;
				map->light[ last_entry ].inclusions     = new int[ map->light[i].max_inclusions+1 ];
				for (long i2 = 0; i2 < map->light[ last_entry ].max_inclusions; i2++) map->light[ last_entry ].inclusions[ i2 ] = map->light[ i ].inclusions[ i2 ];
			}
			return;
		}
	}

	for (i = 0; i < map->header.max_sounds; i++)
	{
		if (map->sound[i].select_rgb[0] == select_rgb[0] &&
			map->sound[i].select_rgb[1] == select_rgb[1] &&
			map->sound[i].select_rgb[2] == select_rgb[2])
		{
			map->InsertSound (map->sound[i].xyz[0], map->sound[i].xyz[1], map->sound[i].xyz[2], map->sound[i].filename, map->sound[i].angle[0], map->sound[i].angle[1], map->sound[i].angle[2]);
			return;
		}
	
	}

	MessageBox (hWnd, "Unable to Duplicate", "Error", MB_OK);
}


/* SkyBoxDlgProc *****************************************************************************************************/
LRESULT CALLBACK SkyBoxDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			if (map->header.use_skybox)
			{
				SetDlgItemText (hWnd, IDC_SKYBOX_FRONT, map->skybox.front.filename);
				SetDlgItemText (hWnd, IDC_SKYBOX_BACK, map->skybox.back.filename);
				SetDlgItemText (hWnd, IDC_SKYBOX_LEFT, map->skybox.left.filename);
				SetDlgItemText (hWnd, IDC_SKYBOX_RIGHT, map->skybox.right.filename);
				SetDlgItemText (hWnd, IDC_SKYBOX_TOP, map->skybox.top.filename);
				SetDlgItemText (hWnd, IDC_SKYBOX_BOTTOM, map->skybox.bottom.filename);

				SendDlgItemMessage (hWnd, IDC_SKYBOX_USE, BM_SETCHECK, BST_CHECKED, 0);
			}
		} break;

		case WM_COMMAND:
		{
			if (wParam == IDCANCEL) EndDialog (hWnd, 0);
			else if (wParam == IDOK)
			{
				GetDlgItemText (hWnd, IDC_SKYBOX_FRONT, map->skybox.front.filename, 500);
				GetDlgItemText (hWnd, IDC_SKYBOX_BACK, map->skybox.back.filename, 500);
				GetDlgItemText (hWnd, IDC_SKYBOX_LEFT, map->skybox.left.filename, 500);
				GetDlgItemText (hWnd, IDC_SKYBOX_RIGHT, map->skybox.right.filename, 500);
				GetDlgItemText (hWnd, IDC_SKYBOX_TOP, map->skybox.top.filename, 500);
				GetDlgItemText (hWnd, IDC_SKYBOX_BOTTOM, map->skybox.bottom.filename, 500);

				if (SendDlgItemMessage (hWnd, IDC_SKYBOX_USE, BM_GETCHECK, 0, 0) == BST_CHECKED) map->header.use_skybox = true;
				else map->header.use_skybox = false;

				EndDialog (hWnd, 1);
			}
		} break;
	}
	return (0);
}


/* SelectLightDlgProc *****************************************************************************************************/
LRESULT CALLBACK SelectLightDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage (hWnd, IDC_SELECTLIGHT_LIGHTNAME, LB_RESETCONTENT, 0, 0);
			for (long i = 0; i < map->header.max_lights; i++) SendDlgItemMessage (hWnd, IDC_SELECTLIGHT_LIGHTNAME, LB_ADDSTRING, 0, (LPARAM)map->light[i].name);
			SendDlgItemMessage (hWnd, IDC_SELECTLIGHT_LIGHTNAME, LB_SETCURSEL, 0, 0);
		} break;
		case WM_COMMAND:
		{
			if (wParam == IDOK) 
			{
				long cursel = SendDlgItemMessage(hWnd, IDC_SELECTLIGHT_LIGHTNAME, LB_GETCURSEL, 0, 0);							
				EndDialog (hWnd, cursel);
			}
			else if (wParam == IDCANCEL) EndDialog (hWnd, -1);
		} break;
	} 
	return (0);
}


/* AssignToLight *****************************************************************************************************/
void AssignToLight(HWND hWnd)
{
	long light;
	long obj = -1;


	for (long i = 0; i < map->header.max_objects; i++)	
	{
		if (map->object[i].select_rgb[0] == select_rgb[0] &&
			map->object[i].select_rgb[1] == select_rgb[1] &&
			map->object[i].select_rgb[2] == select_rgb[2]) obj = i;	
	}
	if (obj < 0) 
	{
		MessageBox (hWnd, "Could not find selection", "Oops!", MB_OK);
		return;
	}

	light = DialogBox (GlobalInstance, MAKEINTRESOURCE(IDD_SELECTLIGHT), NULL, (DLGPROC)SelectLightDlgProc);
	if (light < 0)
	{
		MessageBox (hWnd, "No light selected", "Error", MB_OK);
		return;
	}

	for (i = 0; i < map->light[ light ].max_inclusions; i++)
	{
		if (map->light[ light ].inclusions[ i ] == obj)
		{
			MessageBox (hWnd, "The object is already assigned to this light!", "Oops!", MB_OK);
			return;
		}
	}

	if (map->light[ light ].max_inclusions == 0) map->light[ light ].inclusions = new int[ 1 ];
	else
	{
		int *temp;

		temp = new int[ map->light[light].max_inclusions+1 ];
		for (i = 0; i < map->light[ light ].max_inclusions; i++) temp[i] = map->light[ light ].inclusions[ i ];
		
		delete [] map->light[ light ].inclusions;
		map->light[ light ].inclusions = new int[ map->light[light].max_inclusions+2 ];

		for (i = 0; i < map->light[ light ].max_inclusions; i++) map->light[ light ].inclusions[ i ] = temp[ i ];
		delete temp;
	}

	map->light[ light ].inclusions[ map->light[light].max_inclusions ] = obj;
	map->light[ light ].max_inclusions++;
}


/* RemoveFromLight *****************************************************************************************************/
void RemoveFromLight(HWND hWnd, long obj = -1, long light = -1)
{
	if (obj == -1)
	{
		for (long i = 0; i < map->header.max_objects; i++)	
		{
			if (map->object[i].select_rgb[0] == select_rgb[0] &&
				map->object[i].select_rgb[1] == select_rgb[1] &&
				map->object[i].select_rgb[2] == select_rgb[2]) obj = i;	
		}
		if (obj < 0) 
		{
			MessageBox (hWnd, "Could not find selection", "Oops!", MB_OK);
			return;
		}

		light = DialogBox (GlobalInstance, MAKEINTRESOURCE(IDD_SELECTLIGHT), NULL, (DLGPROC)SelectLightDlgProc);
		if (light < 0)
		{
			MessageBox (hWnd, "No light selected", "Error", MB_OK);
			return;
		}
	}

	if (map->light[ light ].max_inclusions > 0) 	
	{
		int *temp;

		temp = new int[ map->light[light].max_inclusions+1 ];
		for (long i = 0; i < map->light[ light ].max_inclusions; i++) temp[i] = map->light[ light ].inclusions[ i ];
		
		delete [] map->light[ light ].inclusions;
		map->light[ light ].inclusions = new int[ map->light[light].max_inclusions ];

		for (i = 0; i < map->light[ light ].max_inclusions; i++) 
		{
			if (i < obj) map->light[ light ].inclusions[ i ] = temp[ i ];
			else if (i > obj) map->light[ light ].inclusions[ i-1 ] = temp[ i ];
		}
		delete temp;
		map->light[ light ].max_inclusions--;	
	}
	
}


/* Delete *****************************************************************************************************/
void Delete(HWND hWnd)
{
	long idx = 0;

	for (long i = 0; i < map->header.max_entities; i++)
	{
		if (map->entity[i].select_rgb[0] == select_rgb[0] &&
			map->entity[i].select_rgb[1] == select_rgb[1] &&
			map->entity[i].select_rgb[2] == select_rgb[2])
		{
			if (map->header.max_entities > 1)
			{
				MAP_ENTITY *temp;

				temp = new MAP_ENTITY[ map->header.max_entities+1 ];
				for (long i2 = 0; i2 < map->header.max_entities; i2++) temp[i2] = map->entity[i2];

				delete [] map->entity;
				map->entity = new MAP_ENTITY[ map->header.max_entities ];

				idx = 0;
				for (i2 = 0; i2 < map->header.max_entities; i2++)
				{
					if (i2 != i)
					{
						map->entity[idx] = temp[i2];
						idx++;
					}
				}
				delete [] temp;
			}
			else 
			{
				delete [] map->entity; 
				map->entity = NULL;
			}
			map->header.max_entities--;

			return;
		}
	}


	for (i = 0; i < map->header.max_items; i++)
	{
		if (map->item[i].select_rgb[0] == select_rgb[0] &&
			map->item[i].select_rgb[1] == select_rgb[1] &&
			map->item[i].select_rgb[2] == select_rgb[2])
		{
			if (map->header.max_items > 1)
			{
				MAP_ITEM *temp;

				temp = new MAP_ITEM[ map->header.max_items+1 ];
				for (long i2 = 0; i2 < map->header.max_items; i2++) temp[i] = map->item[i];

				delete [] map->item;
				map->item = new MAP_ITEM[ map->header.max_items ];

				idx = 0;
				for (i2 = 0; i2 < map->header.max_items; i2++)
				{
					if (i2 != i)
					{
						map->item[idx] = temp[i2];
						idx++;
					}
				}
				delete [] temp;
			}
			else
			{
				delete [] map->item;
				map->item = NULL;
			}
			map->header.max_items--;

			return;
		}
	}


	for (i = 0; i < map->header.max_sounds; i++)
	{
		if (map->sound[i].select_rgb[0] == select_rgb[0] &&
			map->sound[i].select_rgb[1] == select_rgb[1] &&
			map->sound[i].select_rgb[2] == select_rgb[2])
		{
			if (map->header.max_sounds > 1)
			{
				MAP_SOUND *temp;

				temp = new MAP_SOUND[ map->header.max_sounds+1 ];
				for (long i2 = 0; i2 < map->header.max_sounds; i2++) temp[i2] = map->sound[i2];

				delete [] map->sound;
				map->sound = new MAP_SOUND[ map->header.max_sounds ];

				idx = 0;
				for (i2 = 0; i2 < map->header.max_sounds; i2++)
				{
					if (i2 != i)
					{
						map->sound[idx] = temp[i2];
						idx++;
					}
				}
				delete [] temp;
			}
			else
			{
				delete [] map->sound;
				map->sound = NULL;
			}
			map->header.max_sounds--;

			return;
		}

	}


	for (i = 0; i < map->header.max_lights; i++)
	{
		if (map->light[i].select_rgb[0] == select_rgb[0] &&
			map->light[i].select_rgb[1] == select_rgb[1] &&
			map->light[i].select_rgb[2] == select_rgb[2])
		{
			if (map->header.max_lights > 1)
			{
				MAP_LIGHT *temp;

				temp = new MAP_LIGHT[ map->header.max_lights+1 ];
				for (long i2 = 0; i2 < map->header.max_lights; i2++)
				{
					temp[i2] = map->light[i2];
					if (map->light[i2].max_inclusions > 0)
					{
						temp[i2].inclusions = new int[ temp[i2].max_inclusions+1 ];
						for (long i3 = 0; i3 < map->light[i2].max_inclusions; i3++) temp[i2].inclusions[i3] = map->light[i2].inclusions[i3];

						delete [] map->light[i2].inclusions;
					}
					else temp[i2].inclusions = NULL;
				}

				delete [] map->light;
				map->light = new MAP_LIGHT[ map->header.max_lights ];

				idx = 0;
				for (i2 = 0; i2 < map->header.max_lights; i2++)
				{
					if (i2 != i)
					{
						map->light[idx] = temp[i2];
						if (map->light[idx].max_inclusions > 0)
						{
							map->light[idx].inclusions = new int[ map->light[i2].max_inclusions+1 ];
							for (long i3 = 0; i3 < map->light[idx].max_inclusions; i3++) map->light[idx].inclusions[i3] = temp[i2].inclusions[i3];

							delete [] temp->inclusions;
						}
						idx++;
					}
				}
				delete [] temp;
			}
			else
			{
				if (map->light[0].max_inclusions > 0) delete [] map->light[0].inclusions;
				delete [] map->light;
				map->light = NULL;
			}
			map->header.max_lights--;

			return;
		}
	}


	for (i = 0; i < map->header.max_objects; i++)
	{
		if (map->object[i].select_rgb[0] == select_rgb[0] &&
			map->object[i].select_rgb[1] == select_rgb[1] &&
			map->object[i].select_rgb[2] == select_rgb[2])
		{
			if (map->header.max_objects > 0)
			{
				MAP_OBJECT	*temp;

				for (long light = 0; light < map->header.max_lights; light++)
				{
					for (long obj = 0; obj < map->light[light].max_inclusions; obj++)
					{
						if (map->light[light].inclusions[obj] == i) RemoveFromLight (hWnd, obj, light);
					}

					for (obj = 0; obj < map->light[light].max_inclusions; obj++)
					{
						if (map->light[light].inclusions[obj] > i) map->light[light].inclusions[obj]--;
					}
				}


				temp = new MAP_OBJECT[ map->header.max_objects+1 ];
				for (long i2 = 0; i2 < map->header.max_objects; i2++)
				{
					temp[i2].is_collidable	=	map->object[i2].is_collidable;
					temp[i2].is_visible		=	map->object[i2].is_visible;
					temp[i2].max_textures	=	map->object[i2].max_textures;
					temp[i2].max_triangles	=	map->object[i2].max_triangles;
					temp[i2].max_vertices	=	map->object[i2].max_vertices;
					strcpy (temp[i2].name, map->object[i2].name);
					temp[i2].select_rgb[0]	=	map->object[i2].select_rgb[0];
					temp[i2].select_rgb[1]	=	map->object[i2].select_rgb[1];
					temp[i2].select_rgb[2]	=	map->object[i2].select_rgb[2];
					temp[i2].special		=	map->object[i2].special;
					temp[i2].type			=	map->object[i2].type;


					if (map->object[i2].max_textures > 0)
					{
						temp[i2].texture = new MAP_TEXTURE[ map->object[i2].max_textures+1 ];
						for (long tex = 0; tex < map->object[i2].max_textures; tex++) temp[i2].texture[tex] = map->object[i2].texture[tex];
						delete [] map->object[i2].texture;
					}
					else temp[i2].texture = NULL;


					if (map->object[i2].max_triangles > 0)
					{
						temp[i2].triangle = new MAP_TRIANGLE[ map->object[i2].max_triangles+1 ];
						for (long tri = 0; tri < map->object[i2].max_triangles; tri++) temp[i2].triangle[tri] = map->object[i2].triangle[tri];
						delete [] map->object[i2].triangle;
					}
					else temp[i2].triangle = NULL;


					if (map->object[i2].max_vertices > 0)
					{
						temp[i2].vertex = new MAP_VERTEX[ map->object[i2].max_vertices+1 ];
						for (long ver = 0; ver < map->object[i2].max_vertices; ver++) temp[i2].vertex[ver] = map->object[i2].vertex[ver];
						delete [] map->object[i2].vertex;
					}
					else temp[i2].vertex = NULL;
				}


				delete [] map->object;
				map->object = new MAP_OBJECT[ map->header.max_objects+1 ];


				idx = 0;
				for (i2 = 0; i2 < map->header.max_objects; i2++)
				{
					if (i != i2)
					{
						map->object[idx].is_collidable	=	temp[i2].is_collidable;
						map->object[idx].is_visible		=	temp[i2].is_visible;
						map->object[idx].max_textures	=	temp[i2].max_textures;
						map->object[idx].max_triangles	=	temp[i2].max_triangles;
						map->object[idx].max_vertices	=	temp[i2].max_vertices;
						strcpy (map->object[idx].name, temp[i2].name);
						map->object[idx].select_rgb[0]	=	temp[i2].select_rgb[0];
						map->object[idx].select_rgb[1]	=	temp[i2].select_rgb[1];
						map->object[idx].select_rgb[2]	=	temp[i2].select_rgb[2];
						map->object[idx].special		=	temp[i2].special;
						map->object[idx].type			=	temp[i2].type;

						if (map->object[idx].max_textures > 0)
						{
							map->object[idx].texture = new MAP_TEXTURE[ map->object[idx].max_textures+1 ];
							for (long tex = 0; tex < map->object[idx].max_textures; tex++) map->object[idx].texture[tex] = temp[i2].texture[tex];
							delete [] temp[i2].texture;
						}
						else map->object[idx].texture = NULL;


						if (map->object[idx].max_triangles > 0)
						{
							map->object[idx].triangle = new MAP_TRIANGLE[ map->object[idx].max_triangles+1 ];
							for (long tri = 0; tri < map->object[idx].max_triangles; tri++) map->object[idx].triangle[tri] = temp[i2].triangle[tri];
							delete [] temp[i2].triangle;
						}
						else map->object[i2].triangle = NULL;


						if (map->object[idx].max_vertices > 0)
						{
							map->object[idx].vertex = new MAP_VERTEX[ map->object[idx].max_vertices+1 ];
							for (long ver = 0; ver < map->object[idx].max_vertices; ver++) map->object[idx].vertex[ver] = temp[i2].vertex[ver];
							delete [] temp[i2].vertex;
						}
						else map->object[idx].vertex = NULL;
						idx++;
					}
				}
			}
			else
			{
				if (map->object[0].max_vertices > 0) delete [] map->object[0].vertex;
				if (map->object[0].max_triangles > 0) delete [] map->object[0].triangle;
				if (map->object[0].max_textures > 0) delete [] map->object[0].texture;
				delete [] map->object;
				map->object = NULL;
			}
			map->header.max_objects--;

			return;
		}
	}

	MessageBox (hWnd, "Unable to Delete", "Error", MB_OK);
}


/* EditObjectDlgProc *****************************************************************************************************/
LRESULT CALLBACK EditObjectDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			bool found = false;

			for (long i = 0; i < map->header.max_objects; i++)
			{
				if (map->object[i].select_rgb[0] == select_rgb[0] &&
					map->object[i].select_rgb[1] == select_rgb[1] &&				
					map->object[i].select_rgb[2] == select_rgb[2])
				{
					char temp[500];

					sprintf (temp, "%i", i);
					SetDlgItemText (hWnd, IDC_EDITOBJECT_NUMBER, temp);

					SetDlgItemText (hWnd, IDC_EDITOBJECT_NAME, map->object[i].name);
					
					sprintf (temp, "%i", map->object[i].type);
					SetDlgItemText (hWnd, IDC_EDITOBJECT_TYPE, temp);
										
					if (map->object[i].is_visible) SendDlgItemMessage (hWnd, IDC_EDITOBJECT_VISIBLE, BM_SETCHECK, BST_CHECKED, 0);
					if (map->object[i].is_collidable) SendDlgItemMessage (hWnd, IDC_EDITOBJECT_COLLIDABLE, BM_SETCHECK, BST_CHECKED, 0);
			
					sprintf (temp, "%f", map->object[i].vertex[3].xyz[1]);
					SetDlgItemText (hWnd, IDC_EDITOBJECT_STARTY, temp);

					if (map->object[i].type == OBJECTTYPE_WALL)
					{
						sprintf (temp, "%f", map->object[i].vertex[0].xyz[1]-map->object[i].vertex[3].xyz[1]);
						SetDlgItemText (hWnd, IDC_EDITOBJECT_HEIGHT, temp);
					}
					
					sprintf (temp, "%f", map->object[i].vertex[0].fog_depth);
					SetDlgItemText (hWnd, IDC_EDITOBJECT_FOG, temp);

					SendDlgItemMessage (hWnd, IDC_EDITOBJECT_SPECIAL, LB_RESETCONTENT, 0, 0);
					SendDlgItemMessage (hWnd, IDC_EDITOBJECT_SPECIAL, LB_ADDSTRING, 0, (LPARAM)"None.");
					SendDlgItemMessage (hWnd, IDC_EDITOBJECT_SPECIAL, LB_ADDSTRING, 0, (LPARAM)"End Level");
					SendDlgItemMessage (hWnd, IDC_EDITOBJECT_SPECIAL, LB_ADDSTRING, 0, (LPARAM)"-10% Health");
					SendDlgItemMessage (hWnd, IDC_EDITOBJECT_SPECIAL, LB_SETCURSEL, map->object[i].special, 0);

					found = true;
				}
			} 

			if (!found) 
			{
				MessageBox (hWnd, "Could not find an object to edit", "Oops!", MB_OK);
				EndDialog (hWnd, 0);
			}
		}break;
		case WM_COMMAND:
		{
			if (wParam == IDCANCEL) EndDialog (hWnd, 0);
			else if (wParam == IDOK)
			{
				char temp[500];
				long obj;			
				float height;
				float starty;

				GetDlgItemText (hWnd, IDC_EDITOBJECT_NUMBER, temp, 500);
				sscanf (temp, "%i", &obj);

				GetDlgItemText (hWnd, IDC_EDITOBJECT_NAME, map->object[obj].name, 500);

				if (SendDlgItemMessage (hWnd, IDC_EDITOBJECT_VISIBLE, BM_GETCHECK, 0, 0) == BST_CHECKED) map->object[obj].is_visible = true;	
				if (SendDlgItemMessage (hWnd, IDC_EDITOBJECT_COLLIDABLE, BM_GETCHECK, 0, 0) == BST_CHECKED) map->object[obj].is_collidable = true;	

				map->object[obj].special = SendDlgItemMessage (hWnd, IDC_EDITOBJECT_SPECIAL, LB_GETCURSEL, 0, 0);

				GetDlgItemText (hWnd, IDC_EDITOBJECT_FOG, temp, 500);
				for (long i = 0; i < map->object[obj].max_vertices; i++) sscanf (temp, "%f", &map->object[obj].vertex[i].fog_depth);

				
				GetDlgItemText (hWnd, IDC_EDITOBJECT_STARTY, temp, 500);
				sscanf (temp, "%f", &starty);
				for (i = 0; i < map->object[obj].max_vertices; i++) map->object[obj].vertex[i].xyz[1] = starty;
				

				if (map->object[obj].type == OBJECTTYPE_WALL)
				{
					GetDlgItemText (hWnd, IDC_EDITOBJECT_HEIGHT, temp, 500);
					sscanf (temp, "%f", &height);
					
					map->object[obj].vertex[0].xyz[1] = starty + height;
					map->object[obj].vertex[1].xyz[1] = starty + height;
				}

				EndDialog (hWnd, 1);
			}
		} break;
	}
	return (0);
}


/* FogDlgProc **************************************************************************************************/
LRESULT CALLBACK FogDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG: 
			{
				char temp[500];

				if (map->header.use_fog) SendDlgItemMessage (hWnd, IDC_FOG_USE, BM_SETCHECK, BST_CHECKED, 0);
				else SendDlgItemMessage (hWnd, IDC_FOG_USE, BM_SETCHECK, BST_UNCHECKED, 0);

				SendDlgItemMessage (hWnd, IDC_FOG_MODE, CB_RESETCONTENT, 0, 0);
				SendDlgItemMessage (hWnd, IDC_FOG_MODE, CB_ADDSTRING, 0, (LPARAM)"Linear");
				SendDlgItemMessage (hWnd, IDC_FOG_MODE, CB_ADDSTRING, 0, (LPARAM)"Exp");
				SendDlgItemMessage (hWnd, IDC_FOG_MODE, CB_ADDSTRING, 0, (LPARAM)"Exp2");				
				
				switch (map->fog.mode)
				{	
					case GL_EXP: SendDlgItemMessage (hWnd, IDC_FOG_MODE, CB_SETCURSEL, 1, 0); break;
					case GL_EXP2: SendDlgItemMessage (hWnd, IDC_FOG_MODE, CB_SETCURSEL, 2, 0); break;
					default: SendDlgItemMessage (hWnd, IDC_FOG_MODE, CB_SETCURSEL, 0, 0); 
				}
				
				sprintf (temp, "%1.2f", map->fog.density);
				SetDlgItemText (hWnd, IDC_FOG_DENSITY, temp);

				sprintf (temp, "%1.2f", map->fog.start);
				SetDlgItemText (hWnd, IDC_FOG_START, temp);

				sprintf (temp, "%1.2f", map->fog.end);
				SetDlgItemText (hWnd, IDC_FOG_END, temp);

				sprintf (temp, "%3.0f", (map->fog.rgba[0] * 255.0f));
				SetDlgItemText (hWnd, IDC_FOG_R, temp);

				sprintf (temp, "%3.0f", (map->fog.rgba[1] * 255.0f));
				SetDlgItemText (hWnd, IDC_FOG_G, temp);

				sprintf (temp, "%3.0f", (map->fog.rgba[2] * 255.0f));
				SetDlgItemText (hWnd, IDC_FOG_B, temp);

				sprintf (temp, "%3.0f", (map->fog.rgba[3] * 255.0f));
				SetDlgItemText (hWnd, IDC_FOG_A, temp);
			} break;

		case WM_COMMAND:
			{
				if (wParam == IDCANCEL) EndDialog (hWnd, 0);
				else if (wParam == IDOK)
				{
					char	temp[500]; 

					if (SendDlgItemMessage (hWnd, IDC_FOG_USE, BM_GETCHECK, 0, 0) == BST_CHECKED) map->header.use_fog = true;
					else map->header.use_fog = false;

					switch (SendDlgItemMessage(hWnd, IDC_FOG_MODE, CB_GETCURSEL, 0, 0))
					{
						case 0: map->fog.mode = GL_LINEAR; break;
						case 1: map->fog.mode = GL_EXP; break;
						case 2: map->fog.mode = GL_EXP2; break;		
					}

					GetDlgItemText (hWnd, IDC_FOG_DENSITY, temp, 500);
					sscanf (temp, "%f", &map->fog.density);

					GetDlgItemText (hWnd, IDC_FOG_START, temp, 500);
					sscanf (temp, "%f", &map->fog.start);

					GetDlgItemText (hWnd, IDC_FOG_END, temp, 500);
					sscanf (temp, "%f", &map->fog.end);

					GetDlgItemText (hWnd, IDC_FOG_R, temp, 500);
					sscanf (temp, "%f", &map->fog.rgba[0]);
					map->fog.rgba[0] /= 255.0f;

					GetDlgItemText (hWnd, IDC_FOG_G, temp, 500);
					sscanf (temp, "%f", &map->fog.rgba[1]);
					map->fog.rgba[1] /= 255.0f;

					GetDlgItemText (hWnd, IDC_FOG_B, temp, 500);
					sscanf (temp, "%f", &map->fog.rgba[2]);
					map->fog.rgba[2] /= 255.0f;

					GetDlgItemText (hWnd, IDC_FOG_A, temp, 500);
					sscanf (temp, "%f", &map->fog.rgba[3]);
					map->fog.rgba[3] /= 255.0f;

					EndDialog (hWnd, 1);
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
		ShowSelectedButton();
	}
	else if (lParam == (LPARAM)bCreateFloor) 
	{
		creation_coords.type = OBJECTTYPE_FLOOR;
		ShowSelectedButton();
	}
	else if (lParam == (LPARAM)bCreateCeiling) 
	{
		creation_coords.type = OBJECTTYPE_CEILING;
		ShowSelectedButton();
	}
	else if (lParam == (LPARAM)bPlaceStartPosition)
	{
		creation_coords.type = ITEM_START_POSITION;
		ShowSelectedButton();
	}
	else if (lParam == (LPARAM)bPlaceDMPosition)
	{
		creation_coords.type = ITEM_DM_POSITION;
		ShowSelectedButton();
	}
	else if (lParam == (LPARAM)bInsertEntity)
	{
		creation_coords.type = ITEM_ENTITY;
		ShowSelectedButton();
	}
	else if (lParam == (LPARAM)bInsertItem)
	{
		creation_coords.type = ITEM;
		ShowSelectedButton();
	}
	else if (lParam == (LPARAM)bInsertSound)
	{
		creation_coords.type = SOUND;
		ShowSelectedButton();
	}
	else if (lParam == (LPARAM)bInsertLight)
	{
		creation_coords.type = LIGHT;
		ShowSelectedButton();
	}
	else if (lParam == (LPARAM)bSelectObject) 
	{
		creation_coords.type = NULL;			
		ShowSelectedButton();
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
	else if (wParam == ID_LAYERS_ENTITY)
	{
		if (layer.draw_entity) CheckMenuItem (Menu, ID_LAYERS_ENTITY, MF_UNCHECKED);		
		else CheckMenuItem (Menu, ID_LAYERS_ENTITY, MF_CHECKED);
		layer.draw_entity = !layer.draw_entity;		
	}
	else if (wParam == ID_LAYERS_ITEM)
	{
		if (layer.draw_item) CheckMenuItem (Menu, ID_LAYERS_ITEM, MF_UNCHECKED);
		else CheckMenuItem (Menu, ID_LAYERS_ITEM, MF_CHECKED);
		layer.draw_item = !layer.draw_item;
	}
	else if (wParam == ID_LAYERS_SOUND)
	{
		if (layer.draw_sound) CheckMenuItem (Menu, ID_LAYERS_SOUND, MF_UNCHECKED);
		else CheckMenuItem (Menu, ID_LAYERS_SOUND, MF_CHECKED);
		layer.draw_sound = !layer.draw_sound;
	}
	else if (wParam == ID_LAYERS_LIGHT)
	{
		if (layer.draw_light) CheckMenuItem (Menu, ID_LAYERS_LIGHT, MF_UNCHECKED);
		else CheckMenuItem (Menu, ID_LAYERS_LIGHT, MF_CHECKED);
		layer.draw_light = !layer.draw_light;
	}
	else if (wParam == ID_MAP_SKYBOX) DialogBox (GlobalInstance, MAKEINTRESOURCE(IDD_SKYBOX), NULL, (DLGPROC)SkyBoxDlgProc);
	else if (wParam == ID_MAP_FOG) DialogBox (GlobalInstance, MAKEINTRESOURCE(IDD_FOG), NULL, (DLGPROC)FogDlgProc);
		
	// Popup Menu Items
	else if (wParam == ID_POPUP_MOVE) 
	{		
		if (select_rgb[0] != 255 && select_rgb[1] != 255 && select_rgb[2] != 255)
		{
			creation_coords.start	= creation_coords.finish;
			creation_coords.mode	= CREATE_MODE_START;
			creation_coords.type	= MODE_TYPE_MOVE;
		}
		else MessageBox (hWnd, "An Object must be Selected", "Error", MB_OK);
	}
	else if (wParam == ID_POPUP_DELETE) Delete(hWnd);
	else if (wParam == ID_POPUP_TEXTURE) DialogBox (GlobalInstance, MAKEINTRESOURCE(IDD_ASSIGNTEXTURE), NULL, (DLGPROC)AssignTextureDlgProc);
	else if (wParam == ID_POPUP_DUPLICATE) Duplicate(hWnd);
	else if (wParam == ID_POPUP_ASSIGNTOLIGHT) AssignToLight(hWnd);
	else if (wParam == ID_POPUP_REMOVEFROMLIGHT) RemoveFromLight(hWnd);
	else if (wParam == ID_POPUP_EDIT) DialogBox (GlobalInstance, MAKEINTRESOURCE(IDD_EDIT_OBJECT), NULL, (DLGPROC)EditObjectDlgProc);
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


/* DMPositionDlgProc **********************************************************************************************************/
LRESULT CALLBACK DMPositionDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage (hWnd, IDC_DM_POSITION_TYPE, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage (hWnd, IDC_DM_POSITION_TYPE, CB_ADDSTRING, 0, (LPARAM)"Player 1");
			SendDlgItemMessage (hWnd, IDC_DM_POSITION_TYPE, CB_ADDSTRING, 0, (LPARAM)"Player 2");
			SendDlgItemMessage (hWnd, IDC_DM_POSITION_TYPE, CB_SETCURSEL, 0, 1);
		} break;
		case WM_COMMAND:
		{
			if (wParam == IDCANCEL) EndDialog (hWnd, 0);
			else if (wParam == IDOK) 
			{
				long player = SendDlgItemMessage (hWnd, IDC_DM_POSITION_TYPE, CB_GETCURSEL, 0, 0);
				
				map->details.deathmatch[ player ].xyz[0] = creation_coords.start.world_x;
				map->details.deathmatch[ player ].xyz[1] = creation_coords.start.world_y;
				map->details.deathmatch[ player ].xyz[2] = creation_coords.start.world_z;
				
				EndDialog (hWnd, 1);
			}
		} break;
	}
	return (0);
}


/* InsertEntityDlgProc **********************************************************************************************************/
LRESULT CALLBACK InsertEntityDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage (hWnd, IDC_INSERT_ENTITY_TYPE, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage (hWnd, IDC_INSERT_ENTITY_TYPE, CB_ADDSTRING, 0, (LPARAM)"Joe");
			SendDlgItemMessage (hWnd, IDC_INSERT_ENTITY_TYPE, CB_SETCURSEL, 0, 1);

			SetDlgItemText (hWnd, IDC_INSERT_ENTITY_HEALTH, "100");
			SetDlgItemText (hWnd, IDC_INSERT_ENTITY_STRENGTH, "10");
			SetDlgItemText (hWnd, IDC_INSERT_ENTITY_ARMOUR, "0");
		} break;
		case WM_COMMAND:
		{
			if (wParam == IDCANCEL) EndDialog (hWnd, 0);
			else if (wParam == IDOK) 
			{
				char temp[500];
				long type;
				long health;
				long strength;
				long armour;

				type = SendDlgItemMessage (hWnd, IDC_INSERT_ENTITY_TYPE, CB_GETCURSEL, 0, 0);		
				
				GetDlgItemText (hWnd, IDC_INSERT_ENTITY_HEALTH, temp, 500);
				sscanf (temp,"%i",&health);

				GetDlgItemText (hWnd, IDC_INSERT_ENTITY_STRENGTH, temp, 500);
				sscanf (temp,"%i",&strength);

				GetDlgItemText (hWnd, IDC_INSERT_ENTITY_ARMOUR, temp, 500);
				sscanf (temp,"%i",&armour);

				map->InsertEntity(type, creation_coords.start.world_x, creation_coords.start.world_y, creation_coords.start.world_z, 0,0,0, health, strength, armour);
				EndDialog (hWnd, 1);
			}
		} break;
	}
	return (0);
}


/* InsertItemDlgProc **********************************************************************************************************/
LRESULT CALLBACK InsertItemDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage (hWnd, IDC_INSERT_ITEM_TYPE, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage (hWnd, IDC_INSERT_ITEM_TYPE, CB_ADDSTRING, 0, (LPARAM)"Gun");
			SendDlgItemMessage (hWnd, IDC_INSERT_ITEM_TYPE, CB_SETCURSEL, 0, 1);
			SetDlgItemText (hWnd, IDC_INSERT_ITEM_RESPAWN_TIME, "0");
			SetDlgItemText (hWnd, IDC_INSERT_ITEM_RESPAWN_WAIT, "0");
		} break;
		case WM_COMMAND:
		{
			if (wParam == IDCANCEL) EndDialog (hWnd, 0);
			else if (wParam == IDOK) 
			{	
				char temp[500];
				long type;
				long respawn_wait;
				long respawn_time;

				type = SendDlgItemMessage (hWnd, IDC_INSERT_ITEM_TYPE, CB_GETCURSEL, 0, 0);

				GetDlgItemText (hWnd, IDC_INSERT_ITEM_RESPAWN_WAIT, temp, 500);
				sscanf (temp, "%i", &respawn_wait);

				GetDlgItemText (hWnd, IDC_INSERT_ITEM_RESPAWN_TIME, temp, 500);
				sscanf (temp, "%i", &respawn_time);

				map->InsertItem (creation_coords.finish.world_x, creation_coords.finish.world_y, creation_coords.finish.world_z, type, respawn_wait, respawn_time);
				EndDialog (hWnd, 1);
			}
		} break;
	}
	return (0);
}

/* InsertSoundDlgProc **********************************************************************************************************/
LRESULT CALLBACK InsertSoundDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG: SetDlgItemText (hWnd, IDC_INSERTSOUND_FILENAME, "my_sound.wav"); break;
		case WM_COMMAND:
		{
			if (wParam == IDCANCEL) EndDialog (hWnd, 0);
			else if (wParam == IDOK) 
			{
				char filename[500];
				GetDlgItemText (hWnd, IDC_INSERTSOUND_FILENAME, filename, 500);
				
				map->InsertSound (creation_coords.finish.world_x, 0, creation_coords.finish.world_z, filename);

				EndDialog (hWnd, 1);
			}
		} break;
	}
	return (0);
}


/* InsertLightDlgProc **********************************************************************************************************/
LRESULT CALLBACK InsertLightDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG: 
			{
				char temp[500];

				SetDlgItemText (hWnd, IDC_INSERTLIGHT_R, "255"); 
				SetDlgItemText (hWnd, IDC_INSERTLIGHT_G, "255"); 
				SetDlgItemText (hWnd, IDC_INSERTLIGHT_B, "255"); 
				SetDlgItemText (hWnd, IDC_INSERTLIGHT_XA, "0"); 
				SetDlgItemText (hWnd, IDC_INSERTLIGHT_YA, "0"); 
				SetDlgItemText (hWnd, IDC_INSERTLIGHT_ZA, "0");

				sprintf (temp, "%f", creation_coords.finish.world_x);
				SetDlgItemText (hWnd, IDC_INSERTLIGHT_X, temp);	
				
				SetDlgItemText (hWnd, IDC_INSERTLIGHT_Y, "0.0");
				
				sprintf (temp, "%f", creation_coords.finish.world_z);
				SetDlgItemText (hWnd, IDC_INSERTLIGHT_Z, temp);
			}break;			
		case WM_COMMAND:
		{
			if (wParam == IDCANCEL) EndDialog (hWnd, 0);
			else if (wParam == IDOK) 
			{
				char	temp[500];
				GLfloat	r;
				GLfloat g;
				GLfloat b;
				GLfloat xa;
				GLfloat ya;
				GLfloat za;
				GLfloat x;
				GLfloat y;
				GLfloat z;
				
				GetDlgItemText (hWnd, IDC_INSERTLIGHT_R, temp, 500);
				sscanf (temp, "%f", &r);
				r = r / 255.0f;

				GetDlgItemText (hWnd, IDC_INSERTLIGHT_G, temp, 500);
				sscanf (temp, "%f", &g);
				g = g / 255.0f;

				GetDlgItemText (hWnd, IDC_INSERTLIGHT_B, temp, 500);
				sscanf (temp, "%f", &b);
				b = b / 255.0f;

				GetDlgItemText (hWnd, IDC_INSERTLIGHT_XA, temp, 500);
				sscanf (temp, "%f", &xa);

				GetDlgItemText (hWnd, IDC_INSERTLIGHT_YA, temp, 500);
				sscanf (temp, "%f", &ya);

				GetDlgItemText (hWnd, IDC_INSERTLIGHT_ZA, temp, 500);
				sscanf (temp, "%f", &za);

				GetDlgItemText (hWnd, IDC_INSERTLIGHT_X, temp, 500);
				sscanf (temp, "%f", &x);

				GetDlgItemText (hWnd, IDC_INSERTLIGHT_Y, temp, 500);
				sscanf (temp, "%f", &y);

				GetDlgItemText (hWnd, IDC_INSERTLIGHT_Z, temp, 500);
				sscanf (temp, "%f", &z);

				sprintf (temp, "Light #%i", map->header.max_lights);
				map->InsertLight (temp, "lightmap.bmp", x, y, z, xa, ya, za, r, g, b);
				
				EndDialog (hWnd, 1);
			}
		} break;
	}
	return (0);
}


/* Move ********************************************************************************************************/
void Move()
{ 
	float	x;
	float	z;

	x = (float)(creation_coords.finish.world_x - creation_coords.start.world_x);	
	z = (float)(creation_coords.finish.world_z - creation_coords.start.world_z);
	
	creation_coords.start = creation_coords.finish;


	if (map->details.single_player.select_rgb[0] == select_rgb[0] &&
		map->details.single_player.select_rgb[1] == select_rgb[1] &&
		map->details.single_player.select_rgb[2] == select_rgb[2])
	{
		map->details.single_player.xyz[0] += x;
		map->details.single_player.xyz[1] += z;
		return;
	}	
	
	for (long i = 0; i < 2; i++)
	{
		if (map->details.deathmatch[i].select_rgb[0] == select_rgb[0] &&
			map->details.deathmatch[i].select_rgb[1] == select_rgb[1] &&
			map->details.deathmatch[i].select_rgb[2] == select_rgb[2])
		{
			map->details.deathmatch[i].xyz[0] += x;
			map->details.deathmatch[i].xyz[2] += z;		
			return;
		}
	}

	for (i = 0; i < map->header.max_objects; i++)
	{
		if (map->object[i].select_rgb[0] == select_rgb[0] &&
			map->object[i].select_rgb[1] == select_rgb[1] &&
			map->object[i].select_rgb[2] == select_rgb[2])
		{
			for (long i2 = 0; i2 < map->object[i].max_vertices; i2++)
			{
				map->object[i].vertex[i2].xyz[0] += x;
				map->object[i].vertex[i2].xyz[2] += z;
			}
			return;
		}
	}
	
	for (i = 0; i < map->header.max_entities; i++)
	{
		if (map->entity[i].select_rgb[0] == select_rgb[0] &&
			map->entity[i].select_rgb[1] == select_rgb[1] &&
			map->entity[i].select_rgb[2] == select_rgb[2])
		{
			map->entity[i].xyz[0] += x;
			map->entity[i].xyz[2] += z;
			return;
		}
	}

	for (i = 0; i < map->header.max_items; i++)
	{
		if (map->item[i].select_rgb[0] == select_rgb[0] &&
			map->item[i].select_rgb[1] == select_rgb[1] &&
			map->item[i].select_rgb[2] == select_rgb[2])
		{
			map->item[i].xyz[0] += x;
			map->item[i].xyz[2] += z;
			return;
		}
	}

	for (i = 0; i < map->header.max_sounds; i++)
	{
		if (map->sound[i].select_rgb[0] == select_rgb[0] &&
			map->sound[i].select_rgb[1] == select_rgb[1] &&
			map->sound[i].select_rgb[2] == select_rgb[2])
		{
			map->sound[i].xyz[0] += x;
			map->sound[i].xyz[2] += z;
			return;
		}
	}

	for (i = 0; i < map->header.max_lights; i++)
	{
		if (map->light[i].select_rgb[0] == select_rgb[0] &&
			map->light[i].select_rgb[1] == select_rgb[1] && 
			map->light[i].select_rgb[2] == select_rgb[2])
		{
			map->light[i].xyz[0] += x;
			map->light[i].xyz[2] += z;
			return;
		}
	}
}


/* SelectAnObject ********************************************************************************************************/
void SelectAnObject()
{
	GLubyte	rgb[3];
	RECT	rect;
	long	x;
	long	y;
	long	width;
	long	height;
	bool	found=false;


	memset (&select_rgb, 0xFF, sizeof(select_rgb));

	GetClientRect (RenderWindow, &rect);
	width	= rect.right - rect.left;
	height	= rect.bottom - rect.top;
	x		= creation_coords.finish.mouse_x - DEFAULT_BUTTON_WIDTH;
	y		= height - creation_coords.finish.mouse_y;

	glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glLineWidth(4.0);
		DrawSolid();
		if (layer.draw_entity) DrawEntities(true);
		if (layer.draw_item) DrawItems(true);		
		if (layer.draw_sound) DrawSounds(true);
		if (layer.draw_light) DrawLights(true);
		
		DrawStartPosition();
		DrawDeathMatchPositions();
	glLineWidth(1.0);
	
	glReadBuffer (GL_BACK);
	glReadPixels (x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &rgb);
	
	for (long i = 0; i < map->header.max_objects; i++)
	{
		if (map->object[i].select_rgb[0] == rgb[0] &&
			map->object[i].select_rgb[1] == rgb[1] &&
			map->object[i].select_rgb[2] == rgb[2]) found = true;
	}

	for (i = 0; i < map->header.max_entities; i++)
	{
		if (map->entity[i].select_rgb[0] == rgb[0] &&
			map->entity[i].select_rgb[1] == rgb[1] &&
			map->entity[i].select_rgb[2] == rgb[2]) found = true;
	}

	for (i = 0; i < map->header.max_items; i++)
	{
		if (map->item[i].select_rgb[0] == rgb[0] &&
			map->item[i].select_rgb[1] == rgb[1] &&
			map->item[i].select_rgb[2] == rgb[2]) found = true;
	}

	for (i = 0; i < map->header.max_lightmaps; i++)
	{
		if (map->item[i].select_rgb[0] == rgb[0] && 
			map->item[i].select_rgb[1] == rgb[1] &&
			map->item[i].select_rgb[2] == rgb[2]) found = true;
	}

	for (i = 0; i < map->header.max_lights; i++)
	{
		if (map->light[i].select_rgb[0] == rgb[0] &&
			map->light[i].select_rgb[1] == rgb[1] &&
			map->light[i].select_rgb[2] == rgb[2]) found = true;
	}

	for (i = 0; i < map->header.max_sounds; i++)
	{
		if (map->sound[i].select_rgb[0] == rgb[0] &&
			map->sound[i].select_rgb[1] == rgb[1] &&
			map->sound[i].select_rgb[2] == rgb[2]) found = true;
	}

	if ((map->details.single_player.select_rgb[0] == rgb[0] && 
		map->details.single_player.select_rgb[1] == rgb[1] && 
		map->details.single_player.select_rgb[2] == rgb[2]) || 

		(map->details.deathmatch[0].select_rgb[0] == rgb[0] && 
		map->details.deathmatch[0].select_rgb[1] == rgb[1] && 
		map->details.deathmatch[0].select_rgb[2] == rgb[2]) ||

		(map->details.deathmatch[1].select_rgb[0] == rgb[0] && 
		map->details.deathmatch[1].select_rgb[1] == rgb[1] && 
		map->details.deathmatch[1].select_rgb[2] == rgb[2])) found = true;

	if (found)
	{
		select_rgb[0] = rgb[0];
		select_rgb[1] = rgb[1];
		select_rgb[2] = rgb[2];
	}
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
		else if (creation_coords.type == ITEM_START_POSITION)
		{
			long rgb = map->GenerateColor();

			map->details.single_player.select_rgb[0] = GetRValue(rgb);
			map->details.single_player.select_rgb[1] = GetGValue(rgb);
			map->details.single_player.select_rgb[2] = GetBValue(rgb);

			map->details.single_player.xyz[0] = creation_coords.start.world_x;
			map->details.single_player.xyz[1] = creation_coords.start.world_y;
			map->details.single_player.xyz[2] = creation_coords.start.world_z;			
		}		
		else if (creation_coords.type == ITEM_DM_POSITION) DialogBox (GlobalInstance, MAKEINTRESOURCE(IDD_DM_POSITION), NULL, (DLGPROC)DMPositionDlgProc);
		else if (creation_coords.type == ITEM_ENTITY) DialogBox (GlobalInstance, MAKEINTRESOURCE(IDD_INSERT_ENTITY), NULL, (DLGPROC)InsertEntityDlgProc);		
		else if (creation_coords.type == ITEM) DialogBox(GlobalInstance, MAKEINTRESOURCE(IDD_INSERT_ITEM), NULL, (DLGPROC)InsertItemDlgProc);
		else if (creation_coords.type == SOUND) DialogBox (GlobalInstance, MAKEINTRESOURCE(IDD_INSERT_SOUND), NULL, (DLGPROC)InsertSoundDlgProc);
		else if (creation_coords.type == LIGHT) DialogBox (GlobalInstance, MAKEINTRESOURCE(IDD_INSERT_LIGHT), NULL, (DLGPROC)InsertLightDlgProc);
		else if (creation_coords.type == NULL) SelectAnObject();
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

		if (creation_coords.type == MODE_TYPE_MOVE) Move();
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
							0, DEFAULT_BUTTON_HEIGHT, 640, 480,
							NULL, NULL, hInstance, NULL);	
	if (Window == NULL) 
	{
		MessageBox (NULL,"Error: Failed to Create Window","ERROR!",MB_OK);
		return (0);
	}	
	GetClientRect (Window, &rect);


	bSelectObject = CreateWindow( "BUTTON", "Select OBJ", WS_CHILD | WS_VISIBLE,
							0, 0, DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
							Window, NULL, hInstance, NULL);

	bCreateWall = CreateWindow(	"BUTTON", "Create Wall", WS_CHILD | WS_VISIBLE,
							0, 100, DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
							Window, NULL, hInstance, NULL);

	bCreateCeiling = CreateWindow(	"BUTTON", "Create Ceiling", WS_CHILD | WS_VISIBLE,
							0, 100+(DEFAULT_BUTTON_HEIGHT*2), DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
							Window, NULL, hInstance, NULL);

	bCreateFloor = CreateWindow( "BUTTON", "Create Floor", WS_CHILD | WS_VISIBLE,
							0, 100+(DEFAULT_BUTTON_HEIGHT*4), DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
							Window, NULL, hInstance, NULL);
	
	bPlaceStartPosition = CreateWindow( "BUTTON", "Place Start", WS_CHILD | WS_VISIBLE,
							0, 100+(DEFAULT_BUTTON_HEIGHT*6), DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
							Window, NULL, hInstance, NULL);

	bPlaceDMPosition = CreateWindow( "BUTTON", "Place DM", WS_CHILD | WS_VISIBLE,
							0, 100+(DEFAULT_BUTTON_HEIGHT*8), DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
							Window, NULL, hInstance, NULL);

	bInsertEntity = CreateWindow( "BUTTON", "Insert Entity", WS_CHILD | WS_VISIBLE,
							0, 100+(DEFAULT_BUTTON_HEIGHT*10), DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
							Window, NULL, hInstance, NULL);

	bInsertItem = CreateWindow( "BUTTON", "Insert Item", WS_CHILD | WS_VISIBLE,
							0, 100+(DEFAULT_BUTTON_HEIGHT*12), DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
							Window, NULL, hInstance, NULL);

	bInsertSound = CreateWindow( "BUTTON", "Insert Sound", WS_CHILD | WS_VISIBLE,
							0, 100+(DEFAULT_BUTTON_HEIGHT*14), DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
							Window, NULL, hInstance, NULL);

	bInsertLight = CreateWindow( "BUTTON", "Insert Light", WS_CHILD | WS_VISIBLE,
							0, 100+(DEFAULT_BUTTON_HEIGHT*16), DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT,
							Window, NULL, hInstance, NULL);

	RenderWindow= CreateWindow( "STATIC", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
							DEFAULT_BUTTON_WIDTH, 0, rect.right-rect.left-DEFAULT_BUTTON_WIDTH, rect.bottom-rect.top,
							Window, NULL, hInstance, NULL);

	Menu = LoadMenu (hInstance, MAKEINTRESOURCE(IDR_MENU));
	SetMenu (Window, Menu);
	
	PopupMenu = LoadMenu (hInstance, MAKEINTRESOURCE(IDR_POPUP_MENU));

	
	if (!raster.Init(RenderWindow)) return (0);	
	if (GetDeviceCaps(raster.hDC, BITSPIXEL) < 24) MessageBox (Window, "This program is designed to run in 24/32-bit bit depths. To ensure compatibility, please change your bit depth to 24 or 32-bit color.","Warning",MB_OK);

	GetClientRect (RenderWindow, &rect);
	ResizeGLWindow (rect.right-rect.left, rect.bottom-rect.top);

	SetGLDefaults();

	memset (&select_rgb, 0, sizeof(select_rgb));
	memset (&creation_coords, 0, sizeof(creation_coords));	
	memset (&layer, 1, sizeof(layer));
	memset (&config, 0, sizeof(config));
	memset (&select_rgb, 0xFF, sizeof(select_rgb));

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

