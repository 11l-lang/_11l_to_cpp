//Handy Math Library Performance Extension

#ifndef HM_PERF_EXT_H
#define HM_PERF_EXT_H

#include "func.h"
#ifdef _M_AMD64
#include <emmintrin.h>
#else
#include <xmmintrin.h>
#endif
#include <string.h> // for memcmp [in GCC]


template <int size> INLINE bool objEqual(const void *obj1, const void *obj2) {return memcmp(obj1, obj2, size) == 0;}
template <> INLINE bool objEqual<1>(const void *obj1, const void *obj2) {return *(char *)obj1 == *(char *)obj2;}
template <> INLINE bool objEqual<2>(const void *obj1, const void *obj2) {return *(short*)obj1 == *(short*)obj2;}
template <> INLINE bool objEqual<4>(const void *obj1, const void *obj2) {return *(long *)obj1 == *(long *)obj2;}
template <> INLINE bool objEqual<8>(const void *obj1, const void *obj2) {return *(long long*)obj1 == *(long long*)obj2;}
template <class T> INLINE bool objEqual(const T &obj1, const T &obj2) {return objEqual<sizeof(T)>(&obj1, &obj2);}
template <int N> INLINE HMbool operator==(const Tvec<HMbyte  , N> &v, const Tvec<HMbyte  , N> &V) {return  objEqual(v, V);}
template <int N> INLINE HMbool operator!=(const Tvec<HMbyte  , N> &v, const Tvec<HMbyte  , N> &V) {return !objEqual(v, V);}
template <int N> INLINE HMbool operator==(const Tvec<HMubyte , N> &v, const Tvec<HMubyte , N> &V) {return  objEqual(v, V);}
template <int N> INLINE HMbool operator!=(const Tvec<HMubyte , N> &v, const Tvec<HMubyte , N> &V) {return !objEqual(v, V);}
template <int N> INLINE HMbool operator==(const Tvec<HMshort , N> &v, const Tvec<HMshort , N> &V) {return  objEqual(v, V);}
template <int N> INLINE HMbool operator!=(const Tvec<HMshort , N> &v, const Tvec<HMshort , N> &V) {return !objEqual(v, V);}
template <int N> INLINE HMbool operator==(const Tvec<HMushort, N> &v, const Tvec<HMushort, N> &V) {return  objEqual(v, V);}
template <int N> INLINE HMbool operator!=(const Tvec<HMushort, N> &v, const Tvec<HMushort, N> &V) {return !objEqual(v, V);}
template <int N> INLINE HMbool operator==(const Tvec<HMint   , N> &v, const Tvec<HMint   , N> &V) {return  objEqual(v, V);}
template <int N> INLINE HMbool operator!=(const Tvec<HMint   , N> &v, const Tvec<HMint   , N> &V) {return !objEqual(v, V);}
template <int N> INLINE HMbool operator==(const Tvec<HMuint  , N> &v, const Tvec<HMuint  , N> &V) {return  objEqual(v, V);}
template <int N> INLINE HMbool operator!=(const Tvec<HMuint  , N> &v, const Tvec<HMuint  , N> &V) {return !objEqual(v, V);}

template <> INLINE const float min<float>(const float x, const float y)//используется специализация шаблона, т.к. просто перегруженная функция с float выбирается там, где не надо, напр. для min(int, short)
{
	float r;
	_mm_store_ss(&r, _mm_min_ss(_mm_load_ss(&x), _mm_load_ss(&y)));
	return r;
}
template <> INLINE const float max<float>(const float x, const float y)
{
	float r;
	_mm_store_ss(&r, _mm_max_ss(_mm_load_ss(&x), _mm_load_ss(&y)));
	return r;
}

template <int N> INLINE const Tvec<float, N> min(const Tvec<float, N> &x, const Tvec<float, N> &y)
{
	Tvec<float, N> r;
	for (int i=0; i<N; i++) r[i] = min(x[i], y[i]);
	return r;
}
template <int N> INLINE const Tvec<float, N> max(const Tvec<float, N> &x, const Tvec<float, N> &y)
{
	Tvec<float, N> r;
	for (int i=0; i<N; i++) r[i] = max(x[i], y[i]);
	return r;
}

template <int N> INLINE float min(const Tvec<float, N> &v)
{
	float r = v[0];
	for (int i=1; i<N; i++) r = min(r, v[i]);
	return r;
}
template <int N> INLINE float max(const Tvec<float, N> &v)
{
	float r = v[0];
	for (int i=1; i<N; i++) r = max(r, v[i]);
	return r;
}

template <> INLINE const float clamp<float>(const float &x, const float &minVal, const float &maxVal)//аналогично min/max, все функции с несколькими аргументами нужно делать специализацией, а не перегрузкой
{
	return max(minVal, min(x, maxVal));
}

INLINE const float inversesqrt(const float x)
{
	float r;
	_mm_store_ss(&r, _mm_rsqrt_ss(_mm_load_ss(&x)));
	return (3 - x * r * r) * 0.5f * r;//extended with a Newton-Raphson step
}

template <int N> INLINE const Tvec<float, N> normalize(const Tvec<float, N> &v) {return v * inversesqrt(sqlen(v));}
INLINE const quat normalize(const quat &q) {return q * inversesqrt(sqlen(q));}

INLINE void fastnormalize(quat res[2], const quat dq[2])//res and dq can point to the same location
{
	float t = inversesqrt(sqlen(dq[0]));//actual length of dual quat is not so trivial!
	res[0] = dq[0] * t;
	res[1] = dq[1] * t;
}

INLINE const float fastsqrt(const float x)
{
//	return x * inversesqrt(max(x, limits<float>::min()));//3x times faster than _mm_sqrt_ss with accuracy of 22 bits (but compiler optimize this not so well, and sqrtss works faster)
	float r;
	_mm_store_ss(&r, _mm_sqrt_ss(_mm_load_ss(&x)));
	return r;
}

INLINE const float length(const vec2 &v) {return fastsqrt(sqlen(v));}
INLINE const float length(const vec3 &v) {return fastsqrt(sqlen(v));}
INLINE const float length(const vec4 &v) {return fastsqrt(sqlen(v));}
INLINE const float length(const quat &q) {return fastsqrt(sqlen(q));}

INLINE const float rcp(const float x)//SSE Newton-Raphson reciprocal estimate, accurate to 22 significant bits of the mantissa, 2.5x times faster than divss
{
	float r;
	_mm_store_ss(&r, _mm_rcp_ss(_mm_load_ss(&x)));
	return (r + r) - x * r * r;
}
template <int N> INLINE const Tvec<float, N> rcp(const Tvec<float, N> &v) {Tvec<float, N> r; for (int i=0; i<N; i++) r[i] = rcp(v[i]); return r;}

/*INLINE long int lround(double x)
{
#ifdef _M_AMD64
	return _mm_cvtsd_si32(_mm_load_sd(&x));
#else
	int r;
	_asm fld x
	_asm fistp dword ptr [r]
	return r;
#endif
}
INLINE long int lround(float x)
{
	return _mm_cvtss_si32(_mm_load_ss(&x));//assume that current rounding mode is always correct (i.e. round to nearest)
}*/
template <class T, int N> INLINE const Tvec<long int, N> lround(const Tvec<T, N> &v)
{
	Tvec<long int, N> r;
	for (int i=0; i<N; i++)
		r[i] = lround(v[i]);
	return r;
}
template <class T> INLINE const Trange<long int> lround(const Trange<T> &v) {return Trange<long int>(lround(v.min), lround(v.max));}
template <class T, int N> INLINE const Taabb<Tvec<long int, N>> lround(const Taabb<Tvec<T,N>> &v) {return Taabb<Tvec<long int, N>>(lround(v.min), lround(v.max));}

//Fast floor and ceil ("Fast Rounding of Floating Point Numbers in C/C++ on Wintel Platform" - http://ldesoras.free.fr/doc/articles/rounding_en.pdf)
//valid input: |x| < 2^30, wrong results: lfloor(-1e-8) = lceil(1e-8) = 0
INLINE long int lfloor(float x)//http://www.masm32.com/board/index.php?topic=9515.0
{
	x = x + x - 0.5f;
	return _mm_cvtss_si32(_mm_load_ss(&x)) >> 1;
}
INLINE long int lceil(float x)//http://www.masm32.com/board/index.php?topic=9514.0
{
	x = -0.5f - (x + x);
	return -(_mm_cvtss_si32(_mm_load_ss(&x)) >> 1);
}
template <class T, int N> INLINE const Tvec<long int, N> lfloor(const Tvec<T, N> &v) {Tvec<long int, N> r; for (int i=0; i<N; i++) r[i] = lfloor(v[i]); return r;}
template <class T, int N> INLINE const Tvec<long int, N>  lceil(const Tvec<T, N> &v) {Tvec<long int, N> r; for (int i=0; i<N; i++) r[i] =  lceil(v[i]); return r;}
template <int N> INLINE const Tvec<float, N> floor(const Tvec<float, N> &v) {return (Tvec<float, N>)lfloor(v);}
template <int N> INLINE const Tvec<float, N>  ceil(const Tvec<float, N> &v) {return (Tvec<float, N>) lceil(v);}

INLINE const float fract(const float &x) {return x - (float)lfloor(x);}
INLINE const float mod(const float &x, const float &y) {return x - y*lfloor(x/y);}
INLINE const float  snap(const float  &x, const float  &gridSize) {return lround(x/gridSize) * gridSize;}
INLINE const double snap(const double &x, const double &gridSize) {return lround(x/gridSize) * gridSize;}
template <int N> INLINE const Tvec<float , N> snap(const Tvec<float , N> &x, const float  &gridSize) {return (Tvec<float , N>)lround(x/gridSize) * gridSize;}
template <int N> INLINE const Tvec<double, N> snap(const Tvec<double, N> &x, const double &gridSize) {return (Tvec<double, N>)lround(x/gridSize) * gridSize;}
template <int N> INLINE const Tvec<float , N> snap(const Tvec<float , N> &x, const Tvec<float , N> &gridSize) {return (Tvec<float , N>)lround(x/gridSize) * gridSize;}
template <int N> INLINE const Tvec<double, N> snap(const Tvec<double, N> &x, const Tvec<double, N> &gridSize) {return (Tvec<double, N>)lround(x/gridSize) * gridSize;}


const __m128 _quat_mul_mask = _mm_set_ps(-0.f, 0, 0, 0);

INLINE __m128 _quat_mul_sse(__m128 a, __m128 b)
{
	__m128
		swiz1 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3,3,3,3)),
		swiz2 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(0,1,0,2)),
		swiz3 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(0,0,2,1)),
		swiz4 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(1,3,3,3)),
		swiz5 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(1,2,1,0)),
		swiz6 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(2,0,2,1)),
		swiz7 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(2,1,0,2)),
		mul4 = _mm_mul_ps(swiz6, swiz7),
		mul3 = _mm_mul_ps(swiz4, swiz5),
		mul2 = _mm_mul_ps(swiz2, swiz3),
		mul1 = _mm_mul_ps(a, swiz1),
		flip1 = _mm_xor_ps(mul4, _quat_mul_mask),
		flip2 = _mm_xor_ps(mul3, _quat_mul_mask),
		retVal = _mm_sub_ps(mul1, mul2),
		retVal2 = _mm_add_ps(flip1, flip2);
	return _mm_add_ps(retVal, retVal2);
}

template <> INLINE const quat quat::operator*(const quat &q) const
{
	quat r;
#ifndef QUAT_MUL_REVERSE_ORDER
	__m128 a = _mm_loadu_ps(&x), b = _mm_loadu_ps(&q.x);
#else
	__m128 a = _mm_loadu_ps(&q.x), b = _mm_loadu_ps(&x);
#endif
	_mm_storeu_ps(&r.x, _quat_mul_sse(a, b));
	return r;
}

template <> INLINE void dualQuatMul(quat res[2], const quat dq1[2], const quat dq2[2])
{
	__m128 dq10 = _mm_loadu_ps(&dq1[0].x),
		   dq11 = _mm_loadu_ps(&dq1[1].x),
		   dq20 = _mm_loadu_ps(&dq2[0].x),
		   dq21 = _mm_loadu_ps(&dq2[1].x);
	_mm_storeu_ps(&res[1].x, _mm_add_ps(_quat_mul_sse(dq10, dq21), _quat_mul_sse(dq11, dq20)));
	_mm_storeu_ps(&res[0].x, _quat_mul_sse(dq10, dq20));
}

#endif //HM_PERF_EXT_H
