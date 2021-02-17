
/* Texture Class *************************************************************************************************/
class TEXTURE 
{
	public:
		TEXTURE();
		~TEXTURE();

		BITMAPFILEHEADER	file_header;
		BITMAPINFOHEADER	info_header;		
		unsigned char		*data;

		bool Load(char *filename);
		void Release();
};


/* Constructor ***************************************************************************************************/
TEXTURE::TEXTURE()
{
	data = NULL;
	memset (&file_header, 0, sizeof(file_header));
	memset (&info_header, 0, sizeof(info_header));
}


/* Destructor ****************************************************************************************************/
TEXTURE::~TEXTURE()
{
}


/* Load **********************************************************************************************************/
bool TEXTURE::Load(char *filename)
{
	FILE			*fp;
	long			image_size;	
	
	Release();

	fp = fopen(filename, "rb");
	if (fp == NULL) return (false);				
				
		fread (&file_header, sizeof(BITMAPFILEHEADER), 1, fp);
		fread (&info_header,sizeof(BITMAPINFOHEADER), 1, fp);
		if (info_header.biBitCount != 24)
		{		
			memset (&file_header, 0, sizeof(BITMAPFILEHEADER));
			memset (&info_header, 0, sizeof(BITMAPINFOHEADER));
			fclose (fp);
			return (false);
		}

		image_size	= info_header.biWidth * info_header.biHeight * 3;
		data		= new unsigned char[ image_size+1 ];
		fread (data, image_size, 1, fp);		
		
	fclose (fp);

		
	return (true);
}


/* Release *******************************************************************************************************/
void TEXTURE::Release()
{
	if (data != NULL)
	{
		delete [] data;
		data = NULL;
		memset (&file_header, 0, sizeof(file_header));
		memset (&info_header, 0, sizeof(info_header));
	}
}