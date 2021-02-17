/*****************************************************************************************************************
**
**	OpenGL Game Development
**	Ex16_1.CPP (Example 16.1)
**
**	Copyright:	2003-2004, Chris Seddon.
**	Written by: Chris Seddon
**	Date:		Mar 21, 2004
**
*****************************************************************************************************************/

/* Headers *******************************************************************************************************/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "raster.h"
#include "texture.h"
#include "map.h"
#include "model.h"

#include "resource.h"

#include "glext.h"
#include "wglext.h"


/* Constants *****************************************************************************************************/
#define MAX_TEXTURE_NAMES	50
#define MAX_TEXTURE_FILES	13

#define PI					3.1415926535897932384626433832795
#define MOVEMENT_SPEED		0.125
#define TURN_SPEED			12.0


/* Structures ****************************************************************************************************/
typedef struct
{
	char			filename[ 100 ];
	bool			is_transparent;
	long			filter_type;
	long			wrap_type;
} TEXTURE_FILE;


/* Global Variables **********************************************************************************************/
char							*gl_extension				= NULL;
PFNGLPOINTPARAMETERFARBPROC		glPointParameterfARB		= NULL;
PFNGLPOINTPARAMETERFVARBPROC	glPointParameterfvARB		= NULL;

PFNGLACTIVETEXTUREARBPROC       glActiveTextureARB			= NULL;
PFNGLMULTITEXCOORD2FVARBPROC    glMultiTexCoord2fvARB		= NULL;

PFNGLFOGCOORDFPROC				glFogCoordfExt				= NULL;

PFNWGLSWAPINTERVALEXTPROC		wglSwapIntervalExt			= NULL;
PFNWGLGETSWAPINTERVALEXTPROC	wglGetSwapIntervalExt		= NULL;


HWND		Window;
DEVMODE		old_resolution;
DEVMODE		new_resolution;

RASTER		raster;
MAP			map;
MAP_ENTITY	player, enemy;
MODEL		item_gun;

bool		use_texture_compression;
bool		use_anisotropic;


GLuint		TextureName[ MAX_TEXTURE_NAMES ];
TEXTURE_FILE texture_file[] =
	{
		{"hud.bmp", 		true,	GL_NEAREST, GL_REPEAT},
		{"npot_hud.bmp",	true,	GL_LINEAR,	GL_REPEAT},
		{"wall.bmp", 		false,	GL_LINEAR,	GL_REPEAT},
		{"floor.bmp", 		false,	GL_LINEAR, 	GL_REPEAT},
		{"ceiling.bmp",		false,	GL_LINEAR, 	GL_REPEAT},
		{"sky_fr.bmp", 		false,	GL_LINEAR, 	GL_REPEAT},
		{"sky_bk.bmp", 		false,	GL_LINEAR, 	GL_REPEAT},
		{"sky_lf.bmp", 		false,	GL_LINEAR, 	GL_REPEAT},
		{"sky_rt.bmp", 		false,	GL_LINEAR, 	GL_REPEAT},
		{"sky_up.bmp", 		false,	GL_LINEAR, 	GL_REPEAT},
		{"sky_dn.bmp", 		false,	GL_LINEAR,	GL_REPEAT},
		{"light.bmp", 		false, 	GL_NEAREST,	GL_CLAMP},
		{"item_gun.bmp", 	false, 	GL_LINEAR, 	GL_REPEAT},
		{"point.bmp",		false,	GL_LINEAR,	GL_CLAMP}
	};


/* GetTextureID **************************************************************************************************/
long GetTextureID(char *filename)
{
	for (long tex_id = 0; tex_id < MAX_TEXTURE_FILES; tex_id++)
	{
		if (strstr(filename, texture_file[ tex_id ].filename) != NULL) return (tex_id+1);
	}
	return (0);
}


/* LoadMap *******************************************************************************************************/
bool LoadMap(char *filename)
{
	if (!map.Open(filename)) return (false);

	for (long obj = 0; obj < map.header.max_objects; obj++)
	{
		for (long tex_layer = 0; tex_layer < MAX_TEXTURE_LAYERS; tex_layer++)
		{
			map.object[ obj ].texture[ tex_layer ].id = GetTextureID(map.object[ obj ].texture[ tex_layer ].filename);
		}
	}


	if (map.header.use_skybox)
	{
		map.skybox.front.texid	= GetTextureID(map.skybox.front.filename);
		map.skybox.back.texid	= GetTextureID(map.skybox.back.filename);
		map.skybox.left.texid	= GetTextureID(map.skybox.left.filename);
		map.skybox.right.texid	= GetTextureID(map.skybox.right.filename);
		map.skybox.top.texid	= GetTextureID(map.skybox.top.filename);
		map.skybox.bottom.texid = GetTextureID(map.skybox.bottom.filename);
	}


	if (map.header.use_fog)
	{
		glFogi (GL_FOG_MODE, map.fog.mode);
		glFogf (GL_FOG_DENSITY, map.fog.density);
		glFogf (GL_FOG_START, map.fog.start);
		glFogf (GL_FOG_END, map.fog.end);
		glFogfv (GL_FOG_COLOR, map.fog.rgba);
		glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
		glEnable (GL_FOG);
	}
	else glDisable (GL_FOG);


	return (true);
}


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



/* RenderObject **************************************************************************************************/
void RenderObject(long cur_obj)
{	
	glBegin (GL_TRIANGLES);
	for (long tri = 0; tri < map.object[ cur_obj ].max_triangles; tri++)
	{
		long vertex_1 = map.object[ cur_obj ].triangle[ tri ].point[0];
		long vertex_2 = map.object[ cur_obj ].triangle[ tri ].point[1];
		long vertex_3 = map.object[ cur_obj ].triangle[ tri ].point[2];

		for (long layer = 0; layer < MAX_TEXTURE_LAYERS; layer++) glMultiTexCoord2fvARB (GL_TEXTURE0_ARB+layer, map.object[ cur_obj ].triangle[ tri ].uv[ layer ].uv1);
		glFogCoordfExt (map.object[ cur_obj ].vertex[ vertex_1 ].fog_depth);
		glVertex3dv (map.object[ cur_obj ].vertex[ vertex_1 ].xyz);

		for (layer = 0; layer < MAX_TEXTURE_LAYERS; layer++) glMultiTexCoord2fvARB (GL_TEXTURE0_ARB+layer, map.object[ cur_obj ].triangle[ tri ].uv[ layer ].uv2);
		glFogCoordfExt (map.object[ cur_obj ].vertex[ vertex_2 ].fog_depth);
		glVertex3dv (map.object[ cur_obj ].vertex[ vertex_2 ].xyz);

		for (layer = 0; layer < MAX_TEXTURE_LAYERS; layer++) glMultiTexCoord2fvARB (GL_TEXTURE0_ARB+layer, map.object[ cur_obj ].triangle[ tri ].uv[ layer ].uv3);
		glFogCoordfExt (map.object[ cur_obj ].vertex[ vertex_3 ].fog_depth);
		glVertex3dv (map.object[ cur_obj ].vertex[ vertex_3 ].xyz);
	}
	glEnd();
}


/* RenderSkyBox **************************************************************************************************/
void RenderSkyBox()
{
	glBindTexture (GL_TEXTURE_2D, TextureName[ map.skybox.front.texid ]);
	glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 1.0f);
		glVertex3f (-1.0f, -1.0f, -1.0f);

		glTexCoord2f (1.0f, 1.0f);
		glVertex3f (1.0f, -1.0f, -1.0f);

		glTexCoord2f (1.0f, 0.0f);
		glVertex3f (1.0f, 1.0f, -1.0f);

		glTexCoord2f (0.0f, 0.0f);
		glVertex3f (-1.0f, 1.0f, -1.0f);
	glEnd();

	glBindTexture (GL_TEXTURE_2D, TextureName[ map.skybox.back.texid ]);
	glBegin (GL_QUADS);
		glTexCoord2f (1.0f, 1.0f);
		glVertex3f (-1.0f, -1.0f, 1.0f);

		glTexCoord2f (0.0f, 1.0f);
		glVertex3f (1.0f, -1.0f, 1.0f);

		glTexCoord2f (0.0f, 0.0f);
		glVertex3f (1.0f, 1.0f, 1.0f);

		glTexCoord2f (1.0f, 0.0f);
		glVertex3f (-1.0f, 1.0f, 1.0f);
	glEnd();


	glBindTexture (GL_TEXTURE_2D, TextureName[ map.skybox.right.texid ]);
	glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 1.0f);
		glVertex3f (1.0f, -1.0f, -1.0f);

		glTexCoord2f (1.0f, 1.0f);
		glVertex3f (1.0f, -1.0f, 1.0f);

		glTexCoord2f (1.0f, 0.0f);
		glVertex3f (1.0f, 1.0f, 1.0f);

		glTexCoord2f (0.0f, 0.0f);
		glVertex3f (1.0f, 1.0f, -1.0f);
	glEnd();

	glBindTexture (GL_TEXTURE_2D, TextureName[ map.skybox.left.texid ]);
	glBegin (GL_QUADS);
		glTexCoord2f (1.0f, 1.0f);
		glVertex3f (-1.0f, -1.0f, -1.0f);

		glTexCoord2f (0.0f, 1.0f);
		glVertex3f (-1.0f, -1.0f, 1.0f);

		glTexCoord2f (0.0f, 0.0f);
		glVertex3f (-1.0f, 1.0f, 1.0f);

		glTexCoord2f (1.0f, 0.0f);
		glVertex3f (-1.0f, 1.0f, -1.0f);
	glEnd();

	glBindTexture (GL_TEXTURE_2D, TextureName[ map.skybox.top.texid ]);
	glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 0.0f);
		glVertex3f (-1.0f, 1.0f, -1.0f);

		glTexCoord2f (1.0f, 0.0f);
		glVertex3f (-1.0f, 1.0f, 1.0f);

		glTexCoord2f (1.0f, 1.0f);
		glVertex3f (1.0f, 1.0f, 1.0f);

		glTexCoord2f (0.0f, 1.0f);
		glVertex3f (1.0f, 1.0f, -1.0f);
	glEnd();

	glBindTexture (GL_TEXTURE_2D, TextureName[ map.skybox.bottom.texid ]);
	glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 0.0f);
		glVertex3f (-1.0f, -1.0f, -1.0f);

		glTexCoord2f (1.0f, 0.0f);
		glVertex3f (-1.0f, -1.0f, 1.0f);

		glTexCoord2f (1.0f, 1.0f);
		glVertex3f (1.0f, -1.0f, 1.0f);

		glTexCoord2f (0.0f, 1.0f);
		glVertex3f (1.0f, -1.0f, -1.0f);
	glEnd();
}


/* RenderLight ***************************************************************************************************/
void RenderLight(long cur_light)
{
	GLfloat s_plane[] = {1.0f, 0.0f, 0.0f, 0.0f};
	GLfloat t_plane[] = {0.0f, 1.0f, 0.0f, 0.0f};
	GLfloat r_plane[] = {0.0f, 0.0f, 1.0f, 0.0f};
	GLfloat q_plane[] = {0.0f, 0.0f, 0.0f, 1.0f};

	glTexGenfv (GL_S, GL_EYE_PLANE, s_plane);
	glTexGenfv (GL_T, GL_EYE_PLANE, t_plane);
	glTexGenfv (GL_R, GL_EYE_PLANE, r_plane);
	glTexGenfv (GL_Q, GL_EYE_PLANE, q_plane);

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);

	glMatrixMode (GL_TEXTURE);
		glPushMatrix();
			glRotated (map.light[ cur_light ].angle[0], 0.0f, 1.0f, 0.0f);
			glRotated (map.light[ cur_light ].angle[1], 0.0f, 0.0f, 1.0f);
			glRotated (map.light[ cur_light ].angle[2], 1.0f, 0.0f, 0.0f);
			glTranslated (map.light[ cur_light ].xyz[0], map.light[ cur_light ].xyz[1], map.light[ cur_light ].xyz[2]);
	glMatrixMode (GL_MODELVIEW);


	glColor4fv (map.light[ cur_light ].rgba);
	for (long obj = 0; obj < map.light[ cur_light ].max_inclusions; obj++)
	{
		RenderObject ( map.light[ cur_light ].inclusions[ obj ] );
	}

	glMatrixMode (GL_TEXTURE);
		glPopMatrix();
		glLoadIdentity();
	glMatrixMode (GL_MODELVIEW);

	glDisable (GL_TEXTURE_GEN_S);
	glDisable (GL_TEXTURE_GEN_T);
	glDisable (GL_TEXTURE_GEN_R);
	glDisable (GL_TEXTURE_GEN_Q);
}


/* LoadModel *****************************************************************************************************/
bool LoadModel (char *filename, MODEL & model)
{
	if (!model.Load(filename)) return (false);

	for (long mat = 0; mat < model.max_materials; mat++)
	{
		model.mdl_material[ mat ].tex_id = GetTextureID(model.mdl_material[ mat ].filename);
	}

	return (true);
}


/* RenderModel ***************************************************************************************************/
void RenderModel(MODEL model)
{
	for (long obj = 0; obj < model.max_objects; obj++)
	{
		long mat = model.mdl_object[ obj ].material;

		glBindTexture (GL_TEXTURE_2D, TextureName[ model.mdl_material[mat].tex_id ]);
		glBegin (GL_TRIANGLES);
		for (long tri = 0; tri < model.mdl_object[ obj ].max_triangles; tri++)
		{
			long p1		= model.mdl_object[ obj ].triangle[ tri ].point[0];
			long p2		= model.mdl_object[ obj ].triangle[ tri ].point[1];
			long p3		= model.mdl_object[ obj ].triangle[ tri ].point[2];
			long uv1	= model.mdl_object[ obj ].triangle[ tri ].uv_point[0];
			long uv2	= model.mdl_object[ obj ].triangle[ tri ].uv_point[1];
			long uv3	= model.mdl_object[ obj ].triangle[ tri ].uv_point[2];

			glTexCoord2fv (model.mdl_object[ obj ].uv_coord[ uv1 ].uv);
			glVertex3fv (model.mdl_object[ obj ].vertex[ p1 ].xyz);

			glTexCoord2fv (model.mdl_object[ obj ].uv_coord[ uv2 ].uv);
			glVertex3fv (model.mdl_object[ obj ].vertex[ p2 ].xyz);

			glTexCoord2fv (model.mdl_object[ obj ].uv_coord[ uv3 ].uv);
			glVertex3fv (model.mdl_object[ obj ].vertex[ p3 ].xyz);
		}
		glEnd();
	}
}


/* RenderItems ***************************************************************************************************/
void RenderItems()
{
	for (long itm = 0; itm < map.header.max_items; itm++)
	{
		glPushMatrix();
			glTranslated (map.item[ itm ].xyz[0], map.item[ itm ].xyz[1], map.item[ itm ].xyz[2]);
			RenderModel( item_gun );
		glPopMatrix();
	}
}


/* RenderMap *****************************************************************************************************/
void RenderMap()
{
	if (map.header.max_objects == 0) return;

	if (map.header.use_skybox) RenderSkyBox();

	RenderItems();

	for (long obj = 0; obj < map.header.max_objects; obj++)
	{
		glColor3f (1.0f, 1.0f, 1.0f);

		for (long layer = 0; layer < MAX_TEXTURE_LAYERS; layer++)
		{
			if (map.object[ obj ].texture[ layer ].id > 0)
			{
				glActiveTextureARB (GL_TEXTURE0_ARB + layer);
				glBindTexture (GL_TEXTURE_2D, TextureName[ map.object[ obj ].texture[ layer ].id ]);
			}
		}

		RenderObject ( obj );
	}
	glActiveTextureARB (GL_TEXTURE0_ARB);


	glBlendFunc (GL_ONE, GL_ONE);
	for (long lit = 0; lit < map.header.max_lights; lit++)
	{
		glBindTexture (GL_TEXTURE_2D, TextureName[ GetTextureID("light.bmp") ]);
		RenderLight ( lit );
	}	
}


/* Render ********************************************************************************************************/
void Render()
{
	glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glPushMatrix();
		glRotatef (player.angle[0], 0.0, 1.0, 0.0);
		glTranslated (player.xyz[0], player.xyz[1], player.xyz[2]);
		RenderMap();
	glPopMatrix();

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (strstr(gl_extension, "ARB_texture_non_power_of_two") != NULL) glBitBlt(GetTextureID("npot_hud.bmp"), 0,240,640,240);
	else glBitBlt(1, 0,240,640,240);

	SwapBuffers(raster.hDC);
}


/* LoadTexture *******************************************************************************************************/
void LoadTexture(char *filename, long texture_id, long mag_filter, long min_filter, long wrap_type)
{
	TEXTURE texture;
	long	internal_format;	

	if (!texture.Load (filename)) return;
		glBindTexture (GL_TEXTURE_2D, TextureName[ texture_id ]);
		
		if (use_texture_compression) internal_format = GL_COMPRESSED_RGBA_ARB;
		else internal_format = GL_RGBA;

		glTexImage2D (GL_TEXTURE_2D, 0, internal_format, texture.info_header.biWidth, texture.info_header.biHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, texture.data);
		gluBuild2DMipmaps (GL_TEXTURE_2D, internal_format, texture.info_header.biWidth, texture.info_header.biHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, texture.data);

		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_type);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_type);
		if (use_anisotropic)
		{
			GLfloat anisotropy;
			glGetFloatv (GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
		}
		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	texture.Release();
}


/* LoadTransparentTexture ********************************************************************************************/
void LoadTransparentTexture(char *filename, long texture_id, long mag_filter, long min_filter, long wrap_type, unsigned char trans_r, unsigned char trans_g, unsigned char trans_b)
{
	TEXTURE			texture;
	unsigned char	*alpha_texture=NULL;
	long			texel;
	long			internal_format;


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
		if (use_texture_compression) internal_format = GL_COMPRESSED_RGBA_ARB;
		else internal_format = GL_RGBA;
		glTexImage2D (GL_TEXTURE_2D, 0, internal_format, texture.info_header.biWidth, texture.info_header.biHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, alpha_texture);
		gluBuild2DMipmaps (GL_TEXTURE_2D, internal_format, texture.info_header.biWidth, texture.info_header.biHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, alpha_texture);

		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_type);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_type);
		if (use_anisotropic)
		{
			GLfloat anisotropy;
			glGetFloatv (GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
		}
		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		delete [] alpha_texture;
		alpha_texture = NULL;
	texture.Release();
}


/* SetGLDefaults *****************************************************************************************************/
void SetGLDefaults()
{
	long	tex_id;
	char	texture_path[] = "../media/";
	char	filename[50];

	glMatrixMode (GL_PROJECTION);
		gluPerspective (90.0f, (float)new_resolution.dmPelsWidth / (float)new_resolution.dmPelsHeight, 0.00001f, 5.0f);
	glMatrixMode (GL_MODELVIEW);

	glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
	glEnable (GL_DEPTH_TEST);
	glDisable (GL_CULL_FACE);
	glEnable (GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glDepthFunc (GL_LEQUAL);

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


				if (SendDlgItemMessage (hWnd, IDC_TEXTURE_COMPRESSION, BM_GETCHECK, 0, 0) == BST_CHECKED) use_texture_compression = true;
				else use_texture_compression = false;

				if (SendDlgItemMessage (hWnd, IDC_ANISOTROPIC, BM_GETCHECK, 0, 0) == BST_CHECKED) use_anisotropic = true;
				else use_anisotropic = false;

				EndDialog (hWnd, 1);
			}
		} break;
	}
	return (0);
}


/* MoveForward ****************************************************************************************************/
void MoveForward(MAP_ENTITY &entity)
{
	double radians = PI / 180.0 * entity.angle[0];
	entity.xyz[0] -= sin(radians) * MOVEMENT_SPEED;
	entity.xyz[2] += cos(radians) * MOVEMENT_SPEED;
}


/* MoveBackward ***************************************************************************************************/
void MoveBackward(MAP_ENTITY &entity)
{
	double radians = PI / 180.0 * entity.angle[0];
	entity.xyz[0] += sin(radians) * MOVEMENT_SPEED;
	entity.xyz[2] -= cos(radians) * MOVEMENT_SPEED;
}


/* StrafeRight ****************************************************************************************************/
void StrafeRight(MAP_ENTITY &entity)
{
	double	new_angle = entity.angle[0] + 90;
	double	radians	= PI / 180.0 * new_angle;
	entity.xyz[0] -= sin(radians) * MOVEMENT_SPEED;
	entity.xyz[2] += cos(radians) * MOVEMENT_SPEED;
}


/* StrafeLeft *****************************************************************************************************/
void StrafeLeft(MAP_ENTITY &entity)
{
	double	new_angle = entity.angle[0] - 90;
	double	radians	= PI / 180.0 * new_angle;
	entity.xyz[0] -= sin(radians) * MOVEMENT_SPEED;
	entity.xyz[2] += cos(radians) * MOVEMENT_SPEED;
}


/* TurnLeft ******************************************************************************************************/
void TurnLeft(MAP_ENTITY &entity)
{
	entity.angle[0] -= TURN_SPEED;
	if (entity.angle[0] < 0.0) entity.angle[0] = 360.0;
}


/* TurnRight *****************************************************************************************************/
void TurnRight(MAP_ENTITY &entity)
{
	entity.angle[0] += TURN_SPEED;
	if (entity.angle[0] > 360.0) entity.angle[0] = 0.0;
}


/* CheckInput ****************************************************************************************************/
void CheckInput()
{
	POINT point;

	if (GetAsyncKeyState (VK_ESCAPE)) PostQuitMessage(0);
	else if (GetAsyncKeyState (VK_UP) || GetAsyncKeyState(VK_MBUTTON)) MoveForward(player);
	else if (GetAsyncKeyState (VK_DOWN) || GetAsyncKeyState(VK_RBUTTON)) MoveBackward(player);
	else if (GetAsyncKeyState (VK_LEFT)) StrafeLeft(player);
	else if (GetAsyncKeyState (VK_RIGHT)) StrafeRight(player);

	GetCursorPos (&point);
	if (point.x > (int)(new_resolution.dmPelsWidth/2)) TurnRight(player);
	else if (point.x < (int)(new_resolution.dmPelsWidth/2)) TurnLeft(player);
	SetCursorPos (new_resolution.dmPelsWidth/2, new_resolution.dmPelsHeight/2);
}


/* LoadGLExtensions **********************************************************************************************/
bool LoadGLExtensions()
{
	gl_extension = (char *)glGetString(GL_EXTENSIONS);

	if (strstr(gl_extension,"GL_ARB_multitexture") != NULL)
	{
		glActiveTextureARB       = (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
		glMultiTexCoord2fvARB    = (PFNGLMULTITEXCOORD2FVARBPROC)wglGetProcAddress("glMultiTexCoord2fvARB");
	}
	else return (false);


	if (strstr(gl_extension,"GL_ARB_texture_compression") != NULL && use_texture_compression) use_texture_compression = true;
	else use_texture_compression = false;


	if (strstr(gl_extension,"GL_EXT_texture_filter_anisotropic") != NULL && use_anisotropic) use_anisotropic = true;
	else use_anisotropic = false;
	

	if (strstr(gl_extension,"GL_EXT_fog_coord") != NULL) 
	{
		glFogCoordfExt = (PFNGLFOGCOORDFPROC)wglGetProcAddress("glFogCoordfEXT");
		if (glFogCoordfExt == NULL) MessageBox (NULL, "failed to load glFogCoord", NULL, MB_OK);
	}


	use_anisotropic = false;
	use_texture_compression = false;

	if (strstr(gl_extension,"WGL_EXT_swap_control") != NULL)
	{
		wglSwapIntervalExt		= (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
		wglGetSwapIntervalExt	= (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
		wglSwapIntervalExt (0);
	}
	
	return (true);
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

	if (!LoadGLExtensions())
	{
		MessageBox (NULL, "Error: Unable to Load Extensions", "ERROR!", MB_OK);
		raster.Release(Window);
		ChangeDisplaySettings (&old_resolution, 0);
		return (0);
	}

	SetGLDefaults();

	LoadMap ("../media/test.map");	
	LoadModel ("../media/item_gun.ase", item_gun);

	ShowCursor(false);
	SetCursorPos(new_resolution.dmPelsWidth/2, new_resolution.dmPelsHeight/2);

	while (1)
	{
		Render();
		CheckInput();

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