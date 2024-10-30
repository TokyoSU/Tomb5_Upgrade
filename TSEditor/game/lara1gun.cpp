#include "../tomb5/pch.h"
#include "delstuff.h"
#include "lara1gun.h"
#include "lara2gun.h"
#include "larafire.h"
#include "gameflow.h"
#include "effects.h"
#include "effect2.h"
#include "control.h"
#include "sound.h"
#include "items.h"
#include "sound.h"
#include "tomb4fx.h"
#include "traps.h"
#include "box.h"
#include "collide.h"
#include "switch.h"
#include "sphere.h"
#include "objects.h"
#include "../specific/function_stubs.h"
#include "../specific/3dmath.h"
#include "camera.h"
#include "../specific/input.h"
#include "lara.h"
#include "savegame.h"

static char HKTimer = 0;
static char HKShotsFired = 0;

void draw_shotgun_meshes(long weapon_type)
{
	Lara.weaponBackItem = 0;
	Lara.meshPtrs[LM_RHAND] = meshes[objects[WeaponObjectMesh(weapon_type)].mesh_index + 2 * LM_RHAND];
}

void undraw_shotgun_meshes(long weapon_type)
{
	Lara.weaponBackItem = (short)WeaponObject(weapon_type);
	Lara.meshPtrs[LM_RHAND] = meshes[objects[LARA].mesh_index + 2 * LM_RHAND];
}

void ready_shotgun(long weapon_type)
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

void RifleHandler(long weapon_type)
{
	WEAPON_INFO* winfo;
	PHD_VECTOR pos;
	long r, g, b;

	winfo = &weapons[weapon_type];

	if (Lara.gunType != WEAPON_CROSSBOW || LaserSight)
		LaraGetNewTarget(winfo);
	else
		Lara.target = 0;

	if (KeyInput & IN_ACTION)
		LaraTargetInfo(winfo);

	AimWeapon(winfo, &Lara.leftArm);

	if (Lara.leftArm.lock)
	{
		Lara.torsoRotX = Lara.leftArm.x_rot;
		Lara.torsoRotY = Lara.leftArm.y_rot;

		if (camera.old_type != LOOK_CAMERA && !BinocularRange)
		{
			Lara.headRotY = 0;
			Lara.headRotX = 0;
		}
	}

	if (weapon_type == WEAPON_REVOLVER)
		AnimatePistols(WEAPON_REVOLVER);
	else
		AnimateShotgun(weapon_type);

	if (Lara.rightArm.flash_gun)
	{
		r = (GetRandomControl() & 0x3F) + 192;
		g = (GetRandomControl() & 0x1F) + 128;
		b = GetRandomControl() & 0x3F;

		if (weapon_type == WEAPON_SHOTGUN || weapon_type == WEAPON_HK)
		{
			pos.x = LaraItem->pos.x_pos + (1024 * phd_sin(LaraItem->pos.y_rot) >> W2V_SHIFT) + (GetRandomControl() & 0xFF);
			pos.y = LaraItem->pos.y_pos + ((GetRandomControl() & 0x7F) - 575);
			pos.z = LaraItem->pos.z_pos + (1024 * phd_cos(LaraItem->pos.y_rot) >> W2V_SHIFT) + (GetRandomControl() & 0xFF);

			if (gfLevelFlags & GF_MIRROR && LaraItem->room_number == gfMirrorRoom)
				TriggerDynamic_MIRROR(pos.x, pos.y, pos.z, 12, r, g, b);
			else
				TriggerDynamic(pos.x, pos.y, pos.z, 12, r, g, b);
		}
		else if (weapon_type == WEAPON_REVOLVER)
		{
			pos.x = (GetRandomControl() & 0xFF) - 128;
			pos.y = (GetRandomControl() & 0x7F) - 63;
			pos.z = (GetRandomControl() & 0xFF) - 128;
			GetLaraJointPos(&pos, LMX_HAND_R);

			if (gfLevelFlags & GF_MIRROR && LaraItem->room_number == gfMirrorRoom)
				TriggerDynamic_MIRROR(pos.x, pos.y, pos.z, 12, r, g, b);
			else
				TriggerDynamic(pos.x, pos.y, pos.z, 12, r, g, b);
		}
	}
}

void FireShotgun()
{
	PHD_VECTOR pos;
	PHD_VECTOR pos2;
	long fired, scatter;
	short angles[2];
	short dangles[2];

	angles[0] = Lara.leftArm.y_rot + LaraItem->pos.y_rot;
	angles[1] = Lara.leftArm.x_rot;

	if (!Lara.leftArm.lock)
	{
		angles[0] += Lara.torsoRotY;
		angles[1] += Lara.torsoRotX;
	}

	fired = 0;

	if (Lara.shotgunTypeCarried & W_AMMO1)
		scatter = 1820;
	else
		scatter = 5460;

	for (int i = 0; i < 6; i++)
	{
		dangles[0] = short(angles[0] + scatter * (GetRandomControl() - 0x4000) / 0x10000);
		dangles[1] = short(angles[1] + scatter * (GetRandomControl() - 0x4000) / 0x10000);

		if (FireWeapon(WEAPON_SHOTGUN, Lara.target, LaraItem, &dangles[0]))
			fired = 1;
	}

	if (fired)
	{
		pos.x = 0;
		pos.y = 228;
		pos.z = 32;
		GetLaraJointPos(&pos, LMX_HAND_R);

		pos2.x = 0;
		pos2.y = 1508;
		pos2.z = 32;
		GetLaraJointPos(&pos2, LMX_HAND_R);

		SmokeCountL = 32;
		SmokeWeapon = WEAPON_SHOTGUN;

		if (LaraItem->mesh_bits)
		{
			for (int i = 0; i < 7; i++)
				TriggerGunSmoke(pos.x, pos.y, pos.z, pos2.x - pos.x, pos2.y - pos.y, pos2.z - pos.z, 1, SmokeWeapon, 32);
		}

		Lara.rightArm.flash_gun = weapons[WEAPON_SHOTGUN].flash_time;
		SoundEffect(SFX_EXPLOSION1, &LaraItem->pos, 0x1400000 | SFX_SETPITCH);
		SoundEffect(weapons[WEAPON_SHOTGUN].sample_num, &LaraItem->pos, SFX_DEFAULT);
		savegame.Game.AmmoUsed++;
	}
}

void FireHK(long running)
{
	short angles[2];

	if (Lara.hkTypeCarried & W_AMMO1)
		HKTimer = 12;
	else if (Lara.hkTypeCarried & W_AMMO2)
	{
		HKShotsFired++;

		if (HKShotsFired == 5)
		{
			HKShotsFired = 0;
			HKTimer = 12;
		}
	}

	angles[0] = Lara.leftArm.y_rot + LaraItem->pos.y_rot;
	angles[1] = Lara.leftArm.x_rot;

	if (!Lara.leftArm.lock)
	{
		angles[0] += Lara.torsoRotY;
		angles[1] += Lara.torsoRotX;
	}

	if (running)
	{
		weapons[WEAPON_HK].shot_accuracy = 2184;
		weapons[WEAPON_HK].damage = 1;
	}
	else
	{
		weapons[WEAPON_HK].shot_accuracy = 728;
		weapons[WEAPON_HK].damage = 3;
	}

	if (FireWeapon(WEAPON_HK, Lara.target, LaraItem, angles))
	{
		SmokeCountL = 12;
		SmokeWeapon = WEAPON_HK;
		TriggerGunShell(1, GUNSHELL, WEAPON_HK);
		Lara.rightArm.flash_gun = weapons[WEAPON_HK].flash_time;
	}
}

void FireCrossbow(PHD_3DPOS* Start)
{
	ITEM_INFO* item;
	short* ammo;
	short item_number;

	ammo = get_current_ammo_pointer(WEAPON_CROSSBOW);

	if (!ammo[0])
		return;

	Lara.hasFired = 1;
	Lara.fired = 1;
	item_number = CreateItem();

	if (item_number == NO_ITEM)
		return;

	item = &items[item_number];
	item->object_number = CROSSBOW_BOLT;
	item->shade = -0x3DF0;
	item->room_number = LaraItem->room_number;
	item->pos.x_pos = Start->x_pos;
	item->pos.y_pos = Start->y_pos;
	item->pos.z_pos = Start->z_pos;
	InitialiseItem(item_number);
	item->pos.x_rot = Start->x_rot;
	item->pos.y_rot = Start->y_rot;
	item->pos.z_rot = Start->z_rot;
	item->speed = 512;
	AddActiveItem(item_number);

	if (ammo[0] != -1)
		ammo[0]--;

	SoundEffect(SFX_LARA_CROSSBOW, 0, SFX_DEFAULT);
	savegame.Game.AmmoUsed++;
}

void ControlCrossbow(short item_number)
{
	ITEM_INFO** itemlist;
	MESH_INFO** meshlist;
	ITEM_INFO* item;
	ITEM_INFO* target;
	long speed;
	short room_number;

	item = &items[item_number];
	speed = item->speed;
	item->pos.x_pos += (speed * phd_cos(item->pos.x_rot) >> W2V_SHIFT) * phd_sin(item->pos.y_rot) >> W2V_SHIFT;
	item->pos.y_pos += speed * phd_sin(-item->pos.x_rot) >> W2V_SHIFT;
	item->pos.z_pos += (speed * phd_cos(item->pos.x_rot) >> W2V_SHIFT) * phd_cos(item->pos.y_rot) >> W2V_SHIFT;
	room_number = item->room_number;
	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	itemlist = (ITEM_INFO**)&tsv_buffer[0x2000];
	meshlist = (MESH_INFO**)&tsv_buffer[0x3000];
	GetCollidedObjects(item, 128, 1, itemlist, meshlist, 1);

	if (*itemlist)
	{
		target = *itemlist;

		while (target)
		{
			if (gfLevelFlags & GF_OFFICE && target->object_number == GRAPPLING_TARGET)
			{
				TriggerGrapplingEffect(target->pos.x_pos, target->pos.y_pos + 32, target->pos.z_pos);
				TestTriggersAtXYZ(target->pos.x_pos, target->pos.y_pos, target->pos.z_pos, target->room_number, 1, 0);
				ExplodeItemNode(item, 0, 0, 256);
				SoundEffect(SFX_SMASH_METAL, &item->pos, SFX_DEFAULT);
				KillItem(item_number);
				target->mesh_bits <<= 1;
				return;
			}

			target = *itemlist++;
		}
	}
}

void draw_shotgun(long weapon_type)
{
	ITEM_INFO* item;

	if (Lara.weaponItem == NO_ITEM)
	{
		Lara.weaponItem = CreateItem();
		item = &items[Lara.weaponItem];
		item->object_number = (short)WeaponObject(weapon_type);
		item->anim_number = objects[item->object_number].anim_index + 1;
		item->frame_number = anims[item->anim_number].frame_base;
		item->status = ITEM_ACTIVE;
		item->current_anim_state = 1;
		item->goal_anim_state = 1;
		item->room_number = NO_ROOM;
		Lara.leftArm.frame_base = objects[item->object_number].frame_base; 
		Lara.rightArm.frame_base = objects[item->object_number].frame_base;
	}
	else
		item = &items[Lara.weaponItem];

	AnimateItem(item);

	if (!item->current_anim_state || item->current_anim_state == 6)
		ready_shotgun(weapon_type);
	else if (item->frame_number - anims[item->anim_number].frame_base == weapons[weapon_type].draw_frame)
		draw_shotgun_meshes(weapon_type);
	else if (Lara.waterStatus == LW_UNDERWATER)
		item->goal_anim_state = 6;

	Lara.leftArm.frame_base = anims[item->anim_number].frame_ptr; 
	Lara.rightArm.frame_base = anims[item->anim_number].frame_ptr;
	Lara.leftArm.frame_number = item->frame_number - anims[item->anim_number].frame_base; 
	Lara.rightArm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
	Lara.leftArm.anim_number = item->anim_number; 
	Lara.rightArm.anim_number = item->anim_number;
}

void undraw_shotgun(long weapon_type)
{
	ITEM_INFO* item; 

	item = &items[Lara.weaponItem];
	item->goal_anim_state = 3;
	AnimateItem(item);

	if (item->status == ITEM_DEACTIVATED)
	{
		Lara.gunStatus = LG_NO_ARMS;
		Lara.target = 0;
		Lara.rightArm.lock = 0;
		Lara.leftArm.lock = 0;
		KillItem(Lara.weaponItem);
		Lara.weaponItem = NO_ITEM;
		Lara.rightArm.frame_number = 0;
		Lara.leftArm.frame_number = 0;
	}
	else if (item->current_anim_state == 3 && anims[item->anim_number].frame_base == item->frame_number - 21)
		undraw_shotgun_meshes(weapon_type);

	Lara.rightArm.frame_base = anims[item->anim_number].frame_ptr;
	Lara.leftArm.frame_base = anims[item->anim_number].frame_ptr;
	Lara.rightArm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
	Lara.leftArm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
	Lara.rightArm.anim_number = item->anim_number;
	Lara.leftArm.anim_number = item->anim_number;
}

void AnimateShotgun(long weapon_type)
{
	ITEM_INFO* item;
	PHD_VECTOR pos;
	static long m16_firing;
	long running;

	if (HKTimer)
	{
		m16_firing = 0;
		HKTimer--;
	}

	if (SmokeCountL)
	{
		if (SmokeWeapon == WEAPON_HK)
		{
			pos.x = 0;
			pos.y = 228;
			pos.z = 96;
		}
		else if (SmokeWeapon == WEAPON_SHOTGUN)
		{
			pos.x = -16;
			pos.y = 228;
			pos.z = 32;
		}

		GetLaraJointPos(&pos, LMX_HAND_R);

		if (LaraItem->mesh_bits)
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountL);
	}

	item = &items[Lara.weaponItem];

	if (weapon_type == WEAPON_HK && LaraItem->speed)
		running = 1;
	else
		running = 0;

	switch (item->current_anim_state)
	{
	case 0:
		m16_firing = 0;
		HKTimer = 0;
		HKShotsFired = 0;

		if (Lara.waterStatus == LW_UNDERWATER || running)
			item->goal_anim_state = 6;
		else if (KeyInput & IN_ACTION && !Lara.target || Lara.leftArm.lock)
			item->goal_anim_state = 2;
		else
			item->goal_anim_state = 4;

		break;

	case 2:

		if (item->frame_number == anims[item->anim_number].frame_base)
		{
			item->goal_anim_state = 4;

			if (Lara.waterStatus != LW_UNDERWATER && !running)
			{
				if (KeyInput & IN_ACTION && (!Lara.target || Lara.leftArm.lock))
				{
					if (weapon_type == WEAPON_CROSSBOW)
					{
						FireCrossbow(0);
						item->goal_anim_state = 2;
					}
					else if (weapon_type == WEAPON_HK)
					{
						if ((Lara.hkTypeCarried & (W_AMMO1 | W_AMMO2)) && HKTimer)
							item->goal_anim_state = 0;
						else
						{
							FireHK(0);
							m16_firing = 1;
							item->goal_anim_state = 2;

							if (Lara.hkTypeCarried & W_SILENCER)
								SoundEffect(SFX_HK_SILENCED, 0, SFX_DEFAULT);
							else
							{
								SoundEffect(SFX_EXPLOSION1, &LaraItem->pos, 0x5000800 | SFX_SETPITCH | SFX_SETVOL);
								SoundEffect(SFX_HK_FIRE, &LaraItem->pos, SFX_DEFAULT);
							}
						}
					}
					else
					{
						FireShotgun();
						item->goal_anim_state = 2;
					}
				}
				else if (Lara.leftArm.lock)
					item->goal_anim_state = 0;
			}

			if (item->goal_anim_state != 2 && m16_firing && !(Lara.hkTypeCarried & W_SILENCER))
			{
				StopSoundEffect(SFX_HK_FIRE);
				SoundEffect(SFX_HK_STOP, &LaraItem->pos, SFX_DEFAULT);
				m16_firing = 0;
			}
		}
		else if (m16_firing)
		{
			if (Lara.hkTypeCarried & W_SILENCER)
				SoundEffect(SFX_HK_SILENCED, 0, SFX_DEFAULT);
			else
			{
				SoundEffect(SFX_EXPLOSION1, &LaraItem->pos, 0x5000800 | SFX_SETPITCH | SFX_SETVOL);
				SoundEffect(SFX_HK_FIRE, &LaraItem->pos, SFX_DEFAULT);
			}
		}
		else if (weapon_type == WEAPON_SHOTGUN && !(KeyInput & IN_ACTION) && !Lara.leftArm.lock)
			item->goal_anim_state = 4;

		if (item->frame_number - anims[item->anim_number].frame_base == 12 && weapon_type == WEAPON_SHOTGUN)
			TriggerGunShell(1, SHOTGUNSHELL, WEAPON_SHOTGUN);

		break;

	case 6:
		m16_firing = 0;
		HKTimer = 0;
		HKShotsFired = 0;

		if (Lara.waterStatus != LW_UNDERWATER && !running)
			item->goal_anim_state = 0;
		else if (KeyInput & IN_ACTION && !Lara.target || Lara.leftArm.lock)
			item->goal_anim_state = 8;
		else
			item->goal_anim_state = 7;

		break;

	case 8:

		if (item->frame_number == anims[item->anim_number].frame_base)
		{
			item->goal_anim_state = 7;

			if (running)
			{
				if (KeyInput & IN_ACTION && (!Lara.target || Lara.leftArm.lock))
				{
					if ((Lara.hkTypeCarried & (W_AMMO1 | W_AMMO2)) && HKTimer)
						item->goal_anim_state = 6;
					else
					{
						FireHK(1);
						m16_firing = 1;
						item->goal_anim_state = 8;

						if (Lara.hkTypeCarried & W_SILENCER)
							SoundEffect(SFX_HK_SILENCED, 0, SFX_DEFAULT);
						else
						{
							SoundEffect(SFX_EXPLOSION1, &LaraItem->pos, 0x5000800 | SFX_SETPITCH | SFX_SETVOL);
							SoundEffect(SFX_HK_FIRE, &LaraItem->pos, SFX_DEFAULT);
						}
					}
				}
				else if (Lara.leftArm.lock)
					item->goal_anim_state = 6;
			}

			if (item->goal_anim_state != 8 && m16_firing && !(Lara.hkTypeCarried & W_SILENCER))
			{
				StopSoundEffect(SFX_HK_FIRE);
				SoundEffect(SFX_HK_STOP, &LaraItem->pos, SFX_DEFAULT);
				m16_firing = 0;
			}
		}
		else if (m16_firing)
		{
			if (Lara.hkTypeCarried & W_SILENCER)
				SoundEffect(SFX_HK_SILENCED, 0, SFX_DEFAULT);
			else
			{
				SoundEffect(SFX_EXPLOSION1, &LaraItem->pos, 0x5000800 | SFX_SETPITCH | SFX_SETVOL);
				SoundEffect(SFX_HK_FIRE, &LaraItem->pos, SFX_DEFAULT);
			}
		}

		break;
	}

	AnimateItem(item);
	Lara.rightArm.frame_base = anims[item->anim_number].frame_ptr;
	Lara.rightArm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
	Lara.rightArm.anim_number = item->anim_number;
	Lara.leftArm.frame_base = Lara.rightArm.frame_base;
	Lara.leftArm.frame_number = Lara.rightArm.frame_number;
	Lara.leftArm.anim_number = Lara.rightArm.anim_number;
}

void DoGrenadeDamageOnBaddie(ITEM_INFO* baddie, ITEM_INFO* item)
{
	if (baddie->flags & 0x8000)
		return;

	if (baddie == LaraItem && LaraItem->hit_points > 0)
	{
		LaraItem->hit_points -= 50;

		if (!(room[item->room_number].flags & ROOM_UNDERWATER) && LaraItem->hit_points <= 50)
			LaraBurn();
	}
	else if (!item->item_flags[2])
	{
		baddie->hit_status = 1;

		if (!objects[baddie->object_number].undead)
		{
			HitTarget(baddie, 0, 30, 1);

			if (baddie != LaraItem)
			{
				savegame.Game.AmmoHits++;

				if (baddie->hit_points <= 0)
				{
					savegame.Level.Kills++;
					CreatureDie(baddie - items, 1);
				}
			}
		}
	}
}

void TriggerGrapplingEffect(long x, long y, long z)
{
	SMOKE_SPARKS* sptr;
	long size;

	for (int i = 0; i < 24; i++)
	{
		sptr = &smoke_spark[GetFreeSmokeSpark()];
		sptr->On = 1;
		sptr->sShade = (GetRandomControl() & 0xF) + 40;
		sptr->dShade = (GetRandomControl() & 0xF) + 64;
		sptr->ColFadeSpeed = 4;
		sptr->FadeToBlack = 16;
		sptr->Life = (GetRandomControl() & 3) + 40;
		sptr->sLife = sptr->Life;
		sptr->TransType = 2;
		sptr->x = x + (GetRandomControl() & 0x1F) - 16;
		sptr->y = y + (GetRandomControl() & 0x1F) - 16;
		sptr->z = z + (GetRandomControl() & 0x1F) - 16;
		sptr->Xvel = 2 * (GetRandomControl() & 0x1FF) - 512;
		sptr->Zvel = 2 * (GetRandomControl() & 0x1FF) - 512;

		if (i < 12)
		{
			sptr->Yvel = GetRandomControl() & 0x1F;
			sptr->Friction = 64;
		}
		else
		{
			sptr->Yvel = (GetRandomControl() & 0x1FF) + 256;
			sptr->Friction = 82;
		}

		sptr->Flags = SF_ROTATE;
		sptr->RotAng = (GetRandomControl() & 0xFFF);
		sptr->RotAdd = (GetRandomControl() & 0x40) - 32;
		sptr->MaxYvel = 0;
		sptr->Gravity = 0;
		sptr->mirror = 0;
		size = (GetRandomControl() & 0xF) + 48;
		sptr->Size = uchar(size >> 2);
		sptr->sSize = sptr->Size;
		sptr->dSize = uchar(size << 1);
	}
}

void CrossbowHitSwitchType78(ITEM_INFO* item, ITEM_INFO* target, long MustHitLastNode)
{
	SPHERE* ptr1;
	long dx, dy, dz, num1, cs, cd;
	short TriggerItems[8];
	short NumTrigs, room_number;

	if (target->flags & IFL_SWITCH_ONESHOT)
		return;

	if (!MustHitLastNode)
	{
		num1 = objects[target->object_number].nmeshes;
		cs = num1 - 1;
	}
	else
	{
		num1 = GetSpheres(target, Slist, 1);
		cs = -1;
		cd = 0x7FFFFFFF;
		ptr1 = Slist;

		for (int i = 0; i < num1; i++)
		{
			dx = ptr1->x - item->pos.x_pos;
			dy = ptr1->y - item->pos.y_pos;
			dz = ptr1->z - item->pos.z_pos;
			dy = SQUARE(dx) + SQUARE(dy) + SQUARE(dz) - SQUARE(ptr1->r);

			if (dy < cd)
			{
				cd = dy;
				cs = i;
			}

			ptr1++;
		}
	}

	if (cs == num1 - 1)
	{
		if (target->flags & IFL_CODEBITS && (target->flags & IFL_CODEBITS) != IFL_CODEBITS)
		{
			room_number = target->room_number;
			GetHeight(GetFloor(target->pos.x_pos, target->pos.y_pos - 256, target->pos.z_pos, &room_number), target->pos.x_pos, target->pos.y_pos - 256, target->pos.z_pos);
			TestTriggers(trigger_index, 1, target->flags & IFL_CODEBITS);
		}
		else
		{
			NumTrigs = (short)GetSwitchTrigger(target, TriggerItems, 1);

			for (int i = 0; i < NumTrigs; i++)
			{
				AddActiveItem(TriggerItems[i]);
				items[TriggerItems[i]].status = ITEM_ACTIVE;
				items[TriggerItems[i]].flags |= IFL_CODEBITS;
			}
		}

		if (target->object_number == SWITCH_TYPE7)
			ExplodeItemNode(target, objects[SWITCH_TYPE7].nmeshes - 1, 0, 64);

		AddActiveItem(target - items);
		target->flags |= IFL_CODEBITS | IFL_SWITCH_ONESHOT;
		target->status = ITEM_ACTIVE;
	}
}

void TriggerUnderwaterExplosion(ITEM_INFO* item)
{
	long y, wh;

	TriggerExplosionBubble(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
	TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 2, -2, 1, item->room_number);

	for (int i = 0; i < 3; i++)
		TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 2, -1, 1, item->room_number);

	wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);

	if (wh != NO_HEIGHT)
	{
		y = item->pos.y_pos - wh;

		if (y < 2048)
		{
			splash_setup.x = item->pos.x_pos;
			splash_setup.y = wh;
			splash_setup.z = item->pos.z_pos;
			wh = 2048 - y;
			splash_setup.InnerRadVel = 160;
			splash_setup.MiddleRadVel = 224;
			splash_setup.OuterRadVel = 272;
			splash_setup.InnerRad = short((wh >> 6) + 16);
			splash_setup.InnerSize = short((wh >> 6) + 12);
			splash_setup.InnerYVel = short((-512 - wh) << 3);
			splash_setup.MiddleRad = short((wh >> 6) + 24);
			splash_setup.MiddleSize = short((wh >> 6) + 24);
			splash_setup.MiddleYVel = short((-768 - wh) << 2);
			splash_setup.OuterRad = short((wh >> 6) + 32);
			splash_setup.OuterSize = short((wh >> 6) + 32);
			SetupSplash(&splash_setup);
		}
	}
}
