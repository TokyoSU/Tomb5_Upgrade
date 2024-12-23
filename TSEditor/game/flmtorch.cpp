#include "../tomb5/pch.h"
#include "flmtorch.h"
#include "lara_states.h"
#include "objects.h"
#include "collide.h"
#include "switch.h"
#include "items.h"
#include "delstuff.h"
#include "../specific/function_stubs.h"
#include "effect2.h"
#include "sound.h"
#include "laraflar.h"
#include "larafire.h"
#include "../specific/3dmath.h"
#include "tomb4fx.h"
#include "control.h"
#include "laramisc.h"
#include "../specific/input.h"
#include "draw.h"
#include "lara.h"

short torchroom = NO_ROOM;
static short FireBounds[12] = { 0, 0, 0, 0, 0, 0, -1820, 1820, -5460, 5460, -1820, 1820 };

void FireCollision(short item_num, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item;
	short rot;

	item = &items[item_num];

	if (Lara.gunType == WEAPON_TORCH && Lara.gunStatus == LG_READY && !Lara.leftArm.lock && (item->status & ITEM_ACTIVE) != Lara.litTorch &&
		item->timer != -1 && KeyInput & IN_ACTION && l->current_anim_state == AS_STOP && l->anim_number == ANIM_BREATH && !l->gravity_status)
	{
		rot = item->pos.y_rot;

		if (item->object_number == FLAME_EMITTER)
		{
			FireBounds[0] = -256;
			FireBounds[1] = 256;
			FireBounds[2] = 0;
			FireBounds[3] = 1024;
			FireBounds[4] = -800;
			FireBounds[5] = 800;
		}
		else if (item->object_number == FLAME_EMITTER2)
		{
			FireBounds[0] = -256;
			FireBounds[1] = 256;
			FireBounds[2] = 0;
			FireBounds[3] = 1024;
			FireBounds[4] = -600;
			FireBounds[5] = 600;
		}
		else if (item->object_number == BURNING_ROOTS)
		{
			FireBounds[0] = -384;
			FireBounds[1] = 384;
			FireBounds[2] = 0;
			FireBounds[3] = 2048;
			FireBounds[4] = -384;
			FireBounds[5] = 384;
		}

		item->pos.y_rot = l->pos.y_rot;

		if (TestLaraPosition(FireBounds, item, l))
		{
			if (item->object_number != BURNING_ROOTS)
			{
				l->item_flags[3] = 1;
				l->anim_number = short(ANIM_LIGHT_TORCH1 + (abs(l->pos.y_pos - item->pos.y_pos) >> 8));
			}
			else
				l->anim_number = ANIM_LIGHT_TORCH5;

			l->current_anim_state = AS_CONTROLLED;
			l->frame_number = anims[l->anim_number].frame_base;
			Lara.flareControlLeft = 0;
			Lara.leftArm.lock = 3;
			Lara.generalPtr = (void*)item_num;
		}

		item->pos.y_rot = rot;
	}
	else if (item->object_number == BURNING_ROOTS)
		ObjectCollision(item_num, l, coll);

	if (Lara.generalPtr == (void*)item_num && item->status != ITEM_ACTIVE && l->current_anim_state == AS_CONTROLLED &&
		l->anim_number >= ANIM_LIGHT_TORCH1 && l->anim_number <= ANIM_LIGHT_TORCH5 && l->frame_number - anims[l->anim_number].frame_base == 40)
	{
		TestTriggersAtXYZ(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, 1, item->flags & IFL_CODEBITS);
		item->flags |= IFL_CODEBITS;
		item->item_flags[3] = 0;
		item->status = ITEM_ACTIVE;
		AddActiveItem(item_num);
	}
}

void DoFlameTorch()
{
	PHD_VECTOR pos;
	short state;

	switch (Lara.leftArm.lock)
	{
	case 0:	//holding it

		state = LaraItem->current_anim_state;

		if (Lara.requestGunType != Lara.gunType)
		{
			Lara.leftArm.lock = 2;	//drop it
			Lara.leftArm.frame_number = 31;
			Lara.leftArm.anim_number = objects[TORCH_ANIM].anim_index + 2;
		}
		else if (KeyInput & IN_DRAW && !LaraItem->gravity_status && !LaraItem->fallspeed && state != AS_COMPRESS && state != AS_UPJUMP &&
			state != AS_FORWARDJUMP && state != AS_BACKJUMP && state != AS_RIGHTJUMP && state != AS_LEFTJUMP || Lara.waterStatus == LW_UNDERWATER)
		{
			Lara.leftArm.lock = 1;	//throw it
			Lara.leftArm.frame_number = 1;
			Lara.leftArm.anim_number = objects[TORCH_ANIM].anim_index + 1;

			if (Lara.waterStatus == LW_UNDERWATER)
				Lara.litTorch = 0;
		}

		break;

	case 1:	//throwing it

		if (Lara.leftArm.frame_number < 12 && LaraItem->gravity_status)
		{
			Lara.leftArm.lock = 0;	//keep holding it
			Lara.leftArm.frame_number = 0;
			Lara.leftArm.anim_number = objects[TORCH_ANIM].anim_index;
		}
		else
		{
			Lara.leftArm.frame_number++;

			if (Lara.leftArm.frame_number == 13)
			{
				Lara.litTorch = 0;
				Lara.flareControlLeft = 0;
				Lara.leftArm.lock = 0;
				Lara.gunType = Lara.lastGunType;
				Lara.requestGunType = WEAPON_NONE;
				Lara.gunStatus = LG_NO_ARMS;
			}
			else if (Lara.leftArm.frame_number == 12)
			{
				Lara.meshPtrs[LM_LHAND] = meshes[objects[LARA].mesh_index + (LM_LHAND * 2)];
				CreateFlare(BURNING_TORCH_ITEM, 1);
			}
		}

		break;

	case 2:	//dropping it (when pulling out a weapon)

		Lara.leftArm.frame_number++;

		if (Lara.leftArm.frame_number == 41)
		{
			Lara.litTorch = 0;
			Lara.flareControlLeft = 0;
			Lara.leftArm.lock = 0;
			Lara.lastGunType = WEAPON_NONE;
			Lara.gunType = WEAPON_NONE;
			Lara.gunStatus = LG_NO_ARMS;
		}
		else if (Lara.leftArm.frame_number == 36)
		{
			Lara.meshPtrs[LM_LHAND] = meshes[objects[LARA].mesh_index + (LM_LHAND * 2)];
			CreateFlare(BURNING_TORCH_ITEM, 0);
		}

		break;

	case 3:	//lighting it

		if (LaraItem->current_anim_state != AS_CONTROLLED)
		{
			Lara.litTorch = LaraItem->item_flags[3];
			Lara.flareControlLeft = 1;
			Lara.leftArm.lock = 0;
			Lara.leftArm.frame_number = 0;
			Lara.leftArm.anim_number = objects[TORCH_ANIM].anim_index;
		}

		break;
	}

	if (Lara.flareControlLeft)
		Lara.gunStatus = LG_READY;

	Lara.leftArm.frame_base = anims[Lara.leftArm.anim_number].frame_ptr;

	if (Lara.litTorch)
	{
		pos.x = -32;
		pos.y = 64;
		pos.z = 256;
		GetLaraJointPos(&pos, LMX_HAND_L);
		TriggerDynamic(pos.x, pos.y, pos.z, 12 - (GetRandomControl() & 1), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);

		if (!(wibble & 7))
		   TriggerTorchFlame(LaraItem - items, 0);

		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, (PHD_3DPOS*)&pos, 0);
		torchroom = LaraItem->room_number;
	}
}

void TriggerTorchFlame(short item_number, long node)
{
	SPARKS* sptr;
	long size;

	sptr = &spark[GetFreeSpark()];
	sptr->On = 1;
	sptr->sR = 255;
	sptr->sG = (GetRandomControl() & 0x1F) + 48;
	sptr->sB = 48;
	sptr->dR = (GetRandomControl() & 0x3F) + 192;
	sptr->dG = (GetRandomControl() & 0x3F) + 128;
	sptr->dB = 32;
	sptr->FadeToBlack = 8;
	sptr->ColFadeSpeed = (GetRandomControl() & 3) + 12;
	sptr->TransType = 2;
	sptr->Life = (GetRandomControl() & 7) + 24;
	sptr->sLife = sptr->Life;
	sptr->x = (GetRandomControl() & 0xF) - 8;
	sptr->y = 0;
	sptr->z = (GetRandomControl() & 0xF) - 8;
	sptr->Xvel = (GetRandomControl() & 0xFF) - 128;
	sptr->Yvel = -16 - (GetRandomControl() & 0xF);
	sptr->Zvel = (GetRandomControl() & 0xFF) - 128;
	sptr->Friction = 5;
	sptr->Flags = SF_ATTACHEDNODE | SF_ITEM | SF_ROTATE | SF_DEF | SF_SCALE;
	sptr->RotAng = GetRandomControl() & 0xFFF;

	if (GetRandomControl() & 1)
		sptr->RotAdd = -16 - (GetRandomControl() & 0xF);
	else
		sptr->RotAdd = (GetRandomControl() & 0xF) + 16;

	sptr->Gravity = -16 - (GetRandomControl() & 0x1F);
	sptr->NodeNumber = (uchar)node;
	sptr->MaxYvel = -16 - (GetRandomControl() & 7);
	sptr->FxObj = (uchar)item_number;
	sptr->Scalar = 1;
	size = (GetRandomControl() & 0x1F) + 80;
	sptr->Size = (uchar)size;
	sptr->sSize = sptr->Size;
	sptr->dSize = sptr->Size >> 3;
}

void GetFlameTorch()
{
	if (Lara.gunType == WEAPON_FLARE)
		CreateFlare(FLARE_ITEM, 0);

	Lara.requestGunType = WEAPON_TORCH;
	Lara.gunType = WEAPON_TORCH;
	Lara.flareControlLeft = 1;
	Lara.leftArm.anim_number = objects[TORCH_ANIM].anim_index;
	Lara.gunStatus = LG_READY;
	Lara.leftArm.lock = 0;
	Lara.leftArm.frame_number = 0;
	Lara.leftArm.frame_base = anims[objects[TORCH_ANIM].anim_index].frame_ptr;
	Lara.meshPtrs[LM_LHAND] = meshes[objects[TORCH_ANIM].mesh_index + LM_LHAND * 2];
}

void FlameTorchControl(short item_number)
{
	ITEM_INFO** itemlist;
	MESH_INFO** meshlist;
	ITEM_INFO* item;
	STATIC_INFO* sinfo;
	PHD_3DPOS pos;
	long x, y, z, xv, yv, zv;

	item = &items[item_number];

	if (item->fallspeed)
		item->pos.z_rot += 910;
	else if (!item->speed)
	{
		item->pos.x_rot = 0;
		item->pos.z_rot = 0;
	}

	x = item->pos.x_pos;
	y = item->pos.y_pos;
	z = item->pos.z_pos;
	xv = item->speed * phd_sin(item->pos.y_rot) >> W2V_SHIFT;
	zv = item->speed * phd_cos(item->pos.y_rot) >> W2V_SHIFT;
	item->pos.x_pos += xv;
	item->pos.z_pos += zv;

	if (room[item->room_number].flags & ROOM_UNDERWATER)
	{
		item->fallspeed += (5 - item->fallspeed) >> 1;
		item->speed += (5 - item->speed) >> 1;

		if (item->item_flags[3])
			item->item_flags[3] = 0;
	}
	else
		item->fallspeed += 6;

	yv = item->fallspeed;
	item->pos.y_pos += yv;
	DoProperDetection(item_number, x, y, z, xv, yv, zv);
	itemlist = (ITEM_INFO**)&tsv_buffer[0];
	meshlist = (MESH_INFO**)&tsv_buffer[1024];

	if (GetCollidedObjects(item, 0, 1, itemlist, meshlist, 0))
	{
		mycoll.enable_baddie_push = 1;

		if (itemlist[0])
		{
			if (!objects[itemlist[0]->object_number].intelligent)
				ObjectCollision(itemlist[0] - items, item, &mycoll);
		}
		else
		{
			sinfo = &static_objects[meshlist[0]->static_number];
			pos.x_pos = meshlist[0]->x;
			pos.y_pos = meshlist[0]->y;
			pos.z_pos = meshlist[0]->z;
			pos.y_rot = meshlist[0]->y_rot;
			ItemPushLaraStatic(item, (short*)&sinfo->x_minc, &pos, &mycoll);
		}

		item->speed >>= 1;
	}

	if (item->item_flags[3])
	{
		TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 12 - (GetRandomControl() & 1),
			(GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);

		if (!(wibble & 7))
			TriggerTorchFlame(item_number, 1);

		torchroom = item->room_number;
		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, SFX_DEFAULT);
	}
}
