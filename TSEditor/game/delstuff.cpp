#include "../tomb5/pch.h"
#include "delstuff.h"
#include "../specific/3dmath.h"
#include "draw.h"
#include "lara_states.h"
#include "control.h"
#include "objects.h"
#include "lara.h"
#include "../tomb5/tomb5.h"

short* GLaraShadowframe;
float lara_matrices[180];
float lara_joint_matrices[180];
long LaraNodeAmbient[2];
uchar LaraNodeUnderwater[15];
char SkinVertNums[40][12];
char ScratchVertNums[40][12];

void GetLaraJointPos(PHD_VECTOR* pos, long node)
{
	phd_PushMatrix();
	aMXPtr->m00 = lara_joint_matrices[node * indices_count + M00];
	aMXPtr->m01 = lara_joint_matrices[node * indices_count + M01];
	aMXPtr->m02 = lara_joint_matrices[node * indices_count + M02];
	aMXPtr->m03 = lara_joint_matrices[node * indices_count + M03];
	aMXPtr->m10 = lara_joint_matrices[node * indices_count + M10];
	aMXPtr->m11 = lara_joint_matrices[node * indices_count + M11];
	aMXPtr->m12 = lara_joint_matrices[node * indices_count + M12];
	aMXPtr->m13 = lara_joint_matrices[node * indices_count + M13];
	aMXPtr->m20 = lara_joint_matrices[node * indices_count + M20];
	aMXPtr->m21 = lara_joint_matrices[node * indices_count + M21];
	aMXPtr->m22 = lara_joint_matrices[node * indices_count + M22];
	aMXPtr->m23 = lara_joint_matrices[node * indices_count + M23];
	phd_TranslateRel(pos->x, pos->y, pos->z);
	pos->x = (long)aMXPtr->m03;
	pos->y = (long)aMXPtr->m13;
	pos->z = (long)aMXPtr->m23;
	pos->x += LaraItem->pos.x_pos;
	pos->y += LaraItem->pos.y_pos;
	pos->z += LaraItem->pos.z_pos;
	phd_PopMatrix();
}

void CalcLaraMatrices(long flag)
{
	long* bone;
	short* frame;
	short* frmptr[2];
	long rate, frac;
	short spaz;
	
	bone = &bones[objects[LaraItem->object_number].bone_index];
	frac = GetFrames(LaraItem, frmptr, &rate);

	if (Lara.hitDirection < 0)
	{
		if (frac)
		{
			GLaraShadowframe = GetBoundsAccurate(LaraItem);
			Rich_CalcLaraMatrices_Interpolated(frmptr[0], frmptr[1], frac, rate, bone, flag);
			return;
		}
	}

	if (Lara.hitDirection < 0)
		frame = *frmptr;
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

		frame = &anims[spaz].frame_ptr[Lara.hitFrame * (anims[spaz].interpolation >> 8)];
	}

	Rich_CalcLaraMatrices_Normal(frame, bone, flag);
}

void Rich_CalcLaraMatrices_Normal(short* frame, long* bone, long flag)
{
	PHD_VECTOR vec;
	float* mx;
	short* rot;
	short* rot2;
	short gun;

	if (flag == 1)
		mx = lara_joint_matrices;
	else
		mx = lara_matrices;

	phd_PushMatrix();

	if (!flag || flag == 2)
		phd_TranslateAbs(LaraItem->pos.x_pos, LaraItem->pos.y_pos, LaraItem->pos.z_pos);
	else
		phd_SetTrans(0, 0, 0);

	phd_RotYXZ(LaraItem->pos.y_rot, LaraItem->pos.x_rot, LaraItem->pos.z_rot);

	if (flag == 2)
	{
		vec.z = -16384;
		vec.y = -16384;
		vec.x = -16384;
		ScaleCurrentMatrix(&vec);
	}

	phd_PushMatrix();
	rot = &frame[9];
	phd_TranslateRel(frame[6], frame[7], frame[8]);
	gar_RotYXZsuperpack(&rot, 0);
	memcpy(mx, aMXPtr, 48);
	mx += 12;

	phd_PushMatrix();
	phd_TranslateRel(bone[1], bone[2], bone[3]);
	gar_RotYXZsuperpack(&rot, 0);
	memcpy(mx, aMXPtr, 48);
	mx += 12;

	phd_TranslateRel(bone[5], bone[6], bone[7]);
	gar_RotYXZsuperpack(&rot, 0);
	memcpy(mx, aMXPtr, 48);
	mx += 12;

	phd_TranslateRel(bone[9], bone[10], bone[11]);
	gar_RotYXZsuperpack(&rot, 0);
	memcpy(mx, aMXPtr, 48);
	mx += 12;
	phd_PopMatrix();

	phd_PushMatrix();
	phd_TranslateRel(bone[13], bone[14], bone[15]);
	gar_RotYXZsuperpack(&rot, 0);
	memcpy(mx, aMXPtr, 48);
	mx += 12;

	phd_TranslateRel(bone[17], bone[18], bone[19]);
	gar_RotYXZsuperpack(&rot, 0);
	memcpy(mx, aMXPtr, 48);
	mx += 12;

	phd_TranslateRel(bone[21], bone[22], bone[23]);
	gar_RotYXZsuperpack(&rot, 0);
	memcpy(mx, aMXPtr, 48);
	mx += 12;
	phd_PopMatrix();

	phd_TranslateRel(bone[25], bone[26], bone[27]);

	if (Lara.weaponItem != NO_ITEM && Lara.gunType == WEAPON_HK &&
		(items[Lara.weaponItem].current_anim_state == 0 || items[Lara.weaponItem].current_anim_state == 1 ||
			items[Lara.weaponItem].current_anim_state == 2 || items[Lara.weaponItem].current_anim_state == 3 ||
			items[Lara.weaponItem].current_anim_state == 4))
	{
		rot = &Lara.rightArm.frame_base[Lara.rightArm.frame_number * (anims[Lara.rightArm.anim_number].interpolation >> 8) + 9];
		gar_RotYXZsuperpack(&rot, 7);
	}
	else
		gar_RotYXZsuperpack(&rot, 0);

	phd_RotYXZ(Lara.torsoRotY, Lara.torsoRotX, Lara.torsoRotZ);
	memcpy(mx, aMXPtr, 48);
	mx += 12;

	phd_PushMatrix();
	phd_TranslateRel(bone[53], bone[54], bone[55]);
	rot2 = rot;
	gar_RotYXZsuperpack(&rot2, 6);
	phd_RotYXZ(Lara.headRotY, Lara.headRotX, Lara.headRotZ);
	memcpy(mx, aMXPtr, 48);
	mx += 12;
	phd_PopMatrix();

	gun = WEAPON_NONE;

	if (Lara.gunStatus == LG_READY || Lara.gunStatus == LG_FLARE || Lara.gunStatus == LG_DRAW_GUNS || Lara.gunStatus == LG_UNDRAW_GUNS)
		gun = Lara.gunType;

	switch (gun)
	{
	case WEAPON_NONE:
	case WEAPON_FLARE:
	case WEAPON_TORCH:
		phd_PushMatrix();
		phd_TranslateRel(bone[29], bone[30], bone[31]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[33], bone[34], bone[35]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[37], bone[38], bone[39]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();

		phd_PushMatrix();
		phd_TranslateRel(bone[41], bone[42], bone[43]);

		if (Lara.flareControlLeft)
		{
			rot = &Lara.leftArm.frame_base[(anims[Lara.leftArm.anim_number].interpolation >> 8) * 
				(Lara.leftArm.frame_number - anims[Lara.leftArm.anim_number].frame_base) + 9];

			gar_RotYXZsuperpack(&rot, 11);
		}
		else
			gar_RotYXZsuperpack(&rot, 0);

		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[45], bone[46], bone[47]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[49], bone[50], bone[51]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		break;

	case WEAPON_PISTOLS:
	case WEAPON_UZI:
		phd_PushMatrix();
		phd_TranslateRel(bone[29], bone[30], bone[31]);

		aMXPtr->m00 = aMXPtr[-2].m00;	//going back 2 matrices
		aMXPtr->m01 = aMXPtr[-2].m01;
		aMXPtr->m02 = aMXPtr[-2].m02;
		aMXPtr->m10 = aMXPtr[-2].m10;
		aMXPtr->m11 = aMXPtr[-2].m11;
		aMXPtr->m12 = aMXPtr[-2].m12;
		aMXPtr->m20 = aMXPtr[-2].m20;
		aMXPtr->m21 = aMXPtr[-2].m21;
		aMXPtr->m22 = aMXPtr[-2].m22;

		phd_RotYXZ(Lara.rightArm.y_rot, Lara.rightArm.x_rot, Lara.rightArm.z_rot);
		rot = &Lara.rightArm.frame_base[(anims[Lara.rightArm.anim_number].interpolation >> 8) * 
			(Lara.rightArm.frame_number - anims[Lara.rightArm.anim_number].frame_base) + 9];
		gar_RotYXZsuperpack(&rot, 8);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[33], bone[34], bone[35]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[37], bone[38], bone[39]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();

		phd_PushMatrix();
		phd_TranslateRel(bone[41], bone[42], bone[43]);

		aMXPtr->m00 = aMXPtr[-2].m00;	//going back 2 matrices
		aMXPtr->m01 = aMXPtr[-2].m01;
		aMXPtr->m02 = aMXPtr[-2].m02;
		aMXPtr->m10 = aMXPtr[-2].m10;
		aMXPtr->m11 = aMXPtr[-2].m11;
		aMXPtr->m12 = aMXPtr[-2].m12;
		aMXPtr->m20 = aMXPtr[-2].m20;
		aMXPtr->m21 = aMXPtr[-2].m21;
		aMXPtr->m22 = aMXPtr[-2].m22;

		phd_RotYXZ(Lara.leftArm.y_rot, Lara.leftArm.x_rot, Lara.leftArm.z_rot);
		rot = &Lara.leftArm.frame_base[(anims[Lara.leftArm.anim_number].interpolation >> 8) * 
			(Lara.leftArm.frame_number - anims[Lara.leftArm.anim_number].frame_base) + 9];
		gar_RotYXZsuperpack(&rot, 11);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[45], bone[46], bone[47]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[49], bone[50], bone[51]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		break;

	case WEAPON_REVOLVER:
		phd_PushMatrix();
		phd_TranslateRel(bone[29], bone[30], bone[31]);

		aMXPtr->m00 = aMXPtr[-2].m00;	//going back 2 matrices
		aMXPtr->m01 = aMXPtr[-2].m01;
		aMXPtr->m02 = aMXPtr[-2].m02;
		aMXPtr->m10 = aMXPtr[-2].m10;
		aMXPtr->m11 = aMXPtr[-2].m11;
		aMXPtr->m12 = aMXPtr[-2].m12;
		aMXPtr->m20 = aMXPtr[-2].m20;
		aMXPtr->m21 = aMXPtr[-2].m21;
		aMXPtr->m22 = aMXPtr[-2].m22;

		phd_RotYXZ(Lara.torsoRotY, Lara.torsoRotX, Lara.torsoRotZ);
		rot = &Lara.rightArm.frame_base[(anims[Lara.rightArm.anim_number].interpolation >> 8) * 
			(Lara.rightArm.frame_number - anims[Lara.rightArm.anim_number].frame_base) + 9];
		gar_RotYXZsuperpack(&rot, 8);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[33], bone[34], bone[35]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[37], bone[38], bone[39]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();

		phd_PushMatrix();
		phd_TranslateRel(bone[41], bone[42], bone[43]);

		aMXPtr->m00 = aMXPtr[-2].m00;	//going back 2 matrices
		aMXPtr->m01 = aMXPtr[-2].m01;
		aMXPtr->m02 = aMXPtr[-2].m02;
		aMXPtr->m10 = aMXPtr[-2].m10;
		aMXPtr->m11 = aMXPtr[-2].m11;
		aMXPtr->m12 = aMXPtr[-2].m12;
		aMXPtr->m20 = aMXPtr[-2].m20;
		aMXPtr->m21 = aMXPtr[-2].m21;
		aMXPtr->m22 = aMXPtr[-2].m22;

		phd_RotYXZ(Lara.torsoRotY, Lara.torsoRotX, Lara.torsoRotZ);
		rot = &Lara.leftArm.frame_base[(anims[Lara.leftArm.anim_number].interpolation >> 8) * 
			(Lara.leftArm.frame_number - anims[Lara.leftArm.anim_number].frame_base) + 9];
		gar_RotYXZsuperpack(&rot, 11);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[45], bone[46], bone[47]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[49], bone[50], bone[51]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		break;

	case WEAPON_SHOTGUN:
	case WEAPON_HK:
	case WEAPON_CROSSBOW:
		phd_PushMatrix();
		phd_TranslateRel(bone[29], bone[30], bone[31]);
		rot = &Lara.rightArm.frame_base[Lara.rightArm.frame_number * 
			(anims[Lara.rightArm.anim_number].interpolation >> 8) + 9];
		gar_RotYXZsuperpack(&rot, 8);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[33], bone[34], bone[35]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[37], bone[38], bone[39]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();

		phd_PushMatrix();
		phd_TranslateRel(bone[41], bone[42], bone[43]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[45], bone[46], bone[47]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[49], bone[50], bone[51]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		break;
	}
	
	phd_PopMatrix();
	phd_PopMatrix();
	phd_PopMatrix();
	GLaraShadowframe = frame;
}

void Rich_CalcLaraMatrices_Interpolated(short* frame1, short* frame2, long frac, long rate, long* bone, long flag)
{
	PHD_VECTOR vec;
	float* mx;
	FMatrix* arms;
	short* rot;
	short* rot2;
	short* rotcopy;
	short* rot2copy;
	short gun;

	if (flag == 1)
		mx = lara_joint_matrices;
	else
		mx = lara_matrices;

	phd_PushMatrix();
	arms = aMXPtr;

	if (!flag || flag == 2)
		phd_TranslateAbs(LaraItem->pos.x_pos, LaraItem->pos.y_pos, LaraItem->pos.z_pos);
	else
		phd_SetTrans(0, 0, 0);

	phd_RotYXZ(LaraItem->pos.y_rot, LaraItem->pos.x_rot, LaraItem->pos.z_rot);

	if (flag == 2)
	{
		vec.z = -0x4000;
		vec.y = -0x4000;
		vec.x = -0x4000;
		ScaleCurrentMatrix(&vec);
	}

	phd_PushMatrix();
	rot = frame1 + 9;
	rot2 = frame2 + 9;
	InitInterpolate(frac, rate);
	phd_TranslateRel_ID(frame1[6], frame1[7], frame1[8], frame2[6], frame2[7], frame2[8]);
	gar_RotYXZsuperpack_I(&rot, &rot2, 0);

	phd_PushMatrix();
	InterpolateMatrix();
	memcpy(mx, aMXPtr, 48);
	mx += 12;
	phd_PopMatrix();

	phd_PushMatrix_I();
	phd_TranslateRel_I(bone[1], bone[2], bone[3]);
	gar_RotYXZsuperpack_I(&rot, &rot2, 0);
	phd_PushMatrix();
	InterpolateMatrix();
	memcpy(mx, aMXPtr, 48);
	mx += 12;
	phd_PopMatrix();

	phd_TranslateRel_I(bone[5], bone[6], bone[7]);
	gar_RotYXZsuperpack_I(&rot, &rot2, 0);
	phd_PushMatrix();
	InterpolateMatrix();
	memcpy(mx, aMXPtr, 48);
	mx += 12;
	phd_PopMatrix();

	phd_TranslateRel_I(bone[9], bone[10], bone[11]);
	gar_RotYXZsuperpack_I(&rot, &rot2, 0);
	phd_PushMatrix();
	InterpolateMatrix();
	memcpy(mx, aMXPtr, 48);
	mx += 12;
	phd_PopMatrix();
	phd_PopMatrix_I();

	phd_PushMatrix_I();
	phd_TranslateRel_I(bone[13], bone[14], bone[15]);
	gar_RotYXZsuperpack_I(&rot, &rot2, 0);
	phd_PushMatrix();
	InterpolateMatrix();
	memcpy(mx, aMXPtr, 48);
	mx += 12;
	phd_PopMatrix();

	phd_TranslateRel_I(bone[17], bone[18], bone[19]);
	gar_RotYXZsuperpack_I(&rot, &rot2, 0);
	phd_PushMatrix();
	InterpolateMatrix();
	memcpy(mx, aMXPtr, 48);
	mx += 12;
	phd_PopMatrix();

	phd_TranslateRel_I(bone[21], bone[22], bone[23]);
	gar_RotYXZsuperpack_I(&rot, &rot2, 0);
	phd_PushMatrix();
	InterpolateMatrix();
	memcpy(mx, aMXPtr, 48);
	mx += 12;
	phd_PopMatrix();
	phd_PopMatrix_I();

	phd_TranslateRel_I(bone[25], bone[26], bone[27]);

	if (Lara.weaponItem != NO_ITEM && Lara.gunType == WEAPON_HK &&
		(items[Lara.weaponItem].current_anim_state == 0 || items[Lara.weaponItem].current_anim_state == 1 ||
			items[Lara.weaponItem].current_anim_state == 2 || items[Lara.weaponItem].current_anim_state == 3 ||
			items[Lara.weaponItem].current_anim_state == 4))
	{
		rot = &Lara.rightArm.frame_base[Lara.rightArm.frame_number * (anims[Lara.rightArm.anim_number].interpolation >> 8) + 9];
		rot2 = rot;
		gar_RotYXZsuperpack_I(&rot, &rot2, 7);
	}
	else
		gar_RotYXZsuperpack_I(&rot, &rot2, 0);

	phd_RotYXZ_I(Lara.torsoRotY, Lara.torsoRotX, Lara.torsoRotZ);
	phd_PushMatrix();
	InterpolateMatrix();
	memcpy(mx, aMXPtr, 48);
	mx += 12;
	phd_PopMatrix();

	phd_PushMatrix_I();
	phd_TranslateRel_I(bone[53], bone[54], bone[55]);
	rotcopy = rot;
	rot2copy = rot2;
	gar_RotYXZsuperpack_I(&rotcopy, &rot2copy, 6);
	phd_RotYXZ_I(Lara.headRotY, Lara.headRotX, Lara.headRotZ);
	phd_PushMatrix();
	InterpolateMatrix();
	memcpy(mx, aMXPtr, 48);
	mx += 12;
	phd_PopMatrix();

	phd_PopMatrix_I();
	gun = WEAPON_NONE;

	if (Lara.gunStatus == LG_READY || Lara.gunStatus == LG_FLARE || Lara.gunStatus == LG_DRAW_GUNS || Lara.gunStatus == LG_UNDRAW_GUNS)
		gun = Lara.gunType;

	switch (gun)
	{
	case WEAPON_NONE:
	case WEAPON_FLARE:
	case WEAPON_TORCH:
		phd_PushMatrix_I();
		phd_TranslateRel_I(bone[29], bone[30], bone[31]);
		gar_RotYXZsuperpack_I(&rot, &rot2, 0);
		phd_PushMatrix();
		InterpolateMatrix();
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();

		phd_TranslateRel_I(bone[33], bone[34], bone[35]);
		gar_RotYXZsuperpack_I(&rot, &rot2, 0);
		phd_PushMatrix();
		InterpolateMatrix();
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();

		phd_TranslateRel_I(bone[37], bone[38], bone[39]);
		gar_RotYXZsuperpack_I(&rot, &rot2, 0);
		phd_PushMatrix();
		InterpolateMatrix();
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();
		phd_PopMatrix_I();

		phd_PushMatrix_I();
		phd_TranslateRel_I(bone[41], bone[42], bone[43]);

		if (Lara.flareControlLeft)
		{
			rot2 = &Lara.leftArm.frame_base[(anims[Lara.leftArm.anim_number].interpolation >> 8) * 
				(Lara.leftArm.frame_number - anims[Lara.leftArm.anim_number].frame_base) + 9];
			rot = rot2;
			gar_RotYXZsuperpack_I(&rot, &rot2, 11);
		}
		else
			gar_RotYXZsuperpack_I(&rot, &rot2, 0);

		phd_PushMatrix();
		InterpolateMatrix();
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();

		phd_TranslateRel_I(bone[45], bone[46], bone[47]);
		gar_RotYXZsuperpack_I(&rot, &rot2, 0);
		phd_PushMatrix();
		InterpolateMatrix();
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();

		phd_TranslateRel_I(bone[49], bone[50], bone[51]);
		gar_RotYXZsuperpack_I(&rot, &rot2, 0);
		phd_PushMatrix();
		InterpolateMatrix();
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();
		break;

	case WEAPON_PISTOLS:
	case WEAPON_UZI:
		phd_PushMatrix_I();
		phd_TranslateRel_I(bone[29], bone[30], bone[31]);
		aInterpolateArmMatrix(arms);
		phd_RotYXZ(Lara.rightArm.y_rot, Lara.rightArm.x_rot, Lara.rightArm.z_rot);
		rot = &Lara.rightArm.frame_base[(anims[Lara.rightArm.anim_number].interpolation >> 8) * (Lara.rightArm.frame_number - anims[Lara.rightArm.anim_number].frame_base) + 9];
		gar_RotYXZsuperpack(&rot, 8);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[33], bone[34], bone[35]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[37], bone[38], bone[39]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix_I();

		phd_PushMatrix_I();
		phd_TranslateRel_I(bone[41], bone[42], bone[43]);
		aInterpolateArmMatrix(arms);
		phd_RotYXZ(Lara.leftArm.y_rot, Lara.leftArm.x_rot, Lara.leftArm.z_rot);
		rot = &Lara.leftArm.frame_base[(anims[Lara.leftArm.anim_number].interpolation >> 8) * 
			(Lara.leftArm.frame_number - anims[Lara.leftArm.anim_number].frame_base) + 9];
		gar_RotYXZsuperpack(&rot, 11);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[45], bone[46], bone[47]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[49], bone[50], bone[51]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		break;

	case WEAPON_REVOLVER:
		phd_PushMatrix_I();
		phd_TranslateRel_I(bone[29], bone[30], bone[31]);
		aInterpolateArmMatrix(arms);
		phd_RotYXZ_I(Lara.torsoRotY, Lara.torsoRotX, Lara.torsoRotZ);
		rot = &Lara.rightArm.frame_base[(anims[Lara.rightArm.anim_number].interpolation >> 8) * 
			(Lara.rightArm.frame_number - anims[Lara.rightArm.anim_number].frame_base) + 9];
		gar_RotYXZsuperpack(&rot, 8);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[33], bone[34], bone[35]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[37], bone[38], bone[39]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix_I();

		phd_PushMatrix_I();
		phd_TranslateRel_I(bone[41], bone[42], bone[43]);
		aInterpolateArmMatrix(arms);
		phd_RotYXZ_I(Lara.torsoRotY, Lara.torsoRotX, Lara.torsoRotZ);
		rot = &Lara.leftArm.frame_base[(anims[Lara.leftArm.anim_number].interpolation >> 8) * 
			(Lara.leftArm.frame_number - anims[Lara.leftArm.anim_number].frame_base) + 9];
		gar_RotYXZsuperpack(&rot, 11);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[45], bone[46], bone[47]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;

		phd_TranslateRel(bone[49], bone[50], bone[51]);
		gar_RotYXZsuperpack(&rot, 0);
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		break;

	case WEAPON_SHOTGUN:
	case WEAPON_HK:
	case WEAPON_CROSSBOW:
		phd_PushMatrix_I();
		phd_TranslateRel_I(bone[29], bone[30], bone[31]);
		rot2 = &Lara.rightArm.frame_base[Lara.rightArm.frame_number * 
			(anims[Lara.rightArm.anim_number].interpolation >> 8) + 9];
		rot = rot2;
		gar_RotYXZsuperpack_I(&rot, &rot2, 8);
		phd_PushMatrix();
		InterpolateMatrix();
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();

		phd_TranslateRel_I(bone[33], bone[34], bone[35]);
		gar_RotYXZsuperpack_I(&rot, &rot2, 0);
		phd_PushMatrix();
		InterpolateMatrix();
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();

		phd_TranslateRel_I(bone[37], bone[38], bone[39]);
		gar_RotYXZsuperpack_I(&rot, &rot2, 0);
		phd_PushMatrix();
		InterpolateMatrix();
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();
		phd_PopMatrix_I();

		phd_PushMatrix_I();
		phd_TranslateRel_I(bone[41], bone[42], bone[43]);
		gar_RotYXZsuperpack_I(&rot, &rot2, 0);
		phd_PushMatrix();
		InterpolateMatrix();
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();

		phd_TranslateRel_I(bone[45], bone[46], bone[47]);
		gar_RotYXZsuperpack_I(&rot, &rot2, 0);
		phd_PushMatrix();
		InterpolateMatrix();
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();

		phd_TranslateRel_I(bone[49], bone[50], bone[51]);
		gar_RotYXZsuperpack_I(&rot, &rot2, 0);
		phd_PushMatrix();
		InterpolateMatrix();
		memcpy(mx, aMXPtr, 48);
		mx += 12;
		phd_PopMatrix();
		break;
	}

	phd_PopMatrix();
	phd_PopMatrix();
	phd_PopMatrix();
}
