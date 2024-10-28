#include "../tomb5/pch.h"
#include "larafire.h"
#include "flmtorch.h"
#include "gameflow.h"
#include "lara_states.h"
#include "laraflar.h"
#include "lara1gun.h"
#include "lara2gun.h"
#include "control.h"
#include "effect2.h"
#include "effects.h"
#include "sound.h"
#include "items.h"
#include "draw.h"
#include "delstuff.h"
#include "../specific/3dmath.h"
#include "sphere.h"
#include "objects.h"
#include "../specific/function_stubs.h"
#include "camera.h"
#include "../specific/input.h"
#include "lara.h"
#include "lot.h"
#include "savegame.h"

WEAPON_INFO weapons[9] =
{
	{
		0, 0, 0, 0,							//WEAPON_NONE
		0, 0, 0, 0,
		0, 0, 0, 0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	{
		-10920, 10920, -10920, 10920,		//WEAPON_PISTOLS
		-30940, 10920, -14560, 14560,
		-10920, 30940, -14560, 14560,
		1820,
		1456,
		650,
		8192,
		1,
		9,
		3,
		0,
		SFX_LARA_FIRE
	},

	{
		-10920, 10920, -10920, 10920,		//WEAPON_REVOLVER
		-1820, 1820, -14560, 14560,
		0, 0, 0, 0,
		1820,
		728,
		650,
		8192,
		21,
		16,
		3,
		0,
		SFX_REVOLVER
	},

	{
		-10920, 10920, -10920, 10920,		//WEAPON_UZI
		-30940, 10920, -14560, 14560,
		-10920, 30940, -14560, 14560,
		1820,
		1456,
		650,
		8192,
		1,
		3,
		3,
		0,
		SFX_LARA_UZI_FIRE
	},

	{
		-10920, 10920, -10010, 10010,		//WEAPON_SHOTGUN
		-14560, 14560, -11830, 11830,
		-14560, 14560, -11830, 11830,
		1820,
		0,
		500,
		8192,
		3,
		9,
		3,
		10,
		SFX_LARA_SHOTGUN
	},

	{
		-10920, 10920, -10010, 10010,		//WEAPON_HK
		-14560, 14560, -11830, 11830,
		-14560, 14560, -11830, 11830,
		1820,
		728,
		500,
		12288,
		4,
		0,
		3,
		16,
		0
	},

	{
		-10920, 10920, -10010, 10010,		//WEAPON_CROSSBOW
		-14560, 14560, -11830, 11830,
		-14560, 14560, -11830, 11830,
		1820,
		1456,
		500,
		8192,
		5,
		0,
		2,
		10,
		0
	},

	{
		0, 0, 0, 0,							//WEAPON_FLARE
		0, 0, 0, 0,
		0, 0, 0, 0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	{
		-5460, 5460, -10010, 10010,			//WEAPON_TORCH
		-5460, 5460, -10010, 10010,
		-5460, 5460, -10010, 10010,
		1820,
		1456,
		400,
		8192,
		3,
		0,
		2,
		0,
		SFX_LARA_UZI_FIRE
	},
};

static short HoldStates[] =
{
	AS_WALK,
	AS_STOP,
	AS_POSE,
	AS_TURN_R,
	AS_TURN_L, 
	AS_BACK, 
	AS_FASTTURN, 
	AS_STEPLEFT, 
	AS_STEPRIGHT,
	AS_WADE,
	AS_DUCK,
	AS_DUCKROTL,
	AS_DUCKROTR,
	-1
};

static long CheckForHoldingState(long state)
{
	short* holds;

	holds = HoldStates;

	while (*holds >= 0)
	{
		if (state == *holds)
			return 1;

		holds++;
	}

	return 0;
}

void LaraGun()
{
	short state;

	if (Lara.left_arm.flash_gun > 0)
		Lara.left_arm.flash_gun--;

	if (Lara.right_arm.flash_gun > 0)
		Lara.right_arm.flash_gun--;

	if (Lara.gun_type == WEAPON_TORCH)
	{
		DoFlameTorch();
		return;
	}

	if (LaraItem->hit_points <= 0)
		Lara.gun_status = LG_NO_ARMS;
	else if (Lara.gun_status == LG_NO_ARMS)
	{
		if (KeyInput & IN_DRAW)
			Lara.request_gun_type = Lara.last_gun_type;
		else if (KeyInput & IN_FLARE && !(gfLevelFlags & GF_YOUNGLARA))
		{
			if (LaraItem->current_anim_state == AS_DUCK && LaraItem->anim_number != ANIM_DUCKBREATHE)
				return;

			if (Lara.gun_type == WEAPON_FLARE)
			{
				if (!Lara.left_arm.frame_number)
					Lara.gun_status = LG_UNDRAW_GUNS;
			}
			else if (Lara.num_flares)
			{
				if (Lara.num_flares != -1)
					Lara.num_flares--;

				Lara.request_gun_type = WEAPON_FLARE;
			}
		}

		if (KeyInput & IN_DRAW || Lara.request_gun_type != Lara.gun_type)
		{
			state = LaraItem->current_anim_state;

			if ((state == AS_DUCK || state == AS_DUCKROTL || state == AS_DUCKROTR || state == AS_DUCKROLL) &&
				(Lara.request_gun_type == WEAPON_SHOTGUN || Lara.request_gun_type == WEAPON_HK || Lara.request_gun_type == WEAPON_CROSSBOW))
			{
				if (Lara.gun_type == WEAPON_FLARE)
					Lara.request_gun_type = WEAPON_FLARE;
			}
			else if (Lara.request_gun_type == WEAPON_FLARE || (Lara.water_status == LW_ABOVE_WATER ||
				(Lara.water_status == LW_WADE && Lara.water_surface_dist > -weapons[Lara.gun_type].gun_height)))
			{
				if (Lara.gun_type == WEAPON_FLARE)
				{
					CreateFlare(FLARE_ITEM, 0);
					undraw_flare_meshes();
					Lara.flare_control_left = 0;
					Lara.flare_age = 0;
				}

				Lara.gun_type = Lara.request_gun_type;
				InitialiseNewWeapon();
				Lara.gun_status = LG_DRAW_GUNS;
				Lara.right_arm.frame_number = 0;
				Lara.left_arm.frame_number = 0;
			}
			else
			{
				Lara.last_gun_type = Lara.request_gun_type;

				if (Lara.gun_type == WEAPON_FLARE)
					Lara.request_gun_type = WEAPON_FLARE;
				else
					Lara.gun_type = Lara.request_gun_type;
			}
		}
	}
	else if (Lara.gun_status == LG_READY)
	{
		if ((KeyInput & IN_DRAW || Lara.request_gun_type != Lara.gun_type) || (Lara.water_status != LW_ABOVE_WATER &&
			(Lara.water_status != LW_WADE || Lara.water_surface_dist < -weapons[Lara.gun_type].gun_height)))
			Lara.gun_status = LG_UNDRAW_GUNS;
	}
	else if (Lara.gun_status == LG_HANDS_BUSY && KeyInput & IN_FLARE && LaraItem->current_anim_state == AS_ALL4S && LaraItem->anim_number == ANIM_ALL4S)
		Lara.request_gun_type = WEAPON_FLARE;

	switch (Lara.gun_status)
	{
	case LG_NO_ARMS:

		if (Lara.gun_type == WEAPON_FLARE)
		{
			if (CheckForHoldingState(LaraItem->current_anim_state))
			{
				if (!Lara.flare_control_left)
				{
					Lara.left_arm.frame_number = 95;
					Lara.flare_control_left = 1;
				}
				else if (Lara.left_arm.frame_number)
				{
					Lara.left_arm.frame_number++;

					if (Lara.left_arm.frame_number == 110)
						Lara.left_arm.frame_number = 0;
				}
			}
			else
				Lara.flare_control_left = 0;

			DoFlareInHand(Lara.flare_age);
			set_flare_arm(Lara.left_arm.frame_number);
		}

		break;

	case LG_HANDS_BUSY:

		if (Lara.gun_type == WEAPON_FLARE && Lara.mesh_ptrs[LM_LHAND] == meshes[objects[FLARE_ANIM].mesh_index + 2 * LM_LHAND])
		{
			Lara.flare_control_left = CheckForHoldingState(LaraItem->current_anim_state);
			DoFlareInHand(Lara.flare_age);
			set_flare_arm(Lara.left_arm.frame_number);
		}

		break;

	case LG_DRAW_GUNS:

		if (Lara.gun_type != WEAPON_FLARE && Lara.gun_type != WEAPON_NONE)
			Lara.last_gun_type = Lara.gun_type;

		switch (Lara.gun_type)
		{
		case WEAPON_PISTOLS:
		case WEAPON_REVOLVER:
		case WEAPON_UZI:

			if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA && camera.type != HEAVY_CAMERA)
				camera.type = COMBAT_CAMERA;

			draw_pistols(Lara.gun_type);
			break;

		case WEAPON_SHOTGUN:
		case WEAPON_HK:
		case WEAPON_CROSSBOW:

			if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA && camera.type != HEAVY_CAMERA)
				camera.type = COMBAT_CAMERA;

			draw_shotgun(Lara.gun_type);
			break;

		case WEAPON_FLARE:
			draw_flare();
			break;

		default:
			Lara.gun_status = LG_NO_ARMS;
			break;
		}

		break;

	case LG_UNDRAW_GUNS:
		Lara.mesh_ptrs[LM_HEAD] = meshes[objects[LARA].mesh_index + 2 * LM_HEAD];

		switch (Lara.gun_type)
		{
		case WEAPON_PISTOLS:
		case WEAPON_REVOLVER:
		case WEAPON_UZI:
			undraw_pistols(Lara.gun_type);
			break;

		case WEAPON_SHOTGUN:
		case WEAPON_HK:
		case WEAPON_CROSSBOW:
			undraw_shotgun(Lara.gun_type);
			break;

		case WEAPON_FLARE:
			undraw_flare();
			break;
		}

		break;

	case LG_READY:

		if (KeyInput & IN_ACTION)
			Lara.mesh_ptrs[LM_HEAD] = meshes[objects[LARA_SCREAM].mesh_index + 2 * LM_HEAD];
		else
			Lara.mesh_ptrs[LM_HEAD] = meshes[objects[LARA].mesh_index + 2 * LM_HEAD];

		if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA && camera.type != HEAVY_CAMERA)
			camera.type = COMBAT_CAMERA;

		if (KeyInput & IN_ACTION)
		{
			if (!*get_current_ammo_pointer(Lara.gun_type))
			{
				if (objects[PISTOLS_ITEM].loaded)
					Lara.request_gun_type = WEAPON_PISTOLS;
				else
					Lara.request_gun_type = WEAPON_NONE;
			}
		}

		switch (Lara.gun_type)
		{
		case WEAPON_PISTOLS:
		case WEAPON_UZI:
			PistolHandler(Lara.gun_type);
			break;

		case WEAPON_REVOLVER:
		case WEAPON_SHOTGUN:
		case WEAPON_HK:
		case WEAPON_CROSSBOW:
			RifleHandler(Lara.gun_type);
			break;
		}

		break;

	case LG_FLARE:
		draw_flare();
		break;
	}
}

void InitialiseNewWeapon()
{
	Lara.target = 0;

	Lara.left_arm.x_rot = 0;
	Lara.left_arm.y_rot = 0;
	Lara.left_arm.z_rot = 0;
	Lara.left_arm.frame_number = 0;
	Lara.left_arm.lock = 0;
	Lara.left_arm.flash_gun = 0;

	Lara.right_arm.x_rot = 0;
	Lara.right_arm.y_rot = 0;
	Lara.right_arm.z_rot = 0;
	Lara.right_arm.frame_number = 0;
	Lara.right_arm.lock = 0;
	Lara.right_arm.flash_gun = 0;

	switch (Lara.gun_type)
	{
	case WEAPON_PISTOLS:
	case WEAPON_UZI:
		Lara.left_arm.frame_base = objects[PISTOLS_ANIM].frame_base; 
		Lara.right_arm.frame_base = objects[PISTOLS_ANIM].frame_base;

		if (Lara.gun_status != LG_NO_ARMS)
			draw_pistol_meshes(Lara.gun_type);

		break;

	case WEAPON_REVOLVER:
	case WEAPON_SHOTGUN:
	case WEAPON_HK:
		Lara.left_arm.frame_base = objects[WeaponObject(Lara.gun_type)].frame_base; 
		Lara.right_arm.frame_base = objects[WeaponObject(Lara.gun_type)].frame_base;

		if (Lara.gun_status != LG_NO_ARMS)
			draw_shotgun_meshes(Lara.gun_type);

		break;

	case WEAPON_FLARE:
		Lara.left_arm.frame_base = objects[FLARE_ANIM].frame_base; 
		Lara.right_arm.frame_base = objects[FLARE_ANIM].frame_base;

		if (Lara.gun_status != LG_NO_ARMS)
			draw_flare_meshes();

		break;

	default:
		Lara.left_arm.frame_base = anims[LaraItem->anim_number].frame_ptr; 
		Lara.right_arm.frame_base = anims[LaraItem->anim_number].frame_ptr;
		break;
	}
}

void LaraTargetInfo(WEAPON_INFO* winfo)
{
	GAME_VECTOR src, target;
	short ang[2];

	if (!Lara.target)
	{
		Lara.right_arm.lock = 0;
		Lara.left_arm.lock = 0;
		Lara.target_angles[0] = 0;
		Lara.target_angles[1] = 0;
		return;
	}

	src.x = 0;
	src.y = 0;
	src.z = 0;
	GetLaraJointPos((PHD_VECTOR*)&src, LMX_HAND_R);

	src.x = LaraItem->pos.x_pos;
	src.z = LaraItem->pos.z_pos;
	src.room_number = LaraItem->room_number;
	find_target_point(Lara.target, &target);
	phd_GetVectorAngles(target.x - src.x, target.y - src.y, target.z - src.z, ang);
	ang[0] -= LaraItem->pos.y_rot;
	ang[1] -= LaraItem->pos.x_rot;

	if (LOS(&src, &target))
	{
		if (ang[0] >= winfo->lock_angles[0] && ang[0] <= winfo->lock_angles[1] &&
			ang[1] >= winfo->lock_angles[2] && ang[1] <= winfo->lock_angles[3])
		{
			Lara.left_arm.lock = 1;
			Lara.right_arm.lock = 1;
			Lara.target_angles[0] = ang[0];
			Lara.target_angles[1] = ang[1];
			return;
		}

		if (Lara.left_arm.lock)
		{
			if (ang[0] < winfo->left_angles[0] || ang[0] > winfo->left_angles[1] ||
				ang[1] < winfo->left_angles[2] || ang[1] > winfo->left_angles[3])
				Lara.left_arm.lock = 0;
		}

		if (Lara.right_arm.lock)
		{
			if (ang[0] < winfo->right_angles[0] || ang[0] > winfo->right_angles[1] ||
				ang[1] < winfo->left_angles[2] || ang[1] > winfo->left_angles[3])
				Lara.right_arm.lock = 0;
		}
	}
	else
	{
		Lara.right_arm.lock = 0;
		Lara.left_arm.lock = 0;
	}

	Lara.target_angles[0] = ang[0];
	Lara.target_angles[1] = ang[1];
}

void LaraGetNewTarget(WEAPON_INFO* winfo)
{
	ITEM_INFO* item;
	ITEM_INFO* bestitem;
	static ITEM_INFO* TargetList[8];
	static ITEM_INFO* LastTargets[8];
	CREATURE_INFO* creature;
	GAME_VECTOR src, target;
	long x, y, z, slot, dist, maxdist, maxdist2, bestdist;
	short ang[2];
	short bestyrot, targets, match;

	if (BinocularRange)
	{
		Lara.target = 0;
		return;
	}

	bestitem = 0;
	src.x = LaraItem->pos.x_pos;
	src.y = LaraItem->pos.y_pos - 650;
	src.z = LaraItem->pos.z_pos;
	src.room_number = LaraItem->room_number;
	bestyrot = 0x7FFF;
	bestdist = 0x7FFFFFFF;
	maxdist = winfo->target_dist;
	maxdist2 = SQUARE(maxdist);
	creature = baddie_slots;
	targets = 0;

	for (slot = 0; slot < MAX_LOT; slot++, creature++)
	{
		if (creature->item_num == NO_ITEM)
			continue;

		item = &items[creature->item_num];

		if (item->hit_points <= 0)
			continue;

		x = item->pos.x_pos - src.x;
		y = item->pos.y_pos - src.y;
		z = item->pos.z_pos - src.z;

		if (abs(x) <= maxdist && abs(y) <= maxdist && abs(z) <= maxdist)
		{
			dist = SQUARE(x) + SQUARE(y) + SQUARE(z);

			if (dist < maxdist2)
			{
				find_target_point(item, &target);

				if (LOS(&src, &target))
				{
					phd_GetVectorAngles(target.x - src.x, target.y - src.y, target.z - src.z, ang);
					ang[0] -= (Lara.torso_y_rot + LaraItem->pos.y_rot);
					ang[1] -= (Lara.torso_x_rot + LaraItem->pos.x_rot);

					if (ang[0] >= winfo->lock_angles[0] && ang[0] <= winfo->lock_angles[1] &&
						ang[1] >= winfo->lock_angles[2] && ang[1] <= winfo->lock_angles[3])
					{
						TargetList[targets] = item;
						targets++;

						if (abs(ang[0]) < bestyrot + 2730 && dist < bestdist)
						{
							bestdist = dist;
							bestyrot = abs(ang[0]);
							bestitem = item;
						}
					}
				}
			}
		}
	}

	TargetList[targets] = 0;

	if (TargetList[0])
	{
		for (slot = 0; slot < 8; ++slot)
		{
			if (!TargetList[slot])
				Lara.target = 0;

			if (TargetList[slot] == Lara.target)
				break;
		}

		if (savegame.AutoTarget || KeyInput & IN_TARGET)
		{
			if (!Lara.target)
			{
				Lara.target = bestitem;
				LastTargets[0] = 0;
			}
			else if (KeyInput & IN_TARGET)
			{
				Lara.target = 0;

				for (match = 0; match < 8; ++match)
				{
					if (!TargetList[match])
						break;

					for (slot = 0; slot < 8; ++slot)
					{
						if (!LastTargets[slot])
						{
							slot = 8;
							break;
						}

						if (LastTargets[slot] == TargetList[match])
							break;
					}

					if (slot == 8)
					{
						Lara.target = TargetList[match];
						break;
					}
				}

				if (!Lara.target)
				{
					Lara.target = bestitem;
					LastTargets[0] = 0;
				}
			}
		}
	}
	else
		Lara.target = 0;

	if (Lara.target != LastTargets[0])
	{
		for (slot = 7; slot > 0; slot--)
			LastTargets[slot] = LastTargets[slot - 1];

		LastTargets[0] = Lara.target;
	}

	LaraTargetInfo(winfo);
}

void find_target_point(ITEM_INFO* item, GAME_VECTOR* target)
{
	long x, y, z, c, s;
	short* bounds;

	bounds = GetBestFrame(item);
	x = (bounds[0] + bounds[1]) >> 1;
	y = bounds[2] + (bounds[3] - bounds[2]) / 3;
	z = (bounds[4] + bounds[5]) >> 1;
	s = phd_sin(item->pos.y_rot);
	c = phd_cos(item->pos.y_rot);
	target->x = item->pos.x_pos + ((x * c + z * s) >> W2V_SHIFT);
	target->y = item->pos.y_pos + y;
	target->z = item->pos.z_pos + ((z * c - x * s) >> W2V_SHIFT);
	target->room_number = item->room_number;
}

void AimWeapon(WEAPON_INFO* winfo, LARA_ARM* arm)
{
	short speed, x, y;

	speed = winfo->aim_speed;

	if (arm->lock)
	{
		y = Lara.target_angles[0];
		x = Lara.target_angles[1];
	}
	else
	{
		x = 0;
		y = 0;
	}

	if (arm->y_rot >= y - speed && arm->y_rot <= speed + y)
		arm->y_rot = y;
	else if (arm->y_rot < y)
		arm->y_rot += speed;
	else
		arm->y_rot -= speed;

	if (arm->x_rot >= x - speed && arm->x_rot <= speed + x)
		arm->x_rot = x;
	else if (arm->x_rot < x)
		arm->x_rot += speed;
	else
		arm->x_rot -= speed;

	arm->z_rot = 0;
}

long FireWeapon(long weapon_type, ITEM_INFO* target, ITEM_INFO* src, short* angles)
{
	WEAPON_INFO* winfo;
	SPHERE* sptr;
	static PHD_3DPOS bum_view;
	static GAME_VECTOR bum_vdest;
	static GAME_VECTOR bum_vsrc;
	short* ammo;
	long r, nSpheres, bestdist, best;
	short room_number;

	bum_view.x_pos = 0;
	bum_view.y_pos = 0;
	bum_view.z_pos = 0;
	GetLaraJointPos((PHD_VECTOR*)&bum_view, LMX_HAND_R);
	ammo = get_current_ammo_pointer(weapon_type);

	if (!ammo[0])
		return 0;

	if (ammo[0] != -1)
		ammo[0]--;

	winfo = &weapons[weapon_type];

	bum_view.x_pos = src->pos.x_pos;
	bum_view.z_pos = src->pos.z_pos;
	bum_view.x_rot = short(winfo->shot_accuracy * (GetRandomControl() - 0x4000) / 0x10000 + angles[1]);
	bum_view.y_rot = short(winfo->shot_accuracy * (GetRandomControl() - 0x4000) / 0x10000 + angles[0]);
	bum_view.z_rot = 0;
	phd_GenerateW2V(&bum_view);

	nSpheres = GetSpheres(target, Slist, 0);
	best = -1;
	bestdist = 0x7FFFFFFF;

	for (int i = 0; i < nSpheres; i++)
	{
		sptr = &Slist[i];
		r = sptr->r;

		if (abs(sptr->x) < r && abs(sptr->y) < r && sptr->z > r && SQUARE(sptr->x) + SQUARE(sptr->y) <= SQUARE(r))
		{
			if (sptr->z - r < bestdist)
			{
				bestdist = sptr->z - r;
				best = i;
			}
		}
	}

	Lara.has_fired = 1;
	Lara.Fired = 1;
	bum_vsrc.x = bum_view.x_pos;
	bum_vsrc.y = bum_view.y_pos;
	bum_vsrc.z = bum_view.z_pos;
	room_number = src->room_number;
	GetFloor(bum_view.x_pos, bum_view.y_pos, bum_view.z_pos, &room_number);
	bum_vsrc.room_number = room_number;

	if (best < 0)
	{
		bum_vdest.x = bum_vsrc.x + (20480 * phd_mxptr->m20 >> W2V_SHIFT);
		bum_vdest.y = bum_vsrc.y + (20480 * phd_mxptr->m21 >> W2V_SHIFT);
		bum_vdest.z = bum_vsrc.z + (20480 * phd_mxptr->m22 >> W2V_SHIFT);
		GetTargetOnLOS(&bum_vsrc, &bum_vdest, 0, 1);
		return -1;
	}

	savegame.Game.AmmoHits++;
	bum_vdest.x = bum_vsrc.x + (bestdist * phd_mxptr->m20 >> W2V_SHIFT);
	bum_vdest.y = bum_vsrc.y + (bestdist * phd_mxptr->m21 >> W2V_SHIFT);
	bum_vdest.z = bum_vsrc.z + (bestdist * phd_mxptr->m22 >> W2V_SHIFT);

	if (!GetTargetOnLOS(&bum_vsrc, &bum_vdest, 0, 1))
		HitTarget(target, &bum_vdest, winfo->damage, 0);

	return 1;
}

void HitTarget(ITEM_INFO* item, GAME_VECTOR* hitpos, long damage, long grenade)
{
	OBJECT_INFO* obj;

	obj = &objects[item->object_number];
	item->hit_status = 1;

	if (item->data && item != LaraItem)
		((CREATURE_INFO*)item->data)->hurt_by_lara = 1;

	if (hitpos && obj->HitEffect)
	{
		switch (obj->HitEffect)
		{
		case 1:
			DoBloodSplat(hitpos->x, hitpos->y, hitpos->z, (GetRandomControl() & 3) + 3, item->pos.y_rot, item->room_number);
			break;

		case 3:
			TriggerRicochetSpark(hitpos, LaraItem->pos.y_rot, 3, 0);
			break;

		case 2:
			TriggerRicochetSpark(hitpos, LaraItem->pos.y_rot, 3, -5);
			SoundEffect(SFX_SWORD_GOD_HITMET, &item->pos, SFX_DEFAULT);
			break;
		}
	}

	if (obj->undead && !grenade && item->hit_points != -16384)
		return;

	if (item->hit_points > 0 && damage > item->hit_points)
		savegame.Level.Kills++;

	item->hit_points -= (short)damage;
}

void SmashItem(short item_number, long weapon_type)
{
	if (items[item_number].object_number >= SMASH_OBJECT1 && items[item_number].object_number <= SMASH_OBJECT8)
		SmashObject(item_number);
}

long WeaponObject(long weapon_type)
{
	switch (weapon_type)
	{
	case WEAPON_REVOLVER:
		return REVOLVER_ANIM;

	case WEAPON_UZI:
		return UZI_ANIM;

	case WEAPON_SHOTGUN:
		return SHOTGUN_ANIM;

	case WEAPON_HK:
		return HK_ANIM;

	case WEAPON_CROSSBOW:
		return CROSSBOW_ANIM;

	default:
		return PISTOLS_ANIM;
	}
}

long WeaponObjectMesh(long weapon_type)
{
	switch (weapon_type)
	{
	case WEAPON_REVOLVER:

		if (Lara.sixshooter_type_carried & W_LASERSIGHT)
			return LARA_REVOLVER_LASER;
		else
			return REVOLVER_ANIM;

	case WEAPON_UZI:
		return UZI_ANIM;

	case WEAPON_SHOTGUN:
		return SHOTGUN_ANIM;

	case WEAPON_HK:
		return HK_ANIM;

	case WEAPON_CROSSBOW:
		if (Lara.crossbow_type_carried & W_LASERSIGHT)
			return LARA_CROSSBOW_LASER;
		else
			return CROSSBOW_ANIM;

	default:
		return PISTOLS_ANIM;
	}
}

void DoProperDetection(short item_number, long x, long y, long z, long xv, long yv, long zv)
{
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	long ceiling, height, oldtype, oldonobj, oldheight, bs, yang, xs;
	short room_number;

	item = &items[item_number];
	room_number = item->room_number;
	floor = GetFloor(x, y, z, &room_number);
	oldheight = GetHeight(floor, x, y, z);
	room_number = item->room_number;
	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
	height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (item->pos.y_pos >= height)
	{
		bs = 0;
		oldtype = height_type;

		if ((oldtype == BIG_SLOPE || oldtype == DIAGONAL) && oldheight < height)
		{
			yang = (ushort) item->pos.y_rot;

			if (tiltyoff < 0 && yang >= 32768 ||
				tiltyoff > 0 && yang <= 32768 ||
				tiltxoff < 0 && yang >= 16384 && yang <= 49152 ||
				tiltxoff > 0 && (yang <= 16384 || yang >= 49152))
				bs = 1;
		}

		if (y > height + 32 && !bs && ((item->pos.x_pos ^ x) & 0xFFFFFC00 || (item->pos.z_pos ^ z) & 0xFFFFFC00))
		{
			xs = (item->pos.x_pos ^ x) & 0xFFFFFC00 && (item->pos.z_pos ^ z) & 0xFFFFFC00 ? abs(x - item->pos.x_pos) < abs(z - item->pos.z_pos) : 1;
			item->pos.y_rot = (item->pos.x_pos ^ x) & 0xFFFFFC00 && xs ? -item->pos.y_rot : -32768 - item->pos.y_rot;
			item->pos.x_pos = x;
			item->pos.y_pos = y;
			item->pos.z_pos = z;
			item->speed >>= 1;
		}
		else if (oldtype != BIG_SLOPE && oldtype != DIAGONAL)
		{
			if (item->fallspeed > 0)
			{
				if (item->fallspeed > 16)
				{
					item->fallspeed = -(item->fallspeed >> 2);

					if (item->fallspeed < -100)
						item->fallspeed = -100;
				}
				else
				{
					item->speed -= 3;
					item->fallspeed = 0;

					if (item->speed < 0)
						item->speed = 0;
				}
			}

			item->pos.y_pos = height;
		}
		else
		{
			item->speed -= item->speed >> 2;

			if (tiltyoff < 0 && abs(tiltyoff) - abs(tiltxoff) >= 2)
			{
				if ((ushort) item->pos.y_rot > 32768)
				{
					item->pos.y_rot = -1 - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed -= short(2 * tiltyoff);

						if ((ushort) item->pos.y_rot > 16384 && (ushort) item->pos.y_rot < 49152)
						{
							item->pos.y_rot -= 4096;

							if ((ushort) item->pos.y_rot < 16384)
								item->pos.y_rot = 16384;
						}
						else if ((ushort) item->pos.y_rot < 16384)
						{
							item->pos.y_rot += 4096;

							if ((ushort) item->pos.y_rot > 16384)
								item->pos.y_rot = 16384;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
			}
			else if (tiltyoff > 0 && abs(tiltyoff) - abs(tiltxoff) >= 2)
			{
				if ((ushort) item->pos.y_rot < 32768)
				{
					item->pos.y_rot = -1 - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += short(2 * tiltyoff);

						if ((ushort) item->pos.y_rot <= 49152 && (ushort) item->pos.y_rot >= 16384)
						{
							if ((ushort) item->pos.y_rot < 49152)
							{
								item->pos.y_rot += 4096;

								if ((ushort) item->pos.y_rot > 49152)
									item->pos.y_rot = -16384;
							}
						}
						else
						{
							item->pos.y_rot -= 4096;

							if ((ushort) item->pos.y_rot < 49152)
								item->pos.y_rot = -16384;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
			}
			else if (tiltxoff < 0 && abs(tiltxoff) - abs(tiltyoff) >= 2)
			{
				if ((ushort) item->pos.y_rot > 16384 && (ushort) item->pos.y_rot < 49152)
				{
					item->pos.y_rot = 32767 - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed -= short(2 * tiltxoff);

						if ((ushort) item->pos.y_rot < 32768)
						{
							item->pos.y_rot -= 4096;

							if ((ushort) item->pos.y_rot > 61440)
								item->pos.y_rot = 0;
						}
						else
						{
							item->pos.y_rot += 4096;

							if ((ushort) item->pos.y_rot < 4096)
								item->pos.y_rot = 0;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
			}
			else if (tiltxoff > 0 && abs(tiltxoff) - abs(tiltyoff) >= 2)
			{
				if ((ushort) item->pos.y_rot <= 49152 && (ushort) item->pos.y_rot >= 16384)
				{
					if (item->speed < 32)
					{
						item->speed += short(2 * tiltxoff);

						if ((ushort) item->pos.y_rot > 32768)
						{
							item->pos.y_rot -= 4096;

							if ((ushort) item->pos.y_rot < 32768)
								item->pos.y_rot = -32768;
						}
						else if ((ushort) item->pos.y_rot < 32768)
						{
							item->pos.y_rot += 4096;

							if ((ushort) item->pos.y_rot > 32768)
								item->pos.y_rot = -32768;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
				else
				{
					item->pos.y_rot = 32767 - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
			}
			else if (tiltyoff < 0 && tiltxoff < 0)
			{
				if ((ushort) item->pos.y_rot > 24576 && (ushort) item->pos.y_rot < 57344)
				{
					item->pos.y_rot = -16385 - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed -= short(tiltxoff + tiltyoff);

						if ((ushort) item->pos.y_rot > 8192 && (ushort) item->pos.y_rot < 40960)
						{
							item->pos.y_rot -= 4096;

							if ((ushort) item->pos.y_rot < 8192)
								item->pos.y_rot = 8192;
						}
						else if (item->pos.y_rot != 8192)
						{
							item->pos.y_rot += 4096;

							if ((ushort) item->pos.y_rot > 8192)
								item->pos.y_rot = 8192;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
			}
			else if (tiltyoff < 0 && tiltxoff > 0)
			{
				if ((ushort) item->pos.y_rot <= 40960 && (ushort) item->pos.y_rot >= 8192)
				{
					if (item->speed < 32)
					{
						item->speed += short(tiltxoff - tiltyoff);

						if ((ushort) item->pos.y_rot < 57344 && (ushort) item->pos.y_rot > 24576)
						{
							item->pos.y_rot -= 4096;

							if ((ushort) item->pos.y_rot < 24576)
								item->pos.y_rot = 24576;
						}
						else if (item->pos.y_rot != 24576)
						{
							item->pos.y_rot += 4096;

							if ((ushort) item->pos.y_rot > 24576)
								item->pos.y_rot = 24576;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
				else
				{
					item->pos.y_rot = 16383 - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
			}
			else if (tiltyoff > 0 && tiltxoff > 0)
			{
				if ((ushort) item->pos.y_rot <= 57344 && (ushort) item->pos.y_rot >= 24576)
				{
					if (item->speed < 32)
					{
						item->speed += short(tiltxoff + tiltyoff);

						if ((ushort) item->pos.y_rot >= 8192 && (ushort) item->pos.y_rot <= 40960)
						{
							if (item->pos.y_rot != -24576)
							{
								item->pos.y_rot += 4096;

								if ((ushort) item->pos.y_rot > 40960)
									item->pos.y_rot = -24576;
							}
						}
						else
						{
							item->pos.y_rot -= 4096;

							if ((ushort) item->pos.y_rot < 40960)
								item->pos.y_rot = -24576;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
				else
				{
					item->pos.y_rot = -16385 - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
			}
			else if (tiltyoff > 0 && tiltxoff < 0)
			{
				if ((ushort) item->pos.y_rot > 8192 && (ushort) item->pos.y_rot < 40960)
				{
					item->pos.y_rot = 16383 - item->pos.y_rot;

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed >> 1);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += short(tiltyoff - tiltxoff);

						if ((ushort) item->pos.y_rot >= 24576 && (ushort) item->pos.y_rot <= 57344)
						{
							if (item->pos.y_rot != -8192)
							{
								item->pos.y_rot += 4096;

								if ((ushort) item->pos.y_rot > 57344)
									item->pos.y_rot = -8192;
							}
						}
						else
						{
							item->pos.y_rot -= 4096;

							if ((ushort) item->pos.y_rot < 57344)
								item->pos.y_rot = -8192;
						}
					}

					item->fallspeed = item->fallspeed > 0 ? -(item->fallspeed >> 1) : 0;
				}
			}

			item->pos.x_pos = x;
			item->pos.y_pos = y;
			item->pos.z_pos = z;
		}
	}
	else
	{
		if (yv >= 0)
		{
			room_number = item->room_number;
			floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
			height = GetHeight(floor, item->pos.x_pos, y, item->pos.z_pos);
			oldonobj = OnObject;
			room_number = item->room_number;
			floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
			GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

			if (item->pos.y_pos >= height && oldonobj)
			{
				if (item->fallspeed > 0)
				{
					if (item->fallspeed > 16)
					{
						item->fallspeed = -(item->fallspeed >> 2);

						if (item->fallspeed < -100)
							item->fallspeed = -100;
					}
					else
					{
						item->speed -= 3;
						item->fallspeed = 0;

						if (item->speed < 0)
							item->speed = 0;
					}
				}

				item->pos.y_pos = height;
			}
		}

		room_number = item->room_number;
		floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
		ceiling = GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

		if (item->pos.y_pos < ceiling)
		{
			if (y < ceiling && ((item->pos.x_pos ^ x) & 0xFFFFFC00 || (item->pos.z_pos ^ z) & 0xFFFFFC00))
			{
				item->pos.y_rot = (item->pos.x_pos ^ x) & 0xFFFFFC00 ? -item->pos.y_rot : -32768 - item->pos.y_rot;
				item->pos.x_pos = x;
				item->pos.y_pos = y;
				item->pos.z_pos = z;
				item->speed >>= 1;
			}
			else
				item->pos.y_pos = ceiling;

			if (item->fallspeed < 0)
				item->fallspeed = -item->fallspeed;
		}
	}

	room_number = item->room_number;
	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

	if (room_number != item->room_number)
		ItemNewRoom(item_number, room_number);
}

short* get_current_ammo_pointer(long num)
{
	short* ammo;

	switch (num)
	{
	case WEAPON_REVOLVER:
		ammo = &Lara.num_revolver_ammo;
		break;

	case WEAPON_UZI:
		ammo = &Lara.num_uzi_ammo;
		break;

	case WEAPON_SHOTGUN:

		if (Lara.shotgun_type_carried & W_AMMO1)
			ammo = &Lara.num_shotgun_ammo1;
		else
			ammo = &Lara.num_shotgun_ammo2;

		break;

	case WEAPON_HK:
		ammo = &Lara.num_hk_ammo1;
		break;

	case WEAPON_CROSSBOW:

		if (Lara.crossbow_type_carried & W_AMMO1)
			ammo = &Lara.num_crossbow_ammo1;
		else
			ammo = &Lara.num_crossbow_ammo2;

		break;

	default:
		ammo = &Lara.num_pistols_ammo;
		break;
	}

	return ammo;
}
