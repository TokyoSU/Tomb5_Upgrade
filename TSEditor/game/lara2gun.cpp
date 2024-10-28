#include "../tomb5/pch.h"
#include "lara2gun.h"
#include "larafire.h"
#include "objects.h"
#include "delstuff.h"
#include "tomb4fx.h"
#include "sound.h"
#include "../specific/function_stubs.h"
#include "gameflow.h"
#include "effect2.h"
#include "camera.h"
#include "control.h"
#include "../specific/input.h"
#include "lara.h"
#include "savegame.h"

ushort LHolster;

static PISTOL_DEF PistolTable[4] =
{
	{ LARA, 0, 0, 0, 0 },
	{ PISTOLS_ANIM, 4, 5, 13, 24 },
	{ REVOLVER_ANIM , 7, 8, 15, 29 },
	{ UZI_ANIM, 4, 5, 13, 24 }
};

static void set_arm_info(LARA_ARM* arm, long frame)
{
	PISTOL_DEF* p;
	long anim_base;

	p = &PistolTable[Lara.gunType];
	anim_base = objects[p->ObjectNum].anim_index;

	if (frame >= p->Draw1Anim)
	{
		if (frame < p->Draw2Anim)
			anim_base++;
		else if (frame < p->RecoilAnim)
			anim_base += 2;
		else
			anim_base += 3;
	}

	arm->anim_number = (short)anim_base;
	arm->frame_number = (short)frame;
	arm->frame_base = anims[anim_base].frame_ptr;
}

void ready_pistols(long weapon_type)
{
	Lara.gunStatus = LG_READY;
	Lara.target = 0;

	Lara.leftArm.x_rot = 0;
	Lara.leftArm.y_rot = 0;
	Lara.leftArm.z_rot = 0;
	Lara.leftArm.frame_number = 0;
	Lara.leftArm.lock = 0;
	Lara.leftArm.frame_base = objects[WeaponObject(weapon_type)].frame_base;

	Lara.rightArm.x_rot = 0;
	Lara.rightArm.y_rot = 0;
	Lara.rightArm.z_rot = 0;
	Lara.rightArm.frame_number = 0;
	Lara.rightArm.lock = 0;
	Lara.rightArm.frame_base = Lara.leftArm.frame_base;
}

void draw_pistol_meshes(long weapon_type)
{
	long mesh_index;

	mesh_index = objects[WeaponObjectMesh(weapon_type)].mesh_index;

	if (weapon_type != WEAPON_REVOLVER)
		LHolster = LARA_HOLSTERS;

	Lara.holster = LARA_HOLSTERS;
	Lara.meshPtrs[LM_RHAND] = meshes[mesh_index + LM_RHAND * 2];

	if (weapon_type != WEAPON_REVOLVER)
		Lara.meshPtrs[LM_LHAND] = meshes[mesh_index + LM_LHAND * 2];
}

void undraw_pistol_mesh_left(long weapon_type)
{
	WeaponObject(weapon_type);
	Lara.meshPtrs[LM_LHAND] = meshes[objects[LARA].mesh_index + LM_LHAND * 2];

	if (weapon_type == WEAPON_PISTOLS)
		LHolster = LARA_HOLSTERS_PISTOLS;
	else if (weapon_type == WEAPON_UZI)
		LHolster = LARA_HOLSTERS_UZIS;
}

void undraw_pistol_mesh_right(long weapon_type)
{
	WeaponObject(weapon_type);
	Lara.meshPtrs[LM_RHAND] = meshes[objects[LARA].mesh_index + LM_RHAND * 2];

	if (weapon_type == WEAPON_PISTOLS)
		Lara.holster = LARA_HOLSTERS_PISTOLS;
	else if (weapon_type == WEAPON_UZI)
		Lara.holster = LARA_HOLSTERS_UZIS;
	else if (weapon_type == WEAPON_REVOLVER)
		Lara.holster = LARA_HOLSTERS_REVOLVER;
}

void AnimatePistols(long weapon_type)
{
	PISTOL_DEF* p;
	WEAPON_INFO* winfo;
	PHD_VECTOR pos;
	static long uzi_left;
	static long uzi_right;
	short angles[2];
	short anil, anir, sound_already;

	sound_already = 0;

	if (LaraItem->mesh_bits)
	{
		if (SmokeCountL)
		{
			switch (SmokeWeapon)
			{
			case WEAPON_PISTOLS:
				pos.x = 4;
				pos.y = 128;
				pos.z = 40;
				break;

			case WEAPON_REVOLVER:
				pos.x = 16;
				pos.y = 160;
				pos.z = 56;
				break;

			case WEAPON_UZI:
				pos.x = 8;
				pos.y = 140;
				pos.z = 48;
				break;
			}

			GetLaraJointPos(&pos, LMX_HAND_L);
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountL);
		}

		if (SmokeCountR)
		{
			switch (SmokeWeapon)
			{
			case WEAPON_PISTOLS:
				pos.x = -16;
				pos.y = 128;
				pos.z = 40;
				break;

			case WEAPON_REVOLVER:
				pos.x = -32;
				pos.y = 160;
				pos.z = 56;
				break;

			case WEAPON_UZI:
				pos.x = -16;
				pos.y = 140;
				pos.z = 48;
				break;
			}

			GetLaraJointPos(&pos, LMX_HAND_R);
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountR);
		}
	}

	p = &PistolTable[Lara.gunType];
	winfo = &weapons[weapon_type];
	anir = Lara.rightArm.frame_number;

	if (Lara.rightArm.lock || KeyInput & IN_ACTION && !Lara.target)
	{
		if (Lara.rightArm.frame_number >= 0 && Lara.rightArm.frame_number < p->Draw1Anim2)
			anir++;
		else if (Lara.rightArm.frame_number == p->Draw1Anim2)
		{
			if (KeyInput & IN_ACTION)
			{
				if (weapon_type != WEAPON_REVOLVER)
				{
					angles[0] = Lara.rightArm.y_rot + LaraItem->pos.y_rot;
					angles[1] = Lara.rightArm.x_rot;

					if (FireWeapon(weapon_type, Lara.target, LaraItem, angles))
					{
						SmokeCountR = 28;
						SmokeWeapon = weapon_type;
						TriggerGunShell(1, GUNSHELL, weapon_type);
						Lara.rightArm.flash_gun = winfo->flash_time;
						SoundEffect(SFX_EXPLOSION1, &LaraItem->pos, 0x2000000 | SFX_SETPITCH);
						SoundEffect(winfo->sample_num, &LaraItem->pos, SFX_DEFAULT);
						sound_already = 1;

						if (weapon_type == WEAPON_UZI)
							uzi_right = 1;

						savegame.Game.AmmoUsed++;
					}
				}

				anir = p->RecoilAnim;
			}
			else if (uzi_right)
			{
				SoundEffect(winfo->sample_num + 1, &LaraItem->pos, SFX_DEFAULT);
				uzi_right = 0;
			}
		}
		else if (Lara.rightArm.frame_number >= p->RecoilAnim)
		{
			if (weapon_type == WEAPON_UZI)
			{
				SoundEffect(winfo->sample_num, &LaraItem->pos, SFX_DEFAULT);
				uzi_right = 1;
			}

			anir++;

			if (anir == p->RecoilAnim + winfo->recoil_frame)
				anir = p->Draw1Anim2;
		}
	}
	else
	{
		if (Lara.rightArm.frame_number >= p->RecoilAnim)
			anir = p->Draw1Anim2;
		else if (Lara.rightArm.frame_number > 0 && Lara.rightArm.frame_number <= p->Draw1Anim2)
			anir--;

		if (uzi_right)
		{
			SoundEffect(winfo->sample_num + 1, &LaraItem->pos, SFX_DEFAULT);
			uzi_right = 0;
		}
	}

	set_arm_info(&Lara.rightArm, anir);
	anil = Lara.leftArm.frame_number;

	if (Lara.leftArm.lock || KeyInput & IN_ACTION && !Lara.target)
	{
		if (Lara.leftArm.frame_number >= 0 && Lara.leftArm.frame_number < p->Draw1Anim2)
			anil++;
		else if (Lara.leftArm.frame_number == p->Draw1Anim2)
		{
			if (KeyInput & IN_ACTION)
			{
				angles[0] = Lara.leftArm.y_rot + LaraItem->pos.y_rot;
				angles[1] = Lara.leftArm.x_rot;

				if (FireWeapon(weapon_type, Lara.target, LaraItem, angles))
				{
					if (weapon_type == WEAPON_REVOLVER)
					{
						SmokeCountR = 28;
						SmokeWeapon = WEAPON_REVOLVER;
						Lara.rightArm.flash_gun = winfo->flash_time;
					}
					else
					{
						SmokeCountL = 28;
						SmokeWeapon = weapon_type;
						TriggerGunShell(0, GUNSHELL, weapon_type);
						Lara.leftArm.flash_gun = winfo->flash_time;
					}

					if (!sound_already)
					{
						SoundEffect(SFX_EXPLOSION1, &LaraItem->pos, 0x2000000 | SFX_SETPITCH);
						SoundEffect(winfo->sample_num, &LaraItem->pos, 0);
					}

					if (weapon_type == WEAPON_UZI)
						uzi_left = 1;

					savegame.Game.AmmoUsed++;
				}

				anil = p->RecoilAnim;
			}
			else if (uzi_left)
			{
				SoundEffect(winfo->sample_num + 1, &LaraItem->pos, SFX_DEFAULT);
				uzi_left = 0;
			}
		}
		else if (Lara.leftArm.frame_number >= p->RecoilAnim)
		{
			if (weapon_type == WEAPON_UZI)
			{
				SoundEffect(winfo->sample_num, &LaraItem->pos, SFX_DEFAULT);
				uzi_left = 1;
			}

			anil++;

			if (anil == p->RecoilAnim + winfo->recoil_frame)
				anil = p->Draw1Anim2;
		}
	}
	else
	{
		if (Lara.leftArm.frame_number >= p->RecoilAnim)
			anil = p->Draw1Anim2;
		else if (Lara.leftArm.frame_number > 0 && Lara.leftArm.frame_number <= p->Draw1Anim2)
			anil--;

		if (uzi_left)
		{
			SoundEffect(winfo->sample_num + 1, &LaraItem->pos, SFX_DEFAULT);
			uzi_left = 0;
		}
	}

	set_arm_info(&Lara.leftArm, anil);
}

void PistolHandler(long weapon_type)
{
	WEAPON_INFO* winfo;
	PHD_VECTOR pos;
	long r, g, b;

	winfo = &weapons[weapon_type];
	LaraGetNewTarget(winfo);

	if (KeyInput & IN_ACTION)
		LaraTargetInfo(winfo);

	AimWeapon(winfo, &Lara.leftArm);
	AimWeapon(winfo, &Lara.rightArm);

	if (Lara.leftArm.lock && Lara.rightArm.lock)
	{
		Lara.torsoRotY = (Lara.leftArm.y_rot + Lara.rightArm.y_rot) >> 2;
		Lara.torsoRotX = (Lara.leftArm.x_rot + Lara.rightArm.x_rot) >> 2;

		if (camera.old_type != LOOK_CAMERA)
		{
			Lara.headRotY = Lara.torsoRotY;
			Lara.headRotX = Lara.torsoRotX;
		}
	}
	else if (Lara.leftArm.lock && !Lara.rightArm.lock)
	{
		Lara.torsoRotY = Lara.leftArm.y_rot >> 1;
		Lara.torsoRotX = Lara.leftArm.x_rot >> 1;

		if (camera.old_type != LOOK_CAMERA)
		{
			Lara.headRotY = Lara.torsoRotY;
			Lara.headRotX = Lara.torsoRotX;
		}
	}
	else if (!Lara.leftArm.lock && Lara.rightArm.lock)
	{
		Lara.torsoRotY = Lara.rightArm.y_rot >> 1;
		Lara.torsoRotX = Lara.rightArm.x_rot >> 1;

		if (camera.old_type != LOOK_CAMERA)
		{
			Lara.headRotY = Lara.torsoRotY;
			Lara.headRotX = Lara.torsoRotX;
		}
	}

	AnimatePistols(weapon_type);

	if (Lara.leftArm.flash_gun || Lara.rightArm.flash_gun)
	{
		pos.x = (GetRandomControl() & 0xFF) - 128;
		pos.y = (GetRandomControl() & 0x7F) - 63;
		pos.z = (GetRandomControl() & 0xFF) - 128;

		if (Lara.leftArm.flash_gun)
			GetLaraJointPos(&pos, LMX_HAND_R);
		else
			GetLaraJointPos(&pos, LMX_HAND_L);

		r = (GetRandomControl() & 0x3F) + 192;
		g = (GetRandomControl() & 0x1F) + 128;
		b = GetRandomControl() & 0x3F;

		if (gfLevelFlags & GF_MIRROR && LaraItem->room_number == gfMirrorRoom)
			TriggerDynamic_MIRROR(pos.x, pos.y, pos.z, 10, r, g, b);
		else
			TriggerDynamic(pos.x, pos.y, pos.z, 10, r, g, b);
	}
}

void draw_pistols(long weapon_type)
{
	PISTOL_DEF* p;
	short ani;

	ani = Lara.leftArm.frame_number + 1;
	p = &PistolTable[Lara.gunType];

	if (ani < PistolTable[Lara.gunType].Draw1Anim || ani > p->RecoilAnim - 1)
		ani = PistolTable[Lara.gunType].Draw1Anim;
	else if (ani == p->Draw2Anim)
	{
		draw_pistol_meshes(weapon_type);
		SoundEffect(SFX_LARA_HOLSTER_DRAW, &LaraItem->pos, SFX_DEFAULT);
	}
	else if (ani == p->RecoilAnim - 1)
	{
		ready_pistols(weapon_type);
		ani = 0;
	}

	set_arm_info(&Lara.rightArm, ani);
	set_arm_info(&Lara.leftArm, ani);
}

void undraw_pistols(long weapon_type)
{
	PISTOL_DEF* p;
	short anil, anir;

	p = &PistolTable[Lara.gunType];
	anil = Lara.leftArm.frame_number;

	if (Lara.leftArm.frame_number >= PistolTable[Lara.gunType].RecoilAnim)
		anil = p->Draw1Anim2;
	else if (Lara.leftArm.frame_number > 0 && Lara.leftArm.frame_number < p->Draw1Anim)
	{
		Lara.leftArm.x_rot -= Lara.leftArm.x_rot / anil;
		Lara.leftArm.y_rot -= Lara.leftArm.y_rot / anil;
		anil--;
	}
	else if (!Lara.leftArm.frame_number)
	{
		Lara.leftArm.x_rot = 0;
		Lara.leftArm.y_rot = 0;
		Lara.leftArm.z_rot = 0;
		anil = p->RecoilAnim - 1;
	}
	else if (Lara.leftArm.frame_number > p->Draw1Anim)
	{
		anil--;

		if (anil == p->Draw2Anim - 1)
		{
			undraw_pistol_mesh_left(weapon_type);
			SoundEffect(SFX_LARA_HOLSTER_AWAY, &LaraItem->pos, SFX_DEFAULT);
		}
	}

	set_arm_info(&Lara.leftArm, anil);
	anir = Lara.rightArm.frame_number;

	if (Lara.rightArm.frame_number >= p->RecoilAnim)
		anir = p->Draw1Anim2;
	else if (Lara.rightArm.frame_number > 0 && Lara.rightArm.frame_number < p->Draw1Anim)
	{
		Lara.rightArm.x_rot -= Lara.rightArm.x_rot / anir;
		Lara.rightArm.y_rot -= Lara.rightArm.y_rot / anir;
		anir--;
	}
	else if (!Lara.rightArm.frame_number)
	{
		Lara.rightArm.z_rot = 0;
		Lara.rightArm.y_rot = 0;
		Lara.rightArm.x_rot = 0;
		anir = p->RecoilAnim - 1;
	}
	else if (Lara.rightArm.frame_number > p->Draw1Anim)
	{
		anir--;

		if (anir == p->Draw2Anim - 1)
		{
			undraw_pistol_mesh_right(weapon_type);
			SoundEffect(SFX_LARA_HOLSTER_AWAY, &LaraItem->pos, SFX_DEFAULT);
		}
	}

	set_arm_info(&Lara.rightArm, anir);

	if (anil == p->Draw1Anim && anir == p->Draw1Anim)
	{
		Lara.gunStatus = LG_NO_ARMS;
		Lara.leftArm.frame_number = 0;
		Lara.rightArm.frame_number = 0;
		Lara.target = 0;
		Lara.rightArm.lock = 0;
		Lara.leftArm.lock = 0;
	}

	if (!(KeyInput & IN_LOOK))
	{
		Lara.headRotX = (Lara.leftArm.x_rot + Lara.rightArm.x_rot) >> 2;
		Lara.torsoRotX = Lara.headRotX;
		Lara.headRotY = (Lara.leftArm.y_rot + Lara.rightArm.y_rot) >> 2;
		Lara.torsoRotY = Lara.headRotY;
	}
}
