#include "../tomb5/pch.h"
#include "laramisc.h"
#include "lara_states.h"
#include "gameflow.h"
#include "../specific/3dmath.h"
#include "draw.h"
#include "control.h"
#include "collide.h"
#include "effects.h"
#include "sound.h"
#include "laraswim.h"
#include "objects.h"
#include "rope.h"
#include "lara.h"
#include "larasurf.h"
#include "../specific/function_stubs.h"
#include "../specific/audio.h"
#include "../specific/file.h"
#include "camera.h"
#include "spotcam.h"
#include "effect2.h"
#include "../specific/input.h"
#include "savegame.h"
#include "newinv2.h"
#include "../specific/dxshell.h"
#include "../tomb5/tomb5.h"

COLL_INFO mycoll;

static COLL_INFO* lara_coll = &mycoll;

void GetLaraDeadlyBounds()
{
	short* bounds;
	short tbounds[6];

	bounds = GetBoundsAccurate(LaraItem);

	phd_PushUnitMatrix();
	phd_RotYXZ(LaraItem->pos.y_rot, LaraItem->pos.x_rot, LaraItem->pos.z_rot);
	phd_SetTrans(0, 0, 0);
	mRotBoundingBoxNoPersp(bounds, tbounds);
	phd_PopMatrix();

	DeadlyBounds[0] = LaraItem->pos.x_pos + tbounds[0];
	DeadlyBounds[1] = LaraItem->pos.x_pos + tbounds[1];
	DeadlyBounds[2] = LaraItem->pos.y_pos + tbounds[2];
	DeadlyBounds[3] = LaraItem->pos.y_pos + tbounds[3];
	DeadlyBounds[4] = LaraItem->pos.z_pos + tbounds[4];
	DeadlyBounds[5] = LaraItem->pos.z_pos + tbounds[5];
}

void InitialiseLaraAnims(ITEM_INFO* item)
{
	if (room[item->room_number].flags & ROOM_UNDERWATER)
	{
		item->anim_number = ANIM_TREAD;
		item->frame_number = anims[ANIM_TREAD].frame_base;
		item->current_anim_state = AS_TREAD;
		item->goal_anim_state = AS_TREAD;

		item->fallspeed = 0;
		Lara.waterStatus = LW_UNDERWATER;
	}
	else
	{
		item->anim_number = ANIM_STOP;
		item->frame_number = anims[ANIM_STOP].frame_base;
		item->current_anim_state = AS_STOP;
		item->goal_anim_state = AS_STOP;

		Lara.waterStatus = LW_ABOVE_WATER;
	}
}

void InitialiseLaraLoad(short item_num)
{
	Lara.itemNumber = item_num;
	LaraItem = &items[item_num];
}

void LaraCheat(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.deathCount = 0;
	LaraItem->hit_points = 1000;
	LaraUnderWater(item, coll);

	if (KeyInput & IN_WALK && !(KeyInput & IN_LOOK))
	{
		Lara.waterStatus = LW_ABOVE_WATER;
		item->frame_number = anims[ANIM_STOP].frame_base;
		item->anim_number = ANIM_STOP;
		item->pos.z_rot = 0;
		item->pos.x_rot = 0;
		Lara.torsoRotY = 0;
		Lara.torsoRotX = 0;
		Lara.headRotY = 0;
		Lara.headRotX = 0;
		Lara.gunStatus = 0;
		LaraInitialiseMeshes();
		Lara.meshEffects = 0;
		LaraItem->hit_points = 1000;
	}
}

void LaraInitialiseMeshes()
{
	for (int i = 0; i < 15; i++)
	{
		meshes[objects[LARA].mesh_index + 2 * i] = meshes[objects[LARA_SKIN].mesh_index + 2 * i];
		Lara.meshPtrs[i] = meshes[objects[LARA].mesh_index + 2 * i];
	}

	if (gfCurrentLevel >= LVL5_GALLOWS_TREE && gfCurrentLevel <= LVL5_OLD_MILL)
		Lara.meshPtrs[LM_TORSO] = meshes[objects[ANIMATING6_MIP].mesh_index + 2 * LM_TORSO];

	if (Lara.gunType == WEAPON_HK)
		Lara.weaponBackItem = HK_ANIM;
	else if (Lara.shotgunTypeCarried != W_NONE)
		Lara.weaponBackItem = SHOTGUN_ANIM;
	else if (Lara.hkTypeCarried != W_NONE)
		Lara.weaponBackItem = HK_ANIM;

	Lara.gunStatus = LG_NO_ARMS;
	Lara.target = 0;
	Lara.leftArm.frame_number = 0;
	Lara.leftArm.lock = 0;
	Lara.rightArm.frame_number = 0;
	Lara.rightArm.lock = 0;
}

void LaraCheatGetStuff()
{
	if (objects[FLARE_INV_ITEM].loaded)
		Lara.numFlares = -1;

	if (objects[SMALLMEDI_ITEM].loaded)
		Lara.numSmallMedipack = -1;

	if (objects[BIGMEDI_ITEM].loaded)
		Lara.numLargeMedipack = -1;

	if (objects[LASERSIGHT_ITEM].loaded)
		Lara.lasersight = 1;

	if (objects[BOTTLE].loaded)
		Lara.bottle = 1;

	if (objects[CLOTH].loaded)
		Lara.wetcloth = CLOTH_DRY;

	if (objects[UZI_ITEM].loaded)
	{
		Lara.uzisTypeCarried = W_PRESENT | W_AMMO1;
		Lara.numUziAmmo = -1;
	}

	if (objects[SHOTGUN_ITEM].loaded)
	{
		Lara.shotgunTypeCarried = W_PRESENT | W_AMMO1;
		Lara.numShotgunAmmo1 = -1;
		Lara.numShotgunAmmo2 = -1;
	}

	if (objects[REVOLVER_ITEM].loaded)
	{
		Lara.sixshooterTypeCarried = W_PRESENT | W_AMMO1;
		Lara.numRevolverAmmo = -1;
	}
	
	if (objects[HK_ITEM].loaded)
	{
		Lara.hkTypeCarried = W_PRESENT | W_AMMO3;
		Lara.numHKAmmo1 = -1;
	}

	if (objects[CROSSBOW_ITEM].loaded)
	{
		Lara.crossbowTypeCarried = W_PRESENT | W_AMMO1 | W_LASERSIGHT;
		Lara.numCrossbowAmmo1 = -1;
		Lara.numCrossbowAmmo2 = 0;
	}

	dels_give_lara_items_cheat();
}

void LaraCheatyBits()
{
#ifndef _DEBUG	//public releases have cheats in inventory, except for DOZY
	if (keymap[SDL_SCANCODE_D] && keymap[SDL_SCANCODE_O] && keymap[SDL_SCANCODE_Z] && keymap[SDL_SCANCODE_Y])
	{
		lara_item->pos.y_pos -= 128;

		if (lara.water_status != LW_FLYCHEAT)
		{
			lara_item->anim_number = ANIM_SWIMCHEAT;
			lara_item->frame_number = anims[ANIM_SWIMCHEAT].frame_base;
			lara_item->current_anim_state = AS_SWIMCHEAT;
			lara_item->goal_anim_state = AS_SWIMCHEAT;
			lara.water_status = LW_FLYCHEAT;
			lara_item->gravity_status = 0;
			lara_item->pos.x_rot = 5460;
			lara_item->fallspeed = 30;
			lara.air = 1800;
			lara.death_count = 0;
			lara.torso_x_rot = 0;
			lara.torso_y_rot = 0;
			lara.head_x_rot = 0;
			lara.head_y_rot = 0;
		}
	}
#else
	if (keymap[SDL_SCANCODE_F1])
	{
		LaraCheatGetStuff();
		LaraItem->hit_points = 1000;
	}

	if (keymap[SDL_SCANCODE_F2])
	{
		LaraItem->pos.y_pos -= 128;

		if (Lara.waterStatus != LW_FLYCHEAT)
		{
			LaraItem->anim_number = ANIM_SWIMCHEAT;
			LaraItem->frame_number = anims[ANIM_SWIMCHEAT].frame_base;
			LaraItem->current_anim_state = AS_SWIMCHEAT;
			LaraItem->goal_anim_state = AS_SWIMCHEAT;
			Lara.waterStatus = LW_FLYCHEAT;
			LaraItem->gravity_status = 0;
			LaraItem->pos.x_rot = 5460;
			LaraItem->fallspeed = 30;
			Lara.air = 1800;
			Lara.deathCount = 0;
			Lara.torsoRotX = 0;
			Lara.torsoRotY = 0;
			Lara.headRotX = 0;
			Lara.headRotY = 0;
		}
	}

	if (keymap[SDL_SCANCODE_F3])
	{
		gfLevelComplete = gfCurrentLevel + 1;
		SCNoDrawLara = 0;
		bDisableLaraControl = 0;
	}
#endif
}

void AnimateLara(ITEM_INFO* item)
{
	ANIM_STRUCT* anim;
	short* cmd;
	long speed, speed2;

	item->frame_number++;
	anim = &anims[item->anim_number];
	
	if (anim->number_changes > 0 && GetChange(item, anim))
	{
		anim = &anims[item->anim_number];
		item->current_anim_state = anim->current_anim_state;
	}

	if (item->frame_number > anim->frame_end)
	{
		if (anim->number_commands > 0)
		{
			cmd = &commands[anim->command_index];

			for (int i = anim->number_commands; i > 0; i--)
			{
				switch (*cmd++)
				{
				case 1:
					TranslateItem(item, *cmd, cmd[1], cmd[2]);
					UpdateLaraRoom(item, -381);
					cmd += 3;
					break;

				case 2:
					item->fallspeed = *cmd++;
					item->speed = *cmd++;
					item->gravity_status = 1;

					if (Lara.calcFallspeed)
					{
						item->fallspeed = Lara.calcFallspeed;
						Lara.calcFallspeed = 0;
					}

					break;

				case 3:

					if (Lara.gunStatus != LG_FLARE)
						Lara.gunStatus = LG_NO_ARMS;

					break;

				case 5:
				case 6:
					cmd += 2;
					break;
				}
			}
		}

		item->anim_number = anim->jump_anim_num;
		item->frame_number = anim->jump_frame_num;
		anim = &anims[item->anim_number];
		item->current_anim_state = anim->current_anim_state;
	}

	if (anim->number_commands > 0)
	{
		cmd = &commands[anim->command_index];

		for (int i = anim->number_commands; i > 0; i--)
		{
			switch (*cmd++)
			{
			case 1:
				cmd += 3;
				break;

			case 2:
				cmd += 2;
				break;

			case 5:

				if (item->frame_number == *cmd)
				{
					if ((cmd[1] & 0xC000) == SFX_LANDANDWATER ||
						((cmd[1] & 0xC000) == SFX_LANDONLY && (Lara.waterSurfaceDistance >= 0 || Lara.waterSurfaceDistance == NO_HEIGHT)) ||
						((cmd[1] & 0xC000) == SFX_WATERONLY && Lara.waterSurfaceDistance < 0 && Lara.waterSurfaceDistance != NO_HEIGHT))
						SoundEffect(cmd[1] & 0x3FFF, &item->pos, SFX_ALWAYS);
				}

				cmd += 2;
				break;

			case 6:

				if (item->frame_number == *cmd)
				{
					FXType = cmd[1] & 0xC000;
					effect_routines[cmd[1] & 0x3FFF](item);
				}

				cmd += 2;
				break;
			}
		}
	}

	speed2 = anim->Xvelocity;

	if (anim->Xacceleration)
		speed2 += anim->Xacceleration * (item->frame_number - anim->frame_base);

	speed2 >>= 16;

	if (item->gravity_status)
	{
		speed = anim->velocity + anim->acceleration * (item->frame_number - anim->frame_base - 1);
		item->speed -= speed >> 16;
		speed += anim->acceleration;
		item->speed += speed >> 16;
		item->fallspeed += (item->fallspeed >= 128) ? 1 : 6;
		item->pos.y_pos += item->fallspeed;
	}
	else
	{
		speed = anim->velocity;

		if (anim->acceleration)
			speed += anim->acceleration * (item->frame_number - anim->frame_base);

		item->speed = speed >> 16;
	}

	if (Lara.ropePtr != -1)
		AlignLaraToRope(item);

	if (!Lara.isMoving)
	{
		item->pos.x_pos += item->speed * phd_sin(Lara.moveAngle) >> W2V_SHIFT;
		item->pos.z_pos += item->speed * phd_cos(Lara.moveAngle) >> W2V_SHIFT;
		item->pos.x_pos += speed2 * phd_sin(Lara.moveAngle + 0x4000) >> W2V_SHIFT;
		item->pos.z_pos += speed2 * phd_cos(Lara.moveAngle + 0x4000) >> W2V_SHIFT;
	}
}

void LaraControl(short item_number)
{
	ITEM_INFO* item;
	long oldx, oldy, oldz;
	long wh, wd, hfw, water;
	short room_number;
	static short SubsuitAir = 0;

	item = LaraItem;

	if (Lara.isMoving && Lara.moveCount++ > 90)
	{
		Lara.isMoving = 0;
		Lara.gunStatus = LG_NO_ARMS;
	}

	if (tomb5.enable_cheats)
		LaraCheatyBits();

	if (!bDisableLaraControl)
		Lara.locationPad = -128;

	oldx = LaraItem->pos.x_pos;
	oldy = LaraItem->pos.y_pos;
	oldz = LaraItem->pos.z_pos;

	if (Lara.gunStatus == LG_HANDS_BUSY && item->current_anim_state == AS_STOP && item->goal_anim_state == AS_STOP
		&& item->anim_number == ANIM_BREATH && !item->gravity_status)
		Lara.gunStatus = LG_NO_ARMS;

	if (item->current_anim_state != AS_DASH && DashTimer < 120)
		DashTimer++;

	Lara.isDucked = 0;
	water = room[item->room_number].flags & ROOM_UNDERWATER;
	wd = GetWaterDepth(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
	wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);

	if (wh != NO_HEIGHT)
		hfw = item->pos.y_pos - wh;
	else
		hfw = NO_HEIGHT;

	Lara.waterSurfaceDistance = -hfw;
	WadeSplash(item, wh, wd);

	switch (Lara.waterStatus)
	{
	case LW_ABOVE_WATER:

		if (hfw != NO_HEIGHT && hfw >= 265)
		{
			if (wd <= 474)
			{
				if (hfw > 256)
				{
					Lara.waterStatus = LW_WADE;

					if (!item->gravity_status)
						item->goal_anim_state = AS_STOP;
				}
			}
			else if (water)
			{
				Lara.air = 1800;
				Lara.waterStatus = LW_UNDERWATER;
				item->gravity_status = 0;
				item->pos.y_pos += 100;
				UpdateLaraRoom(item, 0);
				StopSoundEffect(SFX_LARA_FALL);

				if (item->current_anim_state == AS_SWANDIVE)
				{
					item->pos.x_rot = -8190;
					item->goal_anim_state = AS_DIVE;
					AnimateLara(item);
					item->fallspeed *= 2;
				}
				else if (item->current_anim_state == AS_FASTDIVE)
				{
					item->pos.x_rot = -15470;
					item->goal_anim_state = AS_DIVE;
					AnimateLara(item);
					item->fallspeed *= 2;
				}
				else
				{
					item->pos.x_rot = -8190;
					item->anim_number = 112;
					item->frame_number = anims[112].frame_base;
					item->current_anim_state = AS_DIVE;
					item->goal_anim_state = AS_SWIM;
					item->fallspeed = 3 * item->fallspeed / 2;
				}

				Lara.torsoRotX = 0;
				Lara.torsoRotY = 0;
				Lara.headRotX = 0;
				Lara.headRotY = 0;
				Splash(LaraItem);
			}
		}
		else
			break;

	case LW_WADE:
		camera.target_elevation = -4004;

		if (hfw <= 256)
		{
			Lara.waterStatus = LW_ABOVE_WATER;

			if (item->current_anim_state == AS_WADE)
				item->goal_anim_state = 1;
		}
		else if (hfw > 730)
		{
			Lara.waterStatus = LW_SURFACE;
			item->pos.y_pos += 1 - hfw;

			switch (item->current_anim_state)
			{
			case AS_BACK:
				item->current_anim_state = AS_SURFBACK;
				item->goal_anim_state = AS_SURFBACK;
				item->anim_number = 140;
				item->frame_number = anims[140].frame_base;
				break;

			case AS_STEPRIGHT:
				item->current_anim_state = AS_SURFRIGHT;
				item->goal_anim_state = AS_SURFRIGHT;
				item->anim_number = 144;
				item->frame_number = anims[144].frame_base;
				break;

			case AS_STEPLEFT:
				item->current_anim_state = AS_SURFLEFT;
				item->goal_anim_state = AS_SURFLEFT;
				item->anim_number = 143;
				item->frame_number = anims[143].frame_base;
				break;

			default:
				item->current_anim_state = AS_SURFSWIM;
				item->goal_anim_state = AS_SURFSWIM;
				item->anim_number = 116;
				item->frame_number = anims[116].frame_base;
				break;
			}

			item->gravity_status = 0;
			item->fallspeed = 0;
			Lara.diveCount = 0;
			item->pos.x_rot = 0;
			item->pos.z_rot = 0;
			Lara.torsoRotX = 0;
			Lara.torsoRotY = 0;
			Lara.headRotX = 0;
			Lara.headRotY = 0;
			UpdateLaraRoom(item, 0);
		}

		break;

	case LW_UNDERWATER:
		room_number = item->room_number;
		GetFloor(item->pos.x_pos, item->pos.y_pos - 256, item->pos.z_pos, &room_number);

		if (wd != NO_HEIGHT && abs(hfw) < 256 && !(room[room_number].flags & ROOM_UNDERWATER) && item->anim_number != 114 && item->anim_number != 119)
		{
			Lara.waterStatus = LW_SURFACE;
			item->pos.y_pos = wh + 1;
			item->anim_number = 114;
			item->frame_number = anims[114].frame_base;
			item->current_anim_state = AS_SURFTREAD;
			item->goal_anim_state = AS_SURFTREAD;
			item->pos.x_rot = 0;
			item->pos.z_rot = 0;
			item->fallspeed = 0;
			Lara.diveCount = 11;
			Lara.torsoRotX = 0;
			Lara.torsoRotY = 0;
			Lara.headRotX = 0;
			Lara.headRotY = 0;
			UpdateLaraRoom(item, 0);
			SoundEffect(SFX_LARA_BREATH, &LaraItem->pos, SFX_ALWAYS);
		}
		else if (!water)
		{
			if (wd != NO_HEIGHT && abs(hfw) < 256)
			{
				Lara.waterStatus = LW_SURFACE;
				item->pos.y_pos = wh;
				item->anim_number = 114;
				item->frame_number = anims[114].frame_base;
				item->current_anim_state = AS_SURFTREAD;
				item->goal_anim_state = AS_SURFTREAD;
				item->pos.x_rot = 0;
				item->pos.z_rot = 0;
				item->fallspeed = 0;
				Lara.diveCount = 11;
				Lara.torsoRotX = 0;
				Lara.torsoRotY = 0;
				Lara.headRotX = 0;
				Lara.headRotY = 0;
				UpdateLaraRoom(item, -381);
				SoundEffect(SFX_LARA_BREATH, &item->pos, SFX_ALWAYS);
			}
			else
			{
				Lara.waterStatus = LW_ABOVE_WATER;
				item->anim_number = ANIM_FALLDOWN;
				item->frame_number = anims[ANIM_FALLDOWN].frame_base;
				item->current_anim_state = AS_FORWARDJUMP;
				item->goal_anim_state = AS_FORWARDJUMP;
				item->pos.x_rot = 0;
				item->pos.z_rot = 0;
				item->speed = item->fallspeed / 4;
				item->gravity_status = 1;
				item->fallspeed = 0;
				Lara.torsoRotX = 0;
				Lara.torsoRotY = 0;
				Lara.headRotX = 0;
				Lara.headRotY = 0;
			}
		}

		break;

	case LW_SURFACE:

		if (!water)
		{
			if (hfw <= 256)
			{
				Lara.waterStatus = LW_ABOVE_WATER;
				item->anim_number = ANIM_FALLDOWN;
				item->frame_number = anims[ANIM_FALLDOWN].frame_base;
				item->goal_anim_state = AS_FORWARDJUMP;
				item->current_anim_state = AS_FORWARDJUMP;
				item->speed = item->fallspeed / 4;
				item->gravity_status = 1;			
			}
			else
			{
				Lara.waterStatus = LW_WADE;
				item->anim_number = ANIM_BREATH;
				item->frame_number = anims[ANIM_BREATH].frame_base;
				item->current_anim_state = AS_STOP;
				item->goal_anim_state = AS_WADE;
				AnimateItem(item);
			}

			item->fallspeed = 0;
			item->pos.x_rot = 0;
			item->pos.z_rot = 0;
			Lara.torsoRotX = 0;
			Lara.torsoRotY = 0;
			Lara.headRotX = 0;
			Lara.headRotY = 0;
		}

		break;
	}

//	S_SetReverbType(room[item->room_number].ReverbType);

	if (item->hit_points <= 0)
	{
		item->hit_points = -1;

		if (!Lara.deathCount)
			S_CDStop();

		Lara.deathCount++;

		if (LaraItem->flags & IFL_INVISIBLE)
		{
			Lara.deathCount++;
			return;
		}
	}

	switch (Lara.waterStatus)
	{
	case LW_ABOVE_WATER:
	case LW_WADE:

		if (Lara.gassed)
		{
			if (item->hit_points >= 0)
			{
				Lara.air--;

				if (Lara.air < 0)
				{
					Lara.air = -1;
					item->hit_points -= 5;
				}
			}
		}
		else if (Lara.air < 1800 && item->hit_points >= 0)
		{
			Lara.air += 10;

			if (Lara.air > 1800)
				Lara.air = 1800;
		}

		LaraAboveWater(item, lara_coll);
		break;

	case LW_UNDERWATER:

		if (item->hit_points >= 0)
		{
			if (LaraDrawType == LARA_DIVESUIT)
			{
				if (CheckCutPlayed(40))
				{
					SubsuitAir += Lara.anxiety + 8;

					while (SubsuitAir > 80)
					{
						Lara.air--;
						SubsuitAir -= 80;
					}
				}
			}
			else
				Lara.air--;

			if (Lara.air < 0)
			{
				if (LaraDrawType == LARA_DIVESUIT && Lara.anxiety < 251)
					Lara.anxiety += 4;

				Lara.air = -1;
				item->hit_points -= 5;
			}
		}

		LaraUnderWater(item, lara_coll);
		break;

	case LW_SURFACE:

		if (item->hit_points >= 0)
		{
			Lara.air += 10;

			if (Lara.air > 1800)
				Lara.air = 1800;
		}

		LaraSurface(item, lara_coll);
		break;

	case LW_FLYCHEAT:
		LaraCheat(item, lara_coll);
		break;
	}

	savegame.Game.Distance += phd_sqrt(SQUARE(item->pos.x_pos - oldx) + SQUARE(item->pos.y_pos - oldy) + SQUARE(item->pos.z_pos - oldz));
}
