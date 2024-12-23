#include "../tomb5/pch.h"
#include "file.h"
#include "file_utils.h"
#include "function_stubs.h"
#include "../game/gameflow.h"
#include "../game/setup.h"
#include "alexstuff.h"
#include "../game/init.h"
#include "../game/items.h"
#include "../game/objects.h"
#include "../game/laraskin.h"
#include "drawroom.h"
#include "dxsound.h"
#include "LoadSave.h"
#include "function_table.h"
#include "polyinsert.h"
#include "winmain.h"
#include "output.h"
#include "texture.h"
#include "dxshell.h"
#include "lighting.h"
#include "../game/tomb4fx.h"
#include "../game/deltapak.h"
#include "3dmath.h"
#include "specificfx.h"
#include "../game/box.h"
#include "../game/camera.h"
#include "../game/sound.h"
#include "../game/spotcam.h"
#include "../game/control.h"
#include "../game/draw.h"
#include "audio.h"
#include "../game/effects.h"
#include "../game/effect2.h"
#include "../game/lara.h"
#include "drawbars.h"
#include "../tomb5/tomb5.h"
#include <zlib.h>

TEXTURESTRUCT* textinfo;
SPRITESTRUCT* spriteinfo;
CHANGE_STRUCT* changes;
RANGE_STRUCT* ranges;
AIOBJECT* AIObjects;
THREAD LevelLoadingThread;
short* aranges;
short* frames;
short* commands;
short* floor_data;
short* mesh_base;
long number_cameras;
long nAnimUVRanges;
short nAIObjects;

TEXTURESTRUCT* AnimatingWaterfalls[6];
float AnimatingWaterfallsV[6];

static TEXTURESTRUCT* MonitorScreenTex;
static CFileReader levelCompressed;
static CMemoryFileReader levelUncompressed;
static char* compressed, *decompressed;
static float MonitorScreenU;

FILE* FileOpen(const char* Filename)
{
	FILE* fp;
	char cdFilename[80];

	memset(cdFilename, 0, 80);
	strcat(cdFilename, Filename);
	Log("FileOpen - %s", cdFilename);
	fp = fopen(cdFilename, "rb");

	if (!fp)
		Log("Unable To Open %s", cdFilename);

	return fp;
}

void FileClose(FILE* fp)
{
	Log(__FUNCTION__);
	fclose(fp);
}

long FileSize(FILE* fp)
{
	long size;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return size;
}

long LoadFile(const char* name, char** dest)
{
	FILE* file;
	long size, count;

	Log(__FUNCTION__);
	Log("File - %s", name);
	file = FileOpen(name);

	if (!file)
		return 0;

	size = FileSize(file);

	if (!*dest)
		*dest = (char*)malloc(size);

	count = fread(*dest, 1, size, file);
	Log("Read - %d FileSize - %d", count, size);

	if (count != size)
	{
		Log("Error Reading File");
		FileClose(file);
		free(*dest);
		return 0;
	}

	FileClose(file);
	return size;
}

bool Decompress(char* pDest, char* pCompressed, long compressedSize, long size)
{
	z_stream stream;

	Log(__FUNCTION__);
	memset(&stream, 0, sizeof(z_stream));
	stream.avail_in = compressedSize;
	stream.avail_out = size;
	stream.next_out = (Bytef*)pDest;
	stream.next_in = (Bytef*)pCompressed;
	inflateInit(&stream);
	inflate(&stream, Z_FINISH);

	if (stream.total_out != size)
	{
		Log("Error Decompressing Data");
		return 0;
	}

	inflateEnd(&stream);
	Log("Decompression OK");
	return 1;
}

void AdjustUV(long num)
{
	TEXTURESTRUCT* tex;
	float u, v;
	ushort type;

	Log(__FUNCTION__);

	for (int i = 0; i < num; i++)
	{
		tex = &textinfo[i];
		Textures[tex->tpage].tpage++;
		tex->tpage++;
		u = 1.0F / float(Textures[tex->tpage].width << 1);
		v = 1.0F / float(Textures[tex->tpage].height << 1);
		type = tex->flag & 7;

		if (tex->flag & 0x8000)
		{
			switch (type)
			{
			case 0:
				tex->u1 += u;
				tex->v1 += v;
				tex->u2 -= u;
				tex->v2 += v;
				tex->u3 += u;
				tex->v3 -= v;
				break;

			case 1:
				tex->u1 -= u;
				tex->v1 += v;
				tex->u2 -= u;
				tex->v2 -= v;
				tex->u3 += u;
				tex->v3 += v;
				break;

			case 2:
				tex->u1 -= u;
				tex->v1 -= v;
				tex->u2 += u;
				tex->v2 -= v;
				tex->u3 -= u;
				tex->v3 += v;
				break;

			case 3:
				tex->u1 += u;
				tex->v1 -= v;
				tex->u2 += u;
				tex->v2 += v;
				tex->u3 -= u;
				tex->v3 -= v;
				break;

			case 4:
				tex->u1 -= u;
				tex->v1 += v;
				tex->u2 += u;
				tex->v2 += v;
				tex->u3 -= u;
				tex->v3 -= v;
				break;

			case 5:
				tex->u1 += u;
				tex->v1 += v;
				tex->u2 += u;
				tex->v2 -= v;
				tex->u3 -= u;
				tex->v3 += v;
				break;

			case 6:
				tex->u1 += u;
				tex->v1 -= v;
				tex->u2 -= u;
				tex->v2 -= v;
				tex->u3 += u;
				tex->v3 += v;
				break;

			case 7:
				tex->u1 -= u;
				tex->v1 -= v;
				tex->u2 -= u;
				tex->v2 += v;
				tex->u3 += u;
				tex->v3 -= v;
				break;

			default:
				Log("TextureInfo Type %d Not Found", type);
				break;
			}
		}
		else
		{
			switch (type)
			{
			case 0:
				tex->u1 += u;
				tex->v1 += v;
				tex->u2 -= u;
				tex->v2 += v;
				tex->u3 -= u;
				tex->v3 -= v;
				tex->u4 += u;
				tex->v4 -= v;
				break;

			case 1:
				tex->u1 -= u;
				tex->v1 += v;
				tex->u2 += u;
				tex->v2 += v;
				tex->u3 += u;
				tex->v3 -= v;
				tex->u4 -= u;
				tex->v4 -= v;
				break;

			default:
				Log("TextureInfo Type %d Not Found", type);
				break;
			}
		}
	}
}

void DoMonitorScreen()
{
	float s;
	static long pos, reset;

	if (!MonitorScreenTex)
		return;

	s = (fSin(pos) * 47.0F + 47.0F);
	MonitorScreenTex->u1 = MonitorScreenU + s * (1.0F / 256.0F);
	MonitorScreenTex->u2 = MonitorScreenU + s * (1.0F / 256.0F) + 0.125F;
	MonitorScreenTex->u4 = MonitorScreenU + s * (1.0F / 256.0F);
	MonitorScreenTex->u3 = MonitorScreenU + s * (1.0F / 256.0F) + 0.125F;

	if (reset)
		reset--;
	else
	{
		if (s >= 94 || s <= 0)
			reset = 65;

		pos += 128;
	}
}

void S_GetUVRotateTextures()
{
	TEXTURESTRUCT* tex;
	short* pRange = aranges + 1;
	for (int i = 0; i < nAnimUVRanges; i++, pRange++)
	{
		for (int j = (int)*(pRange++); j >= 0; j--, pRange++)
		{
			tex = &textinfo[*pRange];
			AnimatingTexturesV[i][j][0] = tex->v1;
		}
		pRange--;
	}
}

void FreeLevel()
{
	Log(__FUNCTION__);
	FreeTextures();
	DXFreeSounds();
	malloc_ptr = malloc_buffer;
	malloc_free = malloc_size;
}

bool LoadTextures(long RTPages, long OTPages, long BTPages)
{
	DXTEXTUREINFO* dxtex;
	LPDIRECTDRAWSURFACE4 tSurf;
	LPDIRECT3DTEXTURE2 pTex = nullptr;
	uchar* texData;
	long* d;
	char* pComp;
	char* s;
	long format, skip, uncompSize, realUncompSize, compSize, nTex, c;
	uchar r, g, b, a;

	Log(__FUNCTION__);

	nTextures = 1;
	format = 0;
	skip = 4;
	dxtex = &G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].TextureInfos[G_dxinfo->nTexture];

	if (dxtex->rbpp == 8 && dxtex->gbpp == 8 && dxtex->bbpp == 8 && dxtex->abpp == 8)
	{
		format = 1;
	}
	else if (dxtex->rbpp == 5 && dxtex->gbpp == 5 && dxtex->bbpp == 5 && dxtex->abpp == 1)
	{
		format = 2;
		skip = 2;
	}

	/// ====================================================
	///   LOAD TEXTURE 32 AND 16 BITS
	/// ====================================================

	if (format <= 1)
	{
		// Read 32Bit Textures.
		uncompSize = levelCompressed.ReadLong();
		compSize = levelCompressed.ReadLong();
		compressed = (char*)malloc(compSize);
		decompressed = (char*)malloc(uncompSize);
		levelCompressed.ReadBytes(compressed, compSize);
		Decompress(decompressed, compressed, compSize, uncompSize);
		realUncompSize = uncompSize;
		// Skip 16Bit Textures.
		uncompSize = levelCompressed.ReadLong();
		compSize = levelCompressed.ReadLong();
		levelCompressed.Seek(compSize, RW_SEEK_CUR);
	}
	else
	{
		// Skip 32Bit Textures.
		uncompSize = levelCompressed.ReadLong();
		compSize = levelCompressed.ReadLong();
		levelCompressed.Seek(compSize, RW_SEEK_CUR);
		// Read 16Bit Textures.
		uncompSize = levelCompressed.ReadLong();
		compSize = levelCompressed.ReadLong();
		compressed = (char*)malloc(compSize);
		decompressed = (char*)malloc(uncompSize);
		levelCompressed.ReadBytes(compressed, compSize);
		Decompress(decompressed, compressed, compSize, uncompSize);
		realUncompSize = uncompSize;
	}
	SafeFree(compressed);

	CMemoryFileReader texReader;
	texReader.LoadFile(decompressed, realUncompSize);

	Log("RTPages %d", RTPages);
	uncompSize = RTPages * skip * 0x10000;
	texData = (uchar*)malloc(uncompSize);
	texReader.ReadBytes(texData, uncompSize);
	S_LoadBar();
	for (int i = 0; i < RTPages; i++)
	{
		nTex = nTextures++;
		tSurf = CreateTexturePage(App.TextureSize, App.TextureSize, 0, (long*)(texData + (i * skip * 0x10000)), 0, format);
		DXAttempt(tSurf->QueryInterface(IID_IDirect3DTexture2, (LPVOID*)&pTex));
		Textures[nTex].tex = pTex;
		Textures[nTex].surface = tSurf;
		Textures[nTex].width = App.TextureSize;
		Textures[nTex].height = App.TextureSize;
		Textures[nTex].bump = 0;
		S_LoadBar();
	}
	SafeFree(texData);

	Log("OTPages %d", OTPages);
	uncompSize = OTPages * skip * 0x10000;
	texData = (uchar*)malloc(uncompSize);
	texReader.ReadBytes(texData, uncompSize);
	S_LoadBar();

	for (int i = 0; i < OTPages; i++)
	{
		nTex = nTextures++;
		tSurf = CreateTexturePage(App.TextureSize, App.TextureSize, 0, (long*)(texData + (i * skip * 0x10000)), 0, format);
		DXAttempt(tSurf->QueryInterface(IID_IDirect3DTexture2, (LPVOID*)&pTex));
		Textures[nTex].tex = pTex;
		Textures[nTex].surface = tSurf;
		Textures[nTex].width = App.TextureSize;
		Textures[nTex].height = App.TextureSize;
		Textures[nTex].bump = 0;
		App.dx.lpD3DDevice->SetTexture(0, pTex);
		S_LoadBar();
	}
	SafeFree(texData);

	S_LoadBar();
	Log("BTPages %d", BTPages);

	if (BTPages)
	{
		uncompSize = BTPages * skip * 0x10000;
		texData = (uchar*)malloc(uncompSize);
		texReader.ReadBytes(texData, uncompSize);

		for (int i = 0; i < BTPages; i++)
		{
			if (i < (BTPages >> 1))
				tSurf = CreateTexturePage(App.TextureSize, App.TextureSize, 0, (long*)(texData + (i * skip * 0x10000)), 0, format);
			else
			{
				if (!App.BumpMapping)
					break;
				tSurf = CreateTexturePage(App.BumpMapSize, App.BumpMapSize, 0, (long*)(texData + (i * skip * 0x10000)), 0, format);
			}

			nTex = nTextures++;
			DXAttempt(tSurf->QueryInterface(IID_IDirect3DTexture2, (LPVOID*)&pTex));
			Textures[nTex].tex = pTex;
			Textures[nTex].surface = tSurf;

			if (i < (BTPages >> 1))
			{
				Textures[nTex].width = App.TextureSize;
				Textures[nTex].height = App.TextureSize;
			}
			else
			{
				Textures[nTex].width = App.BumpMapSize;
				Textures[nTex].height = App.BumpMapSize;
			}

			Textures[nTex].bump = 1;
			Textures[nTex].bumptpage = nTex + (BTPages >> 1);
			S_LoadBar();
		}

		SafeFree(texData);
	}
	SafeFree(decompressed);

	/// ====================================================
	///   LOAD MISC TEXTURES
	/// ====================================================

	uncompSize = levelCompressed.ReadLong();
	compSize = levelCompressed.ReadLong();
	compressed = (char*)malloc(compSize);
	decompressed = (char*)malloc(uncompSize);
	levelCompressed.ReadBytes(compressed, compSize);
	Decompress(decompressed, compressed, compSize, uncompSize);
	SafeFree(compressed);
	texData = (uchar*)malloc(0x40000);

	texReader.Release();
	texReader.LoadFile(decompressed, uncompSize);

	// main menu logo
	if (gfCurrentLevel == LVL5_TITLE)
	{
		pComp = 0;
		compressed = 0;

		if (Gameflow->Language == US)
			uncompSize = LoadFile("data\\uslogo.pak", &compressed);
		else if (Gameflow->Language == GERMAN)
			uncompSize = LoadFile("data\\grlogo.pak", &compressed);
		else if (Gameflow->Language == FRENCH)
			uncompSize = LoadFile("data\\frlogo.pak", &compressed);
		else
			uncompSize = LoadFile("data\\logo512.pak", &compressed);

		pComp = (char*)malloc(*(long*)compressed);
		Decompress(pComp, compressed + 4, uncompSize - 4, *(long*)compressed);
		SafeFree(compressed);

		for (int i = 0; i < 2; i++)
		{
			s = pComp + (i * 768);
			d = (long*)texData;

			for (int y = 0; y < 256; y++)
			{
				for (int x = 0; x < 256; x++)
				{
					r = *(s + (x * 3) + (y * 1536));
					g = *(s + (x * 3) + (y * 1536) + 1);
					b = *(s + (x * 3) + (y * 1536) + 2);
					a = 0xFF;

					if (r == 0xFF && b == 0xFF && !g)	//magenta
					{
						r = 0;
						b = 0;
						a = 0;
					}

					c = RGBA(r, g, b, a);
					*d++ = c;
				}
			}

			nTex = nTextures++;
			tSurf = CreateTexturePage(256, 256, 0, (long*)texData, 0, 0);
			DXAttempt(tSurf->QueryInterface(IID_IDirect3DTexture2, (LPVOID*)&pTex));
			Textures[nTex].tex = pTex;
			Textures[nTex].surface = tSurf;
			Textures[nTex].width = 256;
			Textures[nTex].height = 256;
			Textures[nTex].bump = 0;
		}

		SafeFree(pComp);
	}

	//shine
	texReader.ReadBytes(texData, 0x40000);
	nTex = nTextures++;
	tSurf = CreateTexturePage(256, 256, 0, (long*)texData, 0, 0);
	DXAttempt(tSurf->QueryInterface(IID_IDirect3DTexture2, (LPVOID*)&pTex));
	Textures[nTex].tex = pTex;
	Textures[nTex].surface = tSurf;
	Textures[nTex].width = 256;
	Textures[nTex].height = 256;
	Textures[nTex].bump = 0;

	//font
	texReader.ReadBytes(texData, 0x40000);
	nTex = nTextures++;
	tSurf = CreateTexturePage(256, 256, 0, (long*)texData, 0, 0);
	DXAttempt(tSurf->QueryInterface(IID_IDirect3DTexture2, (LPVOID*)&pTex));
	Textures[nTex].tex = pTex;
	Textures[nTex].surface = tSurf;
	Textures[nTex].width = 256;
	Textures[nTex].height = 256;
	Textures[nTex].bump = 0;

	//sky
	texReader.ReadBytes(texData, 0x40000);
	nTex = nTextures++;
	tSurf = CreateTexturePage(256, 256, 0, (long*)texData, 0, 0);
	DXAttempt(tSurf->QueryInterface(IID_IDirect3DTexture2, (LPVOID*)&pTex));
	Textures[nTex].tex = pTex;
	Textures[nTex].surface = tSurf;
	Textures[nTex].width = 256;
	Textures[nTex].height = 256;
	Textures[nTex].bump = 0;

	Log("Created %d Texture Pages", nTextures - 1);

	texReader.Release();
	SafeFree(decompressed);
	SafeFree(texData);
	return 1;
}

static void aLoadRoomStream()
{
	ROOM_INFO* r;
	long num, size;
	char* data;

	levelUncompressed.ReadLong();	//skip unused space
	num = levelUncompressed.ReadLong();
	room = (ROOM_INFO*)MEM_Allocate(num * sizeof(ROOM_INFO));

	for (int i = 0; i < num; i++)
	{
		r = &room[i];
		levelUncompressed.ReadLong();		//X E L A
		size = levelUncompressed.ReadLong();
		data = (char*)MEM_Allocate(size);
		levelUncompressed.ReadBytes(data, size);
		aFixUpRoom(r, data);
	}

	number_rooms = (short)num;
}

bool LoadRooms()
{
	long size;

	Log(__FUNCTION__);
	wibble = 0;

	aLoadRoomStream();
	BuildOutsideTable();

	size = levelUncompressed.ReadLong();
	floor_data = (short*)MEM_Allocate(size * sizeof(short));
	levelUncompressed.ReadBytes(floor_data, sizeof(short) * size);

	Log("Floor Data Size %d @ %x", size, floor_data);
	return 1;
}

bool LoadObjects()
{
	OBJECT_INFO* obj;
	STATIC_INFO* stat;
	short** mesh;
	short** mesh_size;
	long num, num_anims, num_meshes, slot, lp;

	Log(__FUNCTION__);
	memset(objects, 0, NUMBER_OBJECTS * sizeof(OBJECT_INFO));
	memset(static_objects, 0, 70 * sizeof(STATIC_INFO));

	//meshes
	num = levelUncompressed.ReadLong();
	mesh_base = (short*)MEM_Allocate(num * 2);
	levelUncompressed.ReadBytes(mesh_base, num * sizeof(short));

	num = levelUncompressed.ReadLong();
	meshes = (short**)MEM_Allocate(num * sizeof(long) * 2);	//*2 for meshswaps
	levelUncompressed.ReadBytes(meshes, num * sizeof(long));

	for (lp = 0; lp < num; lp++)
		meshes[lp] = &mesh_base[(long)meshes[lp] / 2];

	num_meshes = num;

	//anims
	num_anims = levelUncompressed.ReadLong();
	anims = (ANIM_STRUCT*)MEM_Allocate(num_anims * sizeof(ANIM_STRUCT));
	levelUncompressed.ReadBytes(anims, num_anims * sizeof(ANIM_STRUCT));

	//changes
	num = levelUncompressed.ReadLong();
	changes = (CHANGE_STRUCT*)MEM_Allocate(num * sizeof(CHANGE_STRUCT));
	levelUncompressed.ReadBytes(changes, num * sizeof(CHANGE_STRUCT));

	//ranges
	num = levelUncompressed.ReadLong();
	ranges = (RANGE_STRUCT*)MEM_Allocate(num * sizeof(RANGE_STRUCT));
	levelUncompressed.ReadBytes(ranges, num * sizeof(RANGE_STRUCT));

	//anim commands
	num = levelUncompressed.ReadLong();
	commands = (short*)MEM_Allocate(num * sizeof(short));
	levelUncompressed.ReadBytes(commands, num * sizeof(short));

	//bones
	num = levelUncompressed.ReadLong();
	bones = (long*)MEM_Allocate(num * sizeof(long));
	levelUncompressed.ReadBytes(bones, num * sizeof(long));

	//frames
	num = levelUncompressed.ReadLong();
	frames = (short*)MEM_Allocate(num * sizeof(short));
	levelUncompressed.ReadBytes(frames, num * sizeof(short));

	for (lp = 0; lp < num_anims; lp++)
		anims[lp].frame_ptr = (short*)((long)anims[lp].frame_ptr + (long)frames);

	num = levelUncompressed.ReadLong();
	for (lp = 0; lp < num; lp++)
	{
		slot = levelUncompressed.ReadLong();
		obj = &objects[slot];

		obj->nmeshes = levelUncompressed.ReadShort();
		obj->mesh_index = levelUncompressed.ReadShort();
		obj->bone_index = levelUncompressed.ReadLong();
		obj->frame_base = (short*)levelUncompressed.ReadLong();
		obj->anim_index = levelUncompressed.ReadShort();
		obj->loaded = 1;

		levelUncompressed.ReadShort();	//Skip padding
	}

	if (LaraDrawType != LARA_DIVESUIT)
		CreateSkinningData();

	for (int i = 0; i < NUMBER_OBJECTS; i++)
	{
		obj = &objects[i];
		obj->mesh_index *= 2;
	}

	mesh = meshes;
	mesh_size = &meshes[num_meshes];
	memcpy(mesh_size, mesh, num_meshes * 4);

	for (int i = 0; i < num_meshes; i++)
	{
		*mesh++ = *mesh_size;
		*mesh++ = *mesh_size++;
	}

	InitialiseObjects();
	InitialiseClosedDoors();

	//statics
	num = levelUncompressed.ReadLong();

	for (int i = 0; i < num; i++)
	{
		slot = levelUncompressed.ReadLong();
		stat = &static_objects[slot];

		stat->mesh_number = levelUncompressed.ReadShort();
		levelUncompressed.ReadBytes(&stat->x_minp, 6 * sizeof(short));
		levelUncompressed.ReadBytes(&stat->x_minc, 6 * sizeof(short));
		stat->flags = levelUncompressed.ReadShort();
	}

	for (int i = 0; i < 70; i++)
	{
		stat = &static_objects[i];
		stat->mesh_number *= 2;
	}

	ProcessMeshData(num_meshes * 2);
	return 1;
}

bool LoadSprites()
{
	STATIC_INFO* stat;
	OBJECT_INFO* obj;
	PHDSPRITESTRUCT sprite;
	long num_sprites, num_slots, slot;

	Log(__FUNCTION__);
	levelUncompressed.ReadLong();			//SPR 0 marker

	num_sprites = levelUncompressed.ReadLong();
	spriteinfo = (SPRITESTRUCT*)MEM_Allocate(sizeof(SPRITESTRUCT) * num_sprites);

	for (int i = 0; i < num_sprites; i++)
	{
		levelUncompressed.ReadBytes(&sprite, sizeof(PHDSPRITESTRUCT));
		spriteinfo[i].height = sprite.height;
		spriteinfo[i].offset = sprite.offset;
		spriteinfo[i].tpage = sprite.tpage;
		spriteinfo[i].width = sprite.width;
		spriteinfo[i].x1 = (float)sprite.x1 * (1.0F / 256.0F);
		spriteinfo[i].y1 = (float)sprite.y1 * (1.0F / 256.0F);
		spriteinfo[i].x2 = (float)sprite.x2 * (1.0F / 256.0F);
		spriteinfo[i].y2 = (float)sprite.y2 * (1.0F / 256.0F);
		spriteinfo[i].x1 += (1.0F / 256.0F);
		spriteinfo[i].y1 += (1.0F / 256.0F);
		spriteinfo[i].x2 -= (1.0F / 256.0F);
		spriteinfo[i].y2 -= (1.0F / 256.0F);
		spriteinfo[i].tpage++;
	}

	num_slots = levelUncompressed.ReadLong();
	if (num_slots <= 0)
		return 1;

	for (int i = 0; i < num_slots; i++)
	{
		slot = levelUncompressed.ReadLong();

		if (slot >= NUMBER_OBJECTS)
		{
			slot -= NUMBER_OBJECTS;
			stat = &static_objects[slot];
			stat->mesh_number = levelUncompressed.ReadShort();
			stat->mesh_number = levelUncompressed.ReadShort();
		}
		else
		{
			obj = &objects[slot];
			obj->nmeshes = levelUncompressed.ReadShort();
			obj->mesh_index = levelUncompressed.ReadShort();
			obj->loaded = 1;
		}
	}

	return 1;
}

bool LoadCameras()
{
	Log(__FUNCTION__);
	number_cameras = levelUncompressed.ReadLong();

	if (number_cameras)
	{
		camera.fixed = (OBJECT_VECTOR*)MEM_Allocate(number_cameras * sizeof(OBJECT_VECTOR));
		levelUncompressed.ReadBytes(camera.fixed, number_cameras * sizeof(OBJECT_VECTOR));
	}

	number_spotcams = (short)levelUncompressed.ReadLong();
	if (number_spotcams)
		levelUncompressed.ReadBytes(SpotCam, number_spotcams * sizeof(SPOTCAM));

	return 1;
}

bool LoadSoundEffects()
{
	Log(__FUNCTION__);
	number_sound_effects = levelUncompressed.ReadLong();
	Log("Number of SFX %d", number_sound_effects);

	if (number_sound_effects)
	{
		sound_effects = (OBJECT_VECTOR*)MEM_Allocate(number_sound_effects * sizeof(OBJECT_VECTOR));
		levelUncompressed.ReadBytes(sound_effects, number_sound_effects * sizeof(OBJECT_VECTOR));
	}

	return 1;
}

bool LoadBoxes()
{
	BOX_INFO* box;
	long size;

	Log(__FUNCTION__);
	num_boxes = levelUncompressed.ReadLong();

	boxes = (BOX_INFO*)MEM_Allocate(num_boxes * sizeof(BOX_INFO));
	levelUncompressed.ReadBytes(boxes, num_boxes * sizeof(BOX_INFO));

	size = levelUncompressed.ReadLong();
	overlap = (ushort*)MEM_Allocate(size * sizeof(ushort));
	levelUncompressed.ReadBytes(overlap, size * sizeof(ushort));

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			ground_zone[j][i] = (short*)MEM_Allocate(num_boxes * sizeof(short));
			levelUncompressed.ReadBytes(ground_zone[j][i], num_boxes * sizeof(short));
		}

		ground_zone[4][i] = (short*)MEM_Allocate(num_boxes * sizeof(short));
		levelUncompressed.ReadBytes(ground_zone[4][i], num_boxes * sizeof(short));
	}

	for (int i = 0; i < num_boxes; i++)
	{
		box = &boxes[i];
		if (box->overlap_index & 0x8000)
			box->overlap_index |= 0x4000;
	}

	return 1;
}

bool LoadAnimatedTextures()
{
	long num_anim_ranges;

	Log(__FUNCTION__);
	num_anim_ranges = levelUncompressed.ReadLong();

	aranges = (short*)MEM_Allocate(num_anim_ranges * 2);
	levelUncompressed.ReadBytes(aranges, num_anim_ranges * sizeof(short));

	if (gfCurrentLevel == LVL5_ESCAPE_WITH_THE_IRIS)
		aranges[2] = 2076;

	nAnimUVRanges = levelUncompressed.ReadUChar();
	return 1;
}

static bool LarasNoseTexture(long i)
{
	if (gfCurrentLevel == LVL5_STREETS_OF_ROME && (i == 200 || i == 204))
		return 1;

	if (gfCurrentLevel == LVL5_TRAJAN_MARKETS && (i == 225 || i == 229))
		return 1;

	if (gfCurrentLevel == LVL5_COLOSSEUM && (i == 240 || i == 244))
		return 1;

	if (gfCurrentLevel == LVL5_BASE && (i == 210 || i == 213))
		return 1;

	if (gfCurrentLevel == LVL5_SUBMARINE && (i == 205 || i == 208))
		return 1;

	if (gfCurrentLevel == LVL5_DEEPSEA_DIVE && (i == 201 || i == 205))
		return 1;

	if (gfCurrentLevel == LVL5_SINKING_SUBMARINE && (i == 235 || i == 238))
		return 1;

	if ((gfCurrentLevel >= LVL5_GALLOWS_TREE && gfCurrentLevel <= LVL5_OLD_MILL) && (i == 144 || i == 148))
		return 1;

	if (gfCurrentLevel == LVL5_THIRTEENTH_FLOOR && (i == 99 || i == 103))
		return 1;

	if (gfCurrentLevel == LVL5_ESCAPE_WITH_THE_IRIS && (i == 101 || i == 105))
		return 1;

	if (gfCurrentLevel == LVL5_RED_ALERT && (i == 133 || i == 137))
		return 1;

	return 0;
}

bool LoadTextureInfos()
{
	PHDTEXTURESTRUCT tex;
	long nTInfos;

	Log(__FUNCTION__);

	levelUncompressed.ReadLong();			//TEX 0 marker
	nTInfos = levelUncompressed.ReadLong();
	Log("Texture Infos : %d", nTInfos);
	textinfo = (TEXTURESTRUCT*)MEM_Allocate(nTInfos * sizeof(TEXTURESTRUCT));

	for (int i = 0; i < nTInfos; i++)
	{
		levelUncompressed.ReadBytes(&tex, sizeof(PHDTEXTURESTRUCT));
		textinfo[i].drawtype = tex.drawtype;
		textinfo[i].tpage = tex.tpage & 0x7FFF;
		textinfo[i].flag = tex.tpage ^ (tex.tpage ^ tex.flag) & 0x7FFF;
		
		textinfo[i].u1 = (tex.u1 >> 8) * (1.0F / 256.0F);
		textinfo[i].v1 = (tex.v1 >> 8) * (1.0F / 256.0F);

		if (LarasNoseTexture(i))
		{
			textinfo[i].u2 = (tex.u2 >> 8) * 0.00393690F;
			textinfo[i].v2 = (tex.v2 >> 8) * 0.00393690F;
			textinfo[i].u3 = (tex.u3 >> 8) * 0.00393690F;
			textinfo[i].v3 = (tex.v3 >> 8) * 0.00393690F;
		}
		else
		{
			textinfo[i].u2 = (tex.u2 >> 8) * (1.0F / 256.0F);
			textinfo[i].v2 = (tex.v2 >> 8) * (1.0F / 256.0F);
			textinfo[i].u3 = (tex.u3 >> 8) * (1.0F / 256.0F);
			textinfo[i].v3 = (tex.v3 >> 8) * (1.0F / 256.0F);
		}

		textinfo[i].u4 = (tex.u4 >> 8) * (1.0F / 256.0F);
		textinfo[i].v4 = (tex.v4 >> 8) * (1.0F / 256.0F);
	}

	AdjustUV(nTInfos);
	return 1;
}

bool LoadItems()
{
	ITEM_INFO* item;
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	STATIC_INFO* stat;
	long x, y, z, num_items;

	Log(__FUNCTION__);
	num_items = levelUncompressed.ReadLong();
	if (!num_items)
		return 1;

	items = (ITEM_INFO*)MEM_Allocate(MAX_ITEMS * sizeof(ITEM_INFO));
	level_items = num_items;
	InitialiseClosedDoors();
	InitialiseItemArray(MAX_ITEMS);

	for (int i = 0; i < num_items; i++)
	{
		item = &items[i];
		item->object_number = levelUncompressed.ReadShort();
		item->room_number = levelUncompressed.ReadShort();
		item->pos.x_pos = levelUncompressed.ReadLong();
		item->pos.y_pos = levelUncompressed.ReadLong();
		item->pos.z_pos = levelUncompressed.ReadLong();
		item->pos.y_rot = levelUncompressed.ReadShort();
		item->shade = levelUncompressed.ReadShort();
		item->trigger_flags = levelUncompressed.ReadShort();
		item->flags = levelUncompressed.ReadShort();
	}

	for (int i = 0; i < num_items; i++)
		InitialiseItem(i);

	for (int i = 0; i < number_rooms; i++)
	{
		r = &room[i];

		for (int j = 0; j < r->num_meshes; j++)
		{
			x = (r->mesh[j].x - r->x) >> 10;
			z = (r->mesh[j].z - r->z) >> 10;

			floor = &(r->floor[x * r->x_size + z]);

			if (boxes[floor->box].overlap_index & 0x4000)
				continue;

			stat = &static_objects[r->mesh[j].static_number];
			y = floor->floor << 8;

			if (y <= (r->mesh[j].y - stat->y_maxc + 512) && y < r->mesh[j].y - stat->y_minc)
			{
				if (!stat->x_maxc || !stat->x_minc || !stat->z_maxc || !stat->z_minc ||
					(stat->x_maxc ^ stat->x_minc) & 0x8000 && (stat->z_maxc ^ stat->z_minc) & 0x8000)
				{
					x = (r->mesh[j].x - r->x) >> 10;
					z = (r->mesh[j].z - r->z) >> 10;
					r->floor[x * r->x_size + z].stopper = 1;
				}
			}
		}
	}

	return 1;
}

bool LoadAIInfo()
{
	Log(__FUNCTION__);
	long num = levelUncompressed.ReadLong();
	if (num <= 0)
		return 1;
	nAIObjects = (short)num;
	AIObjects = (AIOBJECT*)MEM_Allocate(num * sizeof(AIOBJECT));
	levelUncompressed.ReadBytes(AIObjects, num * sizeof(AIOBJECT));
	return 1;
}

bool LoadSampleMap()
{
	Log(__FUNCTION__);
	levelUncompressed.ReadShort(); // Sample map size for TRNG else 0
	return 1;
}

bool LoadSamples()
{
	long num_infos, num_samples, uncomp_size, comp_size;

	Log(__FUNCTION__);
	sample_lut = (short*)MEM_Allocate(MAX_SAMPLES * sizeof(short));
	levelUncompressed.ReadBytes(sample_lut, MAX_SAMPLES * sizeof(short));

	num_infos = levelUncompressed.ReadLong(); Log("Number Of Sample Infos %d", num_infos);
	if (!num_infos)
	{
		Log("No Sample Infos");
		return 0;
	}

	sample_infos = (SAMPLE_INFO*)MEM_Allocate(num_infos * sizeof(SAMPLE_INFO));
	levelUncompressed.ReadBytes(sample_infos, num_infos * sizeof(SAMPLE_INFO));

	num_samples = levelUncompressed.ReadLong();
	if (!num_samples)
	{
		Log("No Samples");
		return 0;
	}

	Log("Number Of Samples %d", num_samples);
	num_samples = levelCompressed.ReadLong();
	if (num_samples <= 0)
		return 1;

	InitSampleDecompress();
	for (int i = 0; i < num_samples; i++)
	{
		uncomp_size = levelCompressed.ReadLong();
		comp_size = levelCompressed.ReadLong();
		levelCompressed.ReadBytes(samples_buffer, comp_size);
		if (!DXCreateSampleADPCM(samples_buffer, comp_size, uncomp_size, i))
		{
			FreeSampleDecompress();
			return 0;
		}
	}

	FreeSampleDecompress();
	return 1;
}

static INT LoadLevel(LPVOID name)
{
	OBJECT_INFO* obj;
	TEXTURESTRUCT* tex;
	MESH_DATA* mesh;
	long version, uncompressedSize, compressedSize;
	short RTPages, OTPages, BTPages;
	short data[16];

	Log("Begin " __FUNCTION__);

	FreeLevel();
	nTextures = 1;
	Textures[0].tex = 0;
	Textures[0].surface = 0;
	Textures[0].width = 0;
	Textures[0].height = 0;
	Textures[0].bump = 0;

	if (levelCompressed.LoadFile((char*)name))
	{
		version = levelCompressed.ReadLong();
		RTPages = levelCompressed.ReadShort();
		OTPages = levelCompressed.ReadShort();
		BTPages = levelCompressed.ReadShort();

		S_InitLoadBar(OTPages + BTPages + RTPages + 20);
		S_LoadBar();

		LoadTextures(RTPages, OTPages, BTPages);

		levelCompressed.ReadBytes(data, sizeof(data));
		LaraDrawType = data[0] + LARA_NORMAL;
		WeatherType = data[1];
		// The other 28 bytes is empty.

		/// ========================================
		/// READ GEOMETRY DATA
		/// ========================================

		uncompressedSize = levelCompressed.ReadLong();
		compressedSize = levelCompressed.ReadLong();
		compressed = (char*)malloc(uncompressedSize);
		decompressed = (char*)malloc(uncompressedSize);
		levelCompressed.ReadBytes(compressed, compressedSize);
		levelUncompressed.LoadFile(compressed, uncompressedSize);

		LoadRooms();
		S_LoadBar();

		LoadObjects();
		S_LoadBar();

		LoadSprites();
		S_LoadBar();

		LoadCameras();
		S_LoadBar();

		LoadSoundEffects();
		S_LoadBar();

		LoadBoxes();
		S_LoadBar();

		LoadAnimatedTextures();
		S_LoadBar();

		LoadTextureInfos();
		S_LoadBar();

		//uncompressedSize = levelUncompressed.ReadLong();		//nItems
		//decompressed = (char*)malloc(24 * uncompressedSize);
		//levelUncompressed.ReadBytes(decompressed, 24 * uncompressedSize);
		//SafeFree(decompressed);

		LoadItems();
		S_LoadBar();

		LoadAIInfo();
		S_LoadBar();

		LoadSampleMap();
		S_LoadBar();

		if (acm_ready && !App.SoundDisabled)
			LoadSamples();

		S_LoadBar();

		for (int i = 0; i < 6; i++)
		{
			obj = &objects[WATERFALL1 + i];
			if (obj->loaded)
			{
				tex = &textinfo[mesh_vtxbuf[obj->mesh_index]->gt4[4] & 0x7FFF];
				AnimatingWaterfalls[i] = tex;
				AnimatingWaterfallsV[i] = tex->v1;
			}
		}

		S_LoadBar();
		S_GetUVRotateTextures();
		S_LoadBar();
		SetupGame();
		S_LoadBar();
		SetFadeClip(0, 1);
		reset_cutseq_vars();

		if (gfCurrentLevel == LVL5_STREETS_OF_ROME) find_a_fucking_item(ANIMATING10)->mesh_bits = 11;
		if (gfCurrentLevel == LVL5_OLD_MILL) find_a_fucking_item(ANIMATING16)->mesh_bits = 1;

		MonitorScreenTex = 0;
		obj = &objects[MONITOR_SCREEN];
		if (obj->loaded)
		{
			mesh = (MESH_DATA*)meshes[objects[MONITOR_SCREEN].mesh_index];

			for (int i = 0; i < mesh->ngt4; i++)
			{
				if (mesh->gt4[i * 6 + 5] & 1)	//semitrans quad
				{
					mesh->gt4[i * 6 + 5] &= ~1;	//no more
					MonitorScreenTex = &textinfo[mesh->gt4[i * 6 + 4] & 0x7FFF];
					MonitorScreenU = MonitorScreenTex->u1;
					break;
				}
			}
		}

		aInit();
		ClearFX();
	}

	aMakeCutsceneResident(gfResidentCut[0], gfResidentCut[1], gfResidentCut[2], gfResidentCut[3]);
	LevelLoadingThread.active = false;
	LevelLoadingThread.ended = true;
	return 1;
}

long S_LoadLevelFile(long num)
{
	static long lscreen = 0;
	static long flag = 0;
	long chosen_screen;
	char name[80];

	Log(__FUNCTION__);

	if (!tomb5.tr4_loadscreens || (!num && !bDoCredits && !gfStatus))
	{
		if (!MonoScreenOn)
		{
			chosen_screen = num;

			if (!num)
			{
				if (flag)
				{
					chosen_screen = lscreen % 3 + 15;
					lscreen++;
				}
				else
				{
					flag = 1;
					chosen_screen = -2;
				}
			}

			LoadScreen(chosen_screen + 2, 4);
		}
	}

	strcpy(name, &gfFilenameWad[gfFilenameOffset[num]]);
	loadbar_on = 0;
	strcat(name, ".TRC");

	for (int i = 0; i < 4; i++)
	{
		_BeginScene();
		InitBuckets();
		InitialiseSortList();
		DrawLoadingScreen();
		SortPolyList(SortCount, SortList);
		DrawSortList();
		S_DumpScreenFrame();
	}

	if (!tomb5.tr4_loadscreens || (!num && !bDoCredits && !gfStatus))
	{
		if (MonoScreenOn == 2)
			ReleaseScreen();
	}

	LevelLoadingThread.active = true;
	LevelLoadingThread.ended = false;
	LevelLoadingThread.handle = SDL_CreateThread(LoadLevel, "LoadLevel", name);

	while (LevelLoadingThread.active)
	{
		if (App.dx.Flags & DXF_HWR && loadbar_on)
			S_DrawLoadBar();
	}

	if (App.dx.Flags & DXF_HWR && !S_DrawLoadBar())
		while (!S_DrawLoadBar());

	return 1;
}
