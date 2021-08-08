#ifndef HM_FUNC_H
#define HM_FUNC_H

#include "vec.h"
#include "mat.h"
#include "quat.h"
//#include "aabb.h"



//For internal use only
				   INLINE int dimension(const HMlong_double) {return 1;}
template <class T> INLINE int dimension(const Tvec<T,2> &) {return 2;}
template <class T> INLINE int dimension(const Tvec<T,3> &) {return 3;}
template <class T> INLINE int dimension(const Tvec<T,4> &) {return 4;}

	   INLINE HMlong_double sum(const HMlong_double x)   {return x;}
template <class T> INLINE T sum(const Tvec<T,2> &v) {return v.x+v.y;}
template <class T> INLINE T sum(const Tvec<T,3> &v) {return v.x+v.y+v.z;}
template <class T> INLINE T sum(const Tvec<T,4> &v) {return v.x+v.y+v.z+v.w;}

	   INLINE HMlong_double dereference(const HMlong_double x,const int)  {return x;}
template <class T> INLINE T dereference(const Tvec<T,2> &v,const int i) {return v[i];}
template <class T> INLINE T dereference(const Tvec<T,3> &v,const int i) {return v[i];}
template <class T> INLINE T dereference(const Tvec<T,4> &v,const int i) {return v[i];}

//Type Cast Functions

#if _MSC_VER > 1000
	#pragma warning(push)
	#pragma warning(disable:4800)
#endif // _MSC_VER > 1000

#if defined(_MSC_VER) && _MSC_VER<=1200 //for MSVC++ 6.0 and earlier
	#define dst dstT::TYPE
#else
	#define dst typename dstT::TYPE
#endif

template <class dstT,class srcT> INLINE const dstT vec_cast(const Tvec<srcT,2> &v) {return dstT((dst)v.x, (dst)v.y);}
template <class dstT,class srcT> INLINE const dstT vec_cast(const Tvec<srcT,3> &v) {return dstT((dst)v.x, (dst)v.y, (dst)v.z);}
template <class dstT,class srcT> INLINE const dstT vec_cast(const Tvec<srcT,4> &v) {return dstT((dst)v.x, (dst)v.y, (dst)v.z, (dst)v.w);}

template <class dstT> INLINE const dstT vec_cast(const bvec2 &v) {return dstT((dst)v.x, (dst)v.y);}
template <class dstT> INLINE const dstT vec_cast(const bvec3 &v) {return dstT((dst)v.x, (dst)v.y, (dst)v.z);}
template <class dstT> INLINE const dstT vec_cast(const bvec4 &v) {return dstT((dst)v.x, (dst)v.y, (dst)v.z, (dst)v.w);}

template <class dstT,class srcT> INLINE const dstT mat_cast(const srcT &m) {return dstT(m);}

template <class dstT,class srcT> INLINE const dstT quat_cast(const Tquat<srcT> &q)
{return dstT((dst)q.x, (dst)q.y, (dst)q.z, (dst)q.w);}

template <class Tvec> class Taabb;
template <class dstT,class srcT> INLINE const dstT aabb_cast(const Taabb<srcT> &b)
{return dstT(vec_cast<dst>(b.min), vec_cast<dst>(b.max));}

template <class dstT,class srcT> INLINE const dstT rect_cast(const Taabb<srcT> &r)
{return dstT(vec_cast<dst>(r.min), vec_cast<dst>(r.max));}

template <class T> class Trange;
template <class dstT,class srcT> INLINE const dstT range_cast(const Trange<srcT> &r)
{return dstT(dst(r.min), dst(r.max));}

#undef dst

#if _MSC_VER > 1000
	#pragma warning(pop)
#endif // _MSC_VER > 1000

//Angle Functions
#ifdef PI
#undef PI
#endif
static const HMfloat PIf=3.1415926535897932384626433832795f;
static const HMdouble PI=3.1415926535897932384626433832795;
static const HMlong_double PIl=3.1415926535897932384626433832795L;
template <class T> INLINE const T radians(const T &deg) {return deg*T(0.017453292519943295769236907684886L);}
template <class T> INLINE const T degrees(const T &rad) {return rad*T(57.295779513082320876798154814105L);}

//Exponential Functions
template <class T> INLINE const T exp2(const T &x) {return exp(x*T(0.69314718055994530941723212145818L));}
//template <class T> INLINE const T log2(const T &x) {return log(x)*T(1.4426950408889634073599246810019L);}
template <class T, int N> INLINE const Tvec<T,N> sqrt(const Tvec<T,N> &v)
{
	Tvec<T,N> r;
	for (int i=0; i<N; i++)
		r[i] = sqrt(v[i]);
	return r;
}
template <class T> INLINE const T inversesqrt(const T &x) {return T(1)/sqrt(x);}

//Pseudorandom Number Generating Functions
/*static HMuint HMRandomSeed=0;

INLINE void rseed(const HMuint newSeed)
{HMRandomSeed=newSeed;}

#if _MSC_VER>=1300 && _MSC_VER<1500 //for MSVC++ >=7.0 and <9.0
__declspec(noinline)
#endif
INLINE const HMuint random()
{return HMRandomSeed=1664525L*HMRandomSeed+1013904223L;}

INLINE const HMuint random(const HMuint randMax)
{return ((unsigned long long)random() * randMax) >> 32u;}

INLINE const HMuint random(const HMuint randMin,const HMuint randMax)
{return random(randMax-randMin)+randMin;}

INLINE const HMint random(const HMint randMax) {return (HMint)random(HMuint(randMax));}
INLINE const HMint random(const HMint randMin,const HMint randMax)
{return (HMint)random(HMuint(randMin),HMuint(randMax));}

INLINE const HMlong_double random(const HMlong_double randMax)
{return random()*(randMax/hm_limits<HMuint>::max());}

INLINE const HMlong_double random(const HMlong_double randMin,const HMlong_double randMax)
{return random(randMax-randMin)+randMin;}

INLINE const HMfloat random(const HMfloat randMax) {return (HMfloat)random((HMlong_double)randMax);}
INLINE const HMfloat random(const HMfloat randMin,const HMfloat randMax) {return (HMfloat)random((HMlong_double)randMin,(HMlong_double)randMax);}
INLINE const HMdouble random(const HMdouble randMax) {return (HMdouble)random((HMlong_double)randMax);}
INLINE const HMdouble random(const HMdouble randMin,const HMdouble randMax) {return (HMdouble)random((HMlong_double)randMin,(HMlong_double)randMax);}
*/
//Common Functions
#if defined(_MSC_VER) && _MSC_VER<=1200 //for MSVC++ 6.0 and earlier
#define T_TYPE T::TYPE
INLINE HMfloat abs(HMfloat x) {return fabs(x);}
INLINE HMdouble abs(HMdouble x) {return fabs(x);}
INLINE HMlong_double abs(HMlong_double x) {return fabs(x);}
#else
#define T_TYPE typename T::TYPE
#endif

#ifdef __GNUC__
#include <cmath>
using std::abs;
#endif

template <class T> INLINE const T square(const T &a) {return a*a;}
template <class T> INLINE const T cube(const T &a) {return a*a*a;}
template <class T> INLINE const Tvec<T,2> abs(const Tvec<T,2> &v) {return Tvec<T,2>((T)abs(v.x),(T)abs(v.y));}
template <class T> INLINE const Tvec<T,3> abs(const Tvec<T,3> &v) {return Tvec<T,3>((T)abs(v.x),(T)abs(v.y),(T)abs(v.z));}
template <class T> INLINE const Tvec<T,4> abs(const Tvec<T,4> &v) {return Tvec<T,4>((T)abs(v.x),(T)abs(v.y),(T)abs(v.z),(T)abs(v.w));}
template <class T> INLINE const T sign(const T &x) {return x>0 ? T(1) : (x<0 ? T(-1) : 0);}
template <class T, int N> INLINE const Tvec<T,N> floor(const Tvec<T,N> &v)
{
	Tvec<T,N> r;
	for (int i=0; i<N; i++)
		r[i] = floor(v[i]);
	return r;
}
template <class T> INLINE const Trange<T> floor(const Trange<T> &v) {return Trange<T>(floor(v.min), floor(v.max));}
template <class T> INLINE const Taabb <T> floor(const Taabb <T> &v) {return Taabb <T>(floor(v.min), floor(v.max));}
template <class T> INLINE const T fract(const T &x) {return x-floor(x);}
template <class T, int N> INLINE const Tvec<T,N> ceil(const Tvec<T,N> &v)
{
	Tvec<T,N> r;
	for (int i=0; i<N; i++) r[i] = ceil(v[i]);
	return r;
}
#ifndef PYTHON_REMAINDER                                                  // s = 0
#define MOD_CODE return x%y;                                  // 0.4 sec  //
#else                                                                     // for i in range(100_000_000):
//#define MOD_CODE return ((x % y) + y) % y;                  // 1.1 sec  //     s += i % 10
#define MOD_CODE auto r = x % y; if (r < 0) r += y; return r; // 0.7 sec  //
#endif                                                                    // print(s)
INLINE const HMbyte mod(const HMbyte x,const HMbyte y) {MOD_CODE}
INLINE const HMshort mod(const HMshort x,const HMshort y) {MOD_CODE}
INLINE const HMint mod(const HMint x,const HMint y) {MOD_CODE}
INLINE const HMubyte mod(const HMubyte x,const HMubyte y) {MOD_CODE}
INLINE const HMushort mod(const HMushort x,const HMushort y) {MOD_CODE}
INLINE const HMuint mod(const HMuint x,const HMuint y) {MOD_CODE}
INLINE const int64_t mod(const int64_t x,const int64_t y) {MOD_CODE}
INLINE const uint64_t mod(const uint64_t x,const uint64_t y) {MOD_CODE}
INLINE const int64_t mod(const int x,const int64_t y) {MOD_CODE}
INLINE const int64_t mod(const int64_t x,const int y) {MOD_CODE}
#undef MOD_CODE
template <class T,class Ta> INLINE const T mod(const T &x,const Ta &y) {return x - y*floor(x/y);}
template <class T,class Ta> INLINE const T wrap(const T &x,const Ta &maxVal) {return mod(x,maxVal);}
template <class T,class Ta> INLINE const T wrap(const T &x,const Ta &minVal,const Ta &maxVal)
{return mod(x-minVal,maxVal-minVal)+minVal;}
template <class T,class Ta> INLINE const T snap(const T &x,const Ta &gridSize) {return floor(x/gridSize + T(.5)) * gridSize;}

#define minmaxFuncs(funcName,sign)		\
template <class T> INLINE const T funcName(const T x, const T y)	\
{ return x sign y ? x : y; }										\
template <class T, int N> INLINE const T funcName(const Tvec<T,N> &v)\
{																	\
	T res=v[0];														\
	for (int i=1;i<N;i++)											\
		if (v[i] sign res) res=v[i];								\
	return res;														\
}																	\
template <class T, int N> INLINE const Tvec<T,N> funcName(const Tvec<T,N> &x,const Tvec<T,N> &y)\
{																	\
	Tvec<T,N> res;													\
	for (int i=0;i<N;i++)											\
		res[i]=x[i] sign y[i] ? x[i] : y[i];						\
	return res;														\
}																	\
template <class T> INLINE const T funcName(const T &x,const T &y,const T &z) {return funcName(funcName(x,y),z);}	\
template <class T> INLINE const T funcName(const T &x,const T &y,const T &z,const T &w) {return funcName(funcName(x,y),funcName(z,w));}

minmaxFuncs(min,<)
minmaxFuncs(max,>)

#undef minmaxFunc
#undef minmaxFuncs
//template <class T> INLINE const T min(const T &x,const T &y) {return min2(x,y);}
//template <class T> INLINE const T max(const T &x,const T &y) {return max2(x,y);}
/*template <class T> INLINE const Tvec<T,2> funcName##2(const Tvec<T,2> &x,const Tvec<T,2> &y)	\
{return Tvec<T,2>(x[0] sign y[0] ? x[0] : y[0],												\
				 x[1] sign y[1] ? x[1] : y[1]);}											\
template <class T> INLINE const Tvec<T,3> funcName##2(const Tvec<T,3> &x,const Tvec<T,3> &y)	\
{return Tvec<T,3>(x[0] sign y[0] ? x[0] : y[0],												\
				 x[1] sign y[1] ? x[1] : y[1],												\
				 x[2] sign y[2] ? x[2] : y[2]);}											\
template <class T> INLINE const Tvec<T,4> funcName##2(const Tvec<T,4> &x,const Tvec<T,4> &y)	\
{return Tvec<T,4>(x[0] sign y[0] ? x[0] : y[0],												\
				 x[1] sign y[1] ? x[1] : y[1],												\
				 x[2] sign y[2] ? x[2] : y[2],												\
				 x[3] sign y[3] ? x[3] : y[3]);}											\*/
/*template <class T> INLINE const T max2(const T &x,const T &y) {return x>y ? x : y;}
template <class T> INLINE const T max3(const T &x,const T &y,const T &z) {return max2(max2(x,y),z);}
template <class T> INLINE const T max4(const T &x,const T &y,const T &z,const T &w) {return max2(max2(x,y),max2(z,w));}*/
template <class T> INLINE const T clamp(const T &x,const T &minVal,const T &maxVal)
{return x<minVal ? minVal : (x>maxVal ? maxVal : x);}//{return max(minVal, min(x, maxVal));}
template <class T, int N> INLINE const Tvec<T,N> clamp(const Tvec<T,N> &x,const Tvec<T,N> &minVal,const Tvec<T,N> &maxVal)
{
	Tvec<T,N> res;
	for (int i=0;i<N;i++)
		res[i] = clamp(x[i], minVal[i], maxVal[i]);
	return res;
}

template <class T,class Ta> INLINE const T  mix(const T &x,const T &y,const Ta &a) {return x + (y-x)*a;}
template <class T,class Ta> INLINE const T lerp(const T &x,const T &y,const Ta &a) {return mix(x,y,a);}
template <class T,class Ta> INLINE const T cerp(const T &x,const T &y,const Ta &a) {return mix(x,y,(Ta(1)-cos(a*(Ta)PI))/Ta(2));}
template <class T,class Ta> INLINE const T herp3(const T &x,const T &y,const Ta &a) {return mix(x,y,a*a*(Ta(3)-Ta(2)*a));}
template <class T,class Ta> INLINE const T herp5(const T &x,const T &y,const Ta &a) {return mix(x,y,a*a*a*(Ta(10)+a*(Ta(6)*a-Ta(15))));}
template <class T> INLINE const T unlerp(const T &x, const T &y, const T &a) {return (a - x)/(y - x);}//returns t, such as lerp(x,y,t) = a

//template <class Ta,class T> INLINE const T step(const Ta &edge,const T &x) {return x<edge ? T(0) : T(1);}
template <class Ta,class T> INLINE const T smoothstep(const Ta &edge0,const Ta &edge1,const T &x)
{
	T t;
	t=clamp((x-edge0) / (edge1-edge0),T(0),T(1));
	return t*t*(T(3)-T(2)*t);
}

//Geometric Functions
template <class T> INLINE const T dot(const Tvec<T,2> &v0,const Tvec<T,2> &v1) {return v0.x*v1.x+v0.y*v1.y;}
template <class T> INLINE const T dot(const Tvec<T,3> &v0,const Tvec<T,3> &v1) {return v0.x*v1.x+v0.y*v1.y+v0.z*v1.z;}
template <class T> INLINE const T dot(const Tvec<T,4> &v0,const Tvec<T,4> &v1) {return v0.x*v1.x+v0.y*v1.y+v0.z*v1.z+v0.w*v1.w;}

/*template <class T> INLINE const T sqlen(const Tvec<T,2> &v) {return dot(v,v);}
template <class T> INLINE const T sqlen(const Tvec<T,3> &v) {return dot(v,v);}
template <class T> INLINE const T sqlen(const Tvec<T,4> &v) {return dot(v,v);}*/
template <class T> INLINE const T_TYPE sqlen(const T &v) {return dot(v,v);}
template <class T, int N> INLINE const T sqlen(const Tvec<T, N> &v) {static_assert(sizeof(T) >= 4, "Please use ivec instead"); return dot(v,v);}

//template <class T> INLINE const typename T::TYPE length(const T &v) {return sqrt(sqlen(v));}
template <class T> INLINE const T length(const Tvec<T,2> &v) {return sqrt(sqlen(v));}
template <class T> INLINE const T length(const Tvec<T,3> &v) {return sqrt(sqlen(v));}
template <class T> INLINE const T length(const Tvec<T,4> &v) {return sqrt(sqlen(v));}

//template <class T,class Tr> INLINE const Tr distance(const T &p0,const T &p1) {return length(p0-p1);}
template <class T> INLINE const T_TYPE distance(const T &p0,const T &p1) {return length(p0-p1);}

template <class T> INLINE const T cross(const Tvec<T,2> &v0,const Tvec<T,2> &v1)//=dot(perp(v0), v1)
{return v0.x*v1.y-v0.y*v1.x;}

template <class T> INLINE const Tvec<T,3> cross(const Tvec<T,3> &v0,const Tvec<T,3> &v1)
{return Tvec<T,3>(v0.y*v1.z-v0.z*v1.y, v0.z*v1.x-v0.x*v1.z, v0.x*v1.y-v0.y*v1.x);}

template <class T> INLINE const T normalize(const T &v) {return v/length(v);}
/*template <class T> INLINE const Tvec<T,2> normalize(const Tvec<T,2> &v) {return v/length(v);}
template <class T> INLINE const Tvec<T,3> normalize(const Tvec<T,3> &v) {return v/length(v);}
template <class T> INLINE const Tvec<T,4> normalize(const Tvec<T,4> &v) {return v/length(v);}*/

template <class T> INLINE const Tvec<T,2> perp(const Tvec<T,2> &v) {return Tvec<T,2>(-v.y,v.x);}

template <class T> INLINE const Tvec<T,3> perp(const Tvec<T,3> &v)
{
	if (fabs(v.x)<fabs(v.y))
		return Tvec<T,3>(0,-v.z,v.y);//cross(Tvec<T,3>(1,0,0),n);
	else
		return Tvec<T,3>(v.z,0,-v.x);//cross(Tvec<T,3>(0,1,0),n);
}

template <class T> INLINE const Tvec<T,3> slerp(const Tvec<T,3> &v1,const Tvec<T,3> &v2,const T t)
{
	T cos_a=dot(v1,v2)/sqrt(sqlen(v1)*sqlen(v2));
	if (greaterThanEqual(cos_a,T(1))) return lerp(v1,v2,t);
	if (lessThanEqual(cos_a,T(-1))) return rotate(radians(t*180),perp(v1),v1);
	T a=acos(cos_a),b=a*t;
	T cos_b=cos(b),cos_ab=cos(a-b);
	Tvec<T,3> v1_v2=cross(v1,v2);

	T t1=v1_v2.y*v2.z-v1_v2.z*v2.y,
	  t2=v1_v2.x*v2.z-v1_v2.z*v2.x,
	  t3=v1_v2.x*v2.y-v1_v2.y*v2.x;
	T det=v1.x*t1-v1.y*t2+v1.z*t3;

	return Tvec<T,3>(cos_b*t1+cos_ab*(v1.y*v1_v2.z-v1_v2.y*v1.z),
				    -cos_b*t2-cos_ab*(v1.x*v1_v2.z-v1_v2.x*v1.z),
					 cos_b*t3+cos_ab*(v1.x*v1_v2.y-v1_v2.x*v1.y))/det;
}

template <class T> INLINE const T faceforward(const T &N,const T &I,const T &Nref)
{if (dot(Nref,I)<0) return N; else return -N;}

template <class T> INLINE const Tvec<T,2> reflect(const Tvec<T,2> &V,const Tvec<T,2> &N) {return V-2*dot(V,N)*N;}
template <class T> INLINE const Tvec<T,3> reflect(const Tvec<T,3> &V,const Tvec<T,3> &N) {return V-2*dot(V,N)*N;}
template <class T> INLINE const Tvec<T,3> reflect(const Tvec<T,3> &V,const Tvec<T,4> &Plane)
{return V-2*(dot(V,Plane.xyz())+Plane.w)*Plane.xyz();}
template <class T> INLINE const Tvec<T,4> reflect(const Tvec<T,4> &V,const Tvec<T,4> &Plane)
{return Tvec<T,4>(V.xyz()-2*dot(V,Plane)*Plane.xyz(),V.w);}

template <class T> INLINE const T refract(const T &V,const T &N,const T_TYPE eta)
{
	T_TYPE d=dot(N,V),
		   k=1 - eta*eta*(1-d*d);
	if (k<0)
		return T(0);
	else
		return V*eta - N*(eta*d+sqrt(k));
}

#undef T_TYPE

//Matrix Functions
template <class T, int c, int r> INLINE const Tmat<T,c,r> matrixCompMult(const Tmat<T,c,r> &x,const Tmat<T,c,r> &y)
{
	Tmat<T,c,r> res;
	for (int i=0;i<c*r/*sizeof(res.e)/sizeof(T::TYPE)*/;i++)
		res.m[0][i]=x.m[0][i]*y.m[0][i];
	return res;
}

template <class T, int c, int r> INLINE const T fnorm(const Tmat<T,c,r> &m)
{
	T res=0;
	for (int i=0;i<c*r;i++)
		res+=m.m[0][i]*m.m[0][i];
	return sqrt(res);
}

template <class T> INLINE const T det(const Tmat<T,2,2> &m)
{
	return m[0][0]*m[1][1] - m[0][1]*m[1][0];
}

template <class T> INLINE const T det(const Tmat<T,3,3> &m)
{
	T A00 =	m[1][1]*m[2][2] - m[1][2]*m[2][1],
	  A10 = m[0][2]*m[2][1] - m[0][1]*m[2][2],
	  A20 =	m[0][1]*m[1][2] - m[0][2]*m[1][1];

	return m[0][0] * A00 + m[1][0] * A10 + m[2][0] * A20;
}

template <class T> INLINE const T det(const Tmat<T,4,4> &m)
{
	T det0213 = m[0][2]*m[1][3] - m[0][3]*m[1][2],
	  det0223 = m[0][2]*m[2][3] - m[0][3]*m[2][2],
	  det0233 = m[0][2]*m[3][3] - m[0][3]*m[3][2],
	  det1223 = m[1][2]*m[2][3] - m[1][3]*m[2][2],
	  det1233 = m[1][2]*m[3][3] - m[1][3]*m[3][2],
	  det2233 = m[2][2]*m[3][3] - m[2][3]*m[3][2];

	T A00 =			m[1][1] * det2233 - m[2][1] * det1233 + m[3][1] * det1223,
	  A10 =			m[2][1] * det0233 - m[0][1] * det2233 - m[3][1] * det0223,
	  A20 =			m[0][1] * det1233 - m[1][1] * det0233 + m[3][1] * det0213,
	  A30 =			m[1][1] * det0223 - m[0][1] * det1223 - m[2][1] * det0213;

	return m[0][0] * A00 + m[1][0] * A10 + m[2][0] * A20 + m[3][0] * A30;
}

template <class T> INLINE const Tmat<T,2,2> inverse(const Tmat<T,2,2> &m)
{
	return Tmat<T,2,2>(m[1][1], -m[0][1], -m[1][0], m[0][0])
					/ (m[0][0] * m[1][1] - m[0][1]* m[1][0]);
}

template <class T> INLINE const Tmat<T,3,3> inverse(const Tmat<T,3,3> &m)
{
	T A00 =			m[1][1]*m[2][2] - m[1][2]*m[2][1],
	  A10 = 		m[0][2]*m[2][1] - m[0][1]*m[2][2],
	  A20 =			m[0][1]*m[1][2] - m[0][2]*m[1][1];

	return Tmat<T,3,3>(A00,
					   A10,
					   A20,
					   m[1][2]*m[2][0] - m[1][0]*m[2][2],
					   m[0][0]*m[2][2] - m[0][2]*m[2][0],
					   m[0][2]*m[1][0] - m[0][0]*m[1][2],
					   m[1][0]*m[2][1] - m[1][1]*m[2][0],
					   m[0][1]*m[2][0] - m[0][0]*m[2][1],
					   m[0][0]*m[1][1] - m[0][1]*m[1][0])
					/ (m[0][0] * A00 + m[1][0] * A10 + m[2][0] * A20);
}

template <class T> INLINE const Tmat<T,4,4> inverse(const Tmat<T,4,4> &m)
{
	T det0112 = m[0][1]*m[1][2] - m[0][2]*m[1][1],
	  det0113 = m[0][1]*m[1][3] - m[0][3]*m[1][1],
	  det0122 = m[0][1]*m[2][2] - m[0][2]*m[2][1],
	  det0123 = m[0][1]*m[2][3] - m[0][3]*m[2][1],
	  det0132 = m[0][1]*m[3][2] - m[0][2]*m[3][1],
	  det0133 = m[0][1]*m[3][3] - m[0][3]*m[3][1],
	  det0213 = m[0][2]*m[1][3] - m[0][3]*m[1][2],
	  det0223 = m[0][2]*m[2][3] - m[0][3]*m[2][2],
	  det0233 = m[0][2]*m[3][3] - m[0][3]*m[3][2],
	  det1122 = m[1][1]*m[2][2] - m[1][2]*m[2][1],
	  det1123 = m[1][1]*m[2][3] - m[1][3]*m[2][1],
	  det1132 = m[1][1]*m[3][2] - m[1][2]*m[3][1],
	  det1133 = m[1][1]*m[3][3] - m[1][3]*m[3][1],
	  det1223 = m[1][2]*m[2][3] - m[1][3]*m[2][2],
	  det1233 = m[1][2]*m[3][3] - m[1][3]*m[3][2],
	  det2132 = m[2][1]*m[3][2] - m[2][2]*m[3][1],
	  det2133 = m[2][1]*m[3][3] - m[2][3]*m[3][1],
	  det2233 = m[2][2]*m[3][3] - m[2][3]*m[3][2];

	T A00 =			m[1][1] * det2233 - m[2][1] * det1233 + m[3][1] * det1223,
	  A10 =			m[2][1] * det0233 - m[0][1] * det2233 - m[3][1] * det0223,
	  A20 =			m[0][1] * det1233 - m[1][1] * det0233 + m[3][1] * det0213,
	  A30 =			m[1][1] * det0223 - m[0][1] * det1223 - m[2][1] * det0213;

	return Tmat<T,4,4>(A00,
					   A10,
					   A20,
					   A30,
					   m[2][0] * det1233 - m[1][0] * det2233 - m[3][0] * det1223,
					   m[0][0] * det2233 - m[2][0] * det0233 + m[3][0] * det0223,
					   m[1][0] * det0233 - m[0][0] * det1233 - m[3][0] * det0213,
					   m[0][0] * det1223 - m[1][0] * det0223 + m[2][0] * det0213,
					   m[1][0] * det2133 - m[2][0] * det1133 + m[3][0] * det1123,
					   m[2][0] * det0133 - m[0][0] * det2133 - m[3][0] * det0123,
					   m[0][0] * det1133 - m[1][0] * det0133 + m[3][0] * det0113,
					   m[1][0] * det0123 - m[0][0] * det1123 - m[2][0] * det0113,
					   m[2][0] * det1132 - m[1][0] * det2132 - m[3][0] * det1122,
					   m[0][0] * det2132 - m[2][0] * det0132 + m[3][0] * det0122,
					   m[1][0] * det0132 - m[0][0] * det1132 - m[3][0] * det0112,
					   m[0][0] * det1122 - m[1][0] * det0122 + m[2][0] * det0112)
					/ (m[0][0] * A00 + m[1][0] * A10 + m[2][0] * A20 + m[3][0] * A30);
}

template <class T, int cols, int rows> INLINE const Tmat<T,rows,cols> transpose(const Tmat<T,cols,rows> &m)
{
	Tmat<T,rows,cols> r;
	for (int i=0; i<cols; i++)
		for (int j=0; j<rows; j++) r[j][i] = m[i][j];
	return r;
}

template <class T> INLINE const Tmat<T,3,3> orthonormalize(const Tmat<T,3,3> &m)
{
	Tvec<T,3> u=m[0],v=m[1],n=m[2];
	v=cross(n,u);
	u=normalize(u);
	v=normalize(v);
	n=cross(u,v);
	return Tmat<T,3,3>(u,v,n);
}

template <class T> INLINE const Tmat<T,4,4> scalingMat(const Tvec<T,3> &s)
{
	return Tmat<T,4,4>(s.x,   0,   0, 0,
					     0, s.y,   0, 0,
					     0,   0, s.z, 0,
					     0,   0,   0, 1);
}

template <class T> INLINE const Tmat<T,4,4> scalingMat4(const Tvec<T,3> &s) {return scalingMat(s);}

template <class T> INLINE const Tmat<T,3,3> scalingMat3(const Tvec<T,3> &s)
{
	return Tmat<T,3,3>(s.x,   0,   0,
					     0, s.y,   0,
					     0,   0, s.z);
}

template <class T> INLINE const Tmat<T,3,3> scalingMat(const Tvec<T,2> &s)
{
	return Tmat<T,3,3>(s.x,   0, 0,
					     0, s.y, 0,
					     0,   0, 1);
}

template <class T> INLINE const Tmat<T,3,3> scalingMat3(const Tvec<T,2> &s) {return scalingMat(s);}

template <class T> INLINE const Tmat<T,2,2> scalingMat2(const Tvec<T,2> &s)
{
	return Tmat<T,2,2>(s.x,   0,
					     0, s.y);
}

template <class T> INLINE const Tmat<T,4,4> translationMat(const Tvec<T,3> &tr)
{
	return Tmat<T,4,4>(   1,    0,    0, 0,
					      0,    1,    0, 0,
					      0,    0,    1, 0,
					   tr.x, tr.y, tr.z, 1);
}

template <class T> INLINE const Tmat<T,4,4> translationMat4(const Tvec<T,3> &tr) {return translationMat(tr);}

template <class T> INLINE const Tmat<T,3,3> translationMat(const Tvec<T,2> &tr)
{
	return Tmat<T,3,3>(   1,    0, 0,
					      0,    1, 0,
					   tr.x, tr.y, 1);
}

template <class T> INLINE const Tmat<T,3,3> translationMat3(const Tvec<T,2> &tr) {return translationMat(tr);}

template <class T> INLINE const Tmat<T,3,3> crossMat(const Tvec<T,3> &n)
{
	return Tmat<T,3,3>(   0,  n.z, -n.y,
					   -n.z,    0,  n.x,
					    n.y, -n.x,    0);
}

template <class T> INLINE const Tmat<T,3,3> crossMat3(const Tvec<T,3> &n) {return crossMat(n);}

template <class T> INLINE const Tmat<T,4,4> crossMat4(const Tvec<T,3> &n)
{
	return Tmat<T,4,4>(   0,  n.z, -n.y, 0,
					   -n.z,    0,  n.x, 0,
					    n.y, -n.x,    0, 0,
					      0,    0,    0, 1);
}

//Vector Relational Functions
#define ComponentWiseCompareUnary(funcName)							\
																	\
template <class T> INLINE const bvec2 funcName(const Tvec<T,2> &v)	\
{return bvec2(funcName(v.x),funcName(v.y));}						\
																	\
template <class T> INLINE const bvec3 funcName(const Tvec<T,3> &v)	\
{return bvec3(funcName(v.x),funcName(v.y),funcName(v.z));}			\
																	\
template <class T> INLINE const bvec4 funcName(const Tvec<T,4> &v)	\
{return bvec4(funcName(v.x),funcName(v.y),funcName(v.z),funcName(v.w));}

ComponentWiseCompareUnary(isZero)
ComponentWiseCompareUnary(notZero)

#undef ComponentWiseCompareUnary

#define ComponentWiseCompareBinary(funcName)										\
																					\
template <class T> INLINE const bvec2 funcName(const Tvec<T,2> &a,const Tvec<T,2> &b)	\
{return bvec2(funcName(a.x,b.x),funcName(a.y,b.y));}								\
																					\
template <class T> INLINE const bvec3 funcName(const Tvec<T,3> &a,const Tvec<T,3> &b)	\
{return bvec3(funcName(a.x,b.x),funcName(a.y,b.y),funcName(a.z,b.z));}				\
																					\
template <class T> INLINE const bvec4 funcName(const Tvec<T,4> &a,const Tvec<T,4> &b)	\
{return bvec4(funcName(a.x,b.x),funcName(a.y,b.y),funcName(a.z,b.z),funcName(a.w,b.w));}

ComponentWiseCompareBinary(lessThan)
ComponentWiseCompareBinary(lessThanEqual)
ComponentWiseCompareBinary(greaterThan)
ComponentWiseCompareBinary(greaterThanEqual)
ComponentWiseCompareBinary(equal)
ComponentWiseCompareBinary(notEqual)

#undef ComponentWiseCompareBinary

INLINE HMbool any(const bvec2 &v) {return v.x||v.y;}
INLINE HMbool any(const bvec3 &v) {return v.x||v.y||v.z;}
INLINE HMbool any(const bvec4 &v) {return v.x||v.y||v.z||v.w;}

INLINE HMbool all(const bvec2 &v) {return v.x&&v.y;}
INLINE HMbool all(const bvec3 &v) {return v.x&&v.y&&v.z;}
INLINE HMbool all(const bvec4 &v) {return v.x&&v.y&&v.z&&v.w;}

#ifdef __GNUC__
INLINE const bvec2 operator not(const bvec2 &v) {return bvec2(!v.x,!v.y);}
INLINE const bvec3 operator not(const bvec3 &v) {return bvec3(!v.x,!v.y,!v.z);}
INLINE const bvec4 operator not(const bvec4 &v) {return bvec4(!v.x,!v.y,!v.z,!v.w);}
#else
INLINE const bvec2 not(const bvec2 &v) {return bvec2(!v.x,!v.y);}
INLINE const bvec3 not(const bvec3 &v) {return bvec3(!v.x,!v.y,!v.z);}
INLINE const bvec4 not(const bvec4 &v) {return bvec4(!v.x,!v.y,!v.z,!v.w);}
#endif //__GNUC__

//Noise Functions

static INLINE const HMuint blockMul(const HMuint x,const HMuint y)
{
	return x*y
		  +(x>>16)*(y>>16)
		  +(((x>>(16+1))*((y&0xFFFF)>>1))>>(16-2))
		  +((((x&0xFFFF)>>1)*(y>>(16+1)))>>(16-2));
}

const HMuint noiseNum[4][3]={{3705202873U,2747025859U,0U},
							 {3025910477U,3546377269U,2394005989U},
							 {2707923697U,2926984589U,4102909219U},
							 {2561027561U,3518370727U,3466353563U}};

const HMuint noiseK[4][2][4]={3056977171U,3269783761U,2658032953U,3344787169U,
							  4037351789U,2609616677U,2464778807U,2766689203U,
							  3834873673U,2376325447U,2320361767U,3156823877U,
							  3363093571U,3333579463U,2308363033U,3134625413U,
							  2496754387U,2529548431U,2627810863U,2537233241U,
							  3447375511U,2269853797U,3091444841U,2335077293U,
							  3804676669U,3901832071U,3300504203U,4050898273U,
							  3048137039U,2649152113U,2778566341U,2984586859U};

template <class Tres,class baseT,int sizeofT,class T> INLINE const Tres noise_(const T &v,const baseT (*ferp)(const baseT &x,const baseT &y,const baseT &a))
{
	unsigned int i,j,k,n;
	HMuint iv[sizeofT];
	baseT a[sizeofT];
	for (i=0;i<sizeofT;i++)
	{
		iv[i]=(HMuint)floor(((baseT*)(&v))[i]);
		a[i]=fract(((baseT*)(&v))[i]);
	}

	static bool cacheValid=false;
	static HMuint civ[sizeofT];
	static baseT cf[sizeof(Tres)/sizeof(baseT)][1<<sizeofT];

	bool takeFromCache=true;
	if (cacheValid)
	{
		for (i=0;i<sizeofT;i++)
			if (iv[i]!=civ[i])
			{
				for (;i<sizeofT;i++)
					civ[i]=iv[i];

				takeFromCache=false;
				break;
			}
	}
	else
	{
		for (i=0;i<sizeofT;i++)
			civ[i]=iv[i];

		cacheValid=true;
		takeFromCache=false;
	}

	Tres r(0);
	for (i=0;i<sizeof(Tres)/sizeof(baseT);i++)
	{
		baseT f[1<<sizeofT];

		if (takeFromCache)
		{
			for (j=0;j<(1<<sizeofT);j++)
				f[j]=cf[i][j];
		}
		else
		{
			HMuint ivC[sizeofT][2][2];//[][min/max][+/^]
			for (j=0;j<sizeofT;j++)
				for (k=0;k<2;k++)
				{
					HMuint K=noiseK[i][k][j];
					ivC[j][0][k]=blockMul(iv[j],K);
					ivC[j][1][k]=blockMul(iv[j]+1,K);
				}

			for (j=0;j<(1<<sizeofT);j++)
			{
				HMuint t=noiseNum[i][0];
				for (k=0;k<sizeofT;k++)
					t^=ivC[k][(j>>k)&1][1];

				t=blockMul(t,noiseNum[i][1])+noiseNum[i][2];

				for (k=0;k<sizeofT;k++)
					t+=ivC[k][(j>>k)&1][0];

				cf[i][j]=f[j]=t*(baseT(2)/baseT(hm_limits<HMuint>::max()))-baseT(1);
			}
		}

		for (n=1<<(sizeofT-1),k=sizeofT-1;n;n>>=1,k--)
			for (j=0;j<n;j++)
				f[j]=ferp(f[j],f[j+n],a[k]);

		((baseT*)(&r))[i]=f[0];
	}

	return r;
/*	int i,j;
	HMuint t[sizeofT];
	for (i=0;i<sizeofT;i++)
		t[i]=HMuint(fract(((baseT*)(&v))[i])*baseT(hm_limits<HMuint>::max()));

	Tres r;
	for (i=0;i<dimension(Tres());i++)
	{
		HMuint a=noiseNum[i][0];
		for (j=0;j<sizeofT;j++)
			a^=blockMul(t[j],noiseK[i][1][j]);

		HMuint ir=blockMul(a,noiseNum[i][1])+noiseNum[i][2];

		for (j=0;j<sizeofT;j++)
			ir+=blockMul(t[j],noiseK[i][0][j]);

		((baseT*)(&r))[i]=baseT(ir);
	}

	return r*baseT(2)/baseT(hm_limits<HMuint>::max())-baseT(1);*/
}

#if _MSC_VER > 1000
#pragma warning(push)
#pragma warning(disable:4701)
#endif // _MSC_VER > 1000

template <class Tres,class T> INLINE const Tres inoise_(const T &v)//Integer version
{
	unsigned int i,j;

	Tres r(0);
	for (i=0;i<sizeof(Tres)/sizeof(HMint);i++)
	{
		HMuint a=noiseNum[i][0];
		for (j=0;j<sizeof(T)/sizeof(HMint);j++)
			a^=blockMul(((HMuint*)(&v))[j],noiseK[i][1][j]);

		a=blockMul(a,noiseNum[i][1])+noiseNum[i][2];

		for (j=0;j<sizeof(T)/sizeof(HMint);j++)
			a+=blockMul(((HMuint*)(&v))[j],noiseK[i][0][j]);

		((HMuint*)(&r))[i]=a;
	}

	return r;
}

#if _MSC_VER > 1000
#pragma warning(pop)
#endif // _MSC_VER > 1000

#define noiseFuncs11(T)																	\
INLINE const T noise1(const T &v,const T (*ferp)(const T &,const T &,const T &)=herp5)	\
{return noise_<T,T,1>(v,ferp);}

#define noiseFuncs1N(N,T)																		\
INLINE const T noise1(const Tvec<T,N> &v,const T (*ferp)(const T &,const T &,const T &)=herp5)	\
{return noise_<T,T,N>(v,ferp);}

#define noiseFuncsN1(N,T)																			\
INLINE const Tvec<T,N> noise##N(const T &v,const T (*ferp)(const T &,const T &,const T &)=herp5)	\
{return noise_<Tvec<T,N>,T,1>(v,ferp);}

#define noiseFuncsMN(M,N,T)																					\
INLINE const Tvec<T,M> noise##M(const Tvec<T,N> &v,const T (*ferp)(const T &,const T &,const T &)=herp5)	\
{return noise_<Tvec<T,M>,T,N>(v,ferp);}

#define noiseFuncs(T)	\
noiseFuncs11(T)			\
noiseFuncs1N(2,T)		\
noiseFuncs1N(3,T)		\
noiseFuncs1N(4,T)		\
noiseFuncsN1(2,T)		\
noiseFuncsN1(3,T)		\
noiseFuncsN1(4,T)		\
noiseFuncsMN(2,2,T)		\
noiseFuncsMN(2,3,T)		\
noiseFuncsMN(2,4,T)		\
noiseFuncsMN(3,2,T)		\
noiseFuncsMN(3,3,T)		\
noiseFuncsMN(3,4,T)		\
noiseFuncsMN(4,2,T)		\
noiseFuncsMN(4,3,T)		\
noiseFuncsMN(4,4,T)

noiseFuncs(HMfloat)
noiseFuncs(HMdouble)
noiseFuncs(HMlong_double)

#undef noiseFuncs11
#undef noiseFuncs1N
#undef noiseFuncsN1
#undef noiseFuncsMN

#define noiseFuncs11(T)				\
INLINE const T noise1(const T &v)	\
{return inoise_<T>(v);}

#define noiseFuncs1N(N,T)					\
INLINE const T noise1(const Tvec<T,N> &v)	\
{return inoise_<T>(v);}

#define noiseFuncsN1(N,T)					\
INLINE const Tvec<T,N> noise##N(const T &v) \
{return inoise_<Tvec<T,N> >(v);}

#define noiseFuncsMN(M,N,T)							\
INLINE const Tvec<T,M> noise##M(const Tvec<T,N> &v)	\
{return inoise_<Tvec<T,M> >(v);}

noiseFuncs(HMint)

#undef noiseFuncs11
#undef noiseFuncs1N
#undef noiseFuncsN1
#undef noiseFuncsMN
#undef noiseFuncs

//Pseudo-plane Functions
template <class T> INLINE const Tvec<T,3> planeNormal(const Tvec<T,3> &p0,const Tvec<T,3> &p1,const Tvec<T,3> &p2)
{return cross(p1-p0,p2-p0);}

template <class T> INLINE const Tvec<T,4> plane(const Tvec<T,3> &p0,const Tvec<T,3> &p1,const Tvec<T,3> &p2)
{
	Tvec<T,3> n=planeNormal(p0,p1,p2);
	return Tvec<T,4>(n,-dot(n,p0));
}

template <class T> INLINE const Tvec<T,4> plane(const Tvec<T,3> &n,const Tvec<T,3> &p)
{return Tvec<T,4>(n,-dot(n,p));}

template <class T> INLINE const Tvec<T,4> unitPlane(const Tvec<T,3> &p0,const Tvec<T,3> &p1,const Tvec<T,3> &p2)
{return plane(normalize(planeNormal(p0,p1,p2)),p0);}

template <class T> INLINE const Tvec<T,4> unitPlane(const Tvec<T,3> &n,const Tvec<T,3> &p)
{return plane(normalize(n),p);}

template <class T> INLINE const Tvec<T,4> normalizePlane(const Tvec<T,4> &Plane)
{return Plane/length(Plane.xyz());}

template <class T> INLINE const Tvec<T,4> translatePlane(const Tvec<T,4> &Plane,const Tvec<T,3> &tr)
{return Tvec<T,4>(Plane.xyz(),Plane.w-dot(Plane.xyz(),tr));}

template <class T> INLINE const Tvec<T,3> planePoint(const Tvec<T,4> &Plane)
{return Plane.xyz()*(-Plane.w/sqlen(Plane.xyz()));}

template <class T> INLINE const Tvec<T,3> unitPlanePoint(const Tvec<T,4> &Plane)
{return Plane.xyz()*(-Plane.w);}

template <class T> INLINE const Tvec<T,4> reflectPlane(const Tvec<T,4> &Plane,const Tvec<T,4> &R)
{return plane(reflect(Plane.xyz(),R.xyz()),reflect(planePoint(Plane),R));}

/*template <class T> INLINE const Tvec<T,4> modifyPlaneNormal(const Tvec<T,4> &Plane,const Tvec<T,3> &newN)
{return Tvec<T,4>(newN,dot(newN,Plane.xyz())*Plane.w/sqlen(Plane.xyz()));}

template <class T> INLINE const Tvec<T,4> modifyUnitPlaneNormal(const Tvec<T,4> &Plane,const Tvec<T,3> &newN)
{return Tvec<T,4>(newN,dot(newN,Plane.xyz())*Plane.w);}*/

template <class T> INLINE const T planeDist(const Tvec<T,4> &Plane,const Tvec<T,3> &point)
{return (dot(Plane.xyz(),point)+Plane.w)/length(Plane.xyz());}

template <class T> INLINE const T planeDist(const Tvec<T,4> &Plane,const Tvec<T,4> &point)
{return dot(Plane,point)/(length(Plane.xyz())*point.w);}

template <class T> INLINE const T unitPlaneDist(const Tvec<T,4> &Plane,const Tvec<T,3> &point)
{return dot(Plane.xyz(),point)+Plane.w;}

template <class T> INLINE const T unitPlaneDist(const Tvec<T,4> &Plane,const Tvec<T,4> &point)
{return dot(Plane,point)/point.w;}

template <class T> INLINE HMbool onPlane(const Tvec<T,4> &Plane,const Tvec<T,3> &point)
{return isZero(dot(Plane.xyz(),point)+Plane.w);}

template <class T> INLINE HMbool outOfPlane(const Tvec<T,4> &Plane,const Tvec<T,3> &point)
{return notZero(dot(Plane.xyz(),point)+Plane.w);}

template <class T> INLINE HMbool abovePlane(const Tvec<T,4> &Plane,const Tvec<T,3> &point)
{return greaterThan(dot(Plane.xyz(),point),-Plane.w);}

template <class T> INLINE HMbool onPlaneOrAbove(const Tvec<T,4> &Plane,const Tvec<T,3> &point)
{return greaterThanEqual(dot(Plane.xyz(),point),-Plane.w);}

template <class T> INLINE HMbool belowPlane(const Tvec<T,4> &Plane,const Tvec<T,3> &point)
{return lessThan(dot(Plane.xyz(),point),-Plane.w);}

template <class T> INLINE HMbool onPlaneOrBelow(const Tvec<T,4> &Plane,const Tvec<T,3> &point)
{return lessThanEqual(dot(Plane.xyz(),point),-Plane.w);}

template <class T>
INLINE HMbool rayPlaneIntn(T &t,const Tvec<T,3> &rayStart,const Tvec<T,3> &rayDirn,const Tvec<T,4> &Plane)
{
	T d=dot(Plane.xyz(),rayDirn);
	if (notZero(d))
	{
		t=-unitPlaneDist(Plane,rayStart)/d;
		return HMtrue;
	}
	return HMfalse;
}

template <class T>
INLINE HMbool rayPlaneIntn(Tvec<T,3> &intn,const Tvec<T,3> &rayStart,const Tvec<T,3> &rayDirn,const Tvec<T,4> &Plane,const T min=0)
{
	T t;
	if (rayPlaneIntn(t,rayStart,rayDirn,Plane))
		if (t>=min)
		{
			intn=rayStart+rayDirn*t;
			return HMtrue;
		}

	return HMfalse;
}

template <class T>
INLINE HMbool rayPlaneIntn(Tvec<T,3> &intn,const Tvec<T,3> &rayStart,const Tvec<T,3> &rayDirn,const Tvec<T,4> &Plane,const T min,const T max)
{
	T t;
	if (rayPlaneIntn(t,rayStart,rayDirn,Plane))
		if (t>=min && t<=max)
		{
			intn=rayStart+rayDirn*t;
			return HMtrue;
		}

	return HMfalse;
}

template <class T>
INLINE HMbool segPlaneIntn(T &t,const Tvec<T,3> &segStart,const Tvec<T,3> &segEnd,const Tvec<T,4> &Plane)
{
	return rayPlaneIntn(t,segStart,segEnd-segStart,Plane);
}

template <class T>
INLINE HMbool segPlaneIntn(Tvec<T,3> &intn,const Tvec<T,3> &segStart,const Tvec<T,3> &segEnd,const Tvec<T,4> &Plane,const T min=0)
{
	return rayPlaneIntn(intn,segStart,segEnd-segStart,Plane,min);
}

template <class T>
INLINE HMbool segPlaneIntn(Tvec<T,3> &intn,const Tvec<T,3> &segStart,const Tvec<T,3> &segEnd,const Tvec<T,4> &Plane,const T min,const T max)
{
	return rayPlaneIntn(intn,segStart,segEnd-segStart,Plane,min,max);
}

//LookAt Functions

template <class T> INLINE const Tmat<T,4,4> lookAtDirnMat(const Tvec<T,3> &eye,const Tvec<T,3> &dirn,const Tvec<T,3> &up)
{
	Tvec<T,3> n=-normalize(dirn);
	Tvec<T,3> u=cross(up,n);
	if (u==Tvec<T,3>(0))
	{
#define calcU														\
		if (fabs(n.x)<T(.7))										\
			u=Tvec<T,3>(0,-n.z,n.y);/*cross(Tvec<T,3>(1,0,0),n);*/	\
		else														\
			u=Tvec<T,3>(n.z,0,-n.x);/*cross(Tvec<T,3>(0,1,0),n);*/

		calcU
	}
	u=normalize(u);
	Tvec<T,3> v=cross(n,u);
	return Tmat<T,4,4>(        u.x,         v.x,         n.x, 0,
					           u.y,         v.y,         n.y, 0,
					           u.z,         v.z,         n.z, 0,
					   -dot(u,eye), -dot(v,eye), -dot(n,eye), 1);
}

template <class T> INLINE const Tmat<T,4,4> lookAtMat(const Tvec<T,3> &eye,const Tvec<T,3> &center,const Tvec<T,3> &up)
{
	return lookAtDirnMat(eye,center-eye,up);
}

template <class T> INLINE const Tmat<T,4,4> lookAtDirnMat(const Tvec<T,3> &eye,const Tvec<T,3> &dirn)
{
	Tvec<T,3> n=-normalize(dirn), u;
	calcU
	u=normalize(u);
	Tvec<T,3> v=cross(n,u);
	return Tmat<T,4,4>(        u.x,         v.x,         n.x, 0,
					           u.y,         v.y,         n.y, 0,
					           u.z,         v.z,         n.z, 0,
					   -dot(u,eye), -dot(v,eye), -dot(n,eye), 1);
}

template <class T> INLINE const Tmat<T,4,4> lookAtMat(const Tvec<T,3> &eye,const Tvec<T,3> &center)
{
	return lookAtDirnMat(eye,center-eye);
}

template <class T> INLINE const Tmat<T,4,4> lookAtDirnMat(const Tvec<T,3> &dirn)
{
	Tvec<T,3> n=-normalize(dirn), u;
	calcU
#undef calcU
	u=normalize(u);
	Tvec<T,3> v=cross(n,u);
	return Tmat<T,4,4>(u.x, v.x, n.x, 0,
					   u.y, v.y, n.y, 0,
					   u.z, v.z, n.z, 0,
					     0,   0,   0, 1);
}

//Projective Functions

template <class T> INLINE const Tmat<T,4,4> orthoMatGL(T left, T right, T bottom, T top, T znear = T(-1), T zfar = T(1))
{
	return Tmat<T,4,4>(T(2)/(right-left), 0, 0, 0,
					   0, T(2)/(top-bottom), 0, 0,
					   0, 0, T(2)/(znear-zfar), 0,
					   (right+left)/(left-right), (top+bottom)/(bottom-top), (zfar+znear)/(znear-zfar), 1);
}

template <class T> INLINE const Tmat<T,4,4> orthoMatD3D(T left, T right, T bottom, T top, T znear = T(-1), T zfar = T(1))
{
	return Tmat<T,4,4>(T(2)/(right-left), 0, 0, 0,
					   0, T(2)/(top-bottom), 0, 0,
					   0, 0, T(1)/(znear-zfar), 0,
					   (right+left)/(left-right), (top+bottom)/(bottom-top), znear/(znear-zfar), 1);
}

template <class T> INLINE const Tmat<T,4,4> frustumMatGL(T left, T right, T bottom, T top, T znear, T zfar)
{
	return Tmat<T,4,4>(T(2)*znear/(right-left), 0, 0, 0,
					   0, T(2)*znear/(top-bottom), 0, 0,
					   (right+left)/(right-left), (top+bottom)/(top-bottom), (zfar+znear)/(znear-zfar), -1,
					   0, 0, T(2)*zfar*znear/(znear-zfar), 0);
}

template <class T> INLINE const Tmat<T,4,4> frustumMatD3D(T left, T right, T bottom, T top, T znear, T zfar)
{
	return Tmat<T,4,4>(T(2)*znear/(right-left), 0, 0, 0,
					   0, T(2)*znear/(top-bottom), 0, 0,
					   (right+left)/(right-left), (top+bottom)/(top-bottom), zfar/(znear-zfar), -1,
					   0, 0, zfar*znear/(znear-zfar), 0);
}

template <class T> INLINE const Tmat<T,4,4> perspectiveMatGL(T fovy, T aspect, T znear, T zfar)
{
	T t = tan(fovy/T(2));
	return Tmat<T,4,4>(T(1)/(t*aspect), 0, 0, 0,
					   0, T(1)/t, 0, 0,
					   0, 0, (zfar+znear)/(znear-zfar), -1,
					   0, 0, T(2)*zfar*znear/(znear-zfar), 0);
}

template <class T> INLINE const Tmat<T,4,4> perspectiveMatD3D(T fovy, T aspect, T znear, T zfar)
{
	T t = tan(fovy/T(2));
	return Tmat<T,4,4>(T(1)/(t*aspect), 0, 0, 0,
					   0, T(1)/t, 0, 0,
					   0, 0, zfar/(znear-zfar), -1,
					   0, 0, zfar*znear/(znear-zfar), 0);
}

//Auxiliary Functions

template <class T> INLINE const Tvec<T,3> rotateX(const T angle,const Tvec<T,3> &vec)
{
	T sn=sin(angle),cs=cos(angle);
	return Tvec<T,3>(vec.x,
					 vec.y*cs-vec.z*sn,
					 vec.y*sn+vec.z*cs);
}

template <class T> INLINE const Tmat<T,3,3> rotationXMat(const T angle)
{
	T sn=sin(angle),cs=cos(angle);
	return Tmat<T,3,3>(1,   0,  0,
					   0,  cs, sn,
					   0, -sn, cs);
}

template <class T> INLINE const Tmat<T,3,3> rotationXMat3(const T angle) {return rotationXMat(angle);}

template <class T> INLINE const Tmat<T,4,4> rotationXMat4(const T angle) {return Tmat<T,4,4>(rotationXMat(angle));}

template <class T> INLINE const Tvec<T,3> rotateY(const T angle,const Tvec<T,3> &vec)
{
	T sn=sin(angle),cs=cos(angle);
	return Tvec<T,3>(vec.x*cs+vec.z*sn,
					 vec.y,
					 vec.z*cs-vec.x*sn);
}

template <class T> INLINE const Tmat<T,3,3> rotationYMat(const T angle)
{
	T sn=sin(angle),cs=cos(angle);
	return Tmat<T,3,3>(cs, 0, -sn,
					    0, 1,   0,
					   sn, 0,  cs);
}

template <class T> INLINE const Tmat<T,3,3> rotationYMat3(const T angle) {return rotationYMat(angle);}

template <class T> INLINE const Tmat<T,4,4> rotationYMat4(const T angle) {return Tmat<T,4,4>(rotationYMat(angle));}

template <class T> INLINE const Tvec<T,2> rotate(const T angle,const Tvec<T,2> &vec)
{
	T sn=sin(angle),cs=cos(angle);
	return Tvec<T,2>(vec.x*cs-vec.y*sn,
					 vec.x*sn+vec.y*cs);
}

template <class T> INLINE const Tvec<T,3> rotateZ(const T angle,const Tvec<T,3> &vec)
{
	return Tvec<T,3>(rotate(angle, vec.xy()), vec.z);
}

template <class T> INLINE const Tmat<T,3,3> rotationZMat(const T angle)
{
	T sn=sin(angle),cs=cos(angle);
	return Tmat<T,3,3>( cs, sn, 0,
					   -sn, cs, 0,
					     0,  0, 1);
}

template <class T> INLINE const Tmat<T,3,3> rotationZMat3(const T angle) {return rotationZMat(angle);}

template <class T> INLINE const Tmat<T,4,4> rotationZMat4(const T angle) {return Tmat<T,4,4>(rotationZMat(angle));}

template <class T> INLINE const Tvec<T,3> rotate(const T angle,const Tvec<T,3> &axis,const Tvec<T,3> &vec)
{
	if (notZero(angle))
	{
		Tvec<T,3> ax=normalize(axis);
		Tvec<T,3> proj=ax*dot(ax,vec);
		return proj+cross(ax,vec)*sin(angle)+(vec-proj)*cos(angle);
	} else return vec;
}

template <class T> INLINE const Tmat<T,3,3> rotationMat(const T angle,const Tvec<T,3> &axis)
{
	if (notZero(angle))
	{
		Tvec<T,3> n=normalize(axis);
		T sn=sin(angle),cs=cos(angle),_1_cs=1-cs,
		  nxny1_cs=n.x*n.y*_1_cs,nxnz1_cs=n.x*n.z*_1_cs,nynz1_cs=n.y*n.z*_1_cs;
		Tvec<T,3> nsn=n*sn;
		return Tmat<T,3,3>(cs+n.x*n.x*_1_cs,   nxny1_cs+nsn.z,   nxnz1_cs-nsn.y,
						     nxny1_cs-nsn.z, cs+n.y*n.y*_1_cs,   nynz1_cs+nsn.x,
						     nxnz1_cs+nsn.y,   nynz1_cs-nsn.x, cs+n.z*n.z*_1_cs);
	} else return Tmat<T,3,3>(1);
}

template <class T> INLINE const Tmat<T,3,3> rotationMat3(const T angle,const Tvec<T,3> &axis)
{return rotationMat(angle,axis);}

template <class T> INLINE const Tmat<T,4,4> rotationMat4(const T angle,const Tvec<T,3> &axis)
{return Tmat<T,4,4>(rotationMat(angle,axis));}
/*	if (notZero(angle))
	{
		Tvec<T,3> n=normalize(axis);
		T sn=sin(angle),cs=cos(angle),_1_cs=1-cs,
		  nxny1_cs=n.x*n.y*_1_cs,nxnz1_cs=n.x*n.z*_1_cs,nynz1_cs=n.y*n.z*_1_cs;
		Tvec<T,3> nsn=n*sn;
		return Tmat<T,4,4>(cs+n.x*n.x*_1_cs,   nxny1_cs+nsn.z,   nxnz1_cs-nsn.y, 0,
						  nxny1_cs-nsn.z, cs+n.y*n.y*_1_cs,   nynz1_cs+nsn.x, 0,
						  nxnz1_cs+nsn.y,   nynz1_cs-nsn.x, cs+n.z*n.z*_1_cs, 0,
									   0,				 0,				   0, 1);
	} else return Tmat<T,4,4>(1);
*/

template <class T> INLINE const Tmat<T,4,4> reflectionMat(const Tvec<T,4> &Plane)
{
	return Tmat<T,4,4>(1) - T(2)*Tmat<T,4,4>(Tvec<T,4>(Plane.xyz()*Plane.x,0),
									         Tvec<T,4>(Plane.xyz()*Plane.y,0),
									         Tvec<T,4>(Plane.xyz()*Plane.z,0),
									         Tvec<T,4>(Plane.xyz()*Plane.w,0));
}

//template <class T> INLINE const T project(const T &V,const T &N) {return V-N*dot(V,N);}
template <class T> INLINE const Tvec<T,3> project(const Tvec<T,3> &V,const Tvec<T,4> &Plane)
{return V-Plane.xyz()*(dot(V,Plane.xyz())+Plane.w);}
template <class T> INLINE const Tvec<T,4> project(const Tvec<T,4> &V,const Tvec<T,4> &Plane)
{return Tvec<T,4>(V.xyz()-Plane.xyz()*dot(V,Plane),V.w);}

template <class T> INLINE const Tmat<T,4,4> projectionMat(const Tvec<T,4> &Plane)
{
	return Tmat<T,4,4>(1) - Tmat<T,4,4>(Tvec<T,4>(Plane.xyz()*Plane.x,0),
								        Tvec<T,4>(Plane.xyz()*Plane.y,0),
								        Tvec<T,4>(Plane.xyz()*Plane.z,0),
								        Tvec<T,4>(Plane.xyz()*Plane.w,0));
}

//Quaternion Functions

template <class T> INLINE const Tquat<T> conjugate(const Tquat<T> &q) {return Tquat<T>(-q.x,-q.y,-q.z,q.w);}

template <class T> INLINE const T dot(const Tquat<T> &q0,const Tquat<T> &q1) {return q0.x*q1.x+q0.y*q1.y+q0.z*q1.z+q0.w*q1.w;}
template <class T> INLINE const T length(const Tquat<T> &q) {return sqrt(sqlen(q));}

template <class T> INLINE const Tquat<T> inverse(const Tquat<T> &q) {CHECK_UNIT_QUAT(q); return conjugate(q);}

template <class T> INLINE const Tquat<T> shortestArcQuat(const Tvec<T,3> &from,const Tvec<T,3> &to)
{
	Tquat<T> q;
	q.xyz()=cross(from,to);
	q.w=dot(from,to);
	T len=length(q);
	q.w+=len;//Reducing rotating angle of quaternion to a half
	if (lessThanEqual(q.w,T(0))) return Tquat<T>(perp(from));
	return q/sqrt(2*len*q.w);
}

template <class T> INLINE const Tquat<T> axisAngleToQuat(const Tvec<T,3> &v, const T alpha)
{
	T len=length(v);
	if (notZero(len))
		 return Tquat<T>(v*(sin(alpha/T(2))/len), cos(alpha/T(2)));
	else return Tquat<T>(1);
}

template <class T> INLINE const T quatToAxisAngle(Tvec<T,3> &axis, const Tquat<T> &q)
{
	CHECK_UNIT_QUAT(q);
	if (lessThan(T(fabs(q.w)),T(1)))
	{
		T len=sqrt(1-q.w*q.w);
		axis=q.xyz()/len;
		return 2*(T)atan2(len,q.w);
	}
	else
	{
		axis=Tvec<T,3>(0);
		return 0;
	}
}

template <class T> INLINE const Tvec<T,3> quatToAxisAngle(const Tquat<T> &q)
{
	CHECK_UNIT_QUAT(q);
	if (lessThan(T(fabs(q.w)),T(1)))
	{
		T len=sqrt(1-q.w*q.w);
		return q.xyz()*(2*(T)atan2(len,q.w)/len);
	}
	else
		return Tvec<T,3>(0);
}

template <class T> INLINE const T quatAngle(const Tquat<T> &q)
{
	CHECK_UNIT_QUAT(q);
	return T(2)*(T)acos(q.w);
}

template <class T> INLINE const Tquat<T> nlerp(const Tquat<T> &from, const Tquat<T> &to, const T t)//normalized lerp
{
	return normalize(dot(from,to) >= T(0) ? mix(from,to,t) : (from - (to+from)*t));
}

template <class T> INLINE const Tquat<T> slerp(const Tquat<T> &from, const Tquat<T> &to, const T t)
{
	T fromLen=length(from),toLen=length(to);
	T d=dot(from,to)/(fromLen*toLen),k1,k2;

	if (d<0) {toLen=-toLen; d=-d;}

	if (greaterThanEqual(d,T(1)))
	{
		k1=(1-t)/fromLen;
		k2=   t /  toLen;
	}
	else
	{
		T a=acos(d),sn=sqrt(1-d*d);
		k1=sin((1-t)*a)/(sn*fromLen);
		k2=sin(   t *a)/(sn*  toLen);
	}

	return from*k1+to*k2;
}

template <class T> INLINE const Tquat<T> squad(const Tquat<T> &p,
											   const Tquat<T> &a,
											   const Tquat<T> &b,
											   const Tquat<T> &q,
											   const T t)
{return slerp(slerp(p,q,t), slerp(a,b,t), 2*t*(1-t));}

template <class T> INLINE const Tvec<T,3> unitQuatTransformVector(const Tquat<T> &q, const Tvec<T,3> &v)
{
	return cross(q.xyz(), cross(q.xyz(), v) + q.w*v) * T(2) + v;
}

template <class T> INLINE void inverse(Tquat<T> &qr, Tvec<T,3> &tr, const Tquat<T> &q, const Tvec<T,3> &t)
{
	qr = inverse(q);
	tr = -(qr * t);
}

template <class T> INLINE void quatTransMul(Tquat<T> &qr, Tvec<T,3> &tr, const Tquat<T> &q1, const Tvec<T,3> &t1, const Tquat<T> &q2, const Tvec<T,3> &t2)
{
	tr = unitQuatTransformVector(q1, t2) + t1;
	qr = q1 * q2;
}

//Dual Quaternion Functions

template <class T> INLINE void conjugate(Tquat<T> res[2], const Tquat<T> dq[2])
{
	res[0] = conjugate(dq[0]);
	res[1] = conjugate(dq[1]);
}

template <class T> INLINE const T dot(const Tquat<T> dq0[2],const Tquat<T> dq1[2]) {return dot(dq0[0], dq1[0]) + dot(dq0[1], dq1[1]);}

template <class T> INLINE void fastnormalize(Tquat<T> res[2], const Tquat<T> dq[2])//res and dq can point to the same location
{
	T t = T(1)/length(dq[0]);//actual length of dual quat is not so trivial!
	res[0] = dq[0] * t;
	res[1] = dq[1] * t;
}

template <class T> INLINE void inverse(Tquat<T> res[2], const Tquat<T> dq[2])//res and dq can point to the same location
{
	CHECK_UNIT_QUAT(dq[0] + Tquat<T>(dot(dq[0], dq[1]))/*should be = 0*/);
	conjugate(res, dq);
}

template <class T> INLINE const Tquat<T> quatTransGetDualQuatPart(const Tquat<T> &q, const Tvec<T,3> &t)
{
	return T(0.5)*Tquat<T>(t*q.w + cross(t, q.xyz()), -dot(t,q.xyz()));
}

template <class T> INLINE const Tmat<T,4,4> dualQuatToMat(const Tquat<T> q[2])
{
	Tmat<T,4,4> M(q[0], HMtrue);
	M[3].xyz() = dualQuatGetTrans(q);
	return M;
}

template <class T> INLINE void matToDualQuat(Tquat<T> dq[2], const Tmat<T,4,4> &m)
{
	quatTransToDualQuat(dq, Tquat<T>(m), m[3].xyz());
}

template <class T> INLINE void quatTransToDualQuat(Tquat<T> dq[2], const Tquat<T> &q, const Tvec<T,3> &t)
{
	dq[0] = q;
	dq[1] = quatTransGetDualQuatPart(q, t);
}

template <class T> INLINE const Tvec<T,3> dualQuatToQuatTrans(Tquat<T> &q, const Tquat<T> dq[2])
{
	q = dq[0];
	return dualQuatGetTrans(dq);
}

template <class T> INLINE const Tvec<T,3> dualQuatGetTrans(const Tquat<T> dq[2])
{
	return T(2)*(dq[0].w*dq[1].xyz() - dq[1].w*dq[0].xyz() + cross(dq[0].xyz(), dq[1].xyz()));
}

template <class T> INLINE const Tvec<T,3> dualQuatTransformPoint(const Tquat<T> q[2], const Tvec<T,3> &point)
{
	return (cross(q[0].xyz(), cross(q[0].xyz(), point) + q[0].w*point + q[1].xyz()) + q[0].w*q[1].xyz() - q[1].w*q[0].xyz()) * T(2) + point;
}

template <class T> INLINE const Tvec<T,3> dualQuatTransformVector(const Tquat<T> q[2], const Tvec<T,3> &v)
{
	return unitQuatTransformVector(q[0], v);
}

template <class T> INLINE void dualQuatMul(Tquat<T> res[2], const Tquat<T> dq1[2], const Tquat<T> dq2[2])
{
	res[1] = dq1[0] * dq2[1] + dq1[1] * dq2[0];
	res[0] = dq1[0] * dq2[0];//порядок изменен, чтобы res могло указывать на dq1 или dq2
}

template <class T> INLINE void nlerp(Tquat<T> res[2], const Tquat<T> from[2], const Tquat<T> to[2], const T t)
{
	if (dot(from[0],to[0]) >= T(0))
	{
		res[0] = mix(from[0],to[0],t);
		res[1] = mix(from[1],to[1],t);
	}
	else
	{
		res[0] = from[0] - (to[0]+from[0])*t;//res[0] = mix(from[0],-to[0],t);
		res[1] = from[1] - (to[1]+from[1])*t;//res[1] = mix(from[1],-to[1],t);
	}
	fastnormalize(res, res);
}

#endif //HM_FUNC_H
