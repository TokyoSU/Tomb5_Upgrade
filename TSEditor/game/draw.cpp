#include "../tomb5/pch.h"
#include "draw.h"
#include "../specific/3dmath.h"
#include "../specific/output.h"
#include "delstuff.h"
#include "deltapak.h"
#include "../specific/drawlara.h"
#include "health.h"
#include "objects.h"
#include "gameflow.h"
#include "sound.h"
#include "../specific/specificfx.h"
#include "../specific/drawroom.h"
#include "../specific/polyinsert.h"
#include "../specific/function_stubs.h"
#include "../specific/lighting.h"
#include "items.h"
#include "../specific/file.h"
#include "../specific/polyinsert.h"
#include "door.h"
#include "tomb4fx.h"
#include "../specific/d3dmatrix.h"
#include "rope.h"
#include "rat.h"
#include "bat.h"
#include "spider.h"
#include "twogun.h"
#include "mirror.h"
#include "../specific/alexstuff.h"
#include "lara_states.h"
#include "control.h"
#include "camera.h"
#include "spotcam.h"
#include "effect2.h"
#include "lara.h"
#include "effects.h"
#include "footprnt.h"
#include "../tomb5/tomb5.h"

STATIC_INFO static_objects[70];

FMatrix* aIMXPtr;
FMatrix aIFMStack[40];
Matrix* IMptr;
Matrix IMstack[40];
long IM_rate;
long IM_frac;

long CurrentRoom;
long outside;
short SkyPos;
short SkyPos2;
ushort LightningRGB[3];
ushort LightningRGBs[3];
static short LightningCount;
static short LightningRand;
static short dLightningRand;
static short LightningSFXDelay = 0;
short no_rotation[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static long outside_top;
static long outside_left;
static long outside_right;
static long outside_bottom;

static short* skelly_rhandbak;
static short* skelly_lhandbak;
static short skelly_backgunbak;

static long number_draw_rooms;
static short draw_rooms[100];
static long room_list_start = 0;
static long room_list_end = 0;
static long draw_room_list[128];
static long camera_underwater;
static short ClipRoomNum;

short* GetBoundsAccurate(ITEM_INFO* item)
{
	short* frmptr[2];
	long rate, frac;
	static short interpolated_bounds[6];

	frac = GetFrames(item, frmptr, &rate);

	if (!frac)
		return frmptr[0];

	for (int i = 0; i < 6; i++, frmptr[0]++, frmptr[1]++)
		interpolated_bounds[i] = short(*frmptr[0] + (*frmptr[1] - *frmptr[0]) * frac / rate);

	return interpolated_bounds;
}

short* GetBestFrame(ITEM_INFO* item)
{
	short* frm[2];
	long frac, rate;
	
	frac = GetFrames(item, frm, &rate);

	if (frac > (rate >> 1))
		return frm[1];
	else
		return frm[0];
}

void InitInterpolate(long frac, long rate)
{
	IM_rate = rate;
	IM_frac = frac;
	IMptr = IMstack;
	aIMXPtr = aIFMStack;
	memcpy(IMstack, phd_mxptr, 48);
	memcpy(aIFMStack, aMXPtr, 48);
}

void phd_PopMatrix_I()
{
	phd_PopMatrix();
	IMptr -= 12;
	aIMXPtr -= 12;
}

void phd_PushMatrix_I()
{
	phd_PushMatrix();
	memcpy(IMptr + 12, IMptr, 48);
	memcpy(aIMXPtr + 12, aIMXPtr, 48);
	IMptr += 12;
	aIMXPtr += 12;
}

void phd_RotY_I(short ang)
{
	FMatrix* amxptr;
	Matrix* mxptr;

	phd_RotY(ang);
	mxptr = phd_mxptr;
	amxptr = aMXPtr;
	phd_mxptr = IMptr;
	aMXPtr = aIMXPtr;
	phd_RotY(ang);
	phd_mxptr = mxptr;
	aMXPtr = amxptr;
}

void phd_RotX_I(short ang)
{
	FMatrix* amxptr;
	Matrix* mxptr;

	phd_RotX(ang);
	mxptr = phd_mxptr;
	amxptr = aMXPtr;
	phd_mxptr = IMptr;
	aMXPtr = aIMXPtr;
	phd_RotX(ang);
	phd_mxptr = mxptr;
	aMXPtr = amxptr;
}

void phd_RotZ_I(short ang)
{
	FMatrix* amxptr;
	Matrix* mxptr;

	phd_RotZ(ang);
	mxptr = phd_mxptr;
	amxptr = aMXPtr;
	phd_mxptr = IMptr;
	aMXPtr = aIMXPtr;
	phd_RotZ(ang);
	phd_mxptr = mxptr;
	aMXPtr = amxptr;
}

void phd_TranslateRel_I(long x, long y, long z)
{
	FMatrix* amxptr;
	Matrix* mxptr;

	phd_TranslateRel(x, y, z);
	mxptr = phd_mxptr;
	phd_mxptr = IMptr;
	amxptr = aMXPtr;
	aMXPtr = aIMXPtr;
	phd_TranslateRel(x, y, z);
	phd_mxptr = mxptr;
	aMXPtr = amxptr;
}

void phd_TranslateRel_ID(long x, long y, long z, long x2, long y2, long z2)
{
	FMatrix* amxptr;
	Matrix* mxptr;

	phd_TranslateRel(x, y, z);
	mxptr = phd_mxptr;
	amxptr = aMXPtr;
	phd_mxptr = IMptr;
	aMXPtr = aIMXPtr;
	phd_TranslateRel(x2, y2, z2);
	aMXPtr = amxptr;
	phd_mxptr = mxptr;
}

void phd_RotYXZ_I(short y, short x, short z)
{
	FMatrix* amxptr;
	Matrix* mxptr;

	phd_RotYXZ(y, x, z);
	mxptr = phd_mxptr;
	phd_mxptr = IMptr;
	amxptr = aMXPtr;
	aMXPtr = aIMXPtr;
	phd_RotYXZ(y, x, z);
	phd_mxptr = mxptr;
	aMXPtr = amxptr;
}

void gar_RotYXZsuperpack_I(short** pprot1, short** pprot2, long skip)
{
	FMatrix* amxptr;
	Matrix* mxptr;

	gar_RotYXZsuperpack(pprot1, skip);
	mxptr = phd_mxptr;
	amxptr = aMXPtr;
	phd_mxptr = IMptr;
	aMXPtr = aIMXPtr;
	gar_RotYXZsuperpack(pprot2, skip);
	phd_mxptr = mxptr;
	aMXPtr = amxptr;
}

void gar_RotYXZsuperpack(short** pprot, long skip)
{
	ushort* prot;
	long packed;

	while (skip)
	{
		prot = (ushort*)*pprot;

		if (*prot & 0xC000)
			(*pprot)++;
		else
			*pprot += 2;

		skip--;
	}

	prot = (ushort*)*pprot;

	switch (*prot >> 14)
	{
	case 0:
		packed = (*prot << 16) + *(prot + 1);
		phd_RotYXZpack(packed);
		(*pprot)++;
		break;

	case 1:
		phd_RotX(short((*prot & 0xFFF) << 4));
		break;

	case 2:
		phd_RotY(short((*prot & 0xFFF) << 4));
		break;

	default:
		phd_RotZ(short((*prot & 0xFFF) << 4));
		break;
	}

	(*pprot)++;
}

void phd_PutPolygons_I(short* ptr, long clip)
{
	phd_PushMatrix();
	InterpolateMatrix();
	phd_PutPolygons(ptr, clip);
	phd_PopMatrix();
}

void aInterpolateMatrix()
{
	FMatrix* matrixp;
	FMatrix* iMatrixp;

	matrixp = aMXPtr;
	iMatrixp = aIMXPtr;

	if (IM_rate == 2)
	{
		matrixp->m00 = (iMatrixp->m00 + matrixp->m00) * 0.5F;
		matrixp->m01 = (iMatrixp->m01 + matrixp->m01) * 0.5F;
		matrixp->m02 = (iMatrixp->m02 + matrixp->m02) * 0.5F;
		matrixp->m03 = (iMatrixp->m03 + matrixp->m03) * 0.5F;
		matrixp->m10 = (iMatrixp->m10 + matrixp->m10) * 0.5F;
		matrixp->m11 = (iMatrixp->m11 + matrixp->m11) * 0.5F;
		matrixp->m12 = (iMatrixp->m12 + matrixp->m12) * 0.5F;
		matrixp->m13 = (iMatrixp->m13 + matrixp->m13) * 0.5F;
		matrixp->m20 = (iMatrixp->m20 + matrixp->m20) * 0.5F;
		matrixp->m21 = (iMatrixp->m21 + matrixp->m21) * 0.5F;
		matrixp->m22 = (iMatrixp->m22 + matrixp->m22) * 0.5F;
		matrixp->m23 = (iMatrixp->m23 + matrixp->m23) * 0.5F;
		return;
	}

	if (IM_frac == 2)
	{
		if (IM_rate != 4)
		{
			matrixp->m00 = iMatrixp->m00 - (iMatrixp->m00 - matrixp->m00) * 0.25F;
			matrixp->m01 = iMatrixp->m01 - (iMatrixp->m01 - matrixp->m01) * 0.25F;
			matrixp->m02 = iMatrixp->m02 - (iMatrixp->m02 - matrixp->m02) * 0.25F;
			matrixp->m03 = iMatrixp->m03 - (iMatrixp->m03 - matrixp->m03) * 0.25F;
			matrixp->m10 = iMatrixp->m10 - (iMatrixp->m10 - matrixp->m10) * 0.25F;
			matrixp->m11 = iMatrixp->m11 - (iMatrixp->m11 - matrixp->m11) * 0.25F;
			matrixp->m12 = iMatrixp->m12 - (iMatrixp->m12 - matrixp->m12) * 0.25F;
			matrixp->m13 = iMatrixp->m13 - (iMatrixp->m13 - matrixp->m13) * 0.25F;
			matrixp->m20 = iMatrixp->m20 - (iMatrixp->m20 - matrixp->m20) * 0.25F;
			matrixp->m21 = iMatrixp->m21 - (iMatrixp->m21 - matrixp->m21) * 0.25F;
			matrixp->m22 = iMatrixp->m22 - (iMatrixp->m22 - matrixp->m22) * 0.25F;
			matrixp->m23 = iMatrixp->m23 - (iMatrixp->m23 - matrixp->m23) * 0.25F;
			return;
		}

		matrixp->m00 = (iMatrixp->m00 + matrixp->m00) * 0.5F;
		matrixp->m01 = (iMatrixp->m01 + matrixp->m01) * 0.5F;
		matrixp->m02 = (iMatrixp->m02 + matrixp->m02) * 0.5F;
		matrixp->m03 = (iMatrixp->m03 + matrixp->m03) * 0.5F;
		matrixp->m10 = (iMatrixp->m10 + matrixp->m10) * 0.5F;
		matrixp->m11 = (iMatrixp->m11 + matrixp->m11) * 0.5F;
		matrixp->m12 = (iMatrixp->m12 + matrixp->m12) * 0.5F;
		matrixp->m13 = (iMatrixp->m13 + matrixp->m13) * 0.5F;
		matrixp->m20 = (iMatrixp->m20 + matrixp->m20) * 0.5F;
		matrixp->m21 = (iMatrixp->m21 + matrixp->m21) * 0.5F;
		matrixp->m22 = (iMatrixp->m22 + matrixp->m22) * 0.5F;
		matrixp->m23 = (iMatrixp->m23 + matrixp->m23) * 0.5F;
		return;
	}

	if (IM_frac != 1)
	{
		matrixp->m00 = iMatrixp->m00 - (iMatrixp->m00 - matrixp->m00) * 0.25F;
		matrixp->m01 = iMatrixp->m01 - (iMatrixp->m01 - matrixp->m01) * 0.25F;
		matrixp->m02 = iMatrixp->m02 - (iMatrixp->m02 - matrixp->m02) * 0.25F;
		matrixp->m03 = iMatrixp->m03 - (iMatrixp->m03 - matrixp->m03) * 0.25F;
		matrixp->m10 = iMatrixp->m10 - (iMatrixp->m10 - matrixp->m10) * 0.25F;
		matrixp->m11 = iMatrixp->m11 - (iMatrixp->m11 - matrixp->m11) * 0.25F;
		matrixp->m12 = iMatrixp->m12 - (iMatrixp->m12 - matrixp->m12) * 0.25F;
		matrixp->m13 = iMatrixp->m13 - (iMatrixp->m13 - matrixp->m13) * 0.25F;
		matrixp->m20 = iMatrixp->m20 - (iMatrixp->m20 - matrixp->m20) * 0.25F;
		matrixp->m21 = iMatrixp->m21 - (iMatrixp->m21 - matrixp->m21) * 0.25F;
		matrixp->m22 = iMatrixp->m22 - (iMatrixp->m22 - matrixp->m22) * 0.25F;
		matrixp->m23 = iMatrixp->m23 - (iMatrixp->m23 - matrixp->m23) * 0.25F;
		return;
	}

	matrixp->m00 += (iMatrixp->m00 - matrixp->m00) * 0.25F;
	matrixp->m01 += (iMatrixp->m01 - matrixp->m01) * 0.25F;
	matrixp->m02 += (iMatrixp->m02 - matrixp->m02) * 0.25F;
	matrixp->m03 += (iMatrixp->m03 - matrixp->m03) * 0.25F;
	matrixp->m10 += (iMatrixp->m10 - matrixp->m10) * 0.25F;
	matrixp->m11 += (iMatrixp->m11 - matrixp->m11) * 0.25F;
	matrixp->m12 += (iMatrixp->m12 - matrixp->m12) * 0.25F;
	matrixp->m13 += (iMatrixp->m13 - matrixp->m13) * 0.25F;
	matrixp->m20 += (iMatrixp->m20 - matrixp->m20) * 0.25F;
	matrixp->m21 += (iMatrixp->m21 - matrixp->m21) * 0.25F;
	matrixp->m22 += (iMatrixp->m22 - matrixp->m22) * 0.25F;
	matrixp->m23 += (iMatrixp->m23 - matrixp->m23) * 0.25F;
}

long DrawPhaseGame()
{
	CalcLaraMatrices(0);
	phd_PushUnitMatrix();
	CalcLaraMatrices(1);
	phd_PopMatrix();

	if (GLOBAL_playing_cutseq)
		frigup_lara();

	SetLaraUnderwaterNodes();
	DrawRooms(camera.pos.room_number);
	DrawGameInfo(1);
	S_OutputPolyList();
	camera.number_frames = S_DumpScreen();
	S_AnimateTextures(camera.number_frames);
	return camera.number_frames;
}

void SkyDrawPhase()
{
	if (outside)
	{
		if (!objects[HORIZON].loaded)
		{
			outside = -1;
			return;
		}

		if (gfCurrentLevel == LVL5_GALLOWS_TREE || gfCurrentLevel == LVL5_LABYRINTH || gfCurrentLevel == LVL5_OLD_MILL)
			DrawMoon();

		if (BinocularRange)
			AlterFOV(short(GAME_FOV - BinocularRange));

		phd_PushMatrix();
		phd_TranslateAbs(camera.pos.x, camera.pos.y, camera.pos.z);

		if (gfLevelFlags & GF_LIGHTNING)
		{
			if (!LightningCount && !LightningRand)
			{
				if (!(GetRandomDraw() & 127))
				{
					LightningCount = (GetRandomDraw() & 0x1F) + 16;
					dLightningRand = (GetRandomDraw() & 0xFF) + 256;
					LightningSFXDelay = (GetRandomDraw() & 3) + 12;
				}
			}
			else
			{
				UpdateSkyLightning();

				if (LightningSFXDelay > -1)
					LightningSFXDelay--;

				if (!LightningSFXDelay)
					SoundEffect(SFX_THUNDER_RUMBLE, 0, SFX_DEFAULT);
			}
		}

		nPolyType = 6;
		DrawBuckets();
		DrawSortList();
		phd_PushMatrix();

		if (gfLevelFlags & GF_LAYER1)
		{
			phd_RotY(32760);

			if (gfLevelFlags & GF_LIGHTNING)
				DrawFlatSky(RGBA(LightningRGB[0], LightningRGB[1], LightningRGB[2], 44), SkyPos, -1536, 4);
			else
				DrawFlatSky(*(ulong*)&gfLayer1Col, SkyPos, -1536, 4);
		}

		if (gfLevelFlags & GF_LAYER2)
			DrawFlatSky(0xFF000000 | *(ulong*)&gfLayer2Col, SkyPos2, -1536, 2);

		if (gfLevelFlags & GF_LAYER1 || gfLevelFlags & GF_LAYER2)
			OutputSky();

		phd_PopMatrix();

		if (gfLevelFlags & GF_HORIZON)
		{
			if (gfCurrentLevel == LVL5_TITLE && jobyfrigger)
				phd_PutPolygonsSkyMesh(meshes[objects[CHEF_MIP].mesh_index], -1);
			else
				phd_PutPolygonsSkyMesh(meshes[objects[HORIZON].mesh_index], -1);

			OutputSky();
		}

		if (gfCurrentLevel == LVL5_GALLOWS_TREE || gfCurrentLevel == LVL5_LABYRINTH || gfCurrentLevel == LVL5_OLD_MILL)
		{
			DrawStars();
			OutputSky();
		}

		phd_PopMatrix();

		if (BinocularRange)
			AlterFOV(short(GAME_FOV -  (7 * BinocularRange)));
	}
}

void UpdateSkyLightning()
{
	if (LightningCount <= 0)
	{
		if (LightningRand < 4)
			LightningRand = 0;
		else
			LightningRand -= LightningRand >> 2;
	}
	else
	{
		LightningCount--;

		if (LightningCount)
		{
			dLightningRand = GetRandomDraw() & 0x1FF;
			LightningRand += (dLightningRand - LightningRand) >> 1;
		}
		else
		{
			dLightningRand = 0;
			LightningRand = (GetRandomDraw() & 0x7F) + 400;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		LightningRGB[i] = LightningRGBs[i] + ((LightningRGBs[i] * LightningRand) >> 8);

		if (LightningRGB[i] > 255)
			LightningRGB[i] = 255;
	}
}

void CalculateObjectLighting(ITEM_INFO* item, short* frame)
{
	if (item->shade >= 0)
	{
		S_CalculateStaticMeshLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->shade & 0x7FFF, &room[item->room_number]);
	}
	else
	{
		phd_PushUnitMatrix();
		phd_SetTrans(0, 0, 0);
		phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
		phd_TranslateRel((frame[0] + frame[1]) >> 1, (frame[2] + frame[3]) >> 1, (frame[4] + frame[5]) >> 1);
		long x = item->pos.x_pos + (phd_mxptr->m03 >> W2V_SHIFT);
		long y = item->pos.y_pos + (phd_mxptr->m13 >> W2V_SHIFT);
		long z = item->pos.z_pos + (phd_mxptr->m23 >> W2V_SHIFT);
		phd_PopMatrix();
		current_item = item;
		item->il.item_pos.x = x;
		item->il.item_pos.y = y;
		item->il.item_pos.z = z;
		CalcAmbientLight(item);
		CreateLightList(item);
	}
}

void DrawAnimatingItem(ITEM_INFO* item)
{
	OBJECT_INFO* obj;
	short** meshpp;
	long* bone;
	short* frm[2];
	short* data;
	short* rot;
	short* rot2;
	long frac, rate, clip, bit;

	frac = GetFrames(item, frm, &rate);
	obj = &objects[item->object_number];

	if (obj->shadow_size)
		S_PrintShadow(obj->shadow_size, frm[0], item);

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

	if (obj->object_mip && (obj + 1)->loaded && phd_mxptr->m23 >> 16 > obj->object_mip)
		obj++;

	calc_animating_item_clip_window(item, frm[0]);
	clip = S_GetObjectBounds(frm[0]);

	if (clip)
	{
		CalculateObjectLighting(item, frm[0]);

		if (!item->data)
			data = no_rotation;
		else
			data = (short*)item->data;

		bit = 1;
		meshpp = &meshes[obj->mesh_index];
		bone = &bones[obj->bone_index];

		if (frac)
		{
			InitInterpolate(frac, rate);
			phd_TranslateRel_ID(frm[0][6], frm[0][7], frm[0][8], frm[1][6], frm[1][7], frm[1][8]);
			rot = frm[0] + 9;
			rot2 = frm[1] + 9;
			gar_RotYXZsuperpack_I(&rot, &rot2, 0);

			if (item->mesh_bits & 1)
			{
				if (item->meshswap_meshbits & 1)
					phd_PutPolygons_I(meshpp[1], clip);
				else
					phd_PutPolygons_I(meshpp[0], clip);
			}

			meshpp += 2;

			for (int i = 0; i < obj->nmeshes - 1; i++, bone += 4, meshpp += 2)
			{
				if (bone[0] & 1)
					phd_PopMatrix_I();

				if (bone[0] & 2)
					phd_PushMatrix_I();

				phd_TranslateRel_I(bone[1], bone[2], bone[3]);
				gar_RotYXZsuperpack_I(&rot, &rot2, 0);

				if (bone[0] & 0x1C)
				{
					if (bone[0] & 8)
						phd_RotY_I(*data++);

					if (bone[0] & 4)
						phd_RotX_I(*data++);

					if (bone[0] & 0x10)
						phd_RotZ_I(*data++);
				}

				bit <<= 1;

				if (bit & item->mesh_bits)
				{
					if (bit & item->meshswap_meshbits)
						phd_PutPolygons_I(meshpp[1], clip);
					else
						phd_PutPolygons_I(meshpp[0], clip);
				}
			}
		}
		else
		{
			phd_TranslateRel(frm[0][6], frm[0][7], frm[0][8]);
			rot = frm[0] + 9;
			gar_RotYXZsuperpack(&rot, 0);

			if (item->mesh_bits & 1)
			{
				if (item->meshswap_meshbits & 1)
					phd_PutPolygons(meshpp[1], clip);
				else
					phd_PutPolygons(meshpp[0], clip);
			}

			meshpp += 2;

			for (int i = 0; i < obj->nmeshes - 1; i++, bone += 4, meshpp += 2)
			{
				if (bone[0] & 1)
					phd_PopMatrix();

				if (bone[0] & 2)
					phd_PushMatrix();

				phd_TranslateRel(bone[1], bone[2], bone[3]);
				gar_RotYXZsuperpack(&rot, 0);

				if (bone[0] & 0x1C)
				{
					if (bone[0] & 8)
						phd_RotY(*data++);

					if (bone[0] & 4)
						phd_RotX(*data++);

					if (bone[0] & 0x10)
						phd_RotZ(*data++);
				}

				bit <<= 1;

				if (bit & item->mesh_bits)
				{
					if (bit & item->meshswap_meshbits)
						phd_PutPolygons(meshpp[1], clip);
					else
						phd_PutPolygons(meshpp[0], clip);
				}
			}
		}
	}

	phd_right = phd_winwidth;
	phd_left = 0;
	phd_top = 0;
	phd_bottom = phd_winheight;
	phd_PopMatrix();
}

void PrintObjects(short room_number)
{
	ITEM_INFO* item;
	ROOM_INFO* r;
	OBJECT_INFO* obj;

	CurrentRoom = room_number;
	nPolyType = 1;
	r = &room[room_number];
	r->bound_active = 0;
	DrawStaticObjects(room_number);
	phd_PushMatrix();
	phd_TranslateAbs(r->x, r->y, r->z);
	phd_left = 0;
	phd_top = 0;
	phd_right = phd_winxmax + 1;
	phd_bottom = phd_winymax + 1;
	nPolyType = 2;

	for (int i = r->item_number; i != NO_ITEM; i = items[i].next_item)
	{
		ClipRoomNum = room_number;
		item = &items[i];
		obj = &objects[item->object_number];

		if (item->status != ITEM_INVISIBLE)
		{
			if (item->after_death)
				GlobalAlpha = 0xFE000000 * item->after_death;

		//	if (gfCurrentLevel == LVL5_BASE && item->object_number == BRIDGE_FLAT)
		//		don't draw

			if (gfCurrentLevel != LVL5_BASE || item->object_number != BRIDGE_FLAT)
			{
				if (obj->draw_routine)
					obj->draw_routine(item);

				if (obj->draw_routine_extra)
					obj->draw_routine_extra(item);
			}

			GlobalAlpha = 0xFF000000;
		}

		if (item->after_death < 128 && item->after_death > 0 && !(wibble & 3))
			item->after_death++;

		if (item->after_death == 128)
			KillItem(i);
	}

	nPolyType = 3;

	for (int i = r->fx_number; i != NO_ITEM; i = effects[i].next_fx)
		DrawEffect(i);

	phd_PopMatrix();
	r->left = phd_winxmax;
	r->top = phd_winymax;
	r->right = 0;
	r->bottom = 0;
}

void DrawRooms(short current_room)
{
	ROOM_INFO* r;
	long lx, ly, lz;
	short lr;

	DoMonitorScreen();
	CurrentRoom = current_room;
	r = &room[current_room];
	r->test_left = 0;
	r->test_top = 0;
	phd_left = 0;
	phd_top = 0;
	phd_right = phd_winxmax;
	phd_bottom = phd_winymax;
	r->test_right = phd_winxmax;
	r->test_bottom = phd_winymax;
	outside = r->flags & ROOM_OUTSIDE;
	snow_outside = 0;
	camera_underwater = r->flags & ROOM_UNDERWATER;
	r->bound_active = 2;
	draw_room_list[0] = current_room;
	room_list_start = 0;
	room_list_end = 1;
	number_draw_rooms = 0;

	if (outside)
	{
		outside_top = 0;
		outside_left = 0;
		outside_right = phd_winxmax;
		outside_bottom = phd_winymax;
	}
	else
	{
		outside_top = phd_winymax;
		outside_left = phd_winxmax;
		outside_right = 0;
		outside_bottom = 0;
	}

	GetRoomBounds();
	ProcessClosedDoors();

	if (gfCurrentLevel != LVL5_TITLE)
		SkyDrawPhase();

	if (objects[LARA].loaded && !(LaraItem->flags & IFL_INVISIBLE))
	{
		nPolyType = 4;

		if (LaraItem->mesh_bits && !SCNoDrawLara)
		{
			if (Lara.skelebob)
				SetupSkelebobMeshswaps();

			DrawLara(LaraItem, 0);

			if (Lara.skelebob)
				RestoreLaraMeshswaps();

			phd_PushMatrix();

			if (Lara.rightArm.flash_gun)
			{
				aMXPtr->m00 = lara_matrices[LMX_HAND_R * indices_count + M00];
				aMXPtr->m01 = lara_matrices[LMX_HAND_R * indices_count + M01];
				aMXPtr->m02 = lara_matrices[LMX_HAND_R * indices_count + M02];
				aMXPtr->m03 = lara_matrices[LMX_HAND_R * indices_count + M03];
				aMXPtr->m10 = lara_matrices[LMX_HAND_R * indices_count + M10];
				aMXPtr->m11 = lara_matrices[LMX_HAND_R * indices_count + M11];
				aMXPtr->m12 = lara_matrices[LMX_HAND_R * indices_count + M12];
				aMXPtr->m13 = lara_matrices[LMX_HAND_R * indices_count + M13];
				aMXPtr->m20 = lara_matrices[LMX_HAND_R * indices_count + M20];
				aMXPtr->m21 = lara_matrices[LMX_HAND_R * indices_count + M21];
				aMXPtr->m22 = lara_matrices[LMX_HAND_R * indices_count + M22];
				aMXPtr->m23 = lara_matrices[LMX_HAND_R * indices_count + M23];
				SetGunFlash(Lara.gunType);
			}

			if (Lara.leftArm.flash_gun)
			{
				aMXPtr->m00 = lara_matrices[LMX_HAND_L * indices_count + M00];
				aMXPtr->m01 = lara_matrices[LMX_HAND_L * indices_count + M01];
				aMXPtr->m02 = lara_matrices[LMX_HAND_L * indices_count + M02];
				aMXPtr->m03 = lara_matrices[LMX_HAND_L * indices_count + M03];
				aMXPtr->m10 = lara_matrices[LMX_HAND_L * indices_count + M10];
				aMXPtr->m11 = lara_matrices[LMX_HAND_L * indices_count + M11];
				aMXPtr->m12 = lara_matrices[LMX_HAND_L * indices_count + M12];
				aMXPtr->m13 = lara_matrices[LMX_HAND_L * indices_count + M13];
				aMXPtr->m20 = lara_matrices[LMX_HAND_L * indices_count + M20];
				aMXPtr->m21 = lara_matrices[LMX_HAND_L * indices_count + M21];
				aMXPtr->m22 = lara_matrices[LMX_HAND_L * indices_count + M22];
				aMXPtr->m23 = lara_matrices[LMX_HAND_L * indices_count + M23];
				SetGunFlash(Lara.gunType);
			}

			phd_PopMatrix();
		}

		if (gfLevelFlags & GF_MIRROR && LaraItem->room_number == gfMirrorRoom)
			Draw_Mirror_Lara();
	}

	nPolyType = 4;
	DrawGunflashes();
	nPolyType = 0;

	for (int i = 0; i < 32; i++)
	{
		if (dynamics[i].on)
		{
			if (dynamics[i].z < 0)
				dynamics[i].z = 0;

			if (dynamics[i].x < 0)
				dynamics[i].x = 0;
		}
	}

	phd_PushMatrix();
	phd_TranslateAbs(0, 0, 0);
	SaveD3DCameraMatrix();
	phd_PopMatrix();

	aResetFogBulbList();

	if (tomb5.fog)
	{
		aBuildFogBulbList();
		aBuildFXFogBulbList();
	}

	for (int i = 0; i < number_draw_rooms; i++)
	{
		r = &room[draw_rooms[i]];
		phd_PushMatrix();
		phd_TranslateAbs(r->x, r->y, r->z);
		CurrentRoom = draw_rooms[i];
		phd_left = r->left;
		phd_right = r->right;
		phd_top = r->top;
		phd_bottom = r->bottom;
		aSetViewMatrix();
		InsertRoom(r);
		phd_PopMatrix();
	}

	DrawGunshells();
	nPolyType = 3;

	if (GLOBAL_playing_cutseq)
		DrawCutSeqActors();

	DrawRopeList();
	S_DrawSparks();
	DrawRats();
	DrawBats();
	DrawSpiders();
	lx = LaraItem->pos.x_pos;
	ly = LaraItem->pos.y_pos;
	lz = LaraItem->pos.z_pos;
	lr = LaraItem->room_number;
	LaraItem->pos.x_pos = camera.pos.x;
	LaraItem->pos.y_pos = camera.pos.y;
	LaraItem->pos.z_pos = camera.pos.z;
	LaraItem->room_number = camera.pos.room_number;
	DoWeather();
	DoUwEffect();
	S_DrawFires();
	S_DrawSmokeSparks();
	S_DrawSplashes();
	DrawBubbles();
	DrawDebris();
	DrawBlood();
	DrawDrips();
	DrawShockwaves();
	DrawLightning();
	DrawTwogunLasers();
	S_DrawFootPrints();
	LaraItem->pos.x_pos = lx;
	LaraItem->pos.y_pos = ly;
	LaraItem->pos.z_pos = lz;
	LaraItem->room_number = lr;

	if (LaserSightActive)
		DrawLaserSightSprite();

	for (int i = 0; i < number_draw_rooms; i++)
		PrintObjects(draw_rooms[i]);

	aUpdate();
}

void CalculateObjectLightingLara()
{
	PHD_VECTOR pos;

	if (GLOBAL_playing_cutseq)
		CalculateObjectLightingLaraCutSeq();
	else
	{
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;

		if (LaraItem->anim_number == ANIM_DUCKBREATHE || LaraItem->anim_number == ANIM_ALL4S || LaraItem->anim_number == ANIM_BREATH)
		{
			pos.x = LaraItem->pos.x_pos;

			if (LaraItem->anim_number == ANIM_BREATH)
				pos.y = LaraItem->pos.y_pos - 512;
			else
				pos.y = LaraItem->pos.y_pos - 192;

			pos.z = LaraItem->pos.z_pos;
		}
		else
			GetLaraJointPos(&pos, LMX_TORSO);

		current_item = LaraItem;
		LaraItem->il.item_pos.x = pos.x;
		LaraItem->il.item_pos.y = pos.y;
		LaraItem->il.item_pos.z = pos.z;
		CalcAmbientLight(LaraItem);
		CreateLightList(LaraItem);
	}
}

long GetFrames(ITEM_INFO* item, short* frm[], long* rate)
{
	ANIM_STRUCT* anim;
	long frame, size, frac, num;

	anim = &anims[item->anim_number];
	frm[0] = anim->frame_ptr;
	frm[1] = anim->frame_ptr;
	*rate = anim->interpolation & 0xFF;
	frame = item->frame_number - anim->frame_base;
	size = anim->interpolation >> 8;
	frm[0] += size * (frame / *rate);
	frm[1] = frm[0] + size;
	frac = (frame % *rate);

	if (!frac)
		return 0;

	num = *rate * (frame / *rate + 1);

	if (num > anim->frame_end)
		*rate = *rate + anim->frame_end - num;

	return frac;
}

void SetupSkelebobMeshswaps()
{
	Lara.meshPtrs[LM_HEAD] = meshes[objects[LARA_EXTRA_MESH2].mesh_index + 2 * LM_HEAD];
	Lara.meshPtrs[LM_TORSO] = meshes[objects[LARA_EXTRA_MESH2].mesh_index + 2 * LM_TORSO];
	skelly_backgunbak = Lara.weaponBackItem;
	skelly_rhandbak = Lara.meshPtrs[LM_RHAND];
	skelly_lhandbak = Lara.meshPtrs[LM_LHAND];

	if (Lara.weaponBackItem)
	{
		if (Lara.weaponBackItem == CROSSBOW_ANIM)
			Lara.weaponBackItem = UZI_ANIM;
		else if (Lara.weaponBackItem == HK_ANIM)
			Lara.weaponBackItem = PISTOLS_ANIM;
	}

	if (Lara.gunType == WEAPON_CROSSBOW)
	{
		if (Lara.meshPtrs[LM_RHAND] == meshes[objects[CROSSBOW_ANIM].mesh_index + 2 * LM_RHAND])
			Lara.meshPtrs[LM_RHAND] = meshes[objects[UZI_ANIM].mesh_index + 2 * LM_RHAND];
		else
			Lara.meshPtrs[LM_RHAND] = meshes[objects[LARA_EXTRA_MESH2].mesh_index + 2 * LM_RHAND];
	}
	else if (Lara.gunType == WEAPON_HK)
	{
		if (Lara.meshPtrs[LM_RHAND] == meshes[objects[HK_ANIM].mesh_index + 20])
			Lara.meshPtrs[LM_RHAND] = meshes[objects[PISTOLS_ANIM].mesh_index + 2 * LM_RHAND];
		else
			Lara.meshPtrs[LM_RHAND] = meshes[objects[LARA_EXTRA_MESH2].mesh_index + 2 * LM_RHAND];
	}
	
	Lara.meshPtrs[LM_LHAND] = meshes[objects[LARA_EXTRA_MESH2].mesh_index + 2 * LM_LHAND];
}

void RestoreLaraMeshswaps()
{
	Lara.meshPtrs[LM_HEAD] = meshes[objects[LARA].mesh_index + 2 * LM_HEAD];
	Lara.meshPtrs[LM_TORSO] = meshes[objects[LARA].mesh_index + 2 * LM_TORSO];
	Lara.weaponBackItem = skelly_backgunbak;
	Lara.meshPtrs[LM_RHAND] = skelly_rhandbak;
	Lara.meshPtrs[LM_LHAND] = skelly_lhandbak;
}

void RenderIt(short current_room)
{
	ROOM_INFO* r;

	CurrentRoom = current_room;
	r = &room[current_room];
	r->test_left = 0;
	r->test_top = 0;
	phd_left = 0;
	phd_top = 0;
	phd_right = phd_winxmax;
	phd_bottom = phd_winymax;
	r->test_right = phd_winxmax;
	r->test_bottom = phd_winymax;
	outside = r->flags & ROOM_OUTSIDE;
	camera_underwater = r->flags & ROOM_UNDERWATER;
	r->bound_active = 2;
	draw_room_list[0] = current_room;
	room_list_start = 0;
	room_list_end = 1;
	number_draw_rooms = 0;

	if (outside)
	{
		outside_top = 0;
		outside_left = 0;
		outside_right = phd_winxmax;
		outside_bottom = phd_winymax;
	}
	else
	{
		outside_left = phd_winxmax;
		outside_top = phd_winymax;
		outside_bottom = 0;
		outside_right = 0;
	}

	GetRoomBounds();
	ProcessClosedDoors();
	SkyDrawPhase();
	nPolyType = 0;

	phd_PushMatrix();
	phd_TranslateAbs(0, 0, 0);
	SaveD3DCameraMatrix();
	phd_PopMatrix();

	aResetFogBulbList();

	for (int i = 0; i < number_draw_rooms; i++)
	{
		r = &room[draw_rooms[i]];
		phd_PushMatrix();
		phd_TranslateAbs(r->x, r->y, r->z);
		CurrentRoom = draw_rooms[i];
		phd_left = r->left;
		phd_right = r->right;
		phd_top = r->top;
		phd_bottom = r->bottom;
		aSetViewMatrix();
		InsertRoom(r);
		phd_PopMatrix();
	}

	DoWeather();
	S_DrawFires();
	DrawLightning();

	for (int i = 0; i < number_draw_rooms; i++)
		PrintObjects(draw_rooms[i]);
}

void mRotBoundingBoxNoPersp(short* bounds, short* rotatedBounds)
{
	PHD_VECTOR pos[8];
	long x, y, z;
	short xMin, xMax, yMin, yMax, zMin, zMax;

	xMin = bounds[0];
	xMax = bounds[1];
	yMin = bounds[2];
	yMax = bounds[3];
	zMin = bounds[4];
	zMax = bounds[5];

	pos[0].x = xMin;
	pos[0].y = yMin;
	pos[0].z = zMin;

	pos[1].x = xMax;
	pos[1].y = yMin;
	pos[1].z = zMin;

	pos[2].x = xMin;
	pos[2].y = yMax;
	pos[2].z = zMin;

	pos[3].x = xMax;
	pos[3].y = yMax;
	pos[3].z = zMin;

	pos[4].x = xMin;
	pos[4].y = yMin;
	pos[4].z = zMax;

	pos[5].x = xMax;
	pos[5].y = yMin;
	pos[5].z = zMax;

	pos[6].x = xMin;
	pos[6].y = yMax;
	pos[6].z = zMax;

	pos[7].x = xMax;
	pos[7].y = yMax;
	pos[7].z = zMax;

	xMin = 0x7FFF;
	yMin = 0x7FFF;
	zMin = 0x7FFF;
	xMax = -0x7FFF;
	yMax = -0x7FFF;
	zMax = -0x7FFF;

	for (int i = 0; i < 8; i++)
	{
		x = (pos[i].x * phd_mxptr->m00 + pos[i].y * phd_mxptr->m01 + pos[i].z * phd_mxptr->m02) >> W2V_SHIFT;
		y = (pos[i].x * phd_mxptr->m10 + pos[i].y * phd_mxptr->m11 + pos[i].z * phd_mxptr->m12) >> W2V_SHIFT;
		z = (pos[i].x * phd_mxptr->m20 + pos[i].y * phd_mxptr->m21 + pos[i].z * phd_mxptr->m22) >> W2V_SHIFT;
		if (x < xMin) xMin = (short)x;
		if (x > xMax) xMax = (short)x;
		if (y < yMin) yMin = (short)y;
		if (y > yMax) yMax = (short)y;
		if (z < zMin) zMin = (short)z;
		if (z > zMax) zMax = (short)z;
	}

	rotatedBounds[0] = xMin;
	rotatedBounds[1] = xMax;
	rotatedBounds[2] = yMin;
	rotatedBounds[3] = yMax;
	rotatedBounds[4] = zMin;
	rotatedBounds[5] = zMax;
}

void PrintRooms(short room_number)
{
	ROOM_INFO* r;

	CurrentRoom = room_number;
	r = &room[room_number];
	phd_left = r->left;
	phd_right = r->right;
	phd_top = r->top;
	phd_bottom = r->bottom;
	aSetViewMatrix();
	S_InsertRoom(r, 1);
}

void DrawStaticObjects(short room_number)
{
	ROOM_INFO* r;
	MESH_INFO* mesh;
	STATIC_INFO* sinfo;
	long clip, mip;
	short n;

	CurrentRoom = room_number;
	nPolyType = 1;
	r = &room[room_number];

	phd_PushMatrix();
	phd_TranslateAbs(r->x, r->y, r->z);
	phd_left = r->left;
	phd_right = r->right;
	phd_top = r->top;
	phd_bottom = r->bottom;

	mesh = r->mesh;

	for (int i = r->num_meshes; i > 0; i--, mesh++)
	{
		if (!(mesh->Flags & 1))
			continue;

		phd_PushMatrix();
		phd_TranslateAbs(mesh->x, mesh->y, mesh->z);
		phd_RotY(mesh->y_rot);
		n = mesh->static_number;
		sinfo = &static_objects[n];
		clip = S_GetObjectBounds(&sinfo->x_minp);

		if (clip)
		{
			mip = sinfo->flags & 0x3C;
			S_CalculateStaticMeshLight(mesh->x, mesh->y, mesh->z, mesh->shade, r);

			if (mip)
			{
				if (phd_mxptr->m23 >> 15 > (mip & 0xFFFC) << 8)
					n++;
			}

			sinfo = &static_objects[n];
			phd_PutPolygons(meshes[sinfo->mesh_number], clip);
		}

		phd_PopMatrix();
	}

	phd_PopMatrix();
}

void InterpolateMatrix()
{
	aInterpolateMatrix();

	if (IM_rate == 2 || (IM_frac == 2 && IM_rate == 4))
	{
		phd_mxptr->m00 += (IMptr->m00 - phd_mxptr->m00) >> 1;
		phd_mxptr->m01 += (IMptr->m01 - phd_mxptr->m01) >> 1;
		phd_mxptr->m02 += (IMptr->m02 - phd_mxptr->m02) >> 1;
		phd_mxptr->m03 += (IMptr->m03 - phd_mxptr->m03) >> 1;
		phd_mxptr->m10 += (IMptr->m10 - phd_mxptr->m10) >> 1;
		phd_mxptr->m11 += (IMptr->m11 - phd_mxptr->m11) >> 1;
		phd_mxptr->m12 += (IMptr->m12 - phd_mxptr->m12) >> 1;
		phd_mxptr->m13 += (IMptr->m13 - phd_mxptr->m13) >> 1;
		phd_mxptr->m20 += (IMptr->m20 - phd_mxptr->m20) >> 1;
		phd_mxptr->m21 += (IMptr->m21 - phd_mxptr->m21) >> 1;
		phd_mxptr->m22 += (IMptr->m22 - phd_mxptr->m22) >> 1;
		phd_mxptr->m23 += (IMptr->m23 - phd_mxptr->m23) >> 1;
	}
	else if (IM_frac == 1)
	{
		phd_mxptr->m00 += (IMptr->m00 - phd_mxptr->m00) >> 2;
		phd_mxptr->m01 += (IMptr->m01 - phd_mxptr->m01) >> 2;
		phd_mxptr->m02 += (IMptr->m02 - phd_mxptr->m02) >> 2;
		phd_mxptr->m03 += (IMptr->m03 - phd_mxptr->m03) >> 2;
		phd_mxptr->m10 += (IMptr->m10 - phd_mxptr->m10) >> 2;
		phd_mxptr->m11 += (IMptr->m11 - phd_mxptr->m11) >> 2;
		phd_mxptr->m12 += (IMptr->m12 - phd_mxptr->m12) >> 2;
		phd_mxptr->m13 += (IMptr->m13 - phd_mxptr->m13) >> 2;
		phd_mxptr->m20 += (IMptr->m20 - phd_mxptr->m20) >> 2;
		phd_mxptr->m21 += (IMptr->m21 - phd_mxptr->m21) >> 2;
		phd_mxptr->m22 += (IMptr->m22 - phd_mxptr->m22) >> 2;
		phd_mxptr->m23 += (IMptr->m23 - phd_mxptr->m23) >> 2;
	}
	else
	{
		phd_mxptr->m00 = IMptr->m00 - ((IMptr->m00 - phd_mxptr->m00) >> 2);
		phd_mxptr->m01 = IMptr->m01 - ((IMptr->m01 - phd_mxptr->m01) >> 2);
		phd_mxptr->m02 = IMptr->m02 - ((IMptr->m02 - phd_mxptr->m02) >> 2);
		phd_mxptr->m03 = IMptr->m03 - ((IMptr->m03 - phd_mxptr->m03) >> 2);
		phd_mxptr->m10 = IMptr->m10 - ((IMptr->m10 - phd_mxptr->m10) >> 2);
		phd_mxptr->m11 = IMptr->m11 - ((IMptr->m11 - phd_mxptr->m11) >> 2);
		phd_mxptr->m12 = IMptr->m12 - ((IMptr->m12 - phd_mxptr->m12) >> 2);
		phd_mxptr->m13 = IMptr->m13 - ((IMptr->m13 - phd_mxptr->m13) >> 2);
		phd_mxptr->m20 = IMptr->m20 - ((IMptr->m20 - phd_mxptr->m20) >> 2);
		phd_mxptr->m21 = IMptr->m21 - ((IMptr->m21 - phd_mxptr->m21) >> 2);
		phd_mxptr->m22 = IMptr->m22 - ((IMptr->m22 - phd_mxptr->m22) >> 2);
		phd_mxptr->m23 = IMptr->m23 - ((IMptr->m23 - phd_mxptr->m23) >> 2);
	}
}

void InterpolateArmMatrix(Matrix* mx)
{
	phd_mxptr->m00 = mx->m00;
	phd_mxptr->m01 = mx->m01;
	phd_mxptr->m02 = mx->m02;
	phd_mxptr->m10 = mx->m10;
	phd_mxptr->m11 = mx->m11;
	phd_mxptr->m12 = mx->m12;
	phd_mxptr->m20 = mx->m20;
	phd_mxptr->m21 = mx->m21;
	phd_mxptr->m22 = mx->m22;

	if (IM_rate == 2 || (IM_frac == 2 && IM_rate == 4))
	{
		phd_mxptr->m03 = (phd_mxptr->m03 + IMptr->m03) >> 1;
		phd_mxptr->m13 = (phd_mxptr->m13 + IMptr->m13) >> 1;
		phd_mxptr->m23 = (phd_mxptr->m23 + IMptr->m23) >> 1;
	}
	else if (IM_frac == 1)
	{
		phd_mxptr->m03 += (IMptr->m03 - phd_mxptr->m03) >> 2;
		phd_mxptr->m13 += (IMptr->m13 - phd_mxptr->m13) >> 2;
		phd_mxptr->m23 += (IMptr->m23 - phd_mxptr->m23) >> 2;
	}
	else
	{
		phd_mxptr->m03 = IMptr->m03 - ((IMptr->m03 - phd_mxptr->m03) >> 2);
		phd_mxptr->m13 = IMptr->m13 - ((IMptr->m13 - phd_mxptr->m13) >> 2);
		phd_mxptr->m23 = IMptr->m23 - ((IMptr->m23 - phd_mxptr->m23) >> 2);
	}
}

void aInterpolateArmMatrix(FMatrix* mx)
{
	aMXPtr->m00 = mx->m00;
	aMXPtr->m01 = mx->m01;
	aMXPtr->m02 = mx->m02;
	aMXPtr->m10 = mx->m10;
	aMXPtr->m11 = mx->m11;
	aMXPtr->m12 = mx->m12;
	aMXPtr->m20 = mx->m20;
	aMXPtr->m21 = mx->m21;
	aMXPtr->m22 = mx->m22;

	if (IM_rate == 2 || (IM_frac == 2 && IM_rate == 4))
	{
		aMXPtr->m03 = (aMXPtr->m03 + aIMXPtr->m03) * 0.5F;
		aMXPtr->m13 = (aMXPtr->m13 + aIMXPtr->m13) * 0.5F;
		aMXPtr->m23 = (aMXPtr->m23 + aIMXPtr->m23) * 0.5F;
	}
	else if (IM_frac == 1)
	{
		aMXPtr->m03 += (aIMXPtr->m03 - aMXPtr->m03) * 0.25F;
		aMXPtr->m13 += (aIMXPtr->m13 - aMXPtr->m13) * 0.25F;
		aMXPtr->m23 += (aIMXPtr->m23 - aMXPtr->m23) * 0.25F;
	}
	else
	{
		aMXPtr->m03 = aIMXPtr->m03 - ((aIMXPtr->m03 - aMXPtr->m03) * 0.25F);
		aMXPtr->m13 = aIMXPtr->m13 - ((aIMXPtr->m13 - aMXPtr->m13) * 0.25F);
		aMXPtr->m23 = aIMXPtr->m23 - ((aIMXPtr->m23 - aMXPtr->m23) * 0.25F);
	}
}

void DrawEffect(short fx_num)
{
	FX_INFO* fx;
	OBJECT_INFO* obj;
	short* meshp;

	fx = &effects[fx_num];
	obj = &objects[fx->object_number];

	if (obj->draw_routine && obj->loaded)
	{
		phd_PushMatrix();
		phd_TranslateAbs(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);

		if (phd_mxptr->m23 > phd_znear && phd_mxptr->m23 < phd_zfar)
		{
			phd_RotYXZ(fx->pos.y_rot, fx->pos.x_rot, fx->pos.z_rot);

			if (obj->nmeshes)
				meshp = meshes[obj->mesh_index];
			else
				meshp = meshes[fx->frame_number];

			phd_PutPolygons(meshp, -1);
		}

		phd_PopMatrix();
	}
}

void calc_animating_item_clip_window(ITEM_INFO* item, short* bounds)
{
	ROOM_INFO* r;
	short* door;
	long xMin, xMax, yMin, yMax, zMin, zMax;		//object bounds
	long xMinR, xMaxR, yMinR, yMaxR, zMinR, zMaxR;	//room bounds
	long xMinD, xMaxD, yMinD, yMaxD, zMinD, zMaxD;	//door bounds
	short rotatedBounds[6];
	short nDoors;

	r = &room[ClipRoomNum];

	if (item->object_number >= ANIMATING1 && item->object_number <= ANIMATING16 ||
		item->object_number >= DOOR_TYPE1 && item->object_number <= LIFT_DOORS2_MIP)
	{
		phd_left = r->left;
		phd_right = r->right;
		phd_top = r->top;
		phd_bottom = r->bottom;
		return;
	}

	phd_PushUnitMatrix();
	phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
	phd_mxptr->m03 = 0;
	phd_mxptr->m13 = 0;
	phd_mxptr->m23 = 0;
	mRotBoundingBoxNoPersp(bounds, rotatedBounds);
	phd_PopMatrix();

	xMin = item->pos.x_pos + rotatedBounds[0];
	xMax = item->pos.x_pos + rotatedBounds[1];
	yMin = item->pos.y_pos + rotatedBounds[2];
	yMax = item->pos.y_pos + rotatedBounds[3];
	zMin = item->pos.z_pos + rotatedBounds[4];
	zMax = item->pos.z_pos + rotatedBounds[5];

	xMinR = r->x + 1024;
	xMaxR = xMinR + ((r->y_size - 2) << 10);
	yMinR = r->maxceiling;
	yMaxR = r->minfloor;
	zMinR = r->z + 1024;
	zMaxR = zMinR + ((r->x_size - 2) << 10);

	if (xMin >= xMinR && xMax <= xMaxR && yMin >= yMinR && yMax <= yMaxR && zMin >= zMinR && zMax <= zMaxR)
	{
		phd_left = r->left;
		phd_right = r->right;
		phd_top = r->top;
		phd_bottom = r->bottom;
		return;
	}

	if (camera.pos.room_number != item->room_number && camera.pos.room_number != ClipRoomNum)
	{
		door = r->door;
		nDoors = *door++;

		for (; nDoors > 0; nDoors--, door += 16)
		{
			if (door[0] != camera.pos.room_number)
				continue;

			xMinD = door[4];	//skip door normal
			xMaxD = door[4];
			yMinD = door[5];
			yMaxD = door[5];
			zMinD = door[6];
			zMaxD = door[6];

			if (door[7] < xMinD)
				xMinD = door[7];
			else if (door[7] > xMaxD)
				xMaxD = door[7];

			if (door[8] < yMinD)
				yMinD = door[8];
			else if (door[8] > yMaxD)
				yMaxD = door[8];

			if (door[9] < zMinD)
				zMinD = door[9];
			else if (door[9] > zMaxD)
				zMaxD = door[9];

			if (door[10] < xMinD)
				xMinD = door[10];
			else if (door[10] > xMaxD)
				xMaxD = door[10];

			if (door[11] < yMinD)
				yMinD = door[11];
			else if (door[11] > yMaxD)
				yMaxD = door[11];

			if (door[12] < zMinD)
				zMinD = door[12];
			else if (door[12] > zMaxD)
				zMaxD = door[12];

			if (door[13] < xMinD)
				xMinD = door[13];
			else if (door[13] > xMaxD)
				xMaxD = door[13];

			if (door[14] < yMinD)
				yMinD = door[14];
			else if (door[14] > yMaxD)
				yMaxD = door[14];

			if (door[15] < zMinD)
				zMinD = door[15];
			else if (door[15] > zMaxD)
				zMaxD = door[15];

			xMinD += xMinR - 1024;
			xMaxD += xMinR - 1024;
			yMinD += yMinR;
			yMaxD += yMinR;
			zMinD += zMinR - 1024;
			zMaxD += zMinR - 1024;

			if (xMinD <= xMax && xMaxD >= xMin && yMinD <= yMax && yMaxD >= yMin && zMinD <= zMax && zMaxD >= zMin)
				break;
		}

		if (!nDoors)
		{
			phd_left = r->left;
			phd_right = r->right;
			phd_top = r->top;
			phd_bottom = r->bottom;
		}
	}
}

void SetRoomBounds(short* door, long rn, ROOM_INFO* actualRoom)
{
	ROOM_INFO* r;
	FVECTOR* v;
	FVECTOR* lastV;
	static FVECTOR vbuf[4];
	float x, y, z, tooNear, tL, tR, tT, tB;

	r = &room[rn];

	if (r->left <= actualRoom->test_left && r->right >= actualRoom->test_right && r->top <= actualRoom->test_top && r->bottom >= actualRoom->test_bottom)
		return;

	tL = (float)actualRoom->test_right;
	tR = (float)actualRoom->test_left;
	tB = (float)actualRoom->test_top;
	tT = (float)actualRoom->test_bottom;
	door += 3;
	v = vbuf;
	tooNear = 0;

	for (int i = 0; i < 4; i++, v++, door += 3)
	{
		v->x = aMXPtr->m00 * door[0] + aMXPtr->m01 * door[1] + aMXPtr->m02 * door[2] + aMXPtr->m03;
		v->y = aMXPtr->m10 * door[0] + aMXPtr->m11 * door[1] + aMXPtr->m12 * door[2] + aMXPtr->m13;
		v->z = aMXPtr->m20 * door[0] + aMXPtr->m21 * door[1] + aMXPtr->m22 * door[2] + aMXPtr->m23;
		x = v->x;
		y = v->y;
		z = v->z;

		if (z <= 0)
			tooNear++;
		else
		{
			z /= f_mpersp;

			if (z)
			{
				x = x / z + f_centerx;
				y = y / z + f_centery;
			}
			else
			{
				if (x < 0)
					x = (float)phd_left;
				else
					x = (float)phd_right;

				if (y < 0)
					y = (float)phd_top;
				else
					y = (float)phd_bottom;
			}

			if (x - 1 < tL)
				tL = x - 1;

			if (x + 1 > tR)
				tR = x + 1;

			if (y - 1 < tT)
				tT = y - 1;

			if (y + 1 > tB)
				tB = y + 1;
		}
	}

	if (tooNear == 4)
		return;

	if (tooNear > 0)
	{
		v = vbuf;
		lastV = &vbuf[3];

		for (int i = 0; i < 4; i++, lastV = v, v++)
		{
			if (lastV->z <= 0 == v->z <= 0)
				continue;

			if (v->x < 0 && lastV->x < 0)
				tL = 0;
			else if (v->x > 0 && lastV->x > 0)
				tR = phd_winxmax;
			else
			{
				tL = 0;
				tR = phd_winxmax;
			}

			if (v->y < 0 && lastV->y < 0)
				tT = 0;
			else if (v->y > 0 && lastV->y > 0)
				tB = phd_winymax;
			else
			{
				tT = 0;
				tB = phd_winymax;
			}
		}
	}

	if (tL < actualRoom->test_left)
		tL = actualRoom->test_left;

	if (tR > actualRoom->test_right)
		tR = actualRoom->test_right;

	if (tT < actualRoom->test_top)
		tT = actualRoom->test_top;

	if (tB > actualRoom->test_bottom)
		tB = actualRoom->test_bottom;

	if (tL >= tR || tT >= tB)
		return;

	if (r->bound_active & 2)
	{
		if (tL < r->test_left)
			r->test_left = (short)tL;

		if (tT < r->test_top)
			r->test_top = (short)tT;

		if (tR > r->test_right)
			r->test_right = (short)tR;

		if (tB > r->test_bottom)
			r->test_bottom = (short)tB;
	}
	else
	{
		draw_room_list[room_list_end % 128] = rn;
		room_list_end++;
		r->bound_active |= 2;
		r->test_left = (short)tL;
		r->test_right = (short)tR;
		r->test_top = (short)tT;
		r->test_bottom = (short)tB;
	}
}

void GetRoomBounds()
{
	ROOM_INFO* r;
	ROOM_PORTAL* p;
	short* door;
	long rn, drn, x, y, z;

	while (room_list_start != room_list_end)
	{
		rn = draw_room_list[room_list_start % 128];
		room_list_start++;
		r = &room[rn];
		r->bound_active -= 2;

		if (r->test_left < r->left)
			r->left = r->test_left;

		if (r->test_top < r->top)
			r->top = r->test_top;

		if (r->test_right > r->right)
			r->right = r->test_right;

		if (r->test_bottom > r->bottom)
			r->bottom = r->test_bottom;

		if (!(r->bound_active & 1))
		{
			draw_rooms[number_draw_rooms] = (short)rn;
			number_draw_rooms++;
			r->bound_active |= 1;

			if (r->flags & ROOM_OUTSIDE)
				outside = ROOM_OUTSIDE;
		}

		if (r->flags & ROOM_OUTSIDE)
		{
			if (r->left < outside_left)
				outside_left = r->left;

			if (r->right > outside_right)
				outside_right = r->right;

			if (r->top < outside_top)
				outside_top = r->top;

			if (r->bottom > outside_bottom)
				outside_bottom = r->bottom;
		}

		phd_PushMatrix();
		phd_TranslateAbs(r->x, r->y, r->z);
		door = r->door;

		if (door)
		{
			for (drn = *door++; drn > 0; drn--, door += 15)
			{
				p = (ROOM_PORTAL*)door;
				door++;

				rn = p->rn;

				if (room[rn].flags & ROOM_OUTSIDE)
					snow_outside = 1;

				if (p->normal[0])
				{
					x = r->x + p->v1[0] - w2v_matrix.m03;

					if (x && (x ^ p->normal[0]) < 0)
						SetRoomBounds(door, rn, r);
					else
						continue;
				}

				if (p->normal[1])
				{
					y = r->y + p->v1[1] - w2v_matrix.m13;

					if (y && (y ^ p->normal[1]) < 0)
						SetRoomBounds(door, rn, r);
					else
						continue;
				}

				if (p->normal[2])
				{
					z = r->z + p->v1[2] - w2v_matrix.m23;

					if (z && (z ^ p->normal[2]) < 0)
						SetRoomBounds(door, rn, r);
					else
						continue;
				}
			}
		}

		phd_PopMatrix();
	}
}
