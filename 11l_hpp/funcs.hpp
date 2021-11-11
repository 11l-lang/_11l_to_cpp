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
// [https://stackoverflow.com/questions/45898828/structured-bindings-for-your-own-type-that-isn-t-a-struct-or-a-tuplevia-public] and [https://stackoverflow.com/a/46003908/2692494]
namespace std {
	template <typename Ty, int N> struct tuple_size<Tvec<Ty, N>> : std::integral_constant<size_t, N> { };
	template <typename Ty, int N, size_t i> struct tuple_element<i, Tvec<Ty, N>> { using type = Ty; };
}
template <size_t i, typename Ty, int N> Ty& get(Tvec<Ty, N> &v) { return v[i]; }
template <size_t i, typename Ty, int N> const Ty get(const Tvec<Ty, N> &v) { return v[i]; }

template <int n, typename Ty, int N, typename Value> inline const Value &_set(Tvec<Ty, N> &c, const Value &v) {c[n] = v; return v;}

template <typename Type, int dimension> inline const Tvec<Type, dimension> tuple_sorted(const Tvec<Type, dimension> &v)
{
	Tvec<Type, dimension> r = v;
	std::sort(&r[0], &r[0] + dimension);
	return r;
}

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

template <class Type> inline auto modf(const Type x)
{
	Tvec<Type, 2> r;
	r[0] = modf(x, &r[1]);
	return r;
}

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

inline int   ext_int(int    n) {return n;}
inline Int64 ext_int(Int64  n) {return n;}
inline int   ext_int(float  n) {return (int)n;}
inline Int64 ext_int(double n) {return (Int64)n;}

template <typename Type1, typename Type2> inline auto idiv(Type1 a, Type2 b)
{
	return ext_int(a / b);
}

template <typename Type1, typename Type2> inline auto fdiv(Type1 a, Type2 b)
{
	return double(a) / b;
}

inline float fdiv(float a, float b)
{
	return a / b;
}

#ifdef __GNUC__
inline int bsr(int   x) {return __builtin_clz  (x) ^ 31;}
inline int bsr(Int64 x) {return __builtin_clzll(x) ^ 63;}
inline int bsf(int   x) {return __builtin_ctz  (x);}
inline int bsf(Int64 x) {return __builtin_ctzll(x);}
#elif _MSC_VER
inline int bsr(int x)   {unsigned long r; _BitScanReverse  (&r, x); return r;}
inline int bsf(int x)   {unsigned long r; _BitScanForward  (&r, x); return r;}
#ifdef _M_AMD64
inline int bsr(Int64 x) {unsigned long r; _BitScanReverse64(&r, x); return r;}
inline int bsf(Int64 x) {unsigned long r; _BitScanForward64(&r, x); return r;}
#endif
#else
#error Unsupported compiler
#endif
template <typename Ty> int bit_length(Ty x) {return x != 0 ? bsr(x) + 1 : 0;}

auto divmod(int   x, int   y) {  div_t r =   div(x, y); return make_tuple(r.quot, r.rem);}
auto divmod(Int64 x, Int64 y) {lldiv_t r = lldiv(x, y); return make_tuple(r.quot, r.rem);}
auto divmod(int   x, Int64 y) {return divmod(Int64(x), y);}
auto divmod(Int64 x, int   y) {return divmod(x, Int64(y));}

template <typename Ty> Ty factorial(Ty n)
{
	Ty r = 1;
	for (Ty i = 2; i <= n; i++)
		r *= i;
	return r;
}

template <typename Ty> Ty gcd(Ty a, Ty b)
{
	while (b != 0)
		std::tie(a, b) = std::make_tuple(b, a % b);
	return a;
}

int pow(int x, int y)
{
	return (int)pow(double(x), double(y));
}
Int64 pow(Int64 x, int   y) {return (Int64)pow(double(x), double(y));}
Int64 pow(int   x, Int64 y) {return (Int64)pow(double(x), double(y));}
Int64 pow(Int64 x, Int64 y) {return (Int64)pow(double(x), double(y));}

using std::swap;

template <typename Ty> bool equal(const Ty &a, const Ty &b, const Ty &c)                           {return a == b && a == c;}
template <typename Ty> bool equal(const Ty &a, const Ty &b, const Ty &c, const Ty &d)              {return a == b && a == c && a == d;}
template <typename Ty> bool equal(const Ty &a, const Ty &b, const Ty &c, const Ty &d, const Ty &e) {return a == b && a == c && a == d && a == e;}
