#define _USE_MATH_DEFINES
#include <math.h>
#include "HandyMathLib/hm.h"
#include "HandyMathLib/perf_ext.h"
#undef assert
//namespace std
//{
	template <typename Type, typename = std::enable_if_t<std::is_arithmetic<Type>::value>> Tvec<Type, 2> make_tuple(const Type x, const Type y) { return Tvec<Type, 2>(x, y); }
	template <typename Type, typename = std::enable_if_t<std::is_arithmetic<Type>::value>> Tvec<Type, 3> make_tuple(const Type x, const Type y, const Type z) { return Tvec<Type, 3>(x, y, z); }
	template <typename Type, typename = std::enable_if_t<std::is_arithmetic<Type>::value>> Tvec<Type, 4> make_tuple(const Type x, const Type y, const Type z, const Type w) { return Tvec<Type, 4>(x, y, z, w); }
#define DEFINE_make_tuple(Type) \
	Tvec<Type, 2> make_tuple(const Type x, const Type y) { return Tvec<Type, 2>(x, y); } \
	Tvec<Type, 3> make_tuple(const Type x, const Type y, const Type z) { return Tvec<Type, 3>(x, y, z); } \
	Tvec<Type, 4> make_tuple(const Type x, const Type y, const Type z, const Type w) { return Tvec<Type, 4>(x, y, z, w); }
//	DEFINE_make_tuple(int)
//}

namespace math
{
	static const double pi = M_PI;
	static const double e  = M_E;
}

//template <class T, class Ta> inline T snap(const T x, const Ta grid_size) {return floor(x/grid_size + T(0.5)) * grid_size;}

template <class T, class Ta> inline T round(const T number, const Ta ndigits) {return snap(number, pow(0.1, ndigits));}

//template <class T> inline T fract(const T x) {return x - floor(x);}

inline double log(const double x, const double base) {return log(x) / log(base);}

//inline double radians(double deg) {return deg*0.017453292519943295769236907684886;}
//inline double degrees(double rad) {return rad*57.295779513082320876798154814105;}

//inline const short    mod(short    x, short    y) {return x%y;}
//inline const int      mod(int      x, int      y) {return x%y;}
//inline const unsigned mod(unsigned x, unsigned y) {return x%y;}
//template <class T, class Ta> inline T mod(const T &x, const Ta &y) {return x - y*floor(x/y);}

//template <class T> inline T sign(const T x) {return x>0 ? T(1) : (x<0 ? T(-1) : 0);}

// https://stackoverflow.com/a/776523
#ifdef _MSC_VER
#include <intrin.h>
inline unsigned long long rotl(unsigned long long value, int shift) {return _rotl64(value, shift);}
inline unsigned long long rotr(unsigned long long value, int shift) {return _rotr64(value, shift);}
#else
#include <x86intrin.h>
inline unsigned long long rotl(unsigned long long value, int shift) {return (value << shift) | (value >> (64 - shift));} // https://stackoverflow.com/a/24370769
inline unsigned long long rotr(unsigned long long value, int shift) {return (value >> shift) | (value << (64 - shift));}
#endif
inline int rotl(int value, int shift) {return _rotl(value, shift);}
inline int rotr(int value, int shift) {return _rotr(value, shift);}
