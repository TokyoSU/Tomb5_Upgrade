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

	bool IsEOF() const;
	void Seek(int offset, int type);
protected:
	SDL_RWops* m_file = NULL;
	bool m_endoffile = false;

	template<class T>
	LPCSTR GetTypeName(T id)
	{
		if (typeid(id) == typeid(char))
			return "char";
		else if (typeid(id) == typeid(unsigned char))
			return "uchar";
		else if (typeid(id) == typeid(short))
			return "short";
		else if (typeid(id) == typeid(unsigned short))
			return "ushort";
		else if (typeid(id) == typeid(int))
			return "int";
		else if (typeid(id) == typeid(unsigned int))
			return "uint";
		else if (typeid(id) == typeid(long))
			return "long";
		else if (typeid(id) == typeid(unsigned long))
			return "ulong";
		else if (typeid(id) == typeid(unsigned long long))
			return "uulong";
		else if (typeid(id) == typeid(float))
			return "float";
		else if (typeid(id) == typeid(double))
			return "double";
		return "undefined";
	}

	template<class T>
	T _Read() {
		T result = 0;
		size_t retval = SDL_RWread(m_file, (void*)&result, sizeof(T), 1);
		if (retval == 0 || retval != 1)
		{
			Log("Failed to read file %s !", GetTypeName<T>(result));
			if (SDL_GetError() == NULL)
			{
				Log("Reached end of file !");
				m_endoffile = true;
			}
		}
		return result;
	}
};

class IFileWriter
{
public:
	~IFileWriter();
	void Release();

	void WriteChar(char value);
	void WriteUChar(unsigned char value);
	void WriteShort(short value);
	void WriteUShort(unsigned short value);
	void WriteInt(int value);
	void WriteUInt(unsigned int value);
	void WriteLong(long value);
	void WriteULong(unsigned long value);
	void WriteUULong(unsigned long long value);
	void WriteFloat(float value);
	void WriteDouble(double value);

	void WriteBytes(void* data, size_t size);
protected:
	SDL_RWops* m_file = NULL;
	bool m_endoffile = false;

	template<class T>
	LPCSTR GetTypeName(T id)
	{
		if (typeid(id) == typeid(char))
			return "char";
		else if (typeid(id) == typeid(unsigned char))
			return "uchar";
		else if (typeid(id) == typeid(short))
			return "short";
		else if (typeid(id) == typeid(unsigned short))
			return "ushort";
		else if (typeid(id) == typeid(int))
			return "int";
		else if (typeid(id) == typeid(unsigned int))
			return "uint";
		else if (typeid(id) == typeid(long))
			return "long";
		else if (typeid(id) == typeid(unsigned long))
			return "ulong";
		else if (typeid(id) == typeid(unsigned long long))
			return "uulong";
		else if (typeid(id) == typeid(float))
			return "float";
		else if (typeid(id) == typeid(double))
			return "double";
		return "undefined";
	}

	template<class T>
	void _Write(T data) {
		size_t retval = SDL_RWwrite(m_file, &data, sizeof(T), 1);
		if (retval == 0 || retval != 1)
		{
			Log("Failed to read file %s !", GetTypeName(data));
			if (SDL_GetError() == NULL)
			{
				Log("Reached end of file !");
				m_endoffile = true;
			}
		}
	}
};

class CFileWriter : public IFileWriter
{
public:
	bool OpenOrCreateFile(LPCSTR filePath);
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
