#pragma once

static std::string ReadGetNameFromFName(int key) {
	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
	uint16_t NameOffset = (uint16_t)key;
 
	uint64_t NamePoolChunk = read<uint64_t>(g_pid, g_base_address + 0xC563880 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset); //((ChunkOffset + 2) * 8) ERROR_NAME_SIZE_EXCEEDED
	uint16_t nameEntry = read<uint16_t>(g_pid, NamePoolChunk);
 
	int nameLength = nameEntry >> 6;
	char buff[1024];
	if ((uint32_t)nameLength)
	{
		for (int x = 0; x < nameLength; ++x)
		{
			buff[x] = read<char>(g_pid, NamePoolChunk + 4 + x);
		}
		char* v2 = buff; // rbx
		unsigned int v4 = nameLength;
		unsigned int v5; // eax
		__int64 result; // rax
		int v7; // ecx
		char v8; // kr00_4
		__int64 v9; // ecx
 
		v5 = 0;
		result = 30i64;
		if (v4)
		{
			do
			{
				v7 = v5 | result;
				++v2;
				++v5;
				v8 = ~(BYTE)v7;
				result = (unsigned int)(2 * v7);
				*(v2 - 1) ^= v8;
			} while (v5 < v4);
		}
		buff[nameLength] = '\0';
		return std::string(buff);
	}
	else
	{
		return "";
	}
}
 
static std::string GetNameFromFName(int key)
{
	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
	uint16_t NameOffset = (uint16_t)key;
 
	uint64_t NamePoolChunk = read<uint64_t>(g_pid, g_base_address + 0xC563880 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset); //((ChunkOffset + 2) * 8) ERROR_NAME_SIZE_EXCEEDED
	if (read<uint16_t>(g_pid, NamePoolChunk) < 64)
	{
		auto a1 = read<DWORD>(g_pid, NamePoolChunk + 4);
		return ReadGetNameFromFName(a1);
	}
	else
	{
		return ReadGetNameFromFName(key);
	}
}
