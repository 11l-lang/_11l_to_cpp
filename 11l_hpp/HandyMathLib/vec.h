#ifndef HM_VEC_H
#define HM_VEC_H

#include <math.h>
#include "typedef.h"
#include <type_traits> // for std::enable_if_t [in GCC]



#define floatCompareFuncs(T, e)			\
										\
INLINE HMbool isZero(T x, T epsilon=e)	\
{return fabs(x)<epsilon;}				\
										\
INLINE HMbool notZero(T x, T epsilon=e)	\
{return fabs(x)>=epsilon;}				\
										\
INLINE HMbool equal(T x,T y,T epsilon=e)\
{return isZero(x-y,epsilon);}			\
										\
INLINE HMbool notEqual(T x,T y,T epsilon=e)\
{return notZero(x-y,epsilon);}			\
										\
INLINE HMbool lessThan(T x,T y,T epsilon=e)\
{return x<=y-epsilon;}					\
										\
INLINE HMbool lessThanEqual(T x,T y,T epsilon=e)\
{return x<y+epsilon;}					\
										\
INLINE HMbool greaterThan(T x,T y,T epsilon=e)\
{return x>=y+epsilon;}					\
										\
INLINE HMbool greaterThanEqual(T x,T y,T epsilon=e)\
{return x>y-epsilon;}

floatCompareFuncs(HMfloat,1e-3f)
floatCompareFuncs(HMdouble,1e-8)
floatCompareFuncs(HMlong_double,1e-8l)

#undef floatCompareFuncs


#define intCompareFuncs(T)				\
										\
INLINE HMbool isZero(T x)				\
{return x==0;}							\
										\
INLINE HMbool notZero(T x)				\
{return x!=0;}							\
										\
INLINE HMbool equal(T x,T y)			\
{return x==y;}							\
										\
INLINE HMbool notEqual(T x,T y)			\
{return x!=y;}							\
										\
INLINE HMbool lessThan(T x,T y)			\
{return x<y;}							\
										\
INLINE HMbool lessThanEqual(T x,T y)	\
{return x<=y;}							\
										\
INLINE HMbool greaterThan(T x,T y)		\
{return x>y;}							\
										\
INLINE HMbool greaterThanEqual(T x,T y)	\
{return x>=y;}

intCompareFuncs(HMbyte)
intCompareFuncs(HMubyte)
intCompareFuncs(HMint)
intCompareFuncs(HMuint)
intCompareFuncs(HMshort)
intCompareFuncs(HMushort)

#undef intCompareFuncs

INLINE HMbool    equal(HMbool x,HMbool y) {return x==y;}
INLINE HMbool notEqual(HMbool x,HMbool y) {return x!=y;}


//Base Vector Classes (for internal use only)

template <class T, int N> struct Tvec;
template <class T, int N> struct Tbasevec;
template <class T> struct Tbasevec<T,2>
{
	union {T x,r,s;};
	union {T y,g,t;};
	typedef T TYPE;//for external use only
};
template <class T> struct Tbasevec<T,3> : Tbasevec<T,2>
{
	union {T z,b,p;};
	using Tbasevec<T,2>::x;
	using Tbasevec<T,2>::y;
	Tvec<T,2> &xy() {return (Tvec<T,2>&)x;}
	Tvec<T,2> &rg() {return (Tvec<T,2>&)x;}
	Tvec<T,2> &st() {return (Tvec<T,2>&)x;}
	Tvec<T,2> &yz() {return (Tvec<T,2>&)y;}
	Tvec<T,2> &gb() {return (Tvec<T,2>&)y;}
	Tvec<T,2> &tp() {return (Tvec<T,2>&)y;}
	const Tvec<T,2> &xy() const {return (Tvec<T,2>&)x;}
	const Tvec<T,2> &rg() const {return (Tvec<T,2>&)x;}
	const Tvec<T,2> &st() const {return (Tvec<T,2>&)x;}
	const Tvec<T,2> &yz() const {return (Tvec<T,2>&)y;}
	const Tvec<T,2> &gb() const {return (Tvec<T,2>&)y;}
	const Tvec<T,2> &tp() const {return (Tvec<T,2>&)y;}
};
template <class T> struct Tbasevec<T,4> : Tbasevec<T,3>
{
	union {T w,a,q;};
	using Tbasevec<T,3>::x;
	using Tbasevec<T,3>::y;
	using Tbasevec<T,3>::z;
	Tvec<T,2> &zw() {return (Tvec<T,2>&)z;}
	Tvec<T,2> &ba() {return (Tvec<T,2>&)z;}
	Tvec<T,2> &pq() {return (Tvec<T,2>&)z;}
	const Tvec<T,2> &zw() const {return (Tvec<T,2>&)z;}
	const Tvec<T,2> &ba() const {return (Tvec<T,2>&)z;}
	const Tvec<T,2> &pq() const {return (Tvec<T,2>&)z;}
	Tvec<T,3> &xyz() {return (Tvec<T,3>&)x;}
	Tvec<T,3> &rgb() {return (Tvec<T,3>&)x;}
	Tvec<T,3> &stp() {return (Tvec<T,3>&)x;}
	Tvec<T,3> &yzw() {return (Tvec<T,3>&)y;}
	Tvec<T,3> &gba() {return (Tvec<T,3>&)y;}
	Tvec<T,3> &tpq() {return (Tvec<T,3>&)y;}
	const Tvec<T,3> &xyz() const {return (Tvec<T,3>&)x;}
	const Tvec<T,3> &rgb() const {return (Tvec<T,3>&)x;}
	const Tvec<T,3> &stp() const {return (Tvec<T,3>&)x;}
	const Tvec<T,3> &yzw() const {return (Tvec<T,3>&)y;}
	const Tvec<T,3> &gba() const {return (Tvec<T,3>&)y;}
	const Tvec<T,3> &tpq() const {return (Tvec<T,3>&)y;}
};

//Main Vector Class

template <class T, int N> struct Tvec : Tbasevec<T,N>
{
	using Tbasevec<T,N>::x;
	using Tbasevec<T,N>::y;
	INLINE Tvec() {}
	//cast constructor
	template <class TT> INLINE explicit Tvec(const Tvec<TT,N> &v) {for (int i=0;i<N;i++) (&x)[i]=(T)v[i];}
	//copy constructor
	INLINE Tvec(const Tvec &v) {for (int i=0;i<N;i++) (&x)[i]=v[i];}
	//1 arg
	INLINE explicit Tvec(const T a) {for (int i=0;i<N;i++) (&x)[i]=a;}
	template <int n> INLINE explicit Tvec(const Tvec<T,n> &v)
	{
		typedef char staticCheck[n - N];
		for (int i=0;i<N;i++) (&x)[i]=v[i];
	}
	//vec2
	INLINE Tvec(const T x_,const T y_) {x=x_; y=y_; typedef char staticCheck[N==2 ? 1 : 0];}
	//vec3
	INLINE Tvec(const Tvec<T,2> &xy,const T z_)   {x=xy.x; y=xy.y; Tbasevec<T,N>::z=z_;   typedef char staticCheck[N==3 ? 1 : 0];}
	INLINE Tvec(const T x_,const Tvec<T,2> &yz)   {x=x_;   y=yz.x; Tbasevec<T,N>::z=yz.y; typedef char staticCheck[N==3 ? 1 : 0];}
	INLINE Tvec(const T x_,const T y_,const T z_) {x=x_;   y=y_;   Tbasevec<T,N>::z=z_;   typedef char staticCheck[N==3 ? 1 : 0];}
	//vec4
	//2 arg
	INLINE Tvec(const Tvec<T,3> &xyz,const T w_)         {x=xyz.x; y=xyz.y; Tbasevec<T,N>::z=xyz.z; Tbasevec<T,N>::w=w_;}
	INLINE Tvec(const T x_,const Tvec<T,3> &yzw)         {x=x_;    y=yzw.x; Tbasevec<T,N>::z=yzw.y; Tbasevec<T,N>::w=yzw.z;}
	INLINE Tvec(const Tvec<T,2> &xy,const Tvec<T,2> &zw) {x=xy.x;  y=xy.y;  Tbasevec<T,N>::z=zw.x;  Tbasevec<T,N>::w=zw.y;}
	//3 arg
	INLINE Tvec(const Tvec<T,2> &xy,const T z_,const T w_) {x=xy.x; y=xy.y; Tbasevec<T,N>::z=z_;   Tbasevec<T,N>::w=w_;}
	INLINE Tvec(const T x_,const Tvec<T,2> &yz,const T w_) {x=x_;   y=yz.x; Tbasevec<T,N>::z=yz.y; Tbasevec<T,N>::w=w_;}
	INLINE Tvec(const T x_,const T y_,const Tvec<T,2> &zw) {x=x_;   y=y_;   Tbasevec<T,N>::z=zw.x; Tbasevec<T,N>::w=zw.y;}
	//4 arg
	INLINE Tvec(const T x_,const T y_,const T z_,const T w_) {x=x_; y=y_; Tbasevec<T,N>::z=z_; Tbasevec<T,N>::w=w_;}

	//operators
	INLINE T &operator [] (const int i) {return (&x)[i];}
	INLINE const T &operator [] (const int i) const {return (&x)[i];}
	//INLINE operator T* () {return &x;}
	INLINE operator const T* () const {return &x;}

	//vector ~ a
	template <typename Ty, typename = std::enable_if_t<std::is_arithmetic<Ty>::value>> INLINE const Tvec<decltype(T()+Ty()), N> operator+(const Ty a) const {Tvec<decltype(T()+Ty()), N> r; for (int i=0;i<N;i++) r[i]=(&x)[i] + a; return r;}
	template <typename Ty, typename = std::enable_if_t<std::is_arithmetic<Ty>::value>> INLINE const Tvec<decltype(T()-Ty()), N> operator-(const Ty a) const {Tvec<decltype(T()-Ty()), N> r; for (int i=0;i<N;i++) r[i]=(&x)[i] - a; return r;}
	template <typename Ty, typename = std::enable_if_t<std::is_arithmetic<Ty>::value>> INLINE const Tvec<decltype(T()*Ty()), N> operator*(const Ty a) const {Tvec<decltype(T()*Ty()), N> r; for (int i=0;i<N;i++) r[i]=(&x)[i] * a; return r;}
	template <typename Ty, typename = std::enable_if_t<std::is_arithmetic<Ty>::value>> INLINE const Tvec<decltype(T()/Ty()), N> operator/(const Ty a) const {Tvec<decltype(T()/Ty()), N> r; for (int i=0;i<N;i++) r[i]=(&x)[i] / a; return r;}

	//a ~ vector
	template <typename Ty, typename = std::enable_if_t<std::is_arithmetic<Ty>::value>> INLINE friend const Tvec<decltype(Ty()+T()), N> operator+(const Ty a, const Tvec &v) {Tvec<decltype(Ty()+T()), N> r; for (int i=0;i<N;i++) r[i]=a + v[i]; return r;}
	template <typename Ty, typename = std::enable_if_t<std::is_arithmetic<Ty>::value>> INLINE friend const Tvec<decltype(Ty()-T()), N> operator-(const Ty a, const Tvec &v) {Tvec<decltype(Ty()-T()), N> r; for (int i=0;i<N;i++) r[i]=a - v[i]; return r;}
	template <typename Ty, typename = std::enable_if_t<std::is_arithmetic<Ty>::value>> INLINE friend const Tvec<decltype(Ty()*T()), N> operator*(const Ty a, const Tvec &v) {Tvec<decltype(Ty()*T()), N> r; for (int i=0;i<N;i++) r[i]=a * v[i]; return r;}
	template <typename Ty, typename = std::enable_if_t<std::is_arithmetic<Ty>::value>> INLINE friend const Tvec<decltype(Ty()/T()), N> operator/(const Ty a, const Tvec &v) {Tvec<decltype(Ty()/T()), N> r; for (int i=0;i<N;i++) r[i]=a / v[i]; return r;}

	//vector ~ vector
	template <typename Ty> INLINE const Tvec<decltype(T()+Ty()), N> operator+(const Tvec<Ty, N> &v) const {Tvec<decltype(T()+Ty()), N> r; for (int i=0;i<N;i++) r[i]=(&x)[i] + v[i]; return r;}
	template <typename Ty> INLINE const Tvec<decltype(T()-Ty()), N> operator-(const Tvec<Ty, N> &v) const {Tvec<decltype(T()-Ty()), N> r; for (int i=0;i<N;i++) r[i]=(&x)[i] - v[i]; return r;}
	template <typename Ty> INLINE const Tvec<decltype(T()*Ty()), N> operator*(const Tvec<Ty, N> &v) const {Tvec<decltype(T()*Ty()), N> r; for (int i=0;i<N;i++) r[i]=(&x)[i] * v[i]; return r;}
	template <typename Ty> INLINE const Tvec<decltype(T()/Ty()), N> operator/(const Tvec<Ty, N> &v) const {Tvec<decltype(T()/Ty()), N> r; for (int i=0;i<N;i++) r[i]=(&x)[i] / v[i]; return r;}

	//vector ~= a
	INLINE const Tvec &operator+=(const T a) {for (int i=0;i<N;i++) (&x)[i] += a; return *this;}
	INLINE const Tvec &operator-=(const T a) {for (int i=0;i<N;i++) (&x)[i] -= a; return *this;}
	INLINE const Tvec &operator*=(const T a) {for (int i=0;i<N;i++) (&x)[i] *= a; return *this;}
	INLINE const Tvec &operator/=(const T a) {for (int i=0;i<N;i++) (&x)[i] /= a; return *this;}

	//vector ~= vector
	INLINE const Tvec &operator+=(const Tvec &v) {for (int i=0;i<N;i++) (&x)[i] += v[i]; return *this;}
	INLINE const Tvec &operator-=(const Tvec &v) {for (int i=0;i<N;i++) (&x)[i] -= v[i]; return *this;}
	INLINE const Tvec &operator*=(const Tvec &v) {for (int i=0;i<N;i++) (&x)[i] *= v[i]; return *this;}
	INLINE const Tvec &operator/=(const Tvec &v) {for (int i=0;i<N;i++) (&x)[i] /= v[i]; return *this;}
	INLINE const Tvec &operator =(const Tvec &v) {for (int i=0;i<N;i++) (&x)[i]  = v[i]; return *this;}

	//-vector
	INLINE const Tvec operator-() const {Tvec r; for (int i=0;i<N;i++) r[i]=-(&x)[i]; return r;}

	//++vector and --vector
	INLINE const Tvec operator++() {for (int i=0;i<N;i++) ++(&x)[i]; return *this;}
	INLINE const Tvec operator--() {for (int i=0;i<N;i++) --(&x)[i]; return *this;}

	//vector++ and vector--
	INLINE const Tvec operator++(int notused) {Tvec t(*this); for (int i=0;i<N;i++) ++(&x)[i]; return t;}
	INLINE const Tvec operator--(int notused) {Tvec t(*this); for (int i=0;i<N;i++) --(&x)[i]; return t;}
};

template <class T, int N> INLINE HMbool operator==(const Tvec<T, N> &V, const Tvec<T, N> &v) {for (int i=0;i<N;i++) if (  !::equal(V[i],v[i])) return HMfalse; return HMtrue;}
template <class T, int N> INLINE HMbool operator!=(const Tvec<T, N> &V, const Tvec<T, N> &v) {for (int i=0;i<N;i++) if (::notEqual(V[i],v[i])) return HMtrue; return HMfalse;}


// Bool Vector Classes

struct bvec3;
struct bvec4;

struct bvec2 : Tbasevec<HMbool,2>
{
	INLINE bvec2() {}
	INLINE bvec2(const bvec2 &v) {x=v.x; y=v.y;}
	INLINE explicit bvec2(const HMbool a) {x=a; y=a;}
	INLINE explicit bvec2(const bvec3 &v);
	INLINE explicit bvec2(const bvec4 &v);
	INLINE bvec2(const HMbool x_,const HMbool y_) {x=x_; y=y_;}

	INLINE HMbool &operator [] (const int i) {return (&x)[i];}
	INLINE const HMbool operator [] (const int i) const {return (&x)[i];}
	//INLINE operator HMbool* () {return &x;}
	//INLINE operator const HMbool* () const {return &x;}

	INLINE HMbool operator==(const bvec2 &v) const {return x==v.x && y==v.y;}
	INLINE HMbool operator!=(const bvec2 &v) const {return x!=v.x || y!=v.y;}
};


struct bvec3 : Tbasevec<HMbool,3>
{
	INLINE bvec3() {}
	INLINE bvec3(const bvec3 &v) {x=v.x; y=v.y; z=v.z;}
	INLINE explicit bvec3(const HMbool a) {x=a; y=a; z=a;}
	INLINE explicit bvec3(const bvec4 &v);
	INLINE bvec3(const bvec2 &xy,const HMbool z_) {x=xy.x; y=xy.y; z=z_;}
	INLINE bvec3(const HMbool x_,const bvec2 &yz) {x=x_; y=yz.x; z=yz.y;}
	INLINE bvec3(const HMbool x_,const HMbool y_,const HMbool z_) {x=x_; y=y_; z=z_;}

	INLINE HMbool &operator [] (const int i) {return (&x)[i];}
	INLINE const HMbool operator [] (const int i) const {return (&x)[i];}
	//INLINE operator HMbool* () {return &x;}
	//INLINE operator const HMbool* () const {return &x;}

	INLINE HMbool operator==(const bvec3 &v) const {return x==v.x && y==v.y && z==v.z;}
	INLINE HMbool operator!=(const bvec3 &v) const {return x!=v.x || y!=v.y || z!=v.z;}
};

INLINE bvec2::bvec2(const bvec3 &v) {x=v.x; y=v.y;}


struct bvec4 : Tbasevec<HMbool,4>
{
	INLINE bvec4() {}
	INLINE bvec4(const bvec4 &v) {x=v.x; y=v.y; z=v.z; w=v.w;}
	//1 arg
	INLINE explicit bvec4(const HMbool a) {x=a; y=a; z=a; w=a;}
	//2 arg
	INLINE bvec4(const bvec3 &xyz,const HMbool w_) {x=xyz.x; y=xyz.y; z=xyz.z; w=w_;}
	INLINE bvec4(const HMbool x_,const bvec3 &yzw) {x=x_;    y=yzw.x; z=yzw.y; w=yzw.z;}
	INLINE bvec4(const bvec2 &xy,const bvec2 &zw)  {x=xy.x;  y=xy.y;  z=zw.x;  w=zw.y;}
	//3 arg
	INLINE bvec4(const bvec2 &xy,const HMbool z_,const HMbool w_) {x=xy.x; y=xy.y; z=z_;   w=w_;}
	INLINE bvec4(const HMbool x_,const bvec2 &yz,const HMbool w_) {x=x_;   y=yz.x; z=yz.y; w=w_;}
	INLINE bvec4(const HMbool x_,const HMbool y_,const bvec2 &zw) {x=x_;   y=y_;   z=zw.x; w=zw.y;}
	//4 arg
	INLINE bvec4(const HMbool x_,const HMbool y_,const HMbool z_,const HMbool w_) {x=x_;y=y_;z=z_;w=w_;}

	INLINE HMbool &operator [] (const int i) {return (&x)[i];}
	INLINE const HMbool operator [] (const int i) const {return (&x)[i];}
	//INLINE operator HMbool* () {return &x;}
	//INLINE operator const HMbool* () const {return &x;}

	INLINE HMbool operator==(const bvec4 &v) const {return x==v.x && y==v.y && z==v.z && w==v.w;}
	INLINE HMbool operator!=(const bvec4 &v) const {return x!=v.x || y!=v.y || z!=v.z || w!=v.w;}
};

INLINE bvec2::bvec2(const bvec4 &v) {x=v.x; y=v.y;}
INLINE bvec3::bvec3(const bvec4 &v) {x=v.x; y=v.y; z=v.z;}


#define typedef_vectors(dimension)						 \
typedef Tvec<HMfloat      ,dimension>	  vec##dimension;\
typedef Tvec<HMint        ,dimension>	 ivec##dimension;\
typedef Tvec<HMdouble     ,dimension>	 dvec##dimension;\
typedef Tvec<HMlong_double,dimension>	ldvec##dimension;\
typedef Tvec<HMshort      ,dimension>	 svec##dimension;\
typedef Tvec<HMushort     ,dimension>	usvec##dimension;\
typedef Tvec<HMubyte      ,dimension>	ubvec##dimension;\
typedef Tvec<HMbyte       ,dimension>	sbvec##dimension;

typedef_vectors(2)
typedef_vectors(3)
typedef_vectors(4)

#undef typedef_vectors

#endif //HM_VEC_H
