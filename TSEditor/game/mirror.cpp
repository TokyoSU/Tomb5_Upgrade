#include "../tomb5/pch.h"
#include "mirror.h"
#include "delstuff.h"
#include "../specific/function_table.h"
#include "../specific/drawlara.h"
#include "tomb4fx.h"
#include "../specific/3dmath.h"
#include "gameflow.h"
#include "hair.h"
#include "lara.h"

void Draw_Mirror_Lara()
{
	HAIR_STRUCT* hair;
	GUNSHELL_STRUCT* gunshell;
	long zplane;
	
	zplane = gfMirrorZPlane << 1;
	hair = &hairs[0][0];

	for (int i = 0; i < 6; i++)
	{
		hair->pos.z_pos = zplane - hair->pos.z_pos;
		hair->pos.y_rot = 0x8000 - hair->pos.y_rot;
		hair++;
	}

	for (int i = 0; i < 24; i++)
	{
		gunshell = &Gunshells[i];

		if (gunshell->counter)
			gunshell->pos.z_pos = zplane - gunshell->pos.z_pos;
	}

	LaraItem->pos.z_pos = zplane - LaraItem->pos.z_pos;
	LaraItem->pos.x_rot = -LaraItem->pos.x_rot;
	LaraItem->pos.y_rot = -LaraItem->pos.y_rot;
	LaraItem->pos.z_rot += 0x8000;
	CalcLaraMatrices(2);
	SetCullCW();
	DrawLara(LaraItem, 1);
	DrawGunshells();
	SetCullCCW();
	LaraItem->pos.z_pos = zplane - LaraItem->pos.z_pos;
	LaraItem->pos.x_rot = -LaraItem->pos.x_rot;
	LaraItem->pos.y_rot = -LaraItem->pos.y_rot;
	LaraItem->pos.z_rot += 0x8000;
	phd_PushMatrix();

	if (Lara.right_arm.flash_gun)
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
		SetGunFlash(Lara.gun_type);
	}

	if (Lara.left_arm.flash_gun)
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
		SetGunFlash(Lara.gun_type);
	}

	phd_PopMatrix();

	for (int i = 0; i < 24; i++)
	{
		gunshell = &Gunshells[i];

		if (gunshell->counter)
			gunshell->pos.z_pos = zplane - gunshell->pos.z_pos;
	}

	hair = &hairs[0][0];

	for (int i = 0; i < 6; i++)
	{
		hair->pos.z_pos = zplane - hair->pos.z_pos;
		hair->pos.y_rot = 0x8000 - hair->pos.y_rot;
		hair++;
	}
}
