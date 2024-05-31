#pragma once
#include <stdio.h>
#include <cassert>

#define DEV_LOG(title,message,...) printf("[%s]: ",title); printf(message,__VA_ARGS__); printf("\n");
#define DEV_ASSERT(expression,title,message,...) if(!(expression)) { DEV_LOG(title,message,__VA_ARGS__); assert(false);}
#define BYTE_TO_MB(Byte) Byte/1000000.0
#define MB_TO_BYTE(mb) mb*1000000.0
#define NANO_TO_SECONDS(nano) nano / 1000000000.0f
#define NANO_TO_MS(nano) nano / 1000000.0f
#define MS_TO_SECONDS(ms) ms / 1000.0f
#define SECONDS_TO_MS(seconds) seconds * 1000.0f
#define SECONDS_TO_NANO(seconds) seconds * 1000000000.0f
#define MS_TO_NANO(ms) ms * 1000000.0f
#define uint64_max 0xFFFFFFFFFFFFFFFF
#define GENERATE_FLAGS(flagType,dataType)\
	FORCEINLINE static flagType operator |(const flagType a, const flagType b)\
	{\
		return  (flagType)((dataType)a | (dataType)b);\
	}\
	FORCEINLINE static bool operator &(const flagType a, const flagType b)\
	{\
		return ((dataType)a & (dataType)b);\
	}
