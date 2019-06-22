#pragma once

#include <stdint.h>
namespace util{
	inline bool isLittleEndian()
	{
		static union{ uint8_t i8[2]; uint16_t i16; } a;
		a.i16 = 0x0102;
		if (a.i8[0] == 0x01 && a.i8[1] == 0x02)
		{
			return false;
		}
		return true;
	};

	inline uint16_t reverse16(uint16_t value)
	{
		return ((value >> 8) & 0xff) | ((value << 8) & 0xff00);
	};

	inline uint32_t reverse32(uint32_t value)
	{
		return (reverse16(value & 0xffff) << 16) | reverse16((value >> 16) & 0xffff);
	};

	inline uint64_t reverse64(uint64_t value)
	{
		return (reverse32(value & 0xffffffff) << 32) | (reverse32((value >> 32) & 0xffffffff));
	};

	inline uint16_t toBigEndian(uint16_t value)
	{
		if (isLittleEndian())
		{
			return reverse16(value);
		}
		return value;
	};

	inline uint16_t toLittleEndian(uint16_t value)
	{
		if (isLittleEndian())
		{
			return value;
		}
		return reverse16(value);
	};

	inline uint32_t toBigEndian(uint32_t value)
	{
		if (isLittleEndian())
		{
			return reverse32(value);
		}

		return value;
	};

	inline uint32_t toLittleEndian(uint32_t value){
		if (isLittleEndian())
		{
			return value;
		}
		return reverse32(value);
	}

	inline uint64_t toBigEndian(uint64_t value)
	{
		if (isLittleEndian())
		{
			return reverse64(value);
		}
		return value;
	};

	inline uint64_t toLittleEndian(uint64_t value)
	{
		if (isLittleEndian())
		{
			return value;
		}
		return reverse64(value);
	};
}