/*****************************************************************************************************************
**
**	OpenGL Game Development
**	MAP.H
**
**	Copyright:	2003, Chris Seddon. 
**	Written by: Chris Seddon
**	Date:		Apr 25, 2003
**
******************************************************************************************************************/

/* Headers *******************************************************************************************************/
#include <GL/gl.h>
#include <GL/glu.h>
#include <windows.h>


/* Constant *************************************************************************************************************/
#define MAX_TEXTURE_LAYERS		2
#define MAX_STRING_SIZE			500

#define VERSION					1
#define REVISION				0


/* Structures *******************************************************************************************************/
typedef struct
{
	GLint			version;
	GLint			revision;
} MAP_VERSION;


typedef struct
{
	GLint			max_objects;
	GLint			max_lights;
	GLint			max_lightmaps;
	GLint			max_cameras;

	GLint			max_entities;
	GLint			max_items;
	GLint			max_sounds;

	GLboolean		use_skybox;
	GLboolean		use_fog;
} MAP_HEADER;


typedef struct
{
	char			filename[ MAX_STRING_SIZE ];	
	GLint			texid;
} MAP_SKYBOX_SIDE;


typedef struct
{
	MAP_SKYBOX_SIDE	front;
	MAP_SKYBOX_SIDE back;
	MAP_SKYBOX_SIDE	left;
	MAP_SKYBOX_SIDE right;
	MAP_SKYBOX_SIDE top;
	MAP_SKYBOX_SIDE bottom;
} MAP_SKYBOX;


typedef struct
{
	GLint			mode;
	GLfloat			start;
	GLfloat			end;
	GLfloat			density;
	GLfloat			rgba[4];	
} MAP_FOG;


typedef struct
{
	GLdouble		xyz[3];
	GLfloat			angle[3];
	GLint			model;

	GLubyte			select_rgb[3];
} MAP_STARTING_POSITION;


typedef struct
{
	char			map_name[ MAX_STRING_SIZE ];
	GLint			map_type;
	GLint			map_exit_rules;

	MAP_STARTING_POSITION	single_player;
	MAP_STARTING_POSITION	deathmatch[2];
} MAP_DETAILS;


typedef struct
{
	GLdouble		xyz[3];
	GLfloat			rgba[4];
	GLfloat			normal[3];
	GLfloat			fog_depth;

	GLubyte			select_rgb[3];
} MAP_VERTEX;


typedef struct
{
	GLfloat			uv1[2];	
	GLfloat			uv2[2];
	GLfloat			uv3[2];
} MAP_UV_COORDS;


typedef struct
{
	GLint			point[3];
	MAP_UV_COORDS	uv[ MAX_TEXTURE_LAYERS ];
} MAP_TRIANGLE;


typedef struct
{
	char			filename[ MAX_STRING_SIZE ];
	GLint			id;
	GLint			style;
	GLint			blend_src;
	GLint			blend_dst;
} MAP_TEXTURE;


typedef struct
{	
	char			name[ MAX_STRING_SIZE ];
	GLint			type;
	GLint			special;
	
	GLboolean		is_collidable;
	GLboolean		is_visible;

	GLint			max_textures;
	GLint			max_triangles;
	GLint			max_vertices;
	
	MAP_TEXTURE		*texture;
	MAP_TRIANGLE	*triangle;
	MAP_VERTEX		*vertex;


	GLubyte			select_rgb[3];
} MAP_OBJECT;


typedef struct
{
	char			name[ MAX_STRING_SIZE ];
	GLdouble		xyz[3];
	GLfloat			angle[3];

	GLubyte			select_rgb[3];
} MAP_CAMERA;


typedef struct
{
	char			name[ MAX_STRING_SIZE ];	
	GLint			type;	
	GLdouble		xyz[3];
	GLfloat			angle[3];
	GLfloat			rgba[4];

	char			texture_filename[ MAX_STRING_SIZE ];
	GLint			texture;

	GLint			max_inclusions;
	GLint			*inclusions;

	GLubyte			select_rgb[3];
} MAP_LIGHT;


typedef struct
{
	GLint			type;	
	GLdouble		xyz[3];
	GLfloat			angle[3];

	GLint			health;
	GLint			strength;
	GLint			armour;

	GLubyte			select_rgb[3];
} MAP_ENTITY;


typedef struct
{
	GLint			type;	
	GLint			respawn_wait;
	GLint			respawn_time;
	GLdouble		xyz[3];

	GLubyte			select_rgb[3];
} MAP_ITEM;


typedef struct
{
	char			filename[ MAX_STRING_SIZE ];
	GLint			id;
	GLdouble		xyz[3];
	GLfloat			angle[3];	

	GLubyte			select_rgb[3];
} MAP_SOUND;


/* MAP Class *******************************************************************************************************/
class MAP {

	public:
		MAP_VERSION		version;
		MAP_HEADER		header;
		MAP_SKYBOX		skybox;
		MAP_FOG			fog;
		MAP_DETAILS		details;
		
		MAP_OBJECT		*object;
		MAP_ENTITY		*entity;
		MAP_LIGHT		*light;
		MAP_SOUND		*sound;
		MAP_ITEM		*item;

	MAP();
	~MAP();
};


/* Constructor *******************************************************************************************************/
MAP::MAP()
{	
	version.version		= VERSION;
	version.revision	= REVISION;

	memset (&header, 0, sizeof(header));	
	memset (&details, 0, sizeof(details));
	memset (&skybox, 0, sizeof(skybox));
	memset (&fog, 0, sizeof(fog));

	object				= NULL;
	entity				= NULL;
	light				= NULL;
	sound				= NULL;
	item				= NULL;
}


/* Destructor *******************************************************************************************************/
MAP::~MAP()
{	
}