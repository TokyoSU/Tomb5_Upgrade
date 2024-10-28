#include "../tomb5/pch.h"
#include "sound.h"
#include "deltapak.h"
#include "camera.h"
#include "control.h"
#include "gameflow.h"
#include "lara.h"
#include "../specific/function_stubs.h"
#include "../specific/dxsound.h"
#include "../specific/3dmath.h"

SAMPLE_INFO* sample_infos = NULL;
SoundSlot LaSlot[MAX_SOUND_EFFECT];
short* sample_lut = NULL;
bool sound_active = false;
DWORD sound_cut_flag = NULL;

bool SayNo()
{
	// NOTE: Changed pos from NULL to &lara_item->pos and flags from ALWAYS to DEFAULT,
	// Only allow the sound NO to be on LAND and near LARA.
	return SoundEffect(SFX_LARA_NO, &LaraItem->pos, SFX_DEFAULT);
}

void SOUND_Init()
{
	for (int i = 0; i < _countof(LaSlot); i++)
	{
		SoundSlot& slotRef = LaSlot[i];
		slotRef.nSampleInfo = -1;
	}
}

void SOUND_Stop()
{
	if (!sound_active)
		return;
	S_SoundStopAllSamples();
	for (int i = 0; i < _countof(LaSlot); i++)
	{
		SoundSlot& slotRef = LaSlot[i];
		slotRef.nSampleInfo = -1;
	}
}

void StopSoundEffect(long sfx)
{
	if (!sound_active)
		return;
	long lut = sample_lut[sfx];
	for (int i = 0; i < _countof(LaSlot); i++)
	{
		SoundSlot& slotRef = LaSlot[i];
		if (slotRef.nSampleInfo >= lut && slotRef.nSampleInfo < (lut + ((sample_infos[lut].flags >> 2) & 0xF)))
		{
			S_SoundStopSample(i);
			slotRef.nSampleInfo = -1;
		}
	}
}

void GetPanVolume(SoundSlot* slot)
{
	long dx, dy, dz, radius, distance, nPan, nVolume;

	if (slot->distance || slot->pos.x || slot->pos.y || slot->pos.z)
	{
		dx = slot->pos.x - camera.pos.x;
		dy = slot->pos.y - camera.pos.y;
		dz = slot->pos.z - camera.pos.z;
		radius = sample_infos[slot->nSampleInfo].radius << 10;

		if (dx < -radius || dx > radius || dy < -radius || dy > radius || dz < -radius || dz > radius)
		{
			slot->distance = 0;
			slot->nPan = 0;
			slot->nVolume = 0;
		}
		else
		{
			distance = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);
			if (distance <= SQUARE(radius))
			{
				if (distance >= 0x100000)
					distance = phd_sqrt(distance) - 1024;
				else
					distance = 0;

				nPan = (CamRot.y << 4) + phd_atan(dz, dx);
				nVolume = slot->OrigVolume;

				if (distance)
					nVolume = (nVolume * (4096 - (phd_sin((distance << 14) / radius) >> 2))) >> 12;

				if (nVolume > 0)
				{
					if (nVolume > 0x7FFF)
						nVolume = 0x7FFF;

					slot->nVolume = nVolume;
					slot->nPan = nPan;
					slot->distance = distance;
				}
				else
				{
					slot->distance = 0;
					slot->nPan = 0;
					slot->nVolume = 0;
				}
			}
			else
			{
				slot->distance = 0;
				slot->nPan = 0;
				slot->nVolume = 0;
			}
		}
	}
}

bool SoundEffect(long sfx, PHD_3DPOS* pos, long flags)
{
	if (!sound_active)
		return true;

	if (GLOBAL_playing_cutseq != 0 && sound_cut_flag == 0)
		return false;

	if (sfx == SFX_LARA_NO)
	{
		if (Gameflow->Language == 1)
			sfx = SFX_LARA_NO_FRENCH;
		else if (Gameflow->Language == 6)
			sfx = SFX_LARA_NO_JAPANESE;
	}

	if ((flags & SFX_WATER) != (room[camera.pos.room_number].flags & ROOM_UNDERWATER))
		return false;

	auto lut = sample_lut[sfx];
	if (lut == SFX_LUT_FILE_NOT_PRESENT)
		return false;

	if (lut == SFX_LUT_FILE_NOT_FOUND)
	{
		Log("Non present sample: %d", sfx);
		sample_lut[sfx] = SFX_LUT_FILE_NOT_PRESENT;
		return false;
	}

	auto* info = &sample_infos[lut];
	if (info->number < 0)
		return false;

	if (info->randomness)
	{
		if ((GetRandomDraw() & 0xFF) > info->randomness)
			return false;
	}

	PHD_3DPOS sndPos = {};
	if (pos != NULL)
		sndPos = *pos;
	auto radius = (info->radius + 1) << 10;
	auto distance = 0;
	short pan = 0;

	if (pos)
	{
		auto dx = pos->x_pos - camera.pos.x;
		auto dy = pos->y_pos - camera.pos.y;
		auto dz = pos->z_pos - camera.pos.z;
		if (dx < -radius || dx > radius ||
			dy < -radius || dy > radius ||
			dz < -radius || dz > radius)
			return false;

		distance = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);
		if (distance > SQUARE(radius))
			return 0;

		if (distance >= 0x100000)
			distance = phd_sqrt(distance) - 1024;
		else
			distance = 0;

		if (!(info->flags & 0x1000))
			pan = (CamRot.y << 4) + (short)phd_atan(dz, dx);
	}
	else
	{
		sndPos.x_pos = 0;
		sndPos.y_pos = 0;
		sndPos.z_pos = 0;
	}

	auto volume = info->volume << 6;
	if (flags & SFX_SETVOL)
		volume = (volume * ((flags >> 8) & 0x1F)) >> 5;
	if (info->flags & 0x4000)
		volume -= GetRandomDraw() << 12 >> 15;

	auto oldVolume = volume;
	if (distance)
		volume = (volume * (4096 - (phd_sin((distance << W2V_SHIFT) / radius) >> 2))) >> 12;
	if (volume <= 0)
		return false;
	else if (volume > 0x7FFF)
		volume = 0x7FFF;

	auto pitch = 0;
	if (flags & SFX_SETPITCH)
		pitch = (flags >> 8) & 0xFFFF00;
	else
		pitch = 0x10000;
	pitch += info->pitch << 9;
	if (info->flags & 0x2000)
		pitch += ((6000 * GetRandomDraw()) >> 14) - 6000;

	auto rnd = (info->flags >> 2) & 0xF;
	auto sample = 0;

	if (rnd == 1)
		sample = info->number;
	else
		sample = info->number + ((rnd * GetRandomDraw()) >> 15);

	auto flag = info->flags & 3;
	switch (flag)
	{
	case 1:
		for (int i = 0; i < _countof(LaSlot); i++)
		{
			SoundSlot& slotRef = LaSlot[i];
			if (slotRef.nSampleInfo == lut)
			{
				if (S_SoundSampleIsPlaying(i))
					return false;
				slotRef.nSampleInfo = -1;
			}
		}

		break;

	case 2:
		for (int i = 0; i < _countof(LaSlot); i++)
		{
			SoundSlot& slotRef = LaSlot[i];
			if (slotRef.nSampleInfo == lut)
			{
				S_SoundStopSample(i);
				slotRef.nSampleInfo = -1;
				break;
			}
		}

		break;

	case 3:
		for (int i = 0; i < _countof(LaSlot); i++)
		{
			SoundSlot& slotRef = LaSlot[i];
			if (slotRef.nSampleInfo == lut)
			{
				if (volume <= slotRef.nVolume)
					return false;
				slotRef.OrigVolume = oldVolume;
				slotRef.nVolume = volume;
				slotRef.nPan = pan;
				slotRef.nPitch = pitch;
				slotRef.distance = distance;
				slotRef.pos.x = sndPos.x_pos;
				slotRef.pos.y = sndPos.y_pos;
				slotRef.pos.z = sndPos.z_pos;
				return true;
			}
		}

		break;
	}

	auto sampleIndex = (flag == 3) ? S_SoundPlaySampleLooped(sample, volume, pitch, pan) : S_SoundPlaySample(sample, volume, pitch, pan);
	if (sampleIndex >= 0)
	{
		SoundSlot& slotRef = LaSlot[sampleIndex];
		slotRef.OrigVolume = oldVolume;
		slotRef.nVolume = volume;
		slotRef.nPan = pan;
		slotRef.nPitch = pitch;
		slotRef.nSampleInfo = lut;
		slotRef.distance = distance;
		slotRef.pos.x = sndPos.x_pos;
		slotRef.pos.y = sndPos.y_pos;
		slotRef.pos.z = sndPos.z_pos;
		return true;
	}

	if (sampleIndex == SFX_LUT_FILE_NOT_FOUND)
	{
		auto vol = 0x8000000;
		auto slot = -1;

		for (int i = 0; i < _countof(LaSlot); i++)
		{
			SoundSlot& slotRef = LaSlot[i];
			if ((slotRef.nSampleInfo >= 0) && (slotRef.nVolume <= vol))
			{
				vol = slotRef.nVolume;
				slot = i;
			}
		}

		if (volume > vol)
		{
			S_SoundStopSample(slot);

			SoundSlot& slotRef = LaSlot[slot];
			slotRef.nSampleInfo = -1;

			sampleIndex = (flag == 3) ? S_SoundPlaySampleLooped(sample, (ushort)volume, pitch, (short)pan) : S_SoundPlaySample(sample, (ushort)volume, pitch, (short)pan);
			if (sampleIndex >= 0)
			{
				slotRef = LaSlot[sampleIndex];
				slotRef.OrigVolume = oldVolume;
				slotRef.nVolume = volume;
				slotRef.nPan = pan;
				slotRef.nPitch = pitch;
				slotRef.nSampleInfo = lut;
				slotRef.distance = distance;
				slotRef.pos.x = sndPos.x_pos;
				slotRef.pos.y = sndPos.y_pos;
				slotRef.pos.z = sndPos.z_pos;
				return true;
			}
		}
		return false;
	}

	if (sample >= 0)
		Log("Can't play SFX %d", sfx);

	info->number = -1;
	return false;
}

bool SoundEffectCS(long sfx, PHD_3DPOS* pos, long flags)
{
	if (!sound_active)
		return true;
	auto bak = GLOBAL_playing_cutseq;
	GLOBAL_playing_cutseq = 0;
	auto result = SoundEffect(sfx, pos, flags);
	GLOBAL_playing_cutseq = bak;
	return result;
}
