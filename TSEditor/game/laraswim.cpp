#include "../tomb5/pch.h"
#include "laraswim.h"
#include "effect2.h"
#include "lara_states.h"
#include "lara.h"
#include "control.h"
#include "effects.h"
#include "sound.h"
#include "collide.h"
#include "laramisc.h"
#include "larafire.h"
#include "../specific/function_stubs.h"
#include "../specific/3dmath.h"
#include "camera.h"
#include "../specific/input.h"
#include "subsuit.h"

void LaraTestWaterDepth(ITEM_INFO* item, COLL_INFO* coll)
{
	FLOOR_INFO* floor;
	long wd;
	short room_number;

	room_number = item->room_number;
	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
	wd = GetWaterDepth(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, room_number);

	if (wd == NO_HEIGHT)
	{
		item->pos.x_pos = coll->old.x;
		item->pos.y_pos = coll->old.y;
		item->pos.z_pos = coll->old.z;
		item->fallspeed = 0;
	}
	else if (wd <= 512)
	{
		item->anim_number = ANIM_SWIM2QSTND;
		item->frame_number = anims[ANIM_SWIM2QSTND].frame_base;
		item->current_anim_state = AS_WATEROUT;
		item->goal_anim_state = AS_STOP;
		item->pos.z_rot = 0;
		item->pos.x_rot = 0;
		item->gravity_status = 0;
		item->speed = 0;
		item->fallspeed = 0;
		Lara.water_status = LW_WADE;
		item->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	}
}

void LaraSwimCollision(ITEM_INFO* item, COLL_INFO* coll)
{
	COLL_INFO coll2, coll3;
	long ox, oy, oz, pitch, height;
	short oxr, oyr, hit;

	hit = 0;
	ox = item->pos.x_pos;
	oy = item->pos.y_pos;
	oz = item->pos.z_pos;
	oxr = item->pos.x_rot;
	oyr = item->pos.y_rot;

	if (oxr < -0x4000 || oxr > 0x4000)
	{
		Lara.move_angle = item->pos.y_rot - 0x8000;
		coll->facing = item->pos.y_rot - 0x8000;
	}
	else
	{
		Lara.move_angle = item->pos.y_rot;
		coll->facing = item->pos.y_rot;
	}

	height = 762 * phd_sin(item->pos.x_rot) >> W2V_SHIFT;

	if (height < 0)
		height = -height;

	if (height < 200 + ((LaraDrawType == LARA_DIVESUIT) << 6))
		height = 200 + ((LaraDrawType == LARA_DIVESUIT) << 6);

	coll->bad_neg = -64;
	memcpy(&coll2, coll, sizeof(COLL_INFO));
	memcpy(&coll3, coll, sizeof(COLL_INFO));

	GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos + height / 2, item->pos.z_pos, item->room_number, height);

	coll2.facing += 0x2000;
	GetCollisionInfo(&coll2, item->pos.x_pos, item->pos.y_pos + height / 2, item->pos.z_pos, item->room_number, height);

	coll3.facing -= 0x2000;
	GetCollisionInfo(&coll3, item->pos.x_pos, item->pos.y_pos + height / 2, item->pos.z_pos, item->room_number, height);

	ShiftItem(item, coll);

	switch (coll->coll_type)
	{
	case CT_FRONT:

		if (item->pos.x_rot > 4550)
		{
			item->pos.x_rot += 182;
			hit = 1;
		}
		else if (item->pos.x_rot < -4550)
		{
			item->pos.x_rot -= 182;
			hit = 1;
		}
		else if (item->pos.x_rot > 910)
			item->pos.x_rot += 91;
		else if (item->pos.x_rot < -910)
			item->pos.x_rot -= 91;
		else if (item->pos.x_rot > 0)
			item->pos.x_rot += 45;
		else if (item->pos.x_rot < 0)
			item->pos.x_rot -= 45;
		else
		{
			hit = 1;
			item->fallspeed = 0;
		}

		if (coll2.coll_type == CT_LEFT)
			item->pos.y_rot += 364;
		else if (coll2.coll_type == CT_RIGHT)
			item->pos.y_rot -= 364;
		else if (coll3.coll_type == CT_LEFT)
			item->pos.y_rot += 364;
		else if (coll3.coll_type == CT_RIGHT)
			item->pos.y_rot -= 364;

		break;

	case CT_TOP:

		if (item->pos.x_rot >= -8190)
		{
			hit = 1;
			item->pos.x_rot -= 182;
		}

		break;

	case CT_TOP_FRONT:
		item->fallspeed = 0;
		hit = 1;
		break;

	case CT_LEFT:
		item->pos.y_rot += 364;
		hit = 1;
		break;

	case CT_RIGHT:
		item->pos.y_rot -= 364;
		hit = 1;
		break;

	case CT_CLAMP:
		hit = 2;
		item->pos.x_pos = coll->old.x;
		item->pos.y_pos = coll->old.y;
		item->pos.z_pos = coll->old.z;
		item->fallspeed = 0;
		break;
	}

	if (coll->mid_floor < 0 && coll->mid_floor != NO_HEIGHT)
	{
		hit = 1;
		item->pos.x_rot += 182;
		item->pos.y_pos += coll->mid_floor;
	}

	if ((ox != item->pos.x_pos || oy != item->pos.y_pos || oz != item->pos.z_pos || oxr != item->pos.x_rot || oyr != item->pos.y_rot) &&
		!SubHitCount && hit == 1 && item->fallspeed > 100)
	{
		if (LaraDrawType == LARA_DIVESUIT)
		{
			pitch = 0x8000 + (GetRandomControl() << 1);
			SoundEffect(SFX_SWIMSUIT_METAL_CLASH, &LaraItem->pos, SFX_ALWAYS | SFX_SETPITCH | (pitch << 8));
		}

		SubHitCount = 30;

		if (Lara.Anxiety < 96)
			Lara.Anxiety += 16;
	}

	if (hit != 2 && Lara.water_status != LW_FLYCHEAT)
		LaraTestWaterDepth(item, coll);
}

void SwimTurn(ITEM_INFO* item)
{
	if (KeyInput & IN_FORWARD)
		item->pos.x_rot -= 364;
	else if (KeyInput & IN_BACK)
		item->pos.x_rot += 364;

	if (KeyInput & IN_LEFT)
	{
		Lara.turn_rate -= 409;

		if (Lara.turn_rate < -1092)
			Lara.turn_rate = -1092;

		item->pos.z_rot -= 546;
	}
	else if (KeyInput & IN_RIGHT)
	{
		Lara.turn_rate += 409;

		if (Lara.turn_rate > 1092)
			Lara.turn_rate = 1092;

		item->pos.z_rot += 546;
	}
}

void SwimTurnSubsuit(ITEM_INFO* item)
{
	if (item->pos.y_pos < 14080)
		subsuit.YVel += short((14080 - item->pos.y_pos) >> 4);

	if (KeyInput & IN_FORWARD && item->pos.x_rot > -15470)
		subsuit.dXRot = -8190;
	else if (KeyInput & IN_BACK && item->pos.x_rot < 15470)
		subsuit.dXRot = 8190;
	else
		subsuit.dXRot = 0;

	if (KeyInput & IN_LEFT)
	{
		Lara.turn_rate -= 136;

		if (Lara.turn_rate < -1092)
			Lara.turn_rate = -1092;

		item->pos.z_rot -= 546;
	}
	else if (KeyInput & IN_RIGHT)
	{
		Lara.turn_rate += 136;

		if (Lara.turn_rate > 1092)
			Lara.turn_rate = 1092;

		item->pos.z_rot += 546;
	}
}

void UpdateSubsuitAngles()
{
	short vel, vol;

	if (subsuit.YVel)
	{
		LaraItem->pos.y_pos += subsuit.YVel >> 2;
		subsuit.YVel -= 1 + (subsuit.YVel >> 4);
	}

	vel = abs(LaraItem->fallspeed << 2);
	subsuit.Vel[0] = vel;
	subsuit.Vel[1] = vel;

	if (subsuit.XRot < subsuit.dXRot)
	{
		if (subsuit.XRot < 0 && subsuit.dXRot > 0)
			subsuit.XRot -= (subsuit.XRot >> 2);

		subsuit.XRot += 364;

		if (subsuit.XRot > subsuit.dXRot)
			subsuit.XRot = subsuit.dXRot;
	}
	else if (subsuit.XRot > subsuit.dXRot)
	{
		if (subsuit.XRot > 0 && subsuit.dXRot < 0)
			subsuit.XRot -= (subsuit.XRot >> 2);

		subsuit.XRot -= 364;

		if (subsuit.XRot < subsuit.dXRot)
			subsuit.XRot = subsuit.dXRot;
	}

	if (subsuit.dXRot)
	{
		if (subsuit.XRot > 364)
			LaraItem->pos.x_rot += 364;
		else if (subsuit.XRot < 364)
			LaraItem->pos.x_rot -= 364;
	}

	vel = abs(subsuit.XRot >> 3);
	subsuit.Vel[0] += vel;
	subsuit.Vel[1] += vel;

	if (Lara.turn_rate > 0)
		subsuit.Vel[0] += 2 * abs(Lara.turn_rate);
	else if (Lara.turn_rate < 0)
		subsuit.Vel[1] += 2 * abs(Lara.turn_rate);

	if (subsuit.Vel[0] > 1536)
		subsuit.Vel[0] = 1536;

	if (subsuit.Vel[1] > 1536)
		subsuit.Vel[1] = 1536;

	if (subsuit.Vel[0] || subsuit.Vel[1])
	{
		vol = (subsuit.Vel[0] + subsuit.Vel[1]) >> 6;

		if (vol > 31)
			vol = 31;

		SoundEffect(SFX_LARA_UNDERWATER_ENGINE, &LaraItem->pos, (vol << 8) | SFX_SETVOL | SFX_ALWAYS);
	}
}

void lara_as_swimcheat(ITEM_INFO* item, COLL_INFO* coll)
{
	if (KeyInput & IN_FORWARD)
		item->pos.x_rot -= 546;
	else if (KeyInput & IN_BACK)
		item->pos.x_rot += 546;

	if (KeyInput & IN_LEFT)
	{
		Lara.turn_rate -= 613;

		if (Lara.turn_rate < -1092)
			Lara.turn_rate = -1092;
	}
	else if (KeyInput & IN_RIGHT)
	{
		Lara.turn_rate += 613;

		if (Lara.turn_rate > 1092)
			Lara.turn_rate = 1092;
	}

	if (KeyInput & IN_ACTION)
		TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 31, 255, 255, 255);

	if (KeyInput & IN_ROLL)
		Lara.turn_rate = -2184;

	if (KeyInput & IN_JUMP)
	{
		item->fallspeed += 16;

		if (item->fallspeed > 400)
			item->fallspeed = 400;
	}
	else if (item->fallspeed >= 8)
		item->fallspeed -= item->fallspeed >> 3;
	else
		item->fallspeed = 0;
}

void lara_as_swim(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_UWDEATH;
		return;
	}

	if (KeyInput & IN_ROLL)
	{
		if (LaraDrawType != LARA_DIVESUIT)
		{
			item->current_anim_state = AS_WATERROLL;
			item->anim_number = ANIM_WATERROLL;
			item->frame_number = anims[ANIM_WATERROLL].frame_base;
			return;
		}
	}
	else if (LaraDrawType != LARA_DIVESUIT)
		SwimTurn(item);
	else
		SwimTurnSubsuit(item);

	item->fallspeed += 8;

	if (item->fallspeed > 200)
		item->fallspeed = 200;

	if (!(KeyInput & IN_JUMP))
		item->goal_anim_state = AS_GLIDE;
}

void lara_as_glide(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_UWDEATH;
		return;
	}

	if (KeyInput & IN_ROLL)
	{
		if (LaraDrawType != LARA_DIVESUIT)
		{
			item->current_anim_state = AS_WATERROLL;
			item->anim_number = ANIM_WATERROLL;
			item->frame_number = anims[ANIM_WATERROLL].frame_base;
			return;
		}
	}
	else if (LaraDrawType != LARA_DIVESUIT)
		SwimTurn(item);
	else
		SwimTurnSubsuit(item);

	if (KeyInput & IN_JUMP)
		item->goal_anim_state = AS_SWIM;

	item->fallspeed -= 6;

	if (item->fallspeed < 0)
		item->fallspeed = 0;

	if (item->fallspeed <= 133)
		item->goal_anim_state = AS_TREAD;
}

void lara_as_tread(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hit_points <= 0)
	{
		item->goal_anim_state = AS_UWDEATH;
		return;
	}

	if (KeyInput & IN_ROLL && LaraDrawType != LARA_DIVESUIT)
	{
		item->current_anim_state = AS_WATERROLL;
		item->anim_number = ANIM_WATERROLL;
		item->frame_number = anims[ANIM_WATERROLL].frame_base;
		return;
	}

	if (KeyInput & IN_LOOK)
		LookUpDown();

	if (LaraDrawType == LARA_DIVESUIT)
		SwimTurnSubsuit(item);
	else
		SwimTurn(item);

	if (KeyInput & IN_JUMP)
		item->goal_anim_state = AS_SWIM;

	item->fallspeed -= 6;

	if (item->fallspeed < 0)
		item->fallspeed = 0;

	if (Lara.gun_status == LG_HANDS_BUSY)
		Lara.gun_status = LG_NO_ARMS;
}

void lara_as_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	if (KeyInput & IN_FORWARD)
		item->pos.x_rot -= 182;
}

void lara_as_uwdeath(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.look = 0;
	item->fallspeed -= 8;

	if (item->fallspeed <= 0)
		item->fallspeed = 0;

	if (item->pos.x_rot >= -364 && item->pos.x_rot <= 364)
		item->pos.x_rot = 0;
	else if (item->pos.x_rot < 0)
		item->pos.x_rot += 364;
	else
		item->pos.x_rot -= 364;
}

void lara_as_waterroll(ITEM_INFO* item, COLL_INFO* coll)
{
	item->fallspeed = 0;
}

void lara_col_swim(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraSwimCollision(item, coll);
}

void lara_col_glide(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraSwimCollision(item, coll);
}

void lara_col_tread(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraSwimCollision(item, coll);
}

void lara_col_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraSwimCollision(item, coll);
}

void lara_col_uwdeath(ITEM_INFO* item, COLL_INFO* coll)
{
	long wh;

	item->hit_points = -1;
	Lara.air = -1;
	Lara.gun_status = LG_HANDS_BUSY;
	wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);

	if (wh != NO_HEIGHT)
	{
		if (wh < item->pos.y_pos - 100)
			item->pos.y_pos -= 5;
	}

	LaraSwimCollision(item, coll);
}

void lara_col_waterroll(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraSwimCollision(item, coll);
}

void LaraUnderWater(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->bad_pos = -NO_HEIGHT;
	coll->bad_neg = -400;
	coll->bad_ceiling = 400;
	coll->old.x = item->pos.x_pos;
	coll->old.y = item->pos.y_pos;
	coll->old.z = item->pos.z_pos;
	coll->slopes_are_walls = 0;
	coll->slopes_are_pits = 0;
	coll->lava_is_pit = 0;
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 1;
	coll->radius = 300;
	coll->trigger = 0;

	if (KeyInput & IN_LOOK && Lara.look)
		LookLeftRight();
	else
		ResetLook();

	Lara.look = 1;
	lara_control_routines[item->current_anim_state](item, coll);

	if (LaraDrawType == LARA_DIVESUIT)
	{
		if (Lara.turn_rate < -91)
			Lara.turn_rate += 91;
		else if (Lara.turn_rate > 91)
			Lara.turn_rate -= 91;
		else
			Lara.turn_rate = 0;
	}
	else
	{
		if (Lara.turn_rate < -364)
			Lara.turn_rate += 364;
		else if (Lara.turn_rate > 364)
			Lara.turn_rate -= 364;
		else
			Lara.turn_rate = 0;
	}

	item->pos.y_rot += Lara.turn_rate;

	if (LaraDrawType == LARA_DIVESUIT)
		UpdateSubsuitAngles();

	if (item->pos.z_rot < -364)
		item->pos.z_rot += 364;
	else if (item->pos.z_rot > 364)
		item->pos.z_rot -= 364;
	else
		item->pos.z_rot = 0;

	if (item->pos.x_rot < -15470)
		item->pos.x_rot = -15470;
	else if (item->pos.x_rot > 15470)
		item->pos.x_rot = 15470;

	if (LaraDrawType == LARA_DIVESUIT)
	{
		if (item->pos.z_rot < -8008)
			item->pos.z_rot = -8008;
		else if (item->pos.z_rot > 8008)
			item->pos.z_rot = 8008;
	}
	else if (item->pos.z_rot < -4004)
		item->pos.z_rot = -4004;
	else if (item->pos.z_rot > 4004)
		item->pos.z_rot = 4004;

	if (Lara.current_active && Lara.water_status != LW_FLYCHEAT)
		LaraWaterCurrent(coll);

	AnimateLara(item);
	item->pos.x_pos += (item->fallspeed * phd_sin(item->pos.y_rot) >> 16) * phd_cos(item->pos.x_rot) >> W2V_SHIFT;
	item->pos.y_pos -= item->fallspeed * phd_sin(item->pos.x_rot) >> (W2V_SHIFT + 2);
	item->pos.z_pos += (item->fallspeed * phd_cos(item->pos.y_rot) >> 16) * phd_cos(item->pos.x_rot) >> W2V_SHIFT;
	LaraBaddieCollision(item, coll);
	lara_collision_routines[item->current_anim_state](item, coll);
	UpdateLaraRoom(item, 0);
	LaraGun();
	TestTriggers(coll->trigger, 0, 0);
}

long GetWaterDepth(long x, long y, long z, short room_number)
{
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	long x_floor, y_floor, h;
	short door;

	r = &room[room_number];

	do
	{
		x_floor = (z - r->z) >> 10;
		y_floor = (x - r->x) >> 10;

		if (x_floor <= 0)
		{
			x_floor = 0;

			if (y_floor < 1)
				y_floor = 1;
			else if (y_floor > r->y_size - 2)
				y_floor = r->y_size - 2;
		}
		else if (x_floor >= r->x_size - 1)
		{
			x_floor = r->x_size - 1;

			if (y_floor < 1)
				y_floor = 1;
			else if (y_floor > r->y_size - 2)
				y_floor = r->y_size - 2;
		}
		else if (y_floor < 0)
			y_floor = 0;
		else if (y_floor >= r->y_size)
			y_floor = r->y_size - 1;

		floor = &r->floor[x_floor + y_floor * r->x_size];
		door = GetDoor(floor);

		if (door != NO_ROOM)
		{
			room_number = door;
			r = &room[door];
		}

	} while (door != NO_ROOM);

	if (r->flags & ROOM_UNDERWATER)
	{
		while (floor->sky_room != NO_ROOM)
		{
			r = &room[floor->sky_room];

			if (!(r->flags & ROOM_UNDERWATER))
			{
				h = GetMinimumCeiling(floor, x, z);
				floor = GetFloor(x, y, z, &room_number);
				return GetHeight(floor, x, y, z) - h;
			}

			floor = &r->floor[((z - r->z) >> 10) + r->x_size * ((x - r->x) >> 10)];
		}

		return 0x7FFF;
	}
	else
	{
		while (floor->pit_room != NO_ROOM)
		{
			r = &room[floor->pit_room];

			if (r->flags & ROOM_UNDERWATER)
			{
				h = GetMaximumFloor(floor, x, z);
				floor = GetFloor(x, y, z, &room_number);
				return GetHeight(floor, x, y, z) - h;
			}

			floor = &r->floor[((z - r->z) >> 10) + r->x_size * ((x - r->x) >> 10)];
		}

		return NO_HEIGHT;
	}
}

void LaraWaterCurrent(COLL_INFO* coll)
{
	long angle, speed, sinkval, shifter, absvel;

	if (Lara.current_active)
	{
		sinkval = Lara.current_active - 1;
		speed = camera.fixed[sinkval].data;
		angle = ((mGetAngle(camera.fixed[sinkval].x, camera.fixed[sinkval].z, LaraItem->pos.x_pos, LaraItem->pos.z_pos) - 0x4000) >> 4) & 0xFFF;
		Lara.current_xvel += short((((speed * rcossin_tbl[2 * angle]) >> 2) - Lara.current_xvel) >> 4);
		Lara.current_zvel += short((((speed * rcossin_tbl[2 * angle + 1]) >> 2) - Lara.current_zvel) >> 4);
		LaraItem->pos.y_pos += (camera.fixed[sinkval].y - LaraItem->pos.y_pos) >> 4;
	}
	else
	{
		absvel = abs(Lara.current_xvel);

		if (absvel > 16)
			shifter = 4;
		else if (absvel > 8)
			shifter = 3;
		else
			shifter = 2;

		Lara.current_xvel -= Lara.current_xvel >> shifter;

		if (abs(Lara.current_xvel) < 4)
			Lara.current_xvel = 0;

		absvel = abs(Lara.current_zvel);

		if (absvel > 16)
			shifter = 4;
		else if (absvel > 8)
			shifter = 3;
		else
			shifter = 2;

		Lara.current_zvel -= Lara.current_zvel >> shifter;

		if (abs(Lara.current_zvel) < 4)
			Lara.current_zvel = 0;

		if (!Lara.current_xvel && !Lara.current_zvel)
			return;
	}

	LaraItem->pos.x_pos += Lara.current_xvel >> 8;
	LaraItem->pos.z_pos += Lara.current_zvel >> 8;
	Lara.current_active = 0;
	coll->facing = (short)phd_atan(LaraItem->pos.z_pos - coll->old.z, LaraItem->pos.x_pos - coll->old.x);
	GetCollisionInfo(coll, LaraItem->pos.x_pos, LaraItem->pos.y_pos + 200, LaraItem->pos.z_pos, LaraItem->room_number, 400);

	switch (coll->coll_type)
	{
	case CT_FRONT:

		if (LaraItem->pos.x_rot > 6370)
			LaraItem->pos.x_rot += 182;
		else if (LaraItem->pos.x_rot < -6370)
			LaraItem->pos.x_rot -= 182;
		else
			LaraItem->fallspeed = 0;

		break;

	case CT_TOP:
		LaraItem->pos.x_rot -= 182;
		break;

	case CT_TOP_FRONT:
		LaraItem->fallspeed = 0;
		break;

	case CT_LEFT:
		LaraItem->pos.y_rot += 910;
		break;

	case CT_RIGHT:
		LaraItem->pos.y_rot -= 910;
		break;
	}

	if (coll->mid_floor < 0 && coll->mid_floor != NO_HEIGHT)
		LaraItem->pos.y_pos += coll->mid_floor;

	ShiftItem(LaraItem, coll);
	coll->old.x = LaraItem->pos.x_pos;
	coll->old.y = LaraItem->pos.y_pos;
	coll->old.z = LaraItem->pos.z_pos;
}
