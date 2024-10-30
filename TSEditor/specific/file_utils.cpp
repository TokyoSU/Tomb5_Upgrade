#include "../tomb5/pch.h"
#include "file_utils.h"

IFileReader::~IFileReader()
{
	Release();
}

void IFileReader::Release()
{
	if (m_file)
	{
		SDL_RWclose(m_file);
		m_file = NULL;
	}
}

char IFileReader::ReadChar()
{
	return _Read<char>();
}

unsigned char IFileReader::ReadUChar()
{
	return _Read<unsigned char>();
}

short IFileReader::ReadShort()
{
	return _Read<short>();
}

unsigned short IFileReader::ReadUShort()
{
	return _Read<unsigned short>();
}

int IFileReader::ReadInt()
{
	return _Read<int>();
}

unsigned int IFileReader::ReadUInt()
{
	return _Read<unsigned int>();
}

long IFileReader::ReadLong()
{
	return _Read<long>();
}

unsigned long IFileReader::ReadULong()
{
	return _Read<unsigned long>();
}

unsigned long long IFileReader::ReadUULong()
{
	return _Read<unsigned long long>();
}

float IFileReader::ReadFloat()
{
	return _Read<float>();
}

double IFileReader::ReadDouble()
{
	return _Read<double>();
}

void IFileReader::ReadBytes(void* data, size_t size)
{
	size_t retval = SDL_RWread(m_file, data, sizeof(char), size);
	if (retval == 0 || retval != size)
	{
		Log("Failed to read file !");
		if (SDL_GetError() == NULL)
		{
			Log("Reached end of file !");
			m_endoffile = true;
		}
	}
}

bool IFileReader::IsEOF() const
{
	return m_endoffile;
}

void IFileReader::Seek(int offset, int type)
{
	SDL_RWseek(m_file, offset, type);
}

bool CFileReader::LoadFile(LPCSTR filePath)
{
	m_file = SDL_RWFromFile(filePath, "rb");
	if (m_file == NULL)
	{
		Log("Failed to read file at %s !", filePath);
		return false;
	}
	return true;
}

bool CMemoryFileReader::LoadFile(void* data, size_t size)
{
	m_file = SDL_RWFromMem(data, size);
	if (m_file == NULL)
	{
		Log("Failed to read file from memory !");
		return false;
	}
	return true;
}

IFileWriter::~IFileWriter()
{
	Release();
}

void IFileWriter::Release()
{
	if (m_file)
	{
		SDL_RWclose(m_file);
		m_file = NULL;
	}
}

void IFileWriter::WriteChar(char value)
{
	_Write(value);
}

void IFileWriter::WriteUChar(unsigned char value)
{
	_Write(value);
}

void IFileWriter::WriteShort(short value)
{
	_Write(value);
}

void IFileWriter::WriteUShort(unsigned short value)
{
	_Write(value);
}

void IFileWriter::WriteInt(int value)
{
	_Write(value);
}

void IFileWriter::WriteUInt(unsigned int value)
{
	_Write(value);
}

void IFileWriter::WriteLong(long value)
{
	_Write(value);
}

void IFileWriter::WriteULong(unsigned long value)
{
	_Write(value);
}

void IFileWriter::WriteUULong(unsigned long long value)
{
	_Write(value);
}

void IFileWriter::WriteFloat(float value)
{
	_Write(value);
}

void IFileWriter::WriteDouble(double value)
{
	_Write(value);
}

void IFileWriter::WriteBytes(void* data, size_t size)
{
	SDL_RWwrite(m_file, data, sizeof(char), size);
}

bool CFileWriter::OpenOrCreateFile(LPCSTR filePath)
{
	m_file = SDL_RWFromFile(filePath, "wb");
	if (m_file == NULL)
	{
		Log("Failed to create or open file from filePath: %s", filePath);
		return false;
	}
	return true;
}
