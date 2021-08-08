#ifndef HM_TYPEDEF_H
#define HM_TYPEDEF_H

typedef bool HMbool;
typedef signed char HMbyte;
typedef signed short HMshort;
typedef signed int HMint;
typedef signed long long HMlong_long;
typedef unsigned char HMubyte;
typedef unsigned short HMushort;
typedef unsigned int HMuint;
typedef float HMfloat;
typedef double HMdouble;
typedef long double HMlong_double;

#define HMtrue  true
#define HMfalse false

#if defined(_MSC_VER) && _MSC_VER > 1000
#define INLINE inline//__forceinline
#else
#define INLINE inline
#endif


//Undefine Microsoft macroses
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif


template <class T> class hm_limits
{
public:
	static INLINE const T max();
};

#include <float.h>
#include <limits.h>
#define UCHAR_MIN     0
#define USHRT_MIN     0
#define UINT_MIN      0

#define IMPLEMENT_LIMITS(T, ID)						\
template <> class hm_limits<T>						\
{													\
public:												\
	static INLINE const T min() {return ID##_MIN;}	\
	static INLINE const T max() {return ID##_MAX;}	\
};

IMPLEMENT_LIMITS(HMbyte,SCHAR)
IMPLEMENT_LIMITS(HMubyte,UCHAR)
IMPLEMENT_LIMITS(HMshort,SHRT)
IMPLEMENT_LIMITS(HMushort,USHRT)
IMPLEMENT_LIMITS(HMint,INT)
IMPLEMENT_LIMITS(HMuint,UINT)
IMPLEMENT_LIMITS(HMfloat,FLT)
IMPLEMENT_LIMITS(HMdouble,DBL)
IMPLEMENT_LIMITS(HMlong_double,LDBL)

#undef IMPLEMENT_LIMITS

#endif //HM_TYPEDEF_H
