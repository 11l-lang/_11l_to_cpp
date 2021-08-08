#ifndef HM_AABB_H
#define HM_AABB_H

#include "func.h"
#include <string.h> // for memcmp [in GCC]

template <class T> T minimum(const T &t1, const T &t2) {return min(t1, t2);}
template <class T> T maximum(const T &t1, const T &t2) {return max(t1, t2);}


template <class T> class Trange
{
public:

	T min,max;

	typedef T TYPE;//for external use only

	INLINE Trange() {}
	template <class TT> INLINE explicit Trange(const Trange<TT> &r):min((T)r.min),max((T)r.max) {}
	INLINE Trange(const Trange &b):min(b.min),max(b.max) {}
	INLINE explicit Trange(const T a)
	{
		if (a)
		{
			min = -a;
			max =  a;
		}
		else
		{
			min =  hm_limits<T>::max();
			max = -hm_limits<T>::max();
		}
	}
	INLINE Trange(const T min,const T max):min(min),max(max) {}
	INLINE Trange(const HMuint count,const T *vertices)
	{
		if (count > 0)
		{
			min=max=vertices[0];

			for (HMuint i=1;i<count;i++)
				(*this)|=vertices[i];
		}
		else
			*this=Trange(0);
	}

	INLINE HMbool operator==(const Trange &b) const {return memcmp(this, &b, sizeof(b)) == 0;}
	INLINE HMbool operator!=(const Trange &b) const {return !operator==(b);}

	INLINE const Trange operator+(const T &v) const {return Trange(min+v, max+v);}
	INLINE const Trange operator-(const T &v) const {return Trange(min-v, max-v);}

	INLINE const Trange operator|(const T &v) const
	{return Trange(::minimum(min,v),::maximum(max,v));}

	INLINE const Trange operator|(const Trange &b) const
	{return Trange(::minimum(min,b.min),::maximum(max,b.max));}

	INLINE const Trange operator&(const Trange &b) const
	{
		if (intersect(*this,b))
			return Trange(::maximum(min,b.min),::minimum(max,b.max));
		else
			return Trange(0);
	}

	INLINE const Trange operator^(const T &v) const
	{return Trange(min-v, max+v);}

	INLINE const Trange operator*(const T &k) const {return Trange(min*k,max*k);}
	INLINE const Trange operator/(const T &k) const {return Trange(min/k,max/k);}

	INLINE Trange &operator+=(const T &v)		 {min += v; max += v; return *this;}
	INLINE Trange &operator-=(const T &v)		 {min -= v; max -= v; return *this;}
	INLINE Trange &operator|=(const T &v)		 {return (*this)=(*this)|v;}
	INLINE Trange &operator|=(const Trange &b) {return (*this)=(*this)|b;}
	INLINE Trange &operator&=(const Trange &b) {return (*this)=(*this)&b;}
	INLINE Trange &operator^=(const T &v)		 {return (*this)=(*this)^v;}
	INLINE Trange &operator*=(const T &k)		 {min*=k; max*=k; return *this;}
	INLINE Trange &operator/=(const T &k)		 {min/=k; max/=k; return *this;}

	INLINE Trange &operator=(const Trange &b) {min=b.min; max=b.max; return *this;}

	INLINE const T center() const {return (min+max)/T(2);}
	INLINE const T   size() const {return  max-min;}
};


template <class Tvec> class Taabb : public Trange<Tvec>
{
	typedef typename Tvec::TYPE T;
public:
	using Trange<Tvec>::min;
	using Trange<Tvec>::max;

	INLINE Taabb() {}
	template <class TT> INLINE explicit Taabb(const Taabb<TT> &b) {min=(Tvec)b.min; max=(Tvec)b.max;}
	INLINE Taabb(const Taabb &b):Trange<Tvec>(b.min,b.max) {}
	INLINE explicit Taabb(const typename Tvec::TYPE a)
	{
		if (a)
		{
			min=Tvec(-a);
			max=Tvec( a);
		}
		else
		{
#if _MSC_VER==1200
#define typename
#endif
			min=Tvec( hm_limits<typename Tvec::TYPE>::max());
			max=Tvec(-hm_limits<typename Tvec::TYPE>::max());
#ifdef typename
#undef typename
#endif
		}
	}
	INLINE explicit Taabb(const Tvec &p):Trange<Tvec>(p,p) {}
	INLINE Taabb(const T min, const T max):Trange<Tvec>(Tvec(min),Tvec(max)) {}
	INLINE Taabb(const Tvec &min,const Tvec &max):Trange<Tvec>(min,max) {}
	INLINE Taabb(T minx, T miny, T maxx, T maxy) : Trange<Tvec>(Tvec(minx, miny), Tvec(maxx, maxy)) {}
	INLINE Taabb(T minx, T miny, T minz, T maxx, T maxy, T maxz) : Trange<Tvec>(Tvec(minx, miny, minz), Tvec(maxx, maxy, maxz)) {}
	INLINE Taabb(const HMuint count,const Tvec *vertices)
	{
		if (count > 0)
		{
			min=max=vertices[0];

			for (HMuint i=1;i<count;i++)
				(*this)|=vertices[i];
		}
		else
			*this=Taabb(0);
	}

	INLINE const Taabb operator|(const Tvec &v) const
	{return Taabb(::minimum(min,v),::maximum(max,v));}

	INLINE const Taabb operator|(const Taabb &b) const
	{return Taabb(::minimum(min,b.min),::maximum(max,b.max));}

	INLINE const Taabb operator&(const Taabb &b) const
	{
		if (intersect(*this,b))
			return Taabb(::maximum(min,b.min),::minimum(max,b.max));
		else
			return Taabb(0);
	}

	template <class T> INLINE const Taabb operator^(const T &v) const
	{return Taabb(min-v, max+v);}

	INLINE const Taabb operator*(const typename Tvec::TYPE &k) const {return Taabb(min*k,max*k);}
	INLINE const Taabb operator/(const typename Tvec::TYPE &k) const {return Taabb(min/k,max/k);}

	INLINE const Taabb operator+(const Tvec &v) const {return Taabb(min+v, max+v);}
	INLINE const Taabb operator-(const Tvec &v) const {return Taabb(min-v, max-v);}
	INLINE const Taabb operator*(const Tvec &k) const {return Taabb(min*k, max*k);}
	INLINE const Taabb operator/(const Tvec &k) const {return Taabb(min/k, max/k);}

	INLINE Taabb &operator|=(const Tvec &v)
	{
		for (int i=0;i<dimension(min);i++)
		{
			if (v[i]<min[i]) min[i]=v[i];
			if (v[i]>max[i]) max[i]=v[i];
		}
		return *this;
	}

	INLINE Taabb &operator|=(const Taabb &b)
	{
		for (int i=0;i<dimension(min);i++)
		{
			if (b.min[i]<min[i]) min[i]=b.min[i];
			if (b.max[i]>max[i]) max[i]=b.max[i];
		}
		return *this;
	}

	INLINE Taabb &operator&=(const Taabb &b)
	{
		if (intersect(*this,b))
			for (int i=0;i<dimension(min);i++)
			{
				if (b.min[i]>min[i]) min[i]=b.min[i];
				if (b.max[i]<max[i]) max[i]=b.max[i];
			}
		else
			*this=Taabb(0);

		return *this;
	}

	INLINE Taabb &operator+=(const Tvec &v)				 {min+=v; max+=v; return *this;}
	INLINE Taabb &operator-=(const Tvec &v)				 {min-=v; max-=v; return *this;}
	INLINE Taabb &operator*=(const Tvec &k)				 {min*=k; max*=k; return *this;}
	INLINE Taabb &operator/=(const Tvec &k)				 {min/=k; max/=k; return *this;}
	INLINE Taabb &operator*=(const typename Tvec::TYPE &k) {min*=k; max*=k; return *this;}
	INLINE Taabb &operator/=(const typename Tvec::TYPE &k) {min/=k; max/=k; return *this;}
	INLINE Taabb &operator^=(const typename Tvec::TYPE &k) {return (*this)=(*this)^k;}
	using Trange<Tvec>::operator^=;
};

template <class T>
INLINE const Trange<T> operator|(const T &v,const Trange<T> &b)
{return Trange<T>(::minimum(b.min,v),::maximum(b.max,v));}

template <class T> INLINE HMbool intersect(const Trange<T> &b0,const Trange<T> &b1)
{
	for (int i=0;i<dimension(b0.min);i++)
		if (!(dereference(b0.min,i)<=dereference(b1.max,i) && dereference(b0.max,i)>=dereference(b1.min,i))) return HMfalse;
//		if (abs(dereference(b0.min,i) + dereference(b0.max,i) - dereference(b1.min,i) - dereference(b1.max,i)) > dereference(b0.max,i) - dereference(b0.min,i) + dereference(b1.max,i) - dereference(b1.min,i)) return HMfalse;//abs(b0.center() - b1.center()) > (b0.size() + b1.size())/2

	return HMtrue;
}


typedef Taabb<vec3>		aabb;
typedef Taabb<dvec3>	daabb;
typedef Taabb<ldvec3>	ldaabb;
typedef Taabb<ivec3>	iaabb;
typedef Taabb<svec3>	saabb;

typedef Taabb<vec2>		rect;
typedef Taabb<dvec2>	drect;
typedef Taabb<ldvec2>	ldrect;
typedef Taabb<ivec2>	irect;
typedef Taabb<svec2>	srect;

typedef Trange<HMfloat>			range;
typedef Trange<HMdouble>		drange;
typedef Trange<HMlong_double>	ldrange;
typedef Trange<HMint>			irange;
typedef Trange<HMshort>			srange;

#endif //HM_AABB_H
