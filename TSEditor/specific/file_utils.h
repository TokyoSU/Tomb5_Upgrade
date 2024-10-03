#pragma once
#include "../global/types.h"
#include "function_stubs.h"

class IFileReader
{
public:
	~IFileReader();
	void Release();

	char ReadChar();
	unsigned char ReadUChar();
	short ReadShort();
	unsigned short ReadUShort();
	int ReadInt();
	unsigned int ReadUInt();
	long ReadLong();
	unsigned long ReadULong();
	unsigned long long ReadUULong();
	float ReadFloat();
	double ReadDouble();
	void ReadBytes(void* data, size_t size);

	void Seek(int offset, int type);
protected:
	SDL_RWops* m_file = NULL;
	bool m_endoffile = false;

	template<class T>
	T _Read() {
		T result = 0;
		size_t retval = SDL_RWread(m_file, (void*)&result, sizeof(T), 1);
		if (retval == 0 || retval != 1)
		{
			Log("Failed to read file !");
			if (SDL_GetError() == NULL)
			{
				Log("Reached end of file !");
				m_endoffile = true;
			}
		}
		return result;
	}
};

class CFileReader : public IFileReader
{
public:
	bool LoadFile(LPCSTR filePath);
};

class CMemoryFileReader : public IFileReader
{
public:
	bool LoadFile(void* data, size_t size);
};
