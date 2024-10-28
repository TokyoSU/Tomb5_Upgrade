#include "../tomb5/pch.h"
#include "hair.h"
#include "../specific/3dmath.h"
#include "../specific/output.h"
#include "gameflow.h"
#include "objects.h"
#include "draw.h"
#include "lara_states.h"
#include "control.h"
#include "../specific/function_stubs.h"
#include "delstuff.h"
#include "deltapak.h"
#include "lara.h"
#include "effect2.h"

static char HairRotScratchVertNums[5][12] =
{
	{ 4, 5, 6, 7, -1, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 6, 7, 4, -1, 0, 0, 0, 0, 0, 0, 0 },
	{ 6, 7, 4, 5, -1, 0, 0, 0, 0, 0, 0, 0 },
	{ 7, 4, 5, 6, -1, 0, 0, 0, 0, 0, 0, 0 },
	{ 4, 5, 6, 7, -1, 0, 0, 0, 0, 0, 0, 0 }
};

HAIR_STRUCT hairs[2][7];
static long hair_wind = 0;
static long hair_wind_angle = 0;
static long hair_dwind_angle = 0;
static long first_hair[2];

void DrawHair()
{
	HAIR_STRUCT* hair;
	short** meshpp;
	long ii;

	for (int i = 0; i < 2; i++)
	{
		ii = i * 6;
		meshpp = &meshes[objects[HAIR].mesh_index];
		meshpp += 2;

		hair = &hairs[i][1];

		for (int j = 1; j < 6; j += 2, meshpp += 4, hair += 2)
		{
			phd_PushMatrix();
			phd_TranslateAbs(hair->pos.x_pos, hair->pos.y_pos, hair->pos.z_pos);
			phd_RotY(hair->pos.y_rot);
			phd_RotX(hair->pos.x_rot);
			phd_PutPolygons(*meshpp, -1);

			if (j == 5)
				StashSkinVertices(33 + ii);
			else
			{
				StashSkinVertices(28 + ii + j);
				StashSkinVertices(29 + ii + j);
			}

			phd_PopMatrix();
		}

		meshpp = &meshes[objects[HAIR].mesh_index];

		for (int j = 0; j < 6; j += 2, meshpp += 4)
		{
			SkinVerticesToScratch(28 + ii + j);
			GetCorrectStashPoints(i, j, 29 + ii + j);
			SkinVerticesToScratch(29 + ii + j);
			phd_PutPolygons(*meshpp, -1);
		}

		if (!(gfLevelFlags & GF_YOUNGLARA))
			break;
	}
}

void HairControl(long in_cutscene, long pigtail, short* cutscenething)
{
	OBJECT_INFO* obj;
	HAIR_STRUCT* hair;
	FLOOR_INFO* floor;
	PHD_VECTOR pos;
	SPHERE sphere[6];
	long* bone;
	short* rot[2];
	short* frm[2];
	short* objptr;
	short* frame;
	long frac, rate, water, height, dist, x, y, z, dx, dy, dz;
	short room_num, spaz;

	obj = &objects[LARA];

	if (!cutscenething)
	{
		if (Lara.hitDirection < 0)
		{
			frame = GetBestFrame(LaraItem);
			frac = GetFrames(LaraItem, frm, &rate); // Interpolate the frames if necessary
		}
		else
		{
			if (!Lara.hitDirection)
				spaz = Lara.isDucked ? ANIM_SPAZ_DUCKF : ANIM_SPAZ_FORWARD;
			else if (Lara.hitDirection == 1)
				spaz = Lara.isDucked ? ANIM_SPAZ_DUCKR : ANIM_SPAZ_RIGHT;
			else if (Lara.hitDirection == 2)
				spaz = Lara.isDucked ? ANIM_SPAZ_DUCKB : ANIM_SPAZ_BACK;
			else
				spaz = Lara.isDucked ? ANIM_SPAZ_DUCKL : ANIM_SPAZ_LEFT;

			frame = anims[spaz].frame_ptr;
			frame += Lara.hitFrame * (anims[spaz].interpolation >> 8);
			frm[0] = frame;
			frac = 0;
		}
	}
	else
	{
		frame = cutscenething;
		frm[0] = frame;
		frac = 0;
	}

	phd_PushUnitMatrix();
	phd_SetTrans(LaraItem->pos.x_pos, LaraItem->pos.y_pos, LaraItem->pos.z_pos);
	phd_RotYXZ(LaraItem->pos.y_rot, LaraItem->pos.x_rot, LaraItem->pos.z_rot);

	if (frac)
	{
		InitInterpolate(frac, rate);
		rot[0] = frm[0] + 9;
		rot[1] = frm[1] + 9;
		bone = &bones[obj->bone_index];
		phd_TranslateRel_ID(frm[0][6], frm[0][7], frm[0][8], frm[1][6], frm[1][7], frm[1][8]);
		gar_RotYXZsuperpack_I(&rot[0], &rot[1], 0);

		phd_PushMatrix_I();
		objptr = Lara.meshPtrs[LM_HIPS];
		phd_TranslateRel_I(*objptr, objptr[1], objptr[2]);
		InterpolateMatrix();
		sphere[0].x = (long)aMXPtr->m03;
		sphere[0].y = (long)aMXPtr->m13;
		sphere[0].z = (long)aMXPtr->m23;
		sphere[0].r = objptr[3];
		phd_PopMatrix_I();

		phd_TranslateRel_I(bone[25], bone[26], bone[27]);

		if (Lara.weaponItem != NO_ITEM && Lara.gunType == WEAPON_HK &&
			(items[Lara.weaponItem].current_anim_state == 0 || items[Lara.weaponItem].current_anim_state == 1 ||
				items[Lara.weaponItem].current_anim_state == 2 || items[Lara.weaponItem].current_anim_state == 3 ||
				items[Lara.weaponItem].current_anim_state == 4))
		{
			rot[0] = &Lara.rightArm.frame_base[Lara.rightArm.frame_number * (anims[Lara.rightArm.anim_number].interpolation >> 8) + 9];
			rot[1] = rot[0];
			gar_RotYXZsuperpack_I(&rot[0], &rot[1], 7);
		}
		else
			gar_RotYXZsuperpack_I(&rot[0], &rot[1], 6);

		phd_RotYXZ_I(Lara.torsoRotY, Lara.torsoRotX, Lara.torsoRotZ);

		phd_PushMatrix_I();
		objptr = Lara.meshPtrs[LM_TORSO];

		if (gfCurrentLevel >= LVL5_BASE && gfCurrentLevel <= LVL5_SINKING_SUBMARINE)
			phd_TranslateRel_I(*objptr - 11, objptr[1] - 16, objptr[2] + 25);
		else
			phd_TranslateRel_I(*objptr - 9, objptr[1], objptr[2] + 25);	//repositioned to avoid floating hair

		InterpolateMatrix();
		sphere[1].x = (long)aMXPtr->m03;
		sphere[1].y = (long)aMXPtr->m13;
		sphere[1].z = (long)aMXPtr->m23;

		if (gfCurrentLevel >= LVL5_BASE && gfCurrentLevel <= LVL5_SINKING_SUBMARINE)
			sphere[1].r = 94;
		else
			sphere[1].r = objptr[3];

		if (gfLevelFlags & GF_YOUNGLARA)
			sphere[1].r -= (sphere[1].r >> 2) + (sphere[1].r >> 3);

		phd_PopMatrix_I();

		phd_PushMatrix_I();
		phd_TranslateRel_I(bone[29], bone[30], bone[31]);
		gar_RotYXZsuperpack_I(&rot[0], &rot[1], 0);
		objptr = Lara.meshPtrs[LM_RINARM];
		phd_TranslateRel_I(*objptr, objptr[1], objptr[2]);
		InterpolateMatrix();
		sphere[3].x = (long)aMXPtr->m03;
		sphere[3].y = (long)aMXPtr->m13;
		sphere[3].z = (long)aMXPtr->m23;
		sphere[3].r = 4 * objptr[3] / 3;	//limited rad to avoid floating hair when crawling with the new neck sphere
		phd_PopMatrix_I();

		phd_PushMatrix_I();
		phd_TranslateRel_I(bone[41], bone[42], bone[43]);
		gar_RotYXZsuperpack_I(&rot[0], &rot[1], 2);
		objptr = Lara.meshPtrs[LM_LINARM];
		phd_TranslateRel_I(*objptr, objptr[1], objptr[2]);
		InterpolateMatrix();
		sphere[4].x = (long)aMXPtr->m03;
		sphere[4].y = (long)aMXPtr->m13;
		sphere[4].z = (long)aMXPtr->m23;
		sphere[4].r = 4 * objptr[3] / 3;	//limited rad to avoid floating hair when crawling with the new neck sphere
		phd_PopMatrix_I();
		phd_TranslateRel_I(bone[53], bone[54], bone[55]);
		gar_RotYXZsuperpack_I(&rot[0], &rot[1], 2);
		phd_RotYXZ_I(Lara.headRotY, Lara.headRotX, Lara.headRotZ);

		phd_PushMatrix_I();
		objptr = Lara.meshPtrs[LM_HEAD];
		phd_TranslateRel_I(*objptr - 2, objptr[1], objptr[2]);	//repositioned to avoid floating hair
		InterpolateMatrix();
		sphere[2].x = (long)aMXPtr->m03;
		sphere[2].y = (long)aMXPtr->m13;
		sphere[2].z = (long)aMXPtr->m23;
		sphere[2].r = objptr[3];
		phd_PopMatrix_I();

		if (gfLevelFlags & GF_YOUNGLARA)
		{
			sphere[1].x = (sphere[1].x + sphere[2].x) >> 1;
			sphere[1].y = (sphere[1].y + sphere[2].y) >> 1;
			sphere[1].z = (sphere[1].z + sphere[2].z) >> 1;
		}

		sphere[5].x = (2 * sphere[2].x + sphere[1].x) / 3;//sphere[5] is an extra sphere added between the head and torso sphere to simulate a neck sphere
		sphere[5].y = (2 * sphere[2].y + sphere[1].y) / 3;
		sphere[5].z = (2 * sphere[2].z + sphere[1].z) / 3;
		sphere[5].r = gfLevelFlags & GF_YOUNGLARA ? 0 : 5 * sphere[2].r / 4;

		if (pigtail)
			phd_TranslateRel_I(44, -48, -50);
		else if (gfLevelFlags & GF_YOUNGLARA)
			phd_TranslateRel_I(-52, -48, -50);
		else
			phd_TranslateRel_I(-4, -4, -48);

		InterpolateMatrix();
		pos.x = (long)aMXPtr->m03;
		pos.y = (long)aMXPtr->m13;
		pos.z = (long)aMXPtr->m23;
	}
	else
	{
		rot[0] = frm[0] + 9;
		bone = &bones[obj->bone_index];
		phd_TranslateRel(frm[0][6], frm[0][7], frm[0][8]);
		gar_RotYXZsuperpack(&rot[0], 0);

		phd_PushMatrix();
		objptr = Lara.meshPtrs[LM_HIPS];
		phd_TranslateRel(*objptr, objptr[1], objptr[2]);
		sphere[0].x = (long)aMXPtr->m03;
		sphere[0].y = (long)aMXPtr->m13;
		sphere[0].z = (long)aMXPtr->m23;
		sphere[0].r = objptr[3];
		phd_PopMatrix();

		phd_TranslateRel(bone[25], bone[26], bone[27]);

		if (Lara.weaponItem != NO_ITEM && Lara.gunType == WEAPON_HK &&
			(items[Lara.weaponItem].current_anim_state == 0 || items[Lara.weaponItem].current_anim_state == 1 ||
				items[Lara.weaponItem].current_anim_state == 2 || items[Lara.weaponItem].current_anim_state == 3 ||
				items[Lara.weaponItem].current_anim_state == 4))
		{
			rot[0] = Lara.rightArm.frame_base + Lara.rightArm.frame_number * (anims[Lara.rightArm.anim_number].interpolation >> 8) + 9;
			gar_RotYXZsuperpack(&rot[0], 7);
		}
		else
			gar_RotYXZsuperpack(&rot[0], 6);

		phd_RotYXZ(Lara.torsoRotY, Lara.torsoRotX, Lara.torsoRotZ);

		phd_PushMatrix();
		objptr = Lara.meshPtrs[LM_TORSO];

		if (gfCurrentLevel >= LVL5_BASE && gfCurrentLevel <= LVL5_SINKING_SUBMARINE)
			phd_TranslateRel(*objptr - 11, objptr[1] - 16, objptr[2] + 25);
		else
			phd_TranslateRel(*objptr - 9, objptr[1], objptr[2] + 25);

		sphere[1].x = (long)aMXPtr->m03;
		sphere[1].y = (long)aMXPtr->m13;
		sphere[1].z = (long)aMXPtr->m23;

		if (gfCurrentLevel >= LVL5_BASE && gfCurrentLevel <= LVL5_SINKING_SUBMARINE)
			sphere[1].r = 94;
		else
			sphere[1].r = objptr[3];

		if (gfLevelFlags & GF_YOUNGLARA)
			sphere[1].r -= (sphere[1].r >> 2) + (sphere[1].r >> 3);

		phd_PopMatrix();

		phd_PushMatrix();
		phd_TranslateRel(bone[29], bone[30], bone[31]);
		gar_RotYXZsuperpack(&rot[0], 0);
		objptr = Lara.meshPtrs[LM_RINARM];
		phd_TranslateRel(*objptr, objptr[1], objptr[2]);
		sphere[3].x = (long)aMXPtr->m03;
		sphere[3].y = (long)aMXPtr->m13;
		sphere[3].z = (long)aMXPtr->m23;
		sphere[3].r = 4 * objptr[3] / 3;
		phd_PopMatrix();

		phd_PushMatrix();
		phd_TranslateRel(bone[41], bone[42], bone[43]);
		gar_RotYXZsuperpack(&rot[0], 2);
		objptr = Lara.meshPtrs[LM_LINARM];
		phd_TranslateRel(*objptr, objptr[1], objptr[2]);
		sphere[4].x = (long)aMXPtr->m03;
		sphere[4].y = (long)aMXPtr->m13;
		sphere[4].z = (long)aMXPtr->m23;
		sphere[4].r = 4 * objptr[3] / 3;
		phd_PopMatrix();
		phd_TranslateRel(bone[53], bone[54], bone[55]);
		gar_RotYXZsuperpack(&rot[0], 2);
		phd_RotYXZ(Lara.headRotY, Lara.headRotX, Lara.headRotZ);

		phd_PushMatrix();
		objptr = Lara.meshPtrs[LM_HEAD];
		phd_TranslateRel(*objptr - 2, objptr[1], objptr[2]);
		sphere[2].x = (long)aMXPtr->m03;
		sphere[2].y = (long)aMXPtr->m13;
		sphere[2].z = (long)aMXPtr->m23;
		sphere[2].r = objptr[3];
		phd_PopMatrix();

		if (gfLevelFlags & GF_YOUNGLARA)
		{
			sphere[1].x = (sphere[1].x + sphere[2].x) >> 1;
			sphere[1].y = (sphere[1].y + sphere[2].y) >> 1;
			sphere[1].z = (sphere[1].z + sphere[2].z) >> 1;
		}

		sphere[5].x = (2 * sphere[2].x + sphere[1].x) / 3;
		sphere[5].y = (2 * sphere[2].y + sphere[1].y) / 3;
		sphere[5].z = (2 * sphere[2].z + sphere[1].z) / 3;
		sphere[5].r = gfLevelFlags & GF_YOUNGLARA ? 0 : 5 * sphere[2].r / 4;

		if (pigtail)
			phd_TranslateRel(44, -48, -50);
		else if (gfLevelFlags & GF_YOUNGLARA)
			phd_TranslateRel(-52, -48, -50);
		else
			phd_TranslateRel(-4, -4, -48);

		pos.x = (long)aMXPtr->m03;
		pos.y = (long)aMXPtr->m13;
		pos.z = (long)aMXPtr->m23;
	}

	phd_PopMatrix();
	obj = &objects[HAIR];
	bone = &bones[obj->bone_index];
	hair = &hairs[pigtail][0];

	if (first_hair[pigtail])
	{
		first_hair[pigtail] = 0;
		hair->pos.x_pos = pos.x;
		hair->pos.y_pos = pos.y;
		hair->pos.z_pos = pos.z;

		for (int i = 0; i < 6; i++, bone += 4)
		{
			phd_PushUnitMatrix();
			phd_SetTrans(hair->pos.x_pos, hair->pos.y_pos, hair->pos.z_pos);
			phd_RotYXZ(hair->pos.y_rot, hair->pos.x_rot, 0);
			phd_TranslateRel(bone[1], bone[2], bone[3]);
			(hair + 1)->pos.x_pos = (long)aMXPtr->m03;
			(hair + 1)->pos.y_pos = (long)aMXPtr->m13;
			(hair + 1)->pos.z_pos = (long)aMXPtr->m23;
			phd_PopMatrix();
			hair++;
		}

		SmokeWindZ = 0;
		SmokeWindX = 0;
		hair_wind = 0;
		hair_dwind_angle = 2048;
		hair_wind_angle = 2048;
		return;
	}

	hair->pos.x_pos = pos.x;
	hair->pos.y_pos = pos.y;
	hair->pos.z_pos = pos.z;
	room_num = LaraItem->room_number;

	if (in_cutscene)
		water = NO_HEIGHT;
	else
	{
		x = LaraItem->pos.x_pos + (frame[0] + frame[1]) / 2;;
		y = LaraItem->pos.y_pos + (frame[2] + frame[3]) / 2;
		z = LaraItem->pos.z_pos + (frame[4] + frame[5]) / 2;
		water = GetWaterHeight(x, y, z, room_num);
	}

	hair_wind += (GetRandomControl() & 7) - 3;

	if (hair_wind <= -2)
		hair_wind++;
	else if (hair_wind >= 9)
		hair_wind--;

	hair_dwind_angle = (hair_dwind_angle + 2 * (GetRandomControl() & 0x3F) - 64) & 0x1FFE;

	if (hair_dwind_angle < 1024)
		hair_dwind_angle = 2048 - hair_dwind_angle;
	else if (hair_dwind_angle > 3072)
		hair_dwind_angle += 6144 - (2 * hair_dwind_angle);

	hair_wind_angle = (hair_wind_angle + ((hair_dwind_angle - hair_wind_angle) >> 3)) & 0x1FFE;
	SmokeWindX = (hair_wind * rcossin_tbl[hair_wind_angle]) >> 12;
	SmokeWindZ = (hair_wind * rcossin_tbl[hair_wind_angle + 1]) >> 12;
	hair++;

	for (int i = 1; i < 7; i++, bone += 4)
	{
		pos.x = hair->pos.x_pos;
		pos.y = hair->pos.y_pos;
		pos.z = hair->pos.z_pos;

		if (in_cutscene)
			height = 32767;
		else
		{
			floor = GetFloor(hair->pos.x_pos, hair->pos.y_pos, hair->pos.z_pos, &room_num);
			height = GetHeight(floor, hair->pos.x_pos, hair->pos.y_pos, hair->pos.z_pos);
		}

		hair->pos.x_pos += 3 * hair->vel.x / 4;
		hair->pos.y_pos += 3 * hair->vel.y / 4;
		hair->pos.z_pos += 3 * hair->vel.z / 4;

		if (Lara.waterStatus == LW_ABOVE_WATER && room[room_num].flags & ROOM_NOT_INSIDE)
		{
			hair->pos.x_pos += SmokeWindX;
			hair->pos.z_pos += SmokeWindZ;
		}

		if (water == NO_HEIGHT || hair->pos.y_pos < water)
		{
			hair->pos.y_pos += 10;

			if (water != NO_HEIGHT && hair->pos.y_pos > water)
				hair->pos.y_pos = water;
		}

		if (hair->pos.y_pos > height)
		{
			hair->pos.x_pos = pos.x;

			if (hair->pos.y_pos - height <= 256)	//snap to floor if it goes below, no more than 1 click to avoid hairection when going through corners
				hair->pos.y_pos = height;

			hair->pos.z_pos = pos.z;
		}

		for (int j = 0; j < 6; j++)	//6 instead of 5 for new sphere
		{
			x = hair->pos.x_pos - sphere[j].x;
			y = hair->pos.y_pos - sphere[j].y;
			z = hair->pos.z_pos - sphere[j].z;
			dist = SQUARE(x) + SQUARE(y) + SQUARE(z);

			if (dist < SQUARE(sphere[j].r))
			{
				dist = phd_sqrt(dist);

				if (!dist)
					dist = 1;

				hair->pos.x_pos = sphere[j].x + x * sphere[j].r / dist;
				hair->pos.y_pos = sphere[j].y + y * sphere[j].r / dist;
				hair->pos.z_pos = sphere[j].z + z * sphere[j].r / dist;
			}
		}

		dx = (hair->pos.x_pos - (hair - 1)->pos.x_pos);
		dy = (hair->pos.y_pos - (hair - 1)->pos.y_pos);
		dz = (hair->pos.z_pos - (hair - 1)->pos.z_pos);
		dist = phd_sqrt(SQUARE(dz) + SQUARE(dx));

		if (!((cutseq_num == 17 && ((GLOBAL_cutseq_frame >= 205 && GLOBAL_cutseq_frame <= 211) || (GLOBAL_cutseq_frame >= 474 && GLOBAL_cutseq_frame <= 480))) ||
			(cutseq_num == 8 && (GLOBAL_cutseq_frame >= 840 && GLOBAL_cutseq_frame <= 846)) ||
			(cutseq_num == 27 && (GLOBAL_cutseq_frame >= 928 && GLOBAL_cutseq_frame <= 934))))	//To avoid hair jumps in cuts
		{
			(hair - 1)->pos.y_rot = (short)phd_atan(dz, dx);
			(hair - 1)->pos.x_rot = (short)-phd_atan(dist, dy);
		}

		phd_PushUnitMatrix();
		phd_SetTrans((hair - 1)->pos.x_pos, (hair - 1)->pos.y_pos, (hair - 1)->pos.z_pos);
		phd_RotYXZ((hair - 1)->pos.y_rot, (hair - 1)->pos.x_rot, 0);

		if (i == 6)
			phd_TranslateRel(bone[-3], bone[-2], bone[-1]);
		else
			phd_TranslateRel(bone[1], bone[2], bone[3]);

		hair->pos.x_pos = (long)aMXPtr->m03;
		hair->pos.y_pos = (long)aMXPtr->m13;
		hair->pos.z_pos = (long)aMXPtr->m23;

		if ((cutseq_num == 16 && ((GLOBAL_cutseq_frame >= 409 && GLOBAL_cutseq_frame < 411) || GLOBAL_cutseq_frame == 1873 || GLOBAL_cutseq_frame == 3049)) ||
			(cutseq_num == 17 && ((GLOBAL_cutseq_frame >= 205 && GLOBAL_cutseq_frame <= 211) || (GLOBAL_cutseq_frame >= 474 && GLOBAL_cutseq_frame <= 480))) ||
			(cutseq_num == 8 && (GLOBAL_cutseq_frame >= 840 && GLOBAL_cutseq_frame <= 846)) ||
			(cutseq_num == 27 && (GLOBAL_cutseq_frame >= 928 && GLOBAL_cutseq_frame <= 934)))//To avoid hair jumps in cuts
		{
			hair->vel.x = 0;
			hair->vel.y = 0;
			hair->vel.z = 0;
		}
		else
		{
			hair->vel.x = hair->pos.x_pos - pos.x;
			hair->vel.y = hair->pos.y_pos - pos.y;
			hair->vel.z = hair->pos.z_pos - pos.z;
		}

		phd_PopMatrix();
		hair++;
	}
}

void InitialiseHair()
{
	OBJECT_INFO* obj;
	HAIR_STRUCT* hptr;
	long* bone;

	for (int i = 0; i < 2; i++)
	{
		obj = &objects[HAIR];
		bone = &bones[obj->bone_index];
		bone += 4;
		hptr = &hairs[i][0];
		hptr->pos.y_rot = 0;
		hptr->pos.x_rot = -16384;
		first_hair[i] = 1;

		for (int j = 1; j < 7; j++, bone += 4)
		{
			hptr->pos.x_pos = bone[1];
			hptr->pos.y_pos = bone[2];
			hptr->pos.z_pos = bone[3];
			hptr->pos.x_rot = -16384;
			hptr->pos.y_rot = 0;
			hptr->pos.z_rot = 0;
			hptr->vel.x = 0;
			hptr->vel.y = 0;
			hptr->vel.z = 0;
			hptr++;
		}
	}
}

void GetCorrectStashPoints(long pigtail, long hair_node, long skin_node)
{
	HAIR_STRUCT* hair;
	ushort num, rot1, rot2;

	num = 0;
	hair = &hairs[pigtail][hair_node - 1];

	if (hair_node)
		rot1 = hair->pos.y_rot;
	else
		rot1 = ushort(0x8000 - (CamRot.y << 4));

	rot2 = hair[2].pos.y_rot;

	while (abs(rot1 - rot2) > 0x2000 && abs(rot1 - rot2) < 0xE000)
	{
		rot1 += 0x4000;
		num++;
	}

	for (int i = 0; i < 5; i++)
		ScratchVertNums[skin_node][i] = HairRotScratchVertNums[num][i];
}
