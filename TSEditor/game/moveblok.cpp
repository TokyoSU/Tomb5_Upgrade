#include "../tomb5/pch.h"
#include "moveblok.h"
#include "lara_states.h"
#include "sound.h"
#include "delstuff.h"
#include "control.h"
#include "items.h"
#include "draw.h"
#include "collide.h"
#include "tomb4fx.h"
#include "../specific/input.h"
#include "lara.h"
#include "box.h"

static short MovingBlockBounds[12] = { 0, 0, -256, 0, 0, 0, -1820, 1820, -5460, 5460, -1820, 1820 };
static PHD_VECTOR MovingBlockPos = { 0, 0, 0 };

static void ClearMovableBlockSplitters(long x, long y, long z, short room_number)
{
	FLOOR_INFO* floor;
	short room_num, height;

	floor = GetFloor(x, y, z, &room_number);
	boxes[floor->box].overlap_index = ~0x4000;
	height = boxes[floor->box].height;
	room_num = room_number;
	floor = GetFloor(x + 1024, y, z, &room_number);

	if (floor->box != 0x7FF)
	{
		if (boxes[floor->box].height == height && boxes[floor->box].overlap_index & 0x8000 && boxes[floor->box].overlap_index & 0x4000)
			ClearMovableBlockSplitters(x + 1024, y, z, room_number);
	}

	room_number = room_num;
	floor = GetFloor(x - 1024, y, z, &room_number);

	if (floor->box != 0x7FF)
	{
		if (boxes[floor->box].height == height && boxes[floor->box].overlap_index & 0x8000 && boxes[floor->box].overlap_index & 0x4000)
			ClearMovableBlockSplitters(x - 1024, y, z, room_number);
	}

	room_number = room_num;
	floor = GetFloor(x, y, z + 1024, &room_number);

	if (floor->box != 0x7FF)
	{
		if (boxes[floor->box].height == height && boxes[floor->box].overlap_index & 0x8000 && boxes[floor->box].overlap_index & 0x4000)
			ClearMovableBlockSplitters(x, y, z + 1024, room_number);
	}

	room_number = room_num;
	floor = GetFloor(x, y, z - 1024, &room_number);

	if (floor->box != 0x7FF)
	{
		if (boxes[floor->box].height == height && boxes[floor->box].overlap_index & 0x8000 && boxes[floor->box].overlap_index & 0x4000)
			ClearMovableBlockSplitters(x, y, z - 1024, room_number);
	}
}

void InitialiseMovingBlock(short item_number)
{
	ITEM_INFO* item;
	
	item = &items[item_number];
	ClearMovableBlockSplitters(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
}

void MovableBlock(short item_number)
{
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	PHD_VECTOR pos;
	long offset;
	ushort quadrant;
	short frame, base, room_number;
	static char sfx = 0;

	item = &items[item_number];
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;
	quadrant = ushort(LaraItem->pos.y_rot + 0x2000) / 0x4000;

	switch (LaraItem->anim_number)
	{
	case ANIM_PUSH:
		frame = LaraItem->frame_number;
		base = anims[ANIM_PUSH].frame_base;

		if ((frame >= base + 30 && frame <= base + 67) || (frame >= base + 78 && frame <= base + 125) || (frame >= base + 140 && frame <= base + 160))
		{
			SoundEffect(SFX_PUSHABLE_SOUND, &item->pos, SFX_ALWAYS);
			sfx = 1;
		}
		else if (sfx)
		{
			SoundEffect(SFX_PUSH_BLOCK_END, &item->pos, SFX_ALWAYS);
			sfx = 0;
		}

		GetLaraJointPos(&pos, LMX_HAND_L);

		switch (quadrant)
		{
		case NORTH:
			offset = pos.z + *(long*)&item->item_flags[2] - *(long*)&LaraItem->item_flags[2];

			if (abs(item->pos.z_pos - offset) < 512 && item->pos.z_pos < offset)
				item->pos.z_pos = offset;

			break;

		case EAST:
			offset = pos.x + *(long*)item->item_flags - *(long*)LaraItem->item_flags;

			if (abs(item->pos.x_pos - offset) < 512 && item->pos.x_pos < offset)
				item->pos.x_pos = offset;

			break;

		case SOUTH:
			offset = pos.z + *(long*)&item->item_flags[2] - *(long*)&LaraItem->item_flags[2];

			if (abs(item->pos.z_pos - offset) < 512 && item->pos.z_pos > offset)
				item->pos.z_pos = offset;

			break;

		case WEST:
			offset = pos.x + *(long*)item->item_flags - *(long*)LaraItem->item_flags;

			if (abs(item->pos.x_pos - offset) < 512 && item->pos.x_pos > offset)
				item->pos.x_pos = offset;

			break;
		}


		if (LaraItem->frame_number == anims[LaraItem->anim_number].frame_end - 1)
		{
			if (!(KeyInput & IN_ACTION))
				LaraItem->goal_anim_state = AS_STOP;
			else if (!TestBlockPush(item, 1024, quadrant))
				LaraItem->goal_anim_state = AS_STOP;
		}

		break;

	case ANIM_PULL:
		frame = LaraItem->frame_number;
		base = anims[ANIM_PULL].frame_base;

		if ((frame >= base + 40 && frame <= base + 122) || (frame >= base + 130 && frame <= base + 170))
		{
			SoundEffect(SFX_PUSHABLE_SOUND, &item->pos, SFX_ALWAYS);
			sfx = 1;
		}
		else if (sfx)
		{
			SoundEffect(SFX_PUSH_BLOCK_END, &item->pos, SFX_ALWAYS);
			sfx = 0;
		}

		GetLaraJointPos(&pos, LMX_HAND_L);

		switch (quadrant)
		{
		case NORTH:
			offset = pos.z + *(long*)&item->item_flags[2] - *(long*)&LaraItem->item_flags[2];

			if (abs(item->pos.z_pos - offset) < 512 && item->pos.z_pos > offset)
				item->pos.z_pos = offset;

			break;

		case EAST:
			offset = pos.x + *(long*)item->item_flags - *(long*)LaraItem->item_flags;

			if (abs(item->pos.x_pos - offset) < 512 && item->pos.x_pos > offset)
				item->pos.x_pos = offset;

			break;

		case SOUTH:
			offset = pos.z + *(long*)&item->item_flags[2] - *(long*)&LaraItem->item_flags[2];

			if (abs(item->pos.z_pos - offset) < 512 && item->pos.z_pos < offset)
				item->pos.z_pos = offset;

			break;

		case WEST:
			offset = pos.x + *(long*)item->item_flags - *(long*)LaraItem->item_flags;

			if (abs(item->pos.x_pos - offset) < 512 && item->pos.x_pos < offset)
				item->pos.x_pos = offset;

			break;
		}

		if (LaraItem->frame_number == anims[LaraItem->anim_number].frame_end - 1)
		{
			if (!(KeyInput & IN_ACTION))
				LaraItem->goal_anim_state = AS_STOP;
			else if (!TestBlockPull(item, 1024, quadrant))
				LaraItem->goal_anim_state = AS_STOP;
		}

		break;

	case 417:
	case 418:
		frame = LaraItem->frame_number;

		if (frame == anims[417].frame_base || frame == anims[418].frame_base)
		{
			item->pos.x_pos = (item->pos.x_pos & -512) | 512;
			item->pos.z_pos = (item->pos.z_pos & -512) | 512;
		}

		if (frame == anims[LaraItem->anim_number].frame_end)
		{
			room_number = item->room_number;
			floor = GetFloor(item->pos.x_pos, item->pos.y_pos - 256, item->pos.z_pos, &room_number);
			GetHeight(floor, item->pos.x_pos, item->pos.y_pos - 256, item->pos.z_pos);
			TestTriggers(trigger_index, 1, item->flags & IFL_CODEBITS);
			RemoveActiveItem(item_number);
			item->status = ITEM_INACTIVE;
		}

		break;
	}
}

void MovableBlockCollision(short item_number, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	PHD_VECTOR pos;
	short* bounds;
	short room_number, yrot, quadrant;

	item = &items[item_number];
	room_number = item->room_number;
	floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
	item->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	if (item->room_number != room_number)
		ItemNewRoom(item_number, room_number);

	if (KeyInput & IN_ACTION && l->current_anim_state == AS_STOP && l->anim_number == ANIM_BREATH && !l->gravity_status &&
		Lara.gunStatus == LG_NO_ARMS && item->status == ITEM_INACTIVE && item->trigger_flags >= 0 ||
		(Lara.isMoving && Lara.generalPtr == (void*)item_number))
	{
		room_number = l->room_number;
		GetFloor(item->pos.x_pos, item->pos.y_pos - 256, item->pos.z_pos, &room_number);

		if (item->room_number == room_number)
		{
			bounds = GetBoundsAccurate(item);
			MovingBlockBounds[0] = (bounds[0] >> 1) - 100;
			MovingBlockBounds[1] = (bounds[1] >> 1) + 100;
			MovingBlockBounds[4] = bounds[4] - 200;
			MovingBlockBounds[5] = 0;
			yrot = item->pos.y_rot;
			item->pos.y_rot = (l->pos.y_rot + 0x2000) & 0xC000;

			if (TestLaraPosition(MovingBlockBounds, item, l))
			{
				if (((ushort(yrot + 0x2000) / 0x4000) + (ushort(item->pos.y_rot) / 0x4000)) & 1)
					MovingBlockPos.z = bounds[0] - 95;
				else
					MovingBlockPos.z = bounds[4] - 95;

				if (MoveLaraPosition(&MovingBlockPos, item, l))
				{
					l->anim_number = ANIM_PPREADY;
					l->frame_number = anims[ANIM_PPREADY].frame_base;
					l->current_anim_state = AS_PPREADY;
					l->goal_anim_state = AS_PPREADY;
					Lara.isMoving = 0;
					Lara.gunStatus = LG_HANDS_BUSY;
					Lara.cornerX = (long)item;
				}
				else
					Lara.generalPtr = (void*)item_number;
			}
			else if (Lara.isMoving && Lara.generalPtr == (void*)item_number)
			{
				Lara.isMoving = 0;
				Lara.gunStatus = LG_NO_ARMS;
			}

			item->pos.y_rot = yrot;
		}
	}
	else if (l->current_anim_state == AS_PPREADY && l->frame_number == anims[ANIM_PPREADY].frame_base + 19 && (ITEM_INFO*)Lara.cornerX == item)
	{
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;
		quadrant = ushort(l->pos.y_rot + 0x2000) / 0x4000;

		if (KeyInput & IN_FORWARD)
		{
			if (!TestBlockPush(item, 1024, quadrant))
				return;

			l->goal_anim_state = AS_PUSHBLOCK;
		}
		else if (KeyInput & IN_BACK)
		{
			if (!TestBlockPull(item, 1024, quadrant))
				return;

			l->goal_anim_state = AS_PULLBLOCK;
		}
		else
			return;

		AddActiveItem(item_number);
		item->status = ITEM_ACTIVE;
		Lara.headRotY = 0;
		Lara.headRotX = 0;
		Lara.torsoRotY = 0;
		Lara.torsoRotX = 0;
		GetLaraJointPos(&pos, LMX_HAND_L);
		*(long*)&l->item_flags[0] = pos.x;
		*(long*)&l->item_flags[2] = pos.z;
		*(long*)&item->item_flags[0] = item->pos.x_pos;
		*(long*)&item->item_flags[2] = item->pos.z_pos;
	}
	else
		ObjectCollision(item_number, l, coll);
}

long TestBlockPush(ITEM_INFO* item, long height, ushort quadrant)
{
	ITEM_INFO** itemlist;
	FLOOR_INFO* floor;
	ROOM_INFO* r;
	long x, y, z, rx, rz;
	short room_number;

	x = item->pos.x_pos;
	y = item->pos.y_pos;
	z = item->pos.z_pos;

	switch (quadrant)
	{
	case NORTH:
		z += 1024;
		break;

	case EAST:
		x += 1024;
		break;

	case SOUTH:
		z -= 1024;
		break;

	case WEST:
		x -= 1024;
		break;
	}

	room_number = item->room_number;
	floor = GetFloor(x, y - 256, z, &room_number);
	r = &room[room_number];
	rx = (x - r->x) >> 10;
	rz = (z - r->z) >> 10;

	if (r->floor[rx * r->x_size + rz].stopper)
		return 0;

	if (GetHeight(floor, x, y - 256, z) != y)
		return 0;

	GetHeight(floor, x, y, z);

	if (height_type != WALL)
		return 0;

	y -= height - 100;
	floor = GetFloor(x, y, z, &room_number);

	if (GetCeiling(floor, x, y, z) > y)
		return 0;

	rx = item->pos.x_pos;
	rz = item->pos.z_pos;
	item->pos.x_pos = x;
	item->pos.z_pos = z;
	itemlist = (ITEM_INFO**)&tsv_buffer[0];
	GetCollidedObjects(item, 256, 1, itemlist, 0, 0);
	item->pos.x_pos = rx;
	item->pos.z_pos = rz;
	return !*itemlist;
}

long TestBlockPull(ITEM_INFO* item, long height, ushort quadrant)
{
	ITEM_INFO** itemlist;
	FLOOR_INFO* floor;
	ROOM_INFO* r;
	long x, y, z, destx, destz, rx, rz;
	short room_number;

	destx = 0;
	destz = 0;

	switch (quadrant)
	{
	case NORTH:
		destz = -1024;
		break;

	case EAST:
		destx = -1024;
		break;

	case SOUTH:
		destz = 1024;
		break;

	case WEST:
		destx = 1024;
		break;
	}

	x = item->pos.x_pos + destx;
	y = item->pos.y_pos;
	z = item->pos.z_pos + destz;
	room_number = item->room_number;
	floor = GetFloor(x, y - 256, z, &room_number);
	r = &room[room_number];
	rx = (x - r->x) >> 10;
	rz = (z - r->z) >> 10;

	if (r->floor[rx * r->x_size + rz].stopper)
		return 0;

	if (GetHeight(floor, x, y - 256, z) != y)
		return 0;

	floor = GetFloor(x, y - height, z, &room_number);

	if (floor->ceiling << 8 > y - height)
		return 0;

	rx = item->pos.x_pos;
	rz = item->pos.z_pos;
	itemlist = (ITEM_INFO**)&tsv_buffer[0];
	GetCollidedObjects(item, 256, 1, itemlist, 0, 0);
	item->pos.x_pos = rx;
	item->pos.z_pos = rz;

	if (*itemlist)
		return 0;

	x += destx;
	z += destz;
	room_number = item->room_number;
	floor = GetFloor(x, y - 256, z, &room_number);

	if (GetHeight(floor, x, y - 256, z) != y)
		return 0;

	floor = GetFloor(x, y - 762, z, &room_number);

	if (floor->ceiling << 8 > y - 762)
		return 0;

	x = LaraItem->pos.x_pos + destx;
	y = LaraItem->pos.y_pos;
	z = LaraItem->pos.z_pos + destz;
	room_number = LaraItem->room_number;
	GetFloor(x, y, z, &room_number);
	r = &room[room_number];
	rx = (x - r->x) >> 10;
	rz = (z - r->z) >> 10;

	if (r->floor[rx * r->x_size + rz].stopper)
		return 0;

	rx = LaraItem->pos.x_pos;
	rz = LaraItem->pos.z_pos;
	LaraItem->pos.x_pos = x;
	LaraItem->pos.z_pos = z;
	itemlist = (ITEM_INFO**)&tsv_buffer[0];
	GetCollidedObjects(LaraItem, 256, 1, itemlist, 0, 0);
	LaraItem->pos.x_pos = rx;
	LaraItem->pos.z_pos = rz;
	return !*itemlist || *itemlist == item;
}
