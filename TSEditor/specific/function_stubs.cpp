#include "../tomb5/pch.h"
#include "function_stubs.h"

#define FMT_UNICODE 0 // NOTE: remove unicode error from spdlog.
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

static std::shared_ptr<spdlog::logger> m_log = spdlog::basic_logger_mt("global", "logs/debug.txt", true);
char* malloc_buffer;
char* malloc_ptr;
long malloc_size;
long malloc_free;
long nPolyType;
static long malloc_used;
static long rand_1 = 0xD371F947;
static long rand_2 = 0xD371F947;

void init_game_malloc()
{
	malloc_buffer = (char*)malloc(MALLOC_SIZE); // NOTE: There is where malloc allocate for game_malloc().
	malloc_size = MALLOC_SIZE;
	malloc_ptr = malloc_buffer;
	malloc_free = MALLOC_SIZE;
	malloc_used = 0;
}

void* game_malloc(long size)
{
	char* ptr;

	size = (size + 3) & -4;
	if (size > malloc_free)
	{
		Log("OUT OF MEMORY");
		return 0;
	}

	ptr = malloc_ptr;
	malloc_free -= size;
	malloc_used += size;
	malloc_ptr += size;
	memset(ptr, 0, size);
	return ptr;
}

long GetRandomControl()
{
	rand_1 = 0x41C64E6D * rand_1 + 12345;
	return (rand_1 >> 10) & 0x7FFF;
}

void SeedRandomControl(long seed)
{
	rand_1 = seed;
}

long GetRandomDraw()
{
	rand_2 = 0x41C64E6D * rand_2 + 12345;
	return (rand_2 >> 10) & 0x7FFF;
}

void SeedRandomDraw(long seed)
{
	rand_2 = seed;
}

void Log(const char* s, ...)
{
	char buf[4096];
	va_list args = NULL;
	va_start(args, s);
	vsprintf_s(buf, s, args);
	va_end(args);
	m_log->info(buf);
}
