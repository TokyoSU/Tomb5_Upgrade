#include "../tomb5/pch.h"
#include "deathsld.h"
#include "../specific/function_stubs.h"
#include "../game/lara_states.h"
#include "collide.h"
#include "laramisc.h"
#include "items.h"
#include "control.h"
#include "sound.h"
#include "../specific/3dmath.h"
#include "../specific/input.h"
#include "objects.h"
#include "lara.h"

static short DeathSlideBounds[] = {-256, 256, -100, 100, 256, 512, 0, 0, -4550, 4550, 0, 0};
static PHD_VECTOR DeathSlidePosition = {0, 0, 371};

static GAME_VECTOR* GetDeathSlidePrevPosition(ITEM_INFO* item) {
	return static_cast<GAME_VECTOR*>(item->data);
}

void InitialiseDeathSlide(short item_number)
{
	auto* item = &items[item_number];
	auto* prevPos = (GAME_VECTOR*)MALLOC_AllocateMemory(sizeof(GAME_VECTOR));
	item->data = prevPos;
	prevPos->x = item->pos.x_pos;
	prevPos->y = item->pos.y_pos;
	prevPos->z = item->pos.z_pos;
	prevPos->room_number = item->room_number;
}

void DeathSlideCollision(short item_number, ITEM_INFO* l, COLL_INFO* coll)
{
	if (KeyInput & IN_ACTION && !l->gravity_status && Lara.gunStatus == LG_NO_ARMS && l->current_anim_state == AS_STOP)
	{
		auto* item = &items[item_number];
		if (item->status == ITEM_INACTIVE && TestLaraPosition(DeathSlideBounds, item, l))
		{
			AlignLaraPosition(&DeathSlidePosition, item, l);
			Lara.gunStatus = LG_HANDS_BUSY;
			l->goal_anim_state = AS_DEATHSLIDE;

			do AnimateLara(l); while (l->current_anim_state != AS_NULL);

			if (!item->active)
				AddActiveItem(item_number);

			item->status = ITEM_ACTIVE;
			item->flags |= IFL_INVISIBLE;
		}
	}
}

void ControlDeathSlide(short item_number)
{
	auto* item = &items[item_number];
	if (item->status != ITEM_ACTIVE)
		return;

	if (item->flags & IFL_INVISIBLE)
	{
		if (item->current_anim_state == 1)
		{
			AnimateItem(item);
			return;
		}

		AnimateItem(item);

		if (item->fallspeed < 100)
			item->fallspeed += 5;

		item->pos.x_pos += item->fallspeed * phd_sin(item->pos.y_rot) >> W2V_SHIFT;
		item->pos.y_pos += item->fallspeed >> 2;
		item->pos.z_pos += item->fallspeed * phd_cos(item->pos.y_rot) >> W2V_SHIFT;
		auto room_number = item->room_number;
		GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

		if (item->room_number != room_number)
			ItemNewRoom(item_number, room_number);

		if (LaraItem->current_anim_state == AS_DEATHSLIDE)
		{
			LaraItem->pos.x_pos = item->pos.x_pos;
			LaraItem->pos.y_pos = item->pos.y_pos;
			LaraItem->pos.z_pos = item->pos.z_pos;
		}

		auto x = item->pos.x_pos + (1024 * phd_sin(item->pos.y_rot) >> W2V_SHIFT);
		auto y = item->pos.y_pos + 64;
		auto z = item->pos.z_pos + (1024 * phd_cos(item->pos.y_rot) >> W2V_SHIFT);
		auto* floor = GetFloor(x, y, z, &room_number);

		if (GetHeight(floor, x, y, z) <= y + 256 || GetCeiling(floor, x, y, z) >= y - 256)
		{
			if (LaraItem->current_anim_state == AS_DEATHSLIDE)
			{
				LaraItem->goal_anim_state = 3;
				AnimateLara(LaraItem);
				LaraItem->gravity_status = 1;
				LaraItem->speed = item->fallspeed;
				LaraItem->fallspeed = item->fallspeed >> 2;
			}

			SoundEffect(SFX_COGS_ROME, &item->pos, SFX_DEFAULT);
			RemoveActiveItem(item_number);
			item->status = ITEM_INACTIVE;
			item->flags -= IFL_INVISIBLE;
		}
		else
			SoundEffect(SFX_GOD_HEAD_LASER_LOOPS, &item->pos, SFX_DEFAULT);
	}
	else
	{
		auto* old = GetDeathSlidePrevPosition(item);
		item->pos.x_pos = old->x;
		item->pos.y_pos = old->y;
		item->pos.z_pos = old->z;

		if (item->room_number != old->room_number)
			ItemNewRoom(item_number, old->room_number);

		item->status = ITEM_INACTIVE;
		item->current_anim_state = 1;
		item->goal_anim_state = 1;
		item->anim_number = objects[item->object_number].anim_index;
		item->frame_number = anims[item->anim_number].frame_base;
		RemoveActiveItem(item_number);
	}
}
