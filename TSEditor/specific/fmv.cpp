#include "../tomb5/pch.h"
#include "fmv.h"
#include "dxshell.h"
#include "audio.h"
#include "function_stubs.h"
#include "lighting.h"
#include "function_table.h"
#include "winmain.h"
#include "input.h"
#include "3dmath.h"
#include "../game/text.h"
#include "d3dmatrix.h"
#include "dxsound.h"
#include "gamemain.h"
#include "LoadSave.h"
#include "setupdlg.h"
#include "window.h"
#define PL_MPEG_IMPLEMENTATION
#include "../tomb5/pl_mpeg.h"
#include "output.h"

static SDL_AudioDeviceID m_audioDevice = NULL;
static double m_lastTime = 0.0;
static double m_frameRate = 30.0; // Default value for framerate.
static plm_t* m_file = NULL;
static uint8_t* m_rgb_old = NULL;
static const char* m_fileName = NULL;

static void FMV_VideoDecodeCallback(plm_t* self, plm_frame_t* frame, void* user)
{
	LPDIRECTDRAWSURFACE4 pBackBuffer = G_dxptr->lpBackBuffer;
	DDSURFACEDESC2 surfDesc = {};
	surfDesc.dwSize = sizeof(DDSURFACEDESC2);
	if (DXAttempt(pBackBuffer->Lock(0, &surfDesc, DDLOCK_NOSYSLOCK, NULL)))
		return;
	
	// If it's the same size, just copy it.
	// Else resample the texture to the correct size.
	plm_frame_to_bgra(frame, m_rgb_old, frame->width * 4);
	double scaleWidth = (double)G_dxptr->dwRenderWidth / (double)frame->width;
	double scaleHeight = (double)G_dxptr->dwRenderHeight / (double)frame->height;
	uint8_t* pSurf = (uint8_t*)surfDesc.lpSurface;
	if (pSurf == NULL)
	{
		DXAttempt(pBackBuffer->Unlock(0));
		return;
	}

	for (ulong cy = 0; cy < G_dxptr->dwRenderHeight; cy++)
	{
		for (ulong cx = 0; cx < G_dxptr->dwRenderWidth; cx++)
		{
			ulong pixel = (cy * (G_dxptr->dwRenderWidth * 4)) + (cx * 4);
			ulong nearestMatch = (((int)(cy / scaleHeight) * (frame->width * 4)) + ((int)(cx / scaleWidth) * 4));
			pSurf[pixel + 0] = m_rgb_old[nearestMatch + 0];
			pSurf[pixel + 1] = m_rgb_old[nearestMatch + 1];
			pSurf[pixel + 2] = m_rgb_old[nearestMatch + 2];
			pSurf[pixel + 3] = m_rgb_old[nearestMatch + 3];
		}
	}
	
	DXAttempt(pBackBuffer->Unlock(0));
	DXShowFrame();
}

static void FMV_AudioDecodeCallback(plm_t* self, plm_samples_t* samples, void* user)
{
	int size = sizeof(float) * samples->count * 2;
	SDL_QueueAudio(m_audioDevice, samples->interleaved, size);
}

long PlayFmv(long num)
{
	char name[80], path[80];

	if (fmvs_disabled || !g_Window.IsOpened())
		return 0;

	S_CDStop();
	sprintf(name, "movie\\fmv%01d.mpg", num);
	memset(path, 0, sizeof(path));
	strcat(path, name);
	Log("PlayFMV %s", path);

	App.fmv = TRUE;
	m_file = plm_create_with_filename(path);
	if (m_file == NULL)
	{
		Log("Failed to play fmv at path: <%s>, file possibly not exist ?", path);
		return 0;
	}

	int width = plm_get_width(m_file);
	int height = plm_get_height(m_file);
	int samplerate = plm_get_samplerate(m_file);
	m_rgb_old = (uint8_t*)malloc(width * height * 4);
	m_frameRate = plm_get_framerate(m_file);
	m_fileName = path;

	plm_set_loop(m_file, FALSE);
	plm_set_audio_enabled(m_file, TRUE);
	plm_set_audio_stream(m_file, 0);

	// Setup the audio player.
	if (plm_get_num_audio_streams(m_file) > 0) {
		SDL_AudioSpec audio_spec;
		SDL_memset(&audio_spec, 0, sizeof(audio_spec));
		audio_spec.freq = samplerate;
		audio_spec.format = AUDIO_F32;
		audio_spec.channels = 2;
		audio_spec.samples = 4096;
		m_audioDevice = SDL_OpenAudioDevice(NULL, FALSE, &audio_spec, NULL, NULL);
		if (m_audioDevice == NULL)
			Log("Failed to setup audio for FMV, Error: %s", SDL_GetError());
		SDL_PauseAudioDevice(m_audioDevice, FALSE);
		plm_set_audio_lead_time(m_file, (double)audio_spec.samples / (double)samplerate);
	}
	else
	{
		Log("Failed to setup audio for FMV: %s, No audio stream found !", path);
	}

	plm_set_video_decode_callback(m_file, FMV_VideoDecodeCallback, NULL);
	plm_set_audio_decode_callback(m_file, FMV_AudioDecodeCallback, NULL);

	S_UpdateInput();
	while (!plm_has_ended(m_file))
	{
		g_Window.Update();
		if (G_dxptr->WaitAtBeginScene)
			continue;
		if ((input & IN_OPTION) || (input & IN_DRAW) || !g_Window.IsOpened())
			break;
		double current_time = double(SDL_GetTicks()) / 1000.0;
		double elapsed_time = current_time - m_lastTime;
		if (elapsed_time > 1.0 / m_frameRate) {
			elapsed_time = 1.0 / m_frameRate;
		}
		m_lastTime = current_time;
		plm_decode(m_file, elapsed_time);
		S_UpdateInput();
	}

	SafeFree(m_rgb_old);

	if (m_audioDevice != NULL)
	{
		SDL_CloseAudioDevice(m_audioDevice);
		m_audioDevice = NULL;
	}

	if (m_file != NULL)
	{
		plm_destroy(m_file);
		m_file = NULL;
	}

	HWInitialise();
	ClearSurfaces();

	App.fmv = FALSE;
	return 0;
}

long PlayFmvNow(long num, long u)
{
	switch (num)
	{
	case 0:
		PlayFmv(0);
		break;

	case 1:
		PlayFmv(1);
		break;

	case 2:
		PlayFmv(2);
		break;

	case 3:
		PlayFmv(3);
		PlayFmv(4);
		break;

	case 4:
		PlayFmv(5);
		PlayFmv(6);
		break;

	case 5:
		PlayFmv(7);
		break;

	case 6:
		PlayFmv(8);
		break;
	}
	return 0;
}
