#pragma once
#include "../global/types.h"

void MALLOC_InitializeMemory();
void MALLOC_ReleaseMemory();
void* MALLOC_AllocateMemory(int size);
long GetRandomControl();
void SeedRandomControl(long seed);
long GetRandomDraw();
void SeedRandomDraw(long seed);
void Log(const char* s, ...);

template<class _Ty>
static void SafeRelease(_Ty* ptr, LPCSTR name)
{
	if (ptr)
	{
		Log("Released %s @ %x - RefCnt = %d", name, ptr, ptr->Release());
		ptr = NULL;
	}
	else
	{
		Log("%s Attempt To Release NULL Ptr", name);
	}
}

template<class _Ty>
static void SafeFree(_Ty* ptr)
{
	if (ptr)
	{
		free(ptr);
		ptr = NULL;
	}
}

extern char* malloc_buffer;
extern char* malloc_ptr;
extern long malloc_size;
extern long malloc_free;
extern long nPolyType;
