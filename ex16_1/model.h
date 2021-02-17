/*****************************************************************************************************************
**
**	OpenGL Game Development
**	MODEL.H
**
**	Copyright:	2003, Chris Seddon.
**	Written by: Chris Seddon
**	Date:		Mar 17, 2004
**
******************************************************************************************************************/

/* Constants *****************************************************************************************************/
#define KEYWORD_MATERIAL_COUNT	"*MATERIAL_COUNT"
#define KEYWORD_BITMAP			"*BITMAP"
#define KEYWORD_OBJECT			"*GEOMOBJECT"
#define KEYWORD_NUM_VERTICES	"*MESH_NUMVERTEX"
#define KEYWORD_NUM_TRIANGLES	"*MESH_NUMFACES"
#define KEYWORD_VERTEX			"*MESH_VERTEX"
#define KEYWORD_TRIANGLE		"*MESH_FACE"
#define KEYWORD_NUM_TVERTEX		"*MESH_NUMTVERTEX"
#define KEYWORD_TVERTEX			"*MESH_TVERT"
#define KEYWORD_TFACE			"*MESH_TFACE"
#define KEYWORD_MATERIAL_REF	"*MATERIAL_REF"


/* Structures ****************************************************************************************************/
typedef struct
{
	GLfloat xyz[3];
} MODEL_VERTEX;


typedef struct
{
	GLfloat uv[2];
} MODEL_UV_COORD;


typedef struct
{
	long	point[3];
	long	uv_point[3];
} MODEL_TRIANGLE;


typedef struct
{
	long max_vertices;
	long max_triangles;
	long max_uv_coord;

	long material;

	MODEL_VERTEX	*vertex;
	MODEL_TRIANGLE	*triangle;
	MODEL_UV_COORD	*uv_coord;
} MODEL_OBJECT;


typedef struct
{
	char filename[500];
	long tex_id;
} MODEL_MATERIAL;


/* Class Definition **********************************************************************************************/
class MODEL
{
		long GetObjectCount();
		long CountStrings(char *buffer, char *string);

	public:

		MODEL_MATERIAL	*mdl_material;
		long			max_materials;

		MODEL_OBJECT	*mdl_object;
		long			max_objects;

		MODEL();
		~MODEL();
		bool Load(char *filename);
		void Release();
};


/* Construtor ****************************************************************************************************/
MODEL::MODEL()
{
	max_materials	= 0;
	mdl_material	= NULL;

	max_objects		= 0;
	mdl_object		= NULL;
}


/* Destructor ****************************************************************************************************/
MODEL::~MODEL()
{
}


/* CountStrings **************************************************************************************************/
long MODEL::CountStrings(char *buffer, char *string)
{
	char *tmp, token[] = "	 \t\r\n";
	long count = 0;

	tmp = strtok(strdup(buffer), token);
	while (tmp != NULL)
	{
		if (strcmp(tmp, string) == 0) count++;
		tmp = strtok(NULL, token);
	}

	return (count);
}


/* Load **********************************************************************************************************/
bool MODEL::Load(char *filename)
{
	FILE *fp;
	long filesize;
	char *buffer;
	char *tmp, token[]	= "	 \\\":\t\r\n";
	long cur_material	= -1;
	long cur_object		= -1;

	Release();

	fp = fopen (filename, "rb");
	if (fp == NULL) return (false);
		fseek (fp, 0, SEEK_END);
		filesize = ftell(fp);
		buffer = new char[ filesize+1 ];

		fseek (fp, 0, SEEK_SET);
		fread (buffer, 1, filesize, fp);
	fclose (fp);

	max_objects = CountStrings (buffer, KEYWORD_OBJECT);
	mdl_object	= new MODEL_OBJECT[ max_objects+1 ];

	tmp = strtok(strdup(buffer), token);
	while (tmp != NULL)
	{		
		if (strcmp(tmp, KEYWORD_MATERIAL_COUNT) == 0)		
		{
			max_materials	= strtol(strtok(NULL,token), NULL, 10);
			mdl_material	= new MODEL_MATERIAL[ max_materials+1 ];	
		}
		else if (strcmp(tmp, KEYWORD_BITMAP) == 0)
		{			
			cur_material++;		
			strcpy (mdl_material[ cur_material ].filename, strtok (NULL, token));						
		}
		else if (strcmp(tmp, KEYWORD_OBJECT) == 0)
		{
			cur_object++;

			mdl_object[ cur_object ].max_vertices	= 0;
			mdl_object[ cur_object ].max_triangles	= 0;
			mdl_object[ cur_object ].max_uv_coord	= 0;
			mdl_object[ cur_object ].vertex			= NULL;
			mdl_object[ cur_object ].triangle		= NULL;
			mdl_object[ cur_object ].uv_coord		= NULL;
		}
		else if (strcmp(tmp, KEYWORD_NUM_VERTICES) == 0)
		{
			mdl_object[ cur_object ].max_vertices	= strtol(strtok(NULL,token), NULL, 10);
			mdl_object[ cur_object ].vertex			= new MODEL_VERTEX[ mdl_object[cur_object].max_vertices+1 ];
		}
		else if (strcmp(tmp, KEYWORD_NUM_TRIANGLES) == 0)
		{
			mdl_object[ cur_object ].max_triangles	= strtol(strtok(NULL,token), NULL, 10);
			mdl_object[ cur_object ].triangle		= new MODEL_TRIANGLE[ mdl_object[cur_object].max_triangles+1 ];
		}
		else if (strcmp(tmp, KEYWORD_VERTEX) == 0)
		{
			long cur_vertex = strtol(strtok(NULL,token), NULL, 10);

			sscanf (strtok(NULL,token), "%f", &mdl_object[ cur_object ].vertex[ cur_vertex ].xyz[0]);
			sscanf (strtok(NULL,token), "%f", &mdl_object[ cur_object ].vertex[ cur_vertex ].xyz[2]);
			sscanf (strtok(NULL,token), "%f", &mdl_object[ cur_object ].vertex[ cur_vertex ].xyz[1]);
		}
		else if (strcmp(tmp, KEYWORD_TRIANGLE) == 0)
		{
			long cur_tri = strtol(strtok(NULL,token), NULL, 10);

			strtok(NULL, token);
			mdl_object[ cur_object ].triangle[ cur_tri ].point[0] = strtol(strtok(NULL,token), NULL, 10);

			strtok(NULL, token);
			mdl_object[ cur_object ].triangle[ cur_tri ].point[1] = strtol(strtok(NULL,token), NULL, 10);

			strtok(NULL, token);
			mdl_object[ cur_object ].triangle[ cur_tri ].point[2] = strtol(strtok(NULL,token), NULL, 10);
		}
		else if (strcmp(tmp, KEYWORD_NUM_TVERTEX) == 0)
		{
			mdl_object[ cur_object ].max_uv_coord	= strtol(strtok(NULL,token), NULL, 10);
			mdl_object[ cur_object ].uv_coord		= new MODEL_UV_COORD[ mdl_object[cur_object].max_uv_coord+1 ];
		}
		else if (strcmp(tmp, KEYWORD_TVERTEX) == 0)
		{
			long cur_vertex = strtol(strtok(NULL,token), NULL, 10);

			sscanf (strtok(NULL,token), "%f", &mdl_object[ cur_object ].uv_coord[ cur_vertex ].uv[0]);
			sscanf (strtok(NULL,token), "%f", &mdl_object[ cur_object ].uv_coord[ cur_vertex ].uv[1]);
		}
		else if (strcmp(tmp, KEYWORD_TFACE) == 0)
		{
			long cur_tri = strtol(strtok(NULL,token), NULL, 10);

			mdl_object[ cur_object ].triangle[ cur_tri ].uv_point[0] = strtol(strtok(NULL,token), NULL, 10);
			mdl_object[ cur_object ].triangle[ cur_tri ].uv_point[1] = strtol(strtok(NULL,token), NULL, 10);
			mdl_object[ cur_object ].triangle[ cur_tri ].uv_point[2] = strtol(strtok(NULL,token), NULL, 10);
		}
		else if (strcmp(tmp, KEYWORD_MATERIAL_REF) == 0) mdl_object[ cur_object ].material = strtol(strtok(NULL,token), NULL, 10);		

		tmp = strtok(NULL, token);
	}
	
	delete [] buffer;
	buffer = NULL;

	return (true);
}


/* Release *******************************************************************************************************/
void MODEL::Release()
{
	if (max_objects > 0)
	{
		for (long obj = 0; obj < max_objects; obj++)
		{
			if (mdl_object[ obj ].max_vertices > 0) delete [] mdl_object[ obj ].vertex;
			if (mdl_object[ obj ].max_triangles > 0) delete [] mdl_object[ obj ].triangle;
			if (mdl_object[ obj ].max_uv_coord > 0) delete [] mdl_object[ obj ].uv_coord;
		}
		delete [] mdl_object;
		mdl_object = NULL;
		max_objects = 0;
	}

	if (max_materials > 0)
	{
		delete [] mdl_material;
		mdl_material	= NULL;
		max_materials	= 0;
	}
}