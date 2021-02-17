/*****************************************************************************************************************
**
**	OpenGL Game Development
**	EX18_1.CPP (Example 18.1)
**
**	Copyright:	2003-2004, Chris Seddon. 
**	Written by: Chris Seddon
**	Date:		Apr 14, 2004
**
******************************************************************************************************************/

/* Headers *******************************************************************************************************/
#include "al.h"
#include "alc.h"
#include "alut.h"


/* Constants *****************************************************************************************************/
#define MAX_AUDIO_BUFFERS 1


/* Globals *******************************************************************************************************/
char *sound_files[50] = {						
							"birds.wav"
						};


/* Class Definition **********************************************************************************************/
class AUDIO
{
		ALCdevice	*alDevice;
		ALCcontext	*alContext;
		ALuint		SoundBuffer[ MAX_AUDIO_BUFFERS+1 ];
		ALuint		*SoundSource;
		long		max_sources;

		bool LoadWav (char *filename, long buffer);

	public:

		AUDIO();
		~AUDIO();

		bool Init();
		void Release();
			
		long GetBufferID(char *filename);
		void CreateSources(long sources);
		void ReleaseSources();
		bool SetSource(long source, char *filename, float x, float y, float z, float pitch=1.0, float gain=1.0, bool loop=true);
		void PlaySource(long source);
		void StopSource(long source);		

		void SetListener(float x, float y, float z, float player_angle);
};


/* Constructor ***************************************************************************************************/
AUDIO::AUDIO()
{
	alDevice	= NULL;
	alContext	= NULL;

	SoundSource	= NULL;
	max_sources	= 0;
}


/* Destructor ****************************************************************************************************/
AUDIO::~AUDIO()
{
}


/* Init **********************************************************************************************************/
bool AUDIO::Init()
{	
	alDevice = alcOpenDevice(NULL);
	if (alDevice == NULL)
	{
		MessageBox (NULL,"Error: Failed to Open Device", NULL, MB_OK);
		return (false);
	}

	alContext = alcCreateContext(alDevice, NULL);
	if (alContext == NULL)
	{
		MessageBox (NULL, "Error: Failed to initialize OpenAL", NULL, MB_OK);
		return (false);
	}

	alcMakeContextCurrent(alContext);
	if (alcGetError(alDevice) != ALC_NO_ERROR)
	{
		MessageBox (NULL, "Error: Failed to Make Context Current", NULL, MB_OK);
		return (false);
	}

	alGenBuffers(MAX_AUDIO_BUFFERS, SoundBuffer);
	if (alGetError() != AL_NO_ERROR)
	{
		MessageBox (NULL, "Error: Failed to Generate Buffers", NULL, MB_OK);
		return (false);
	}

	for (long buffer = 0; buffer < MAX_AUDIO_BUFFERS; buffer++)
	{
		LoadWav (sound_files[ buffer ], buffer);
	}

	return (true);
}


/* Release *******************************************************************************************************/
void AUDIO::Release()
{
	if (alDevice == NULL || alContext == NULL) return;

	if (max_sources > 0) ReleaseSources();
	
	alDeleteBuffers(MAX_AUDIO_BUFFERS, SoundBuffer);
	alcDestroyContext(alContext);
	alcCloseDevice(alDevice);
	
	alContext	= NULL;
	alDevice	= NULL;
}


/* GetBufferID ***************************************************************************************************/
long AUDIO::GetBufferID(char *filename)
{
	for (long buffer = 0; buffer < MAX_AUDIO_BUFFERS; buffer++)
	{
		if (strcmp(sound_files[ buffer ], filename) == 0) return (buffer);
	}
	return (-1);
}


/* LoadWav *******************************************************************************************************/
bool AUDIO::LoadWav (char *filename, long buffer)
{
	char		filepath[500];
	char		errmsg[500];
	ALsizei		size;
	ALsizei		freq;
	ALenum		format;
	ALvoid		*data;
	ALboolean	loop;
	

	sprintf (filepath, "../media/%s", filename);
	alutLoadWAVFile(filepath, &format, &data, &size, &freq, &loop);
	if (alGetError() != AL_NO_ERROR)
	{
		sprintf (errmsg, "Error: Failed to Load WAV (%s)", filename);
		MessageBox (NULL, errmsg, NULL, MB_OK);				

		return (false);
	}

	alBufferData(SoundBuffer[ buffer ], format, data, size, freq);
	if (alGetError() != AL_NO_ERROR)
	{		
		sprintf (errmsg, "Error: Failed to Copy Buffer Data %i (%s)", buffer, filename);
		MessageBox (NULL, errmsg, NULL, MB_OK);		

		return (false);
	}
	
	alutUnloadWAV(format, data, size, freq);
	if (alGetError() != AL_NO_ERROR)
	{		
		sprintf (errmsg, "Error: Failed to Unload WAV Data (%s)", filename);
		MessageBox (NULL, errmsg, NULL, MB_OK);	

		return (false);
	}
	return (true);
}


/* SetSource *****************************************************************************************************/
bool AUDIO::SetSource(long source, char *filename, float x, float y, float z, float pitch, float gain, bool loop)
{
	ALfloat position[] = {x, y, z};
	char	errmsg[50];
	long	buffer;

	buffer = GetBufferID(filename);
	if (buffer < 0) 
	{
		sprintf (errmsg, "Error: Failed to Find (%s)", filename);
		MessageBox (NULL, errmsg, NULL, MB_OK);
		return (false);
	}

	alSourcei(SoundSource[ source ], AL_BUFFER, SoundBuffer[ buffer ]);
	alSourcef(SoundSource[ source ], AL_PITCH, pitch);
	alSourcef(SoundSource[ source ], AL_GAIN, gain);
	alSourcefv(SoundSource[ source ], AL_POSITION, position);		
	alSourcei(SoundSource[ source ], AL_LOOPING, loop);
	
	return (true);
}


/* CreateSources *************************************************************************************************/
void AUDIO::CreateSources(long sources)
{
	if (max_sources > 0) ReleaseSources();

	max_sources = sources;
	SoundSource	= new ALuint[ max_sources+1 ];

	alGenSources (max_sources, SoundSource);
}


/* ReleaseSources ************************************************************************************************/
void AUDIO::ReleaseSources()
{
	if (max_sources == 0) return;
	
	alDeleteSources(max_sources, SoundSource);

	delete [] SoundSource;
	SoundSource = NULL;
	
	max_sources = 0;	
}


/* PlaySource ****************************************************************************************************/
void AUDIO::PlaySource(long source)
{
	if (source > max_sources) return;

	alSourcePlay(SoundSource[ source ]);	
	if (alGetError() != AL_NO_ERROR)
	{
		char errmsg[50];
		sprintf (errmsg, "Error: Failed to Play Source %i", source);
		MessageBox (NULL, errmsg, NULL, MB_OK);
	} 	
}


/* StopSource ****************************************************************************************************/
void AUDIO::StopSource(long source)
{	
	if (source > max_sources) return;

	alSourceStop(SoundSource[  source ]);
	if (alGetError() != AL_NO_ERROR)
	{
		char errmsg[50];
		sprintf (errmsg, "Error: Failed to Stop Source %i", source);
		MessageBox (NULL, errmsg, NULL, MB_OK);
	}
}


/* SetListerner **************************************************************************************************/
void AUDIO::SetListener(float x, float y, float z, float player_angle)
{
	ALfloat rad				= (3.141592654f / 180.0f) * player_angle;
	ALfloat position[]		= {x, y, z};
	ALfloat orientation[]	= {(float)sin(rad), 0, (float)-cos(rad), 0,0,0};

	alGetError();
	alcGetError(alDevice);

	alListenerfv(AL_POSITION, position);
	if (alGetError() != AL_NO_ERROR)
	{
		MessageBox (NULL, "Failed to Set the Listener Position", NULL, MB_OK);
		return;
	}

	alListenerfv(AL_ORIENTATION, orientation);
	if (alGetError() != AL_NO_ERROR)
	{
		MessageBox (NULL, "Failed to Set the Listener Orientation", NULL, MB_OK);
		return;
	}
}