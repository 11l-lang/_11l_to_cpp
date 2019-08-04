#ifndef HM_QUAT_H
#define HM_QUAT_H

#ifdef NDEBUG
#define CHECK_UNIT_QUAT(q) void(0)
#else
#include <assert.h>
#define CHECK_UNIT_QUAT(q) assert(isZero(sqlen(q) - 1.f))
#endif

#include "mat.h"
#include "func.h"



template <class T> class Tquat
{
public:

	T x,y,z,w;

	typedef T TYPE;//for external use only

	INLINE Tquat() {}
	template <class TT> INLINE explicit Tquat(const Tquat<TT> &q):x((T)q.x),y((T)q.y),z((T)q.z),w((T)q.w) {}
	INLINE Tquat(const Tquat &q):x(q.x),y(q.y),z(q.z),w(q.w) {}
	INLINE explicit Tquat(const T w):x(0),y(0),z(0),w(w) {}
	INLINE explicit Tquat(const Tvec<T,3> &v, const T w=0):x(v.x),y(v.y),z(v.z),w(w) {}

	template <int cols, int rows> INLINE explicit Tquat(const Tmat<T,cols,rows> &M, HMbool quatIsUnit=HMtrue)
	{
		typedef char staticCheck[(cols>=3 && rows>=3) ? 1 : 0];

		//At least one component (taken in absolute value) must be greater than or
		//equal to 0.5 in any unit length quaternion. Let's find such component!
		T t;
		if ((t=M[0][0]+M[1][1]+M[2][2]) >= 0)//t=4ww-1, so if t>=0, |w|>=.5
		{
			x=M[1][2]-M[2][1];//=(4w)x
			y=M[2][0]-M[0][2];//=(4w)y
			z=M[0][1]-M[1][0];//=(4w)z
			w=++t;			  //=(4w)w
		}
		else if ((t=M[0][0]-M[1][1]-M[2][2]) >= 0)//t=4xx-1, so if t>=0, |x|>=.5
		{
			x=++t;			  //=(4x)x
			y=M[0][1]+M[1][0];//=(4x)y
			z=M[0][2]+M[2][0];//=(4x)z
			w=M[1][2]-M[2][1];//=(4x)w
		}
		else if ((t=M[1][1]-M[0][0]-M[2][2]) >= 0)//t=4yy-1, so if t>=0, |y|>=.5
		{
			x=M[0][1]+M[1][0];//=(4y)x
			y=++t;			  //=(4y)y
			z=M[1][2]+M[2][1];//=(4y)z
			w=M[2][0]-M[0][2];//=(4y)w
		}
		else
		{
			t=M[2][2]-M[0][0]-M[1][1];//must be >=0
			x=M[0][2]+M[2][0];//=(4z)x
			y=M[1][2]+M[2][1];//=(4z)y
			z=++t;			  //=(4z)z
			w=M[0][1]-M[1][0];//=(4z)w
		}
		if (quatIsUnit) *this/=sqrt(t)*T(2);
/*		Tquat<T> q,q2;
		//Finding quaternion that rotates unit coordinate system (UCS) to (M[0],M[1],M[2])
		if (equal(M[0].x,T(-1))) {q.xyz()=Tvec<T,3>(0,1,0); q.w=0;}
		else
		{
			q.xyz()=cross(Tvec<T,3>(1,0,0),M[0]+Tvec<T,3>(1,0,0));
			q.w=dot(Tvec<T,3>(1,0,0),M[0]+Tvec<T,3>(1,0,0));
		}
		//Quaternion q superpose X-axis of UCS with axis M[0], i.e. it rotates UCS to (tx,ty,tz), where tx=M[0]
		Tvec<T,3> ty=normalize(q*Tvec<T,3>(0,1,0));
		//Now we should superpose ty with M[1]
		if (ty==-M[1]) {q2.xyz()=Tvec<T,3>(1,0,0); q2.w=0;}
		else
		{
			q2.xyz()=Tvec<T,3>(-1,0,0)*dot(M[2],ty+M[1]);
			q2.w=dot(M[1],ty+M[1]);
		}
		*this = quatIsUnit ? normalize(q*q2) : q*q2;*/
/*/		//REMARK: Code was taken from game engine "nebuladevice"
		T tr = M[0][0] + M[1][1] + M[2][2]; // trace of martix
		if (tr > 0.0f)                         // if trace positive than "w" is biggest component
		{
			x = M[1][2] - M[2][1];
			y = M[2][0] - M[0][2];
			z = M[0][1] - M[1][0];
			w = tr+1.0f;
			if (quatIsUnit) (*this)*=0.5f/(T)sqrt(w);     // "w" contain the "norm * 4"
		} else                                   // Some of vector components is bigger
		if( (M[0][0] > M[1][1] ) && ( M[0][0] > M[2][2]) )
		{
			x = 1.0f + M[0][0] - M[1][1] - M[2][2];
			y = M[1][0] + M[0][1];
			z = M[2][0] + M[0][2];
			w = M[1][2] - M[2][1];
			if (quatIsUnit) (*this)*=0.5f/(T)sqrt(x);
		} else
		if ( M[1][1] > M[2][2] )
		{
			x = M[1][0] + M[0][1];
			y = 1.0f + M[1][1] - M[0][0] - M[2][2];
			z = M[2][1] + M[1][2];
			w = M[2][0] - M[0][2];
			if (quatIsUnit) (*this)*=0.5f/(T)sqrt(y);
		} else
		{
			x = M[2][0] + M[0][2];
			y = M[2][1] + M[1][2];
			z = 1.0f + M[2][2] - M[0][0] - M[1][1];
			w = M[0][1] - M[1][0];
			if (quatIsUnit) (*this)*=0.5f/(T)sqrt(z);
		}/**/
	}

	INLINE Tquat(const T x,const T y,const T z,const T w):x(x),y(y),z(z),w(w) {}

	//INLINE operator T* () {return &x;}
	//INLINE operator const T* () const {return &x;}

	INLINE Tvec<T,3> &xyz() {return (Tvec<T,3>&)x;}
	INLINE const Tvec<T,3> &xyz() const {return (const Tvec<T,3>&)x;}

	INLINE HMbool operator==(const Tquat &q) const {return equal(x,q.x) && equal(y,q.y) && equal(z,q.z) && equal(w,q.w);}
	INLINE HMbool operator!=(const Tquat &q) const {return notEqual(x,q.x) || notEqual(y,q.y) || notEqual(z,q.z) || notEqual(w,q.w);}

	INLINE const Tquat operator*(const T a) const {return Tquat(x*a,y*a,z*a,w*a);}
	INLINE const Tquat operator/(const T a) const {return Tquat(x/a,y/a,z/a,w/a);}

	INLINE const Tquat operator+(const Tquat &q) const {return Tquat(x+q.x,y+q.y,z+q.z,w+q.w);}
	INLINE const Tquat operator-(const Tquat &q) const {return Tquat(x-q.x,y-q.y,z-q.z,w-q.w);}

#ifndef QUAT_MUL_REVERSE_ORDER
	INLINE const Tquat operator*(const Tquat &q) const
	{	return Tquat(y*q.z - z*q.y + w*q.x + x*q.w,
					 z*q.x - x*q.z + w*q.y + y*q.w,
					 x*q.y - y*q.x + w*q.z + z*q.w,
					 w*q.w - x*q.x - y*q.y - z*q.z);   }

	INLINE const Tvec<T,3> operator*(const Tvec<T,3> &v) const
	//{return ((*this)*Tquat(v)*inverse(*this)).xyz();}
	{
		CHECK_UNIT_QUAT(*this);
		return cross(xyz(), cross(xyz(), v) + w*v) * T(2) + v;
	}
#else
	INLINE const Tquat operator*(const Tquat &q) const// = vec4(w*q.xyz + q.w*xyz + cross(xyz,q.xyz), q.w*w-dot(xyz,q.xyz))
	{	return Tquat(q.y*z - q.z*y + q.w*x + q.x*w,
					 q.z*x - q.x*z + q.w*y + q.y*w,
					 q.x*y - q.y*x + q.w*z + q.z*w,
					 q.w*w - q.x*x - q.y*y - q.z*z);   }

	INLINE friend const Tvec<T,3> operator*(const Tvec<T,3> &v, const Tquat &q)
		//{return (inverse(q)*(Tquat<T>(v)*q)).xyz();}
	{
		CHECK_UNIT_QUAT(q);
		return cross(q.xyz(), cross(q.xyz(), v) + q.w*v) * T(2) + v;
	}
#endif

	INLINE friend const Tquat operator*(const T a, const Tquat &q) {return Tquat(a*q.x,a*q.y,a*q.z,a*q.w);}

	INLINE const Tquat &operator*=(const T a) {x*=a; y*=a; z*=a; w*=a; return *this;}
	INLINE const Tquat &operator/=(const T a) {x/=a; y/=a; z/=a; w/=a; return *this;}

#define op(o) INLINE const Tquat &operator o(const Tquat &q) {x o q.x; y o q.y; z o q.z; w o q.w; return *this;}
	op(+=) op(-=)
#undef op

	INLINE const Tquat &operator*=(const Tquat &q) {return *this=(*this)*q;}

	INLINE const Tquat operator-() const {return Tquat(-x, -y, -z, -w);}
/*	INLINE const Tquat operator++() {return Tquat(++x, ++y, ++z, ++w);}
	INLINE const Tquat operator--() {return Tquat(--x, --y, --z, --w);}
	INLINE const Tquat operator++(int notused) {return Tquat(x++, y++, z++, w++);}
	INLINE const Tquat operator--(int notused) {return Tquat(x--, y--, z--, w--);}*/
};

template <class T, int cols, int rows> INLINE Tmat<T,cols,rows>::Tmat(const Tquat<T> &q, HMbool quatIsUnit)
{
	if (quatIsUnit) CHECK_UNIT_QUAT(q);
	T k=(quatIsUnit ? 2 : T(2)/sqlen(q));
	T x2=k*q.x,
	  y2=k*q.y,
	  z2=k*q.z;

	T xx=x2*q.x,
	  yy=y2*q.y,
	  zz=z2*q.z;

	m[0][0]=T(1)-zz-yy;
	m[1][1]=T(1)-zz-xx;
	m[2][2]=T(1)-xx-yy;

	T xy=x2*q.y, zw=z2*q.w;
	m[0][1]=xy+zw;
	m[1][0]=xy-zw;
	T xz=x2*q.z, yw=y2*q.w;
	m[0][2]=xz-yw;
	m[2][0]=xz+yw;
	T yz=y2*q.z, xw=x2*q.w;
	m[1][2]=yz+xw;
	m[2][1]=yz-xw;

	initFor<3,3>();//fill the rest of matrix components
}


typedef Tquat<HMfloat>			quat;
typedef Tquat<HMdouble>			dquat;
typedef Tquat<HMlong_double>	ldquat;

#endif //HM_QUAT_H
