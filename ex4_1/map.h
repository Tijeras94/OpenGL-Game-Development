/*****************************************************************************************************************
**
**	OpenGL Game Development
**	MAP.H
**
**	Copyright:	2003, Chris Seddon. 
**	Written by: Chris Seddon
**	Date:		May 19, 2003
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
		MAP_CAMERA		*camera;		
		MAP_LIGHT		*light;
		MAP_SOUND		*sound;
		MAP_ITEM		*item;

	MAP();
	~MAP();

	bool ColorExists(GLubyte r, GLubyte g, GLubyte b);
	long GenerateColor();
	long GenerateVertexColor(long obj);
	bool VertexColorExists(long obj, GLubyte r, GLubyte g, GLubyte b);
	
	void InsertObject(char *name, GLint type, GLint special=0, GLboolean is_collidable=true, GLboolean is_visible=true);
	bool InsertVertex(long obj, GLdouble x, GLdouble y, GLdouble z, GLfloat r=1.0, GLfloat g=1.0, GLfloat b=1.0, GLfloat a=1.0, GLfloat nx=0.0, GLfloat ny=0.0, GLfloat nz=0.0, GLfloat fogdepth=0.0);
	bool InsertTriangle(long obj, GLint p1, GLint p2, GLint p3, GLfloat u1, GLfloat v1, GLfloat u2, GLfloat v2, GLfloat u3, GLfloat v3);

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
	camera				= NULL;
	light				= NULL;
	sound				= NULL;
	item				= NULL;	
	
}


/* Destructor *******************************************************************************************************/
MAP::~MAP()
{		
	if (header.max_objects > 0)
	{
		for (long i = 0; i < header.max_objects; i++)
		{
			if (object[i].max_vertices > 0) 
			{
				delete [] object[i].vertex;
				object[i].vertex			= NULL;
				object[i].max_vertices	= 0;
			}

			if (object[i].max_triangles > 0)
			{
				delete [] object[i].triangle;
				object[i].triangle		= NULL;
				object[i].max_triangles	= 0;
			}

			if (object[i].max_textures > 0)
			{
				delete [] object[i].texture;
				object[i].texture		= NULL;
				object[i].max_textures	= 0;
			}
		}

		delete [] object;
		object				= NULL;
		header.max_objects	= 0;
	}


	if (header.max_cameras > 0)
	{
		delete [] camera;
		camera					= NULL;
		header.max_cameras		= 0;
	}


	if (header.max_entities > 0)
	{
		delete [] entity;
		entity					= NULL;
		header.max_entities		= 0;
	}


	if (header.max_items > 0)
	{
		delete [] item;
		item					= NULL;
		header.max_items		= 0;
	}


	if (header.max_sounds > 0)
	{
		delete [] sound;
		sound					= NULL;
		header.max_sounds		= 0;
	}


	if (header.max_lights > 0)
	{
		delete [] light;
		light					= NULL;
		header.max_lights		= 0;
	}
}


/* InsertObject *******************************************************************************************************/
void MAP::InsertObject(char *name, GLint type, GLint special, GLboolean is_collidable, GLboolean is_visible)
{
	MAP_OBJECT	new_object;
	long		rgb = GenerateColor();
	
	
	if (name != NULL) strcpy (new_object.name, name);
	else strcpy (new_object.name, "Unknown");
	new_object.type			= type;
	new_object.special		= special;
	new_object.is_collidable= is_collidable;
	new_object.is_visible	= is_visible;
	new_object.max_vertices	= 0;
	new_object.max_triangles= 0;
	new_object.max_textures	= 0;
	new_object.vertex		= NULL;
	new_object.triangle		= NULL;
	new_object.texture		= NULL;
	new_object.select_rgb[0]= GetRValue (rgb);
	new_object.select_rgb[1]= GetGValue (rgb);
	new_object.select_rgb[2]= GetBValue (rgb);
	
	
	if (header.max_objects == 0) object = new MAP_OBJECT[ header.max_objects+1 ];
	else
	{
		MAP_OBJECT *temp = new MAP_OBJECT[ header.max_objects+1 ];
		for (long i = 0; i < header.max_objects; i++)
		{
			strcpy (temp[i].name, object[i].name);
			temp[i].type			= object[i].type;
			temp[i].special			= object[i].special;
			temp[i].is_collidable	= object[i].is_collidable;
			temp[i].is_visible		= object[i].is_visible;
			temp[i].max_vertices	= object[i].max_vertices;
			temp[i].max_triangles	= object[i].max_triangles;
			temp[i].max_textures	= object[i].max_textures;
			temp[i].select_rgb[0]	= object[i].select_rgb[0];
			temp[i].select_rgb[1]	= object[i].select_rgb[1];
			temp[i].select_rgb[2]	= object[i].select_rgb[2];
			
			if (temp[i].max_vertices > 0) 
			{
				temp[i].vertex = new MAP_VERTEX[ temp[i].max_vertices+1 ];
				for (long i2 = 0; i2 < temp[i].max_vertices; i2++) temp[i].vertex[i2] = object[i].vertex[i2];
				
				delete [] object[i].vertex;
				object[i].vertex = NULL;
			}
			else temp[i].vertex = NULL;


			if (temp[i].max_triangles > 0)
			{
				temp[i].triangle = new MAP_TRIANGLE[ temp[i].max_triangles+1 ];
				for (long i2 = 0; i2 < temp[i].max_triangles; i2++) temp[i].triangle[i2] = object[i].triangle[i2];

				delete [] object[i].triangle;
				object[i].triangle = NULL;
			}
			else temp[i].triangle = NULL;


			if (temp[i].max_textures > 0)
			{
				temp[i].texture = new MAP_TEXTURE[ temp[i].max_textures+1 ];
				for (long i2 = 0; i2 < temp[i].max_textures; i2++) temp[i].texture[i2] = object[i].texture[i2];

				delete [] object[i].texture;
				object[i].texture = NULL;
			}
			else temp[i].texture = NULL;

		}
		delete [] object;
		object = NULL;

		object = new MAP_OBJECT[ header.max_objects+2 ];
		for (i = 0; i < header.max_objects; i++)
		{
			strcpy (object[i].name, temp[i].name);
			object[i].type			= temp[i].type;
			object[i].special		= temp[i].special;
			object[i].is_collidable	= temp[i].is_collidable;
			object[i].is_visible	= temp[i].is_visible;
			object[i].max_vertices	= temp[i].max_vertices;
			object[i].max_triangles	= temp[i].max_triangles;
			object[i].max_textures	= temp[i].max_textures;
			object[i].select_rgb[0]	= temp[i].select_rgb[0];
			object[i].select_rgb[1]	= temp[i].select_rgb[1];
			object[i].select_rgb[2]	= temp[i].select_rgb[2];
			
			if (object[i].max_vertices > 0) 
			{
				object[i].vertex = new MAP_VERTEX[ object[i].max_vertices+1 ];
				for (long i2 = 0; i2 < object[i].max_vertices; i2++) object[i].vertex[i2] = temp[i].vertex[i2];
				
				delete [] temp[i].vertex;
				temp[i].vertex = NULL;
			}
			else object[i].vertex = NULL;


			if (object[i].max_triangles > 0)
			{
				object[i].triangle = new MAP_TRIANGLE[ object[i].max_triangles+1 ];
				for (long i2 = 0; i2 < object[i].max_triangles; i2++) object[i].triangle[i2] = temp[i].triangle[i2];

				delete [] temp[i].triangle;
				temp[i].triangle = NULL;
			}
			else object[i].triangle = NULL;


			if (object[i].max_textures > 0)
			{
				object[i].texture = new MAP_TEXTURE[ temp[i].max_textures+1 ];
				for (long i2 = 0; i2 < temp[i].max_textures; i2++) object[i].texture[i2] = temp[i].texture[i2];

				delete [] temp[i].texture;
				temp[i].texture = NULL;
			}
			else object[i].texture = NULL;

		}

		delete [] temp;
		temp = NULL;
	}

	object[ header.max_objects ] = new_object;
	header.max_objects++;
}


/* GenerateVertexColor **********************************************************************************************************/
long MAP::GenerateVertexColor(long obj)
{
	GLubyte r, g, b;
	
	r = rand()%256;
	g = rand()%256;
	b = rand()%256;
	while (VertexColorExists(obj, r, g, b))
	{
		r = rand()%256;
		g = rand()%256;
		b = rand()%256;
	}
	return (RGB(r,g,b));
}


/* InsertVertex **********************************************************************************************************/
bool MAP::InsertVertex(long obj, GLdouble x, GLdouble y, GLdouble z, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat fogdepth)
{
	MAP_VERTEX	new_vertex;
	long		rgb = GenerateVertexColor(obj);

	if (obj > header.max_objects || obj < 0) return (false);

	new_vertex.xyz[0]		= x;
	new_vertex.xyz[1]		= y;
	new_vertex.xyz[2]		= z;
	new_vertex.rgba[0]		= r;
	new_vertex.rgba[1]		= g;
	new_vertex.rgba[2]		= b;
	new_vertex.rgba[3]		= a;
	new_vertex.normal[0]	= nx;
	new_vertex.normal[1]	= ny;
	new_vertex.normal[2]	= nz;	
	new_vertex.fog_depth	= fogdepth;
	new_vertex.select_rgb[0]= GetRValue(rgb);
	new_vertex.select_rgb[1]= GetGValue(rgb);
	new_vertex.select_rgb[2]= GetBValue(rgb);

	if (object[obj].max_vertices == 0) object[obj].vertex = new MAP_VERTEX[ object[obj].max_vertices+1 ];
	else
	{
		MAP_VERTEX	*temp = new MAP_VERTEX[ object[obj].max_vertices+1 ];
		for (long i = 0; i < object[obj].max_vertices; i++) temp[i] = object[obj].vertex[i];
		
		delete [] object[obj].vertex;		
		object[obj].vertex = new MAP_VERTEX[ object[obj].max_vertices+2 ];

		for (i = 0; i < object[obj].max_vertices; i++) object[obj].vertex[i] = temp[i];
		delete [] temp;
		temp = NULL;
	}
	object[obj].vertex[ object[obj].max_vertices ] = new_vertex;
	object[obj].max_vertices++;

	return (true);
}


/* ColorExists **********************************************************************************************************/
bool MAP::ColorExists(GLubyte r, GLubyte g, GLubyte b)
{
	if ((r == 255 && g == 0 && b == 0) || 
		(r == 0 && g == 255 && b == 0) ||
		(r == 0 && g == 0 && b == 255) ||
		(r == 255 && g == 255 && b == 255)) return (true);

	if (header.max_cameras > 0 )
	{
		for (long i = 0; i < header.max_cameras; i++)
		{
			if (camera[i].select_rgb[0] == r && camera[i].select_rgb[1] == g && camera[i].select_rgb[2] == b) return (true);
		}
	}
	if (header.max_entities > 0)
	{
		for (long i = 0; i < header.max_entities; i++)
		{
			if (entity[i].select_rgb[0] == r && entity[i].select_rgb[1] == g && entity[i].select_rgb[2] == b) return (true);
		}
	}
	if (header.max_items > 0)
	{
		for (long i = 0; i < header.max_items; i++)
		{
			if (item[i].select_rgb[0] == r && item[i].select_rgb[1] == g && item[i].select_rgb[2] == b) return (true);
		}
	}
	if (header.max_lights > 0)
	{
		for (long i = 0; i < header.max_lightmaps; i++)
		{
			if (light[i].select_rgb[0] == r && light[i].select_rgb[1] == g && light[i].select_rgb[2] == b) return (true);
		}
	}	
	if (header.max_objects > 0)
	{
		for (long i = 0; i < header.max_objects; i++)
		{
			if (object[i].select_rgb[0] == r && object[i].select_rgb[1] == g && object[i].select_rgb[2] == b) return (true);
		}
	}
	if (header.max_sounds > 0)
	{
		for (long i = 0; i < header.max_sounds; i++)
		{
			if (sound[i].select_rgb[0] == r && sound[i].select_rgb[1] == g && sound[i].select_rgb[2] == b) return (true);
		}
	}	

	return (false);
}


/* GenerateColor **********************************************************************************************************/
long MAP::GenerateColor()
{
	GLubyte r, g, b;


	r = rand() % 256;
	g = rand() % 256;
	b = rand() % 256;	
	while (ColorExists(r, g, b))
	{
		r = rand() % 256;
		g = rand() % 256;
		b = rand() % 256;	
	}
	

	return (RGB(r,g,b));
}


/* InsertTriangle **********************************************************************************************************/
bool MAP::InsertTriangle(long obj, GLint p1, GLint p2, GLint p3, GLfloat u1, GLfloat v1, GLfloat u2, GLfloat v2, GLfloat u3, GLfloat v3)
{
	MAP_TRIANGLE new_triangle;
	if (obj > header.max_objects) return (false);

	new_triangle.point[0] = p1;
	new_triangle.point[1] = p2;
	new_triangle.point[2] = p3;
	for (long i = 0; i < MAX_TEXTURE_LAYERS; i++) 
	{
		new_triangle.uv[i].uv1[0] = u1;
		new_triangle.uv[i].uv1[1] = v1;

		new_triangle.uv[i].uv2[0] = u2;
		new_triangle.uv[i].uv2[1] = v2;

		new_triangle.uv[i].uv3[0] = u3;
		new_triangle.uv[i].uv3[1] = v3;		
	}

	if (object[ obj ].max_triangles <= 0) object[ obj ].triangle = new MAP_TRIANGLE[1];
	else 
	{
		MAP_TRIANGLE *temp = new MAP_TRIANGLE[ object[obj].max_triangles+1 ];		
		for (long i = 0; i < object[obj].max_triangles; i++) temp[i] = object[obj].triangle[i];

		delete [] object[obj].triangle;
		object[obj].triangle = new MAP_TRIANGLE[ object[obj].max_triangles+2 ];
		for (i = 0; i < object[obj].max_triangles; i++) object[obj].triangle[i] = temp[i];

		delete [] temp;
		temp = NULL;
	}

	object[obj].triangle[ object[obj].max_triangles ] = new_triangle;
	object[obj].max_triangles++;

	return (true);
}


/* VertexColorExists **********************************************************************************************************/
bool MAP::VertexColorExists(long obj, GLubyte r, GLubyte g, GLubyte b)
{
	if ((r == 255 && g == 0 && b == 0) || 
		(r == 0 && g == 255 && b == 0) ||
		(r == 0 && g == 0 && b == 255) ||
		(r == 255 && g == 255 && b == 255)) return (true);

	for (long i = 0; i < object[ obj ].max_vertices; i++)
	{
		if (object[ obj ].select_rgb[0] == r && object[ obj ].select_rgb[1] == g && object[ obj ].select_rgb[2] == b) return (true);
	}

	return (false);
}


