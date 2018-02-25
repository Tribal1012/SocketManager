#pragma once
#include <cstdint>

/* Normal integer type */
typedef uint8_t		uint8;
typedef uint16_t	uint16;
typedef uint32_t	uint32;
typedef uint64_t	uint64;
typedef int8_t		int8;
typedef int16_t		int16;
typedef int32_t		int32;
typedef int64_t		int64;

/* Pointer type */
typedef uint8*		puint8;
typedef uint16*		puint16;
typedef uint32*		puint32;
typedef uint64*		puint64;
typedef int8*		pint8;
typedef int16*		pint16;
typedef int32*		pint32;
typedef int64*		pint64;

/* Special type */
typedef const char*  ccData;		// be constant data type for send
typedef char*		 cData;		    // not constant data type for receive
typedef const uint32 cuint32;       // constant uint32 type for receive
