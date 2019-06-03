#ifndef HM_MAT_H
#define HM_MAT_H

#include "vec.h"



template <class T> class Tquat;

template <class T, int cols, int rows> class Tmat
{
	template <int c, int r> INLINE void initFor()
	{
		typedef char staticCheck[(cols>=c && rows>=r) ? 1 : 0];

		for (int i=c;i<cols;i++)
			for (int j=0;j<rows;j++) m[i][j] = (i==j ? T(1) : 0);
		for (int i=0;i<c;i++)
			for (int j=r;j<rows;j++) m[i][j] = 0;
	}
	enum {N = cols*rows};
public:

	T m[cols][rows];

	typedef T TYPE;//for external use only

	INLINE Tmat() {}
	//cast constructor
	template <class TT> INLINE explicit Tmat(const Tmat<TT,cols,rows> &M) {for (int i=0;i<N;i++) m[0][i]=(T)M.m[0][i];}
	//copy constructor
	INLINE Tmat(const Tmat &M) {for (int i=0;i<N;i++) m[0][i]=M.m[0][i];}
	//1 arg
	INLINE explicit Tmat(const T a)
	{
		for (int i=0;i<cols;i++)
			for (int j=0;j<rows;j++) m[i][j] = (i==j ? a : 0);
	}
	INLINE explicit Tmat(const Tvec<T,(cols<rows)?cols:rows> &diag)
	{
		for (int i=0;i<cols;i++)
			for (int j=0;j<rows;j++)
				m[i][j] = (i==j ? diag[i] : 0);
	}
	INLINE explicit Tmat(const Tquat<T> &q, HMbool quatIsUnit=HMtrue);
	template <int c, int r> INLINE explicit Tmat(const Tmat<T,c,r> &M)//from other dimensions
	{
		for (int i=0;i<cols;i++)
			for (int j=0;j<rows;j++)
				m[i][j] = (i<c && j<r ? M.m[i][j] : (i==j ? T(1) : 0));
	}
	//2 arg
	template <int r> INLINE Tmat(const Tvec<T,r> &m0,const Tvec<T,r> &m1)
	{
		*(Tvec<T,r>*)m[0] = m0;
		*(Tvec<T,r>*)m[1] = m1;
		initFor<2,r>();
	}
	//3 arg
	template <int r> INLINE Tmat(const Tvec<T,r> &m0,const Tvec<T,r> &m1,const Tvec<T,r> &m2)
	{
		*(Tvec<T,r>*)m[0] = m0;
		*(Tvec<T,r>*)m[1] = m1;
		*(Tvec<T,r>*)m[2] = m2;
		initFor<3,r>();
	}
	//4 arg
	template <int r> INLINE Tmat(const Tvec<T,r> &m0,const Tvec<T,r> &m1,const Tvec<T,r> &m2,const Tvec<T,r> &m3)
	{
		*(Tvec<T,r>*)m[0] = m0;
		*(Tvec<T,r>*)m[1] = m1;
		*(Tvec<T,r>*)m[2] = m2;
		*(Tvec<T,r>*)m[3] = m3;
		initFor<4,r>();
	}
	INLINE Tmat(const T m00, const T m01,
				const T m10, const T m11)
	{
		m[0][0]=m00; m[0][1]=m01;
		m[1][0]=m10; m[1][1]=m11;
		initFor<2,2>();
	}

	INLINE Tmat(const T m00, const T m01, const T m02,
				const T m10, const T m11, const T m12,
				const T m20, const T m21, const T m22)
	{
		m[0][0]=m00; m[0][1]=m01; m[0][2]=m02;
		m[1][0]=m10; m[1][1]=m11; m[1][2]=m12;
		m[2][0]=m20; m[2][1]=m21; m[2][2]=m22;
		initFor<3,3>();
	}

	INLINE Tmat(const T m00, const T m01, const T m02, const T m03,
				const T m10, const T m11, const T m12, const T m13,
				const T m20, const T m21, const T m22, const T m23,
				const T m30, const T m31, const T m32, const T m33)
	{
		m[0][0]=m00; m[0][1]=m01; m[0][2]=m02; m[0][3]=m03;
		m[1][0]=m10; m[1][1]=m11; m[1][2]=m12; m[1][3]=m13;
		m[2][0]=m20; m[2][1]=m21; m[2][2]=m22; m[2][3]=m23;
		m[3][0]=m30; m[3][1]=m31; m[3][2]=m32; m[3][3]=m33;
		initFor<4,4>();
	}

	//operators
	INLINE Tvec<T,rows> &operator[](const int i) {return *(Tvec<T,rows>*)m[i];}
	INLINE const Tvec<T,rows> &operator[](const int i) const {return *(const Tvec<T,rows>*)m[i];}
	//INLINE operator T* () {return m[0];}
	//INLINE operator const T* () const {return m[0];}

	INLINE HMbool operator==(const Tmat &M) const {for (int i=0;i<N;i++) if (notEqual(m[0][i],M.m[0][i])) return HMfalse; return HMtrue;}
	INLINE HMbool operator!=(const Tmat &M) const {for (int i=0;i<N;i++) if (notEqual(m[0][i],M.m[0][i])) return HMtrue; return HMfalse;}

	//matrix ~ a
	INLINE const Tmat operator+(const T a) const {Tmat r; for (int i=0;i<N;i++) r.m[0][i]=m[0][i] + a; return r;}
	INLINE const Tmat operator-(const T a) const {Tmat r; for (int i=0;i<N;i++) r.m[0][i]=m[0][i] - a; return r;}
	INLINE const Tmat operator*(const T a) const {Tmat r; for (int i=0;i<N;i++) r.m[0][i]=m[0][i] * a; return r;}
	INLINE const Tmat operator/(const T a) const {Tmat r; for (int i=0;i<N;i++) r.m[0][i]=m[0][i] / a; return r;}

	//a ~ matrix
	INLINE friend const Tmat operator+(const T a, const Tmat &M) {Tmat r; for (int i=0;i<N;i++) r.m[0][i]=a + M.m[0][i]; return r;}
	INLINE friend const Tmat operator-(const T a, const Tmat &M) {Tmat r; for (int i=0;i<N;i++) r.m[0][i]=a - M.m[0][i]; return r;}
	INLINE friend const Tmat operator*(const T a, const Tmat &M) {Tmat r; for (int i=0;i<N;i++) r.m[0][i]=a * M.m[0][i]; return r;}
	INLINE friend const Tmat operator/(const T a, const Tmat &M) {Tmat r; for (int i=0;i<N;i++) r.m[0][i]=a / M.m[0][i]; return r;}

	//matrix ~ matrix
	INLINE const Tmat operator+(const Tmat &M) const {Tmat r; for (int i=0;i<N;i++) r.m[0][i]=m[0][i] + M.m[0][i]; return r;}
	INLINE const Tmat operator-(const Tmat &M) const {Tmat r; for (int i=0;i<N;i++) r.m[0][i]=m[0][i] - M.m[0][i]; return r;}

	//matrix ~= a
	INLINE const Tmat &operator+=(const T a) {for (int i=0;i<N;i++) m[0][i] += a; return *this;}
	INLINE const Tmat &operator-=(const T a) {for (int i=0;i<N;i++) m[0][i] -= a; return *this;}
	INLINE const Tmat &operator*=(const T a) {for (int i=0;i<N;i++) m[0][i] *= a; return *this;}
	INLINE const Tmat &operator/=(const T a) {for (int i=0;i<N;i++) m[0][i] /= a; return *this;}

	//matrix ~= matrix
	INLINE const Tmat &operator+=(const Tmat &M) {for (int i=0;i<N;i++) m[0][i] += M.m[0][i]; return *this;}
	INLINE const Tmat &operator-=(const Tmat &M) {for (int i=0;i<N;i++) m[0][i] -= M.m[0][i]; return *this;}
	INLINE const Tmat &operator =(const Tmat &M) {for (int i=0;i<N;i++) m[0][i]  = M.m[0][i]; return *this;}

	//-matrix
	INLINE const Tmat operator-() const {Tmat res; for (int i=0;i<N;i++) res.m[0][i]=-m[0][i]; return res;}

	//++matrix and --matrix
	INLINE const Tmat operator++() {for (int i=0;i<N;i++) ++m[0][i]; return *this;}
	INLINE const Tmat operator--() {for (int i=0;i<N;i++) --m[0][i]; return *this;}

	//matrix++ and matrix--
	INLINE const Tmat operator++(int notused) {Tmat t(*this); for (int i=0;i<N;i++) ++m[0][i]; return t;}
	INLINE const Tmat operator--(int notused) {Tmat t(*this); for (int i=0;i<N;i++) --m[0][i]; return t;}

	//multiplication operators
	INLINE const Tmat &operator*=(const Tmat &M) {return *this=(*this)*M;}
};

#ifndef D3D_MAT_STYLE
template <class T, int rows, int cols> INLINE const Tvec<T,rows> operator*(const Tmat<T,cols,rows> &M, const Tvec<T,cols> &v)
#else
template <class T, int rows, int cols> INLINE const Tvec<T,rows> operator*(const Tvec<T,cols> &v, const Tmat<T,cols,rows> &M)
#endif
{
	Tvec<T,rows> r;
	for (int i=0;i<rows;i++)
	{
		r[i] = M.m[0][i] * v[0];
		for (int j=1;j<cols;j++) r[i] += M.m[j][i]*v[j];
	}
	return r;
}

#ifndef D3D_MAT_STYLE
template <class T, int rows, int cols> INLINE const Tvec<T,cols> operator*(const Tvec<T,rows> &v, const Tmat<T,cols,rows> &M)
#else
template <class T, int rows, int cols> INLINE const Tvec<T,cols> operator*(const Tmat<T,cols,rows> &M, const Tvec<T,rows> &v)
#endif
{
	Tvec<T,cols> r;
	for (int i=0;i<cols;i++)
	{
		r[i] = M.m[i][0] * v[0];
		for (int j=1;j<rows;j++) r[i] += M.m[i][j]*v[j];
	}
	return r;
}

#ifndef D3D_MAT_STYLE
template <class T, int c1, int r1, int c2> INLINE const Tmat<T,c2,r1> operator*(const Tmat<T,c1,r1> &M1, const Tmat<T,c2,c1> &M2)
#else
template <class T, int c1, int r1, int c2> INLINE const Tmat<T,c2,r1> operator*(const Tmat<T,c2,c1> &M2, const Tmat<T,c1,r1> &M1)
#endif
{
	Tmat<T,c2,r1> r;
	for (int i=0; i<c2; i++)
	{
		Tvec<T,c1> c = M2[i];
		for (int j=0; j<r1; j++)
		{
			r[i][j] = M1.m[0][j]*c[0];
			for (int k=1; k<c1; k++)
				r[i][j] += M1.m[k][j]*c[k];
		}
	}
	return r;
}


#define typedef_matrices(dimension)								 \
typedef Tmat<HMfloat      ,dimension,dimension>	  mat##dimension;\
typedef Tmat<HMint        ,dimension,dimension>	 imat##dimension;\
typedef Tmat<HMdouble     ,dimension,dimension>	 dmat##dimension;\
typedef Tmat<HMlong_double,dimension,dimension>	ldmat##dimension;

typedef_matrices(2)
typedef_matrices(3)
typedef_matrices(4)

#undef typedef_matrices

//Non-square matrices
#define typedef_matrices(cols,rows)							 \
typedef Tmat<HMfloat      ,cols,rows>	  mat##cols##x##rows;\
typedef Tmat<HMint        ,cols,rows>	 imat##cols##x##rows;\
typedef Tmat<HMdouble     ,cols,rows>	 dmat##cols##x##rows;\
typedef Tmat<HMlong_double,cols,rows>	ldmat##cols##x##rows;

typedef_matrices(2,2)
typedef_matrices(2,3)
typedef_matrices(2,4)
typedef_matrices(3,2)
typedef_matrices(3,3)
typedef_matrices(3,4)
typedef_matrices(4,2)
typedef_matrices(4,3)
typedef_matrices(4,4)

#undef typedef_matrices

#endif //HM_MAT_H
