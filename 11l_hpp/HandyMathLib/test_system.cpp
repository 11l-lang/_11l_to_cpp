//#define D3D_MAT_STYLE
//#define QUAT_MUL_REVERSE_ORDER
//#define MAKE_TESTS
#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h> // for int64_t in func.h
#include "hm.h"

#include <iostream>
#include <iomanip>

#ifdef MAKE_TESTS

#define TEST(A,B) do A; while(false)

#define MAKE(A) cout<<"TEST("<<#A<<','<<(A)<<");\n"

#include <string>

std::string typeLetter(HMint) {return "i";}
std::string typeLetter(HMfloat) {return "";}
std::string typeLetter(HMdouble) {return "d";}
std::string typeLetter(HMlong_double) {return "ld";}
int typePrecision(HMint) {return 0;}
int typePrecision(HMfloat) {return 6;}
int typePrecision(HMdouble) {return 9;}
int typePrecision(HMlong_double) {return 9;}

#define operator_shift_ostream_vec(d)											\
template <class T> std::ostream &operator<<(std::ostream &s,const Tvec<T,d> &v)	\
{																				\
	s<<typeLetter(v.x)<<"vec"#d"(";												\
	s.precision(typePrecision(v.x));											\
	for (int i=0;i<d;i++)														\
		s<<v[i]<<(i<(d-1) ? ',' : ')');											\
	return s;																	\
}
operator_shift_ostream_vec(2)
operator_shift_ostream_vec(3)
operator_shift_ostream_vec(4)
#undef operator_shift_ostream_vec

#define operator_shift_ostream_mat(d)											\
template <class T> std::ostream &operator<<(std::ostream &s,const Tmat<T,d,d> &M)\
{																				\
	s<<typeLetter(M[0][0])<<"mat"#d"(";											\
	s.precision(typePrecision(M[0][0]));										\
	for (int i=0;i<d*d;i++)														\
		s<<M.e[i]<<(i<(d*d-1) ? ',' : ')');										\
	return s;																	\
}
operator_shift_ostream_mat(2)
operator_shift_ostream_mat(3)
operator_shift_ostream_mat(4)
#undef operator_shift_ostream_mat

template <class T> std::ostream &operator<<(std::ostream &s,const Tquat<T> &q)
{
	s<<typeLetter(q.x)<<"quat(";
	s.precision(typePrecision(q.x));
	for (int i=0;i<4;i++)
		s<<(&q.x)[i]<<(i<3 ? ',' : ')');
	return s;
}

template <class T> std::ostream &operator<<(std::ostream &s,const Trange<T> &b)
{
	return s<<typeLetter(b.min)<<"range("<<b.min<<','<<b.max<<')';
}

template <class T> std::ostream &operator<<(std::ostream &s,const Taabb<T> &b)
{
	return s<<typeLetter(b.min.x)<<(dimension(b.min)==3 ? "aabb(" : "rect(")<<b.min<<','<<b.max<<')';
}

#else

#define TEST(A,B) do {											\
totalTests++;													\
cout<<left<<setw(75)<<#A;										\
if ((A)==(B)) cout<<"- ok\n";									\
else {totalBugs++; cout<<"- BUG!!! (line: "<<__LINE__<<")\n";}	\
} while (false)

#define MAKE(A) do A; while(false)

#endif

#ifndef D3D_MAT_STYLE
#define MAT_MUL(a,b) (a*b)
#else
#define MAT_MUL(a,b) (b*a)
#endif

#ifndef QUAT_MUL_REVERSE_ORDER
#define QUAT_MUL(a,b) (a*b)
#else
#define QUAT_MUL(a,b) (b*a)
#endif

int main()
{
	using std::cout;
	using std::endl;
	using std::setw;
	using std::left;
	unsigned totalBugs=0,totalTests=0;

	cout<<  "=================Testing \"vec.h\"==================\n\n";

	bvec2 bv2;
	TEST(bvec2(bvec2(HMtrue,HMfalse)),bvec2(HMtrue,HMfalse));
	TEST(bvec2(HMfalse),bvec2(HMfalse,HMfalse));
	TEST(bvec2(HMfalse,HMtrue)[1],HMtrue);
	TEST(bvec2(HMfalse,HMtrue)!=bvec2(HMtrue,HMfalse),HMtrue);
	TEST(bv2=bvec2(HMtrue,HMtrue),bvec2(HMtrue,HMtrue));
	vec2 v2;
	TEST(vec2(vec2(-2,1)),vec2(-2,1));
	TEST(vec2(12),vec2(12,12));
	TEST(vec2(3,-1)[1],-1);
	TEST(vec2(2,0)!=vec2(0,2),HMtrue);
	TEST(v2=vec2(5,8),vec2(5,8));
	TEST(ldvec2(10,-20)+2.0l,ldvec2(12,-18));
	TEST(  vec2(10,-20)-2.0f,vec2(8,-22));
	TEST( dvec2(10,-20)*-2.0,dvec2(-20,40));
	TEST( ivec2(10,-20)/2   ,ivec2(5,-10));
	TEST(ldvec2(1,-2)+ldvec2(-2,1),ldvec2(-1,-1));
	TEST(  vec2(1,-2)-  vec2(-2,1),vec2(3,-3));
	TEST( dvec2(1,-2)* dvec2(-2,1),dvec2(-2,-2));
	TEST( ivec2(1,-2)/ ivec2(-2,1),ivec2(0,-2));
	TEST(ldvec2(10,-20)+=2.0l,ldvec2(12,-18));
	TEST(  vec2(10,-20)-=2.0f,vec2(8,-22));
	TEST( dvec2(10,-20)*=-2.0,dvec2(-20,40));
	TEST( ivec2(10,-20)/=2   ,ivec2(5,-10));
	TEST(ldvec2(1,-2)+=ldvec2(-2,1),ldvec2(-1,-1));
	TEST(  vec2(1,-2)-=  vec2(-2,1),vec2(3,-3));
	TEST( dvec2(1,-2)*= dvec2(-2,1),dvec2(-2,-2));
	TEST( ivec2(1,-2)/= ivec2(-2,1),ivec2(0,-2));
	TEST(2.0l+ldvec2(10,-20),ldvec2(12,-18));
	TEST(2.0f-  vec2(10,-20),vec2(-8,22));
	TEST(-2.0* dvec2(10,-20),dvec2(-20,40));
	TEST(200/  ivec2(10,-20),ivec2(20,-10));
	TEST(-dvec2(1,-2),dvec2(-1,2));
	TEST(++ldvec2(1,-2),ldvec2(2,-1));
	TEST(--  vec2(1,-2),  vec2(0,-3));
	v2=vec2(5,8);
	TEST((v2++)==vec2(5,8) && v2==vec2(6,9),true);
	v2=vec2(5,8);
	TEST((v2--)==vec2(5,8) && v2==vec2(4,7),true);

	bvec3 bv3;
	TEST(bvec3(bvec3(HMtrue,HMfalse,HMfalse)),bvec3(HMtrue,HMfalse,HMfalse));
	TEST(bvec3(HMtrue),bvec3(HMtrue,HMtrue,HMtrue));
	TEST(bvec3(HMtrue,HMfalse,HMtrue)[2],HMtrue);
	TEST(bvec3(HMtrue,HMfalse,HMtrue)!=bvec3(HMtrue,HMfalse,HMtrue),HMfalse);
	TEST(bv3=bvec3(HMtrue,HMtrue,HMfalse),bvec3(HMtrue,HMtrue,HMfalse));
	TEST(bvec2(bvec3(HMfalse,HMtrue,HMfalse)),bvec2(HMfalse,HMtrue));
	vec3 v3;
	TEST(vec3(vec3(-2,1,4)),vec3(-2,1,4));
	TEST(vec3(12),vec3(12,12,12));
	TEST((v3=vec3(3,-1,4))[2],4);
	TEST(v3.st(),vec2(3,-1));
	TEST(v3.gb(),vec2(-1,4));
	TEST(vec3(2,0,-1)!=vec3(2,0,0),HMtrue);
	TEST(v3=vec3(5,8,6),vec3(5,8,6));
	TEST(ldvec3(10,-20,4)+2.0l,ldvec3(12,-18,6));
	TEST(  vec3(10,-20,4)-2.0f,vec3(8,-22,2));
	TEST( dvec3(10,-20,4)*-2.0,dvec3(-20,40,-8));
	TEST( ivec3(10,-20,4)/2   ,ivec3(5,-10,2));
	TEST(ldvec3(1,-2,3)+ldvec3(-2,1,0),ldvec3(-1,-1,3));
	TEST(  vec3(1,-2,3)-  vec3(-2,1,0),vec3(3,-3,3));
	TEST( dvec3(1,-2,3)* dvec3(-2,1,0),dvec3(-2,-2,0));
	TEST( ivec3(1,-2,0)/ ivec3(-2,1,3),ivec3(0,-2,0));
	TEST(ldvec3(10,-20,4)+=2.0l,ldvec3(12,-18,6));
	TEST(  vec3(10,-20,4)-=2.0f,vec3(8,-22,2));
	TEST( dvec3(10,-20,4)*=-2.0,dvec3(-20,40,-8));
	TEST( ivec3(10,-20,4)/=2   ,ivec3(5,-10,2));
	TEST(ldvec3(1,-2,3)+=ldvec3(-2,1,0),ldvec3(-1,-1,3));
	TEST(  vec3(1,-2,3)-=  vec3(-2,1,0),vec3(3,-3,3));
	TEST( dvec3(1,-2,3)*= dvec3(-2,1,0),dvec3(-2,-2,0));
	TEST( ivec3(1,-2,0)/= ivec3(-2,1,3),ivec3(0,-2,0));
	TEST(2.0l+ldvec3(10,-20,4),ldvec3(12,-18,6));
	TEST(2.0f-  vec3(10,-20,4),vec3(-8,22,-2));
	TEST(-2.0* dvec3(10,-20,4),dvec3(-20,40,-8));
	TEST(200/  ivec3(10,-20,4),ivec3(20,-10,50));
	TEST(-dvec3(1,-2,-3),dvec3(-1,2,3));
	TEST(++ldvec3(1,-2,-3),ldvec3(2,-1,-2));
	TEST(--  vec3(1,-2,-3),  vec3(0,-3,-4));
	v3=vec3(5,8,6);
	TEST((v3++)==vec3(5,8,6) && v3==vec3(6,9,7),true);
	v3=vec3(5,8,6);
	TEST((v3--)==vec3(5,8,6) && v3==vec3(4,7,5),true);
	TEST(vec2(vec3(1,2,3)),vec2(1,2));

	bvec4 bv4;
	TEST(bvec4(bvec4(HMtrue,HMfalse,HMfalse,HMtrue)),bvec4(HMtrue,HMfalse,HMfalse,HMtrue));
	TEST(bvec4(HMtrue),bvec4(HMtrue,HMtrue,HMtrue,HMtrue));
	TEST(bvec4(HMtrue,HMfalse,HMtrue,HMfalse)[2],HMtrue);
	TEST(bvec4(HMtrue,HMfalse,HMtrue,HMtrue)!=bvec4(HMtrue,HMfalse,HMtrue,HMfalse),HMtrue);
	TEST(bv4=bvec4(HMtrue,HMtrue,HMfalse,HMfalse),bvec4(HMtrue,HMtrue,HMfalse,HMfalse));
	TEST(bvec2(bvec4(HMfalse,HMtrue,HMfalse,HMtrue)),bvec2(HMfalse,HMtrue));
	TEST(bvec3(bvec4(HMfalse,HMtrue,HMfalse,HMtrue)),bvec3(HMfalse,HMtrue,HMfalse));
	vec4 v4;
	TEST(vec4(vec4(-2,1,4,3)),vec4(-2,1,4,3));
	TEST(vec4(12),vec4(12,12,12,12));
	TEST((v4=vec4(3,-1,4,5))[2],4);
	TEST(vec4(2,0,-1,-2)!=vec4(0,2,-1,-2),HMtrue);
	TEST(v4=vec4(5,8,6,-7),vec4(5,8,6,-7));
	TEST(v4.rgb(),vec3(5,8,6));
	TEST(v4.yzw(),vec3(8,6,-7));
	TEST(v4.xy(),vec2(5,8));
	TEST(v4.tp(),vec2(8,6));
	TEST(v4.ba(),vec2(6,-7));
	TEST(ldvec4(10,-20,4,8)+2.0l,ldvec4(12,-18,6,10));
	TEST(  vec4(10,-20,4,8)-2.0f,vec4(8,-22,2,6));
	TEST( dvec4(10,-20,4,8)*-2.0,dvec4(-20,40,-8,-16));
	TEST( ivec4(10,-20,4,8)/2   ,ivec4(5,-10,2,4));
	TEST(ldvec4(1,-2,3,4)+ldvec4(-2,1,0,-1),ldvec4(-1,-1,3,3));
	TEST(  vec4(1,-2,3,4)-  vec4(-2,1,0,-1),vec4(3,-3,3,5));
	TEST( dvec4(1,-2,3,4)* dvec4(-2,1,0,-1),dvec4(-2,-2,0,-4));
	TEST( ivec4(1,-2,0,4)/ ivec4(-2,1,3,-1),ivec4(0,-2,0,-4));
	TEST(ldvec4(10,-20,4,8)+=2.0l,ldvec4(12,-18,6,10));
	TEST(  vec4(10,-20,4,8)-=2.0f,vec4(8,-22,2,6));
	TEST( dvec4(10,-20,4,8)*=-2.0,dvec4(-20,40,-8,-16));
	TEST( ivec4(10,-20,4,8)/=2   ,ivec4(5,-10,2,4));
	TEST(ldvec4(1,-2,3,4)+=ldvec4(-2,1,0,-1),ldvec4(-1,-1,3,3));
	TEST(  vec4(1,-2,3,4)-=  vec4(-2,1,0,-1),vec4(3,-3,3,5));
	TEST( dvec4(1,-2,3,4)*= dvec4(-2,1,0,-1),dvec4(-2,-2,0,-4));
	TEST( ivec4(1,-2,0,4)/= ivec4(-2,1,3,-1),ivec4(0,-2,0,-4));
	TEST(2.0l+ldvec4(10,-20,4,8),ldvec4(12,-18,6,10));
	TEST(2.0f-  vec4(10,-20,4,8),vec4(-8,22,-2,-6));
	TEST(-2.0* dvec4(10,-20,4,8),dvec4(-20,40,-8,-16));
	TEST(200/  ivec4(10,-20,4,8),ivec4(20,-10,50,25));
	TEST(-dvec4(1,-2,-3,4),dvec4(-1,2,3,-4));
	TEST(++ldvec4(1,-2,-3,4),ldvec4(2,-1,-2,5));
	TEST(--  vec4(1,-2,-3,4),  vec4(0,-3,-4,3));
	v4=vec4(5,8,6,-7);
	TEST((v4++)==vec4(5,8,6,-7) && v4==vec4(6,9,7,-6),true);
	v4=vec4(5,8,6,-7);
	TEST((v4--)==vec4(5,8,6,-7) && v4==vec4(4,7,5,-8),true);
	TEST(vec2(vec4(1,2,3,4)),vec2(1,2));
	TEST(vec3(vec4(1,2,3,4)),vec3(1,2,3));

	cout<<"\n=================Testing \"mat.h\"==================\n\n";

	ldmat2 ldm2(1.1,-.2,
				-.05,.95);
	dmat2 dm2(1.1,-.2,
			-.05,.95);
	mat2 m2(1.1f,-.2f,
			-.05f,.95f);
	imat2 im2(110,-20,
			  -5,95);
	ldmat2 ldM2(1.05,-.22,
				.03,.97);
	dmat2 dM2(1.05,-.22,
			.03,.97);
	mat2 M2(1.05f,-.22f,
			.03f,.97f);
	imat2 iM2(55,-2,
			  3,47);
	TEST(imat2(imat2(110,-20,-5,95)),im2);
	TEST(mat2(12),mat2(12,0,0,12));
	TEST(mat3(mat2(1.1f,-.2f,-.05f,.95f)),mat3(1.1f,-0.2f,0,-0.05f,0.95f,0,0,0,1));
	TEST(dmat4(dmat2(1.1,-.2,-.05,.95)),dmat4(1.1,-0.2,0,0,-0.05,0.95,0,0,0,0,1,0,0,0,0,1));
	TEST(equal(m2[0][1],-.2f),HMtrue);
	TEST(mat2(110,-20,-5,95)!=mat2(111,-20,-5,95),HMtrue);
	TEST(m2=mat2(1.1f,-0.2f,-0.05f,0.95f),mat2(1.1f,-0.2f,-0.05f,0.95f));
	TEST(ldm2+2.0l,ldmat2(3.1,1.8,1.95,2.95));
	TEST(m2-2.0f,mat2(-0.9f,-2.2f,-2.05f,-1.05f));
	TEST(dm2*2.0,dmat2(2.2,-0.4,-0.1,1.9));
	TEST(im2/2,imat2(55,-10,-2,47));
	TEST(ldm2+ldM2,ldmat2(2.15,-0.42,-0.02,1.92));
	TEST(m2- M2,mat2(0.05f,0.02f,-0.08f,-0.02f));
	TEST(MAT_MUL(dm2,dM2),dmat2(1.166,-0.419,-0.0155,0.9155));
	//TEST(im2/ iM2,imat2(2,10,-1,2));
	TEST(MAT_MUL(m2,vec2(1,2)),vec2(1,1.7f));
	TEST(MAT_MUL(vec2(2,1),m2),vec2(2,0.85f));
	TEST(2.0l+ldm2,ldmat2(3.1,1.8,1.95,2.95));
	TEST(2.0f- m2,mat2(0.9f,2.2f,2.05f,1.05f));
	TEST(2.0* dm2,dmat2(2.2,-0.4,-0.1,1.9));
	TEST(300/ im2,imat2(2,-15,-60,3));
	TEST(ldm2+=.01l,ldmat2(1.11,-0.19,-0.04,0.96));
	TEST(m2-=.01f,mat2(1.09f,-0.21f,-0.06f,0.94f));
	TEST(dm2*=1.01,dmat2(1.111,-0.202,-0.0505,0.9595));
	TEST(im2/=2,imat2(55,-10,-2,47));
	TEST(ldm2+=ldM2,ldmat2(2.16,-0.41,-0.01,1.93));
	TEST(m2-= M2,mat2(0.04f,0.01f,-0.09f,-0.03f));
#ifndef D3D_MAT_STYLE
	TEST(dm2*= dM2,dmat2(1.17766,-0.42319,-0.015655,0.924655));
#else
	TEST(dm2=dM2*dm2,dmat2(1.17766,-0.42319,-0.015655,0.924655));
#endif
	TEST(im2=(imat2)matrixCompMult(dmat2(im2),1./dmat2(iM2)),imat2(1,5,0,1));
	TEST(-dm2,dmat2(-1.17766,0.42319,0.015655,-0.924655));
	TEST(++ldm2,ldmat2(3.16,0.59,0.99,2.93));
	TEST(-- m2,mat2(-0.96f,-0.99f,-1.09f,-1.03f));
	TEST(dm2++,dmat2(1.17766,-0.42319,-0.015655,0.924655));
	TEST(dm2,dmat2(2.17766,0.57681,0.984345,1.924655));
	TEST(im2--,imat2(1,5,0,1));
	TEST(im2,imat2(0,4,-1,0));

	ldmat3 ldm3(1.1,-.2,.1,
				-.05,.95,.07,
				-.17,.18,1.2);
	dmat3 dm3(1.1,-.2,.1,
			-.05,.95,.07,
			-.17,.18,1.2);
	mat3 m3(1.1f,-.2f,.1f,
			-.05f,.95f,.07f,
			-.17f,.18f,1.2f);
	imat3 im3(110,-20,10,
			  -5,95,7,
			  -17,18,120);
	ldmat3 ldM3(1.05,-.22,-.11,
				.03,.97,-.04,
				.14,.19,1.12);
	dmat3 dM3(1.05,-.22,-.11,
			.03,.97,-.04,
			.14,.19,1.12);
	mat3 M3(1.05f,-.22f,-.11f,
			.03f,.97f,-.04f,
			.14f,.19f,1.12f);
	imat3 iM3(55,-2,-1,
			  3,47,-4,
			  4,9,66);
	TEST(imat3(imat3(110,-20,10,-5,95,7,-17,18,120)),im3);
	TEST(mat3(12),mat3(12,0,0,0,12,0,0,0,12));
	TEST(mat2(mat3(1.1f,-.2f,.1f,-.05f,.95f,.07f,-.17f,.18f,1.2f)),mat2(1.1f,-0.2f,-0.05f,0.95f));
	TEST(imat4(imat3(110,-20,10,-5,95,7,-17,18,120)),imat4(110,-20,10,0,-5,95,7,0,-17,18,120,0,0,0,0,1));
	TEST(equal(m3[1][2],.07f),HMtrue);
	TEST(mat3(110,-20,10,-5,95,7,-17,18,120)!=mat3(110,-20,10,-5,95,7,-17,18,120),HMfalse);
	TEST(m3=mat3(1.1f,-0.2f,0.1f,-0.05f,0.95f,0.07f,-0.17f,0.18f,1.2f),mat3(1.1f,-0.2f,0.1f,-0.05f,0.95f,0.07f,-0.17f,0.18f,1.2f));
	TEST(ldm3+2.0l,ldmat3(3.1,1.8,2.1,1.95,2.95,2.07,1.83,2.18,3.2));
	TEST(m3-2.0f,mat3(-0.9f,-2.2f,-1.9f,-2.05f,-1.05f,-1.93f,-2.17f,-1.82f,-0.8f));
	TEST(dm3*2.0,dmat3(2.2,-0.4,0.2,-0.1,1.9,0.14,-0.34,0.36,2.4));
	TEST(im3/2,imat3(55,-10,5,-2,47,3,-8,9,60));
	TEST(ldm3+ldM3,ldmat3(2.15,-0.42,-0.01,-0.02,1.92,0.03,-0.03,0.37,2.32));
	TEST(m3- M3,mat3(0.05f,0.02f,0.21f,-0.08f,-0.02f,0.11f,-0.31f,-0.01f,0.08f));
	TEST(MAT_MUL(dm3,dM3),dmat3(1.1847,-0.4388,-0.0424,-0.0087,0.9083,0.0229,-0.0459,0.3541,1.3713));
	//TEST(im3/ iM3,imat3(2,10,-10,-1,2,-1,-4,2,1));
	TEST(MAT_MUL(m3,vec3(1,2,3)),vec3(0.49f,2.24f,3.84f));
	TEST(MAT_MUL(vec3(4,3,2),m3),vec3(4,2.79f,2.26f));
	TEST(2.0l+ldm3,ldmat3(3.1,1.8,2.1,1.95,2.95,2.07,1.83,2.18,3.2));
	TEST(2.0f- m3,mat3(0.9f,2.2f,1.9f,2.05f,1.05f,1.93f,2.17f,1.82f,0.8f));
	TEST(2.0* dm3,dmat3(2.2,-0.4,0.2,-0.1,1.9,0.14,-0.34,0.36,2.4));
	TEST(300/ im3,imat3(2,-15,30,-60,3,42,-17,16,2));
	TEST(ldm3+=.01l,ldmat3(1.11,-0.19,0.11,-0.04,0.96,0.08,-0.16,0.19,1.21));
	TEST(m3-=.01f,mat3(1.09f,-0.21f,0.09f,-0.06f,0.94f,0.06f,-0.18f,0.17f,1.19f));
	TEST(dm3*=1.01,dmat3(1.111,-0.202,0.101,-0.0505,0.9595,0.0707,-0.1717,0.1818,1.212));
	TEST(im3/=2,imat3(55,-10,5,-2,47,3,-8,9,60));
	TEST(ldm3+=ldM3,ldmat3(2.16,-0.41,0,-0.01,1.93,0.04,-0.02,0.38,2.33));
	TEST(m3-= M3,mat3(0.04f,0.01f,0.2f,-0.09f,-0.03f,0.1f,-0.32f,-0.02f,0.07f));
#ifndef D3D_MAT_STYLE
	TEST(dm3*= dM3,dmat3(1.196547,-0.443188,-0.042824,-0.008787,0.917383,0.023129,-0.046359,0.357641,1.385013));
#else
	TEST(dm3=dM3*dm3,dmat3(1.196547,-0.443188,-0.042824,-0.008787,0.917383,0.023129,-0.046359,0.357641,1.385013));
#endif
	TEST(im3=(imat3)matrixCompMult(dmat3(im3),1./dmat3(iM3)),imat3(1,5,-5,0,1,0,-2,1,0));
	TEST(-dm3,dmat3(-1.196547,0.443188,0.042824,0.008787,-0.917383,-0.023129,0.046359,-0.357641,-1.385013));
	TEST(++ldm3,ldmat3(3.16,0.59,1,0.99,2.93,1.04,0.98,1.38,3.33));
	TEST(-- m3,mat3(-0.96f,-0.99f,-0.8f,-1.09f,-1.03f,-0.9f,-1.32f,-1.02f,-0.93f));
	TEST(dm3++,dmat3(1.196547,-0.443188,-0.042824,-0.008787,0.917383,0.023129,-0.046359,0.357641,1.385013));
	TEST(dm3,dmat3(2.196547,0.556812,0.957176,0.991213,1.917383,1.023129,0.953641,1.357641,2.385013));
	TEST(im3--,imat3(1,5,-5,0,1,0,-2,1,0));
	TEST(im3,imat3(0,4,-6,-1,0,-1,-3,0,-1));

	ldmat4 ldm4(1.1,-.2,.1,.15,
				-.05,.95,.07,.2,
				-.17,.18,1.2,-.1,
				.06,-.12,-.13,.9);
	dmat4 dm4(1.1,-.2,.1,.15,
			-.05,.95,.07,.2,
			-.17,.18,1.2,-.1,
			.06,-.12,-.13,.9);
	mat4 m4(1.1f,-.2f,.1f,.15f,
			-.05f,.95f,.07f,.2f,
			-.17f,.18f,1.2f,-.1f,
			.06f,-.12f,-.13f,.9f);
	imat4 im4(110,-20,10,15,
			  -5,95,7,20,
			  -17,18,120,-10,
			  6,-12,-13,90);
	ldmat4 ldM4(1.05,-.22,-.11,-.25,
				.03,.97,-.04,.02,
				.14,.19,1.12,-.21,
				-.16,.1,-.01,.94);
	dmat4 dM4(1.05,-.22,-.11,-.25,
			.03,.97,-.04,.02,
			.14,.19,1.12,-.21,
			-.16,.1,-.01,.94);
	mat4 M4(1.05f,-.22f,-.11f,-.25,
			.03f,.97f,-.04f,.02f,
			.14f,.19f,1.12f,-.21f,
			-.16f,.1f,-.01f,.94f);
	imat4 iM4(55,-2,-1,-5,
			  3,47,-4,2,
			  4,9,66,-2,
			  -6,1,-1,48);
	TEST(imat4(imat4(110,-20,10,15,-5,95,7,20,-17,18,120,-10,6,-12,-13,90)),im4);
	TEST(mat4(12),mat4(12,0,0,0,0,12,0,0,0,0,12,0,0,0,0,12));
	TEST(imat3(imat4(110,-20,10,15,-5,95,7,20,-17,18,120,-10,6,-12,-13,90)),imat3(110,-20,10,-5,95,7,-17,18,120));
	TEST(imat2(imat4(110,-20,10,15,-5,95,7,20,-17,18,120,-10,6,-12,-13,90)),imat2(110,-20,-5,95));
	TEST(m4[3][2],-.13f);
	TEST(mat4(110,-20,10,15,-5,95,7,20,-17,18,120,-10,6,-12,-13,90)!=mat4(110,-20,10,15,-5,95,7,20,-17,18,120,-10,7,-12,-13,90),HMtrue);
	TEST(im4=imat4(110,-20,10,15,-5,95,7,20,-17,18,120,-10,6,-12,-13,90),imat4(110,-20,10,15,-5,95,7,20,-17,18,120,-10,6,-12,-13,90));
	TEST(ldm4+2.0l,ldmat4(3.1,1.8,2.1,2.15,1.95,2.95,2.07,2.2,1.83,2.18,3.2,1.9,2.06,1.88,1.87,2.9));
	TEST(m4-2.0f,mat4(-0.9f,-2.2f,-1.9f,-1.85f,-2.05f,-1.05f,-1.93f,-1.8f,-2.17f,-1.82f,-0.8f,-2.1f,-1.94f,-2.12f,-2.13f,-1.1f));
	TEST(dm4*2.0,dmat4(2.2,-0.4,0.2,0.3,-0.1,1.9,0.14,0.4,-0.34,0.36,2.4,-0.2,0.12,-0.24,-0.26,1.8));
	TEST(im4/2,imat4(55,-10,5,7,-2,47,3,10,-8,9,60,-5,3,-6,-6,45));
	TEST(ldm4+ldM4,ldmat4(2.15,-0.42,-0.01,-0.1,-0.02,1.92,0.03,0.22,-0.03,0.37,2.32,-0.31,-0.1,-0.02,-0.14,1.84));
	TEST(m4- M4,mat4(0.05f,0.02f,0.21f,0.4f,-0.08f,-0.02f,0.11f,0.18f,-0.31f,-0.01f,0.08f,0.11f,0.22f,-0.22f,-0.12f,-0.04f));
	TEST(MAT_MUL(dm4,dM4),dmat4(1.1697,-0.4088,-0.0099,-0.1005,-0.0075,0.9059,0.0203,0.2205,-0.0585,0.3793,1.3986,-0.242,-0.1229,0.0124,-0.1432,0.843));
	//TEST(im4/ iM4,imat4(2,10,-10,-3,-1,2,-1,10,-4,2,1,5,-1,-12,13,1));
	TEST(MAT_MUL(m4,vec4(1,2,3,4)),vec4(0.73f,1.76f,3.32f,3.85f));
	TEST(MAT_MUL(vec4(4,3,2,1),m4),vec4(4.15f,2.99f,2.16f,0.52f));
	TEST(2.0l+ldm4,ldmat4(3.1,1.8,2.1,2.15,1.95,2.95,2.07,2.2,1.83,2.18,3.2,1.9,2.06,1.88,1.87,2.9));
	TEST(2.0f- m4,mat4(0.9f,2.2f,1.9f,1.85f,2.05f,1.05f,1.93f,1.8f,2.17f,1.82f,0.8f,2.1f,1.94f,2.12f,2.13f,1.1f));
	TEST(2.0* dm4,dmat4(2.2,-0.4,0.2,0.3,-0.1,1.9,0.14,0.4,-0.34,0.36,2.4,-0.2,0.12,-0.24,-0.26,1.8));
	TEST(300/ im4,imat4(2,-15,30,20,-60,3,42,15,-17,16,2,-30,50,-25,-23,3));
	TEST(ldm4+=.01l,ldmat4(1.11,-0.19,0.11,0.16,-0.04,0.96,0.08,0.21,-0.16,0.19,1.21,-0.09,0.07,-0.11,-0.12,0.91));
	TEST(m4-=.01f,mat4(1.09f,-0.21f,0.09f,0.14f,-0.06f,0.94f,0.06f,0.19f,-0.18f,0.17f,1.19f,-0.11f,0.05f,-0.13f,-0.14f,0.89f));
	TEST(dm4*=1.01,dmat4(1.111,-0.202,0.101,0.1515,-0.0505,0.9595,0.0707,0.202,-0.1717,0.1818,1.212,-0.101,0.0606,-0.1212,-0.1313,0.909));
	TEST(im4/=2,imat4(55,-10,5,7,-2,47,3,10,-8,9,60,-5,3,-6,-6,45));
	TEST(ldm4+=ldM4,ldmat4(2.16,-0.41,0,-0.09,-0.01,1.93,0.04,0.23,-0.02,0.38,2.33,-0.3,-0.09,-0.01,-0.13,1.85));
	TEST(m4-= M4,mat4(0.04f,0.01f,0.2f,0.39f,-0.09f,-0.03f,0.1f,0.17f,-0.32f,-0.02f,0.07f,0.1f,0.21f,-0.23f,-0.13f,-0.05f));
#ifndef D3D_MAT_STYLE
	TEST(dm4*= dM4,dmat4(1.181397,-0.412888,-0.009999,-0.101505,-0.007575,0.914959,0.020503,0.222705,-0.059085,0.383093,1.412586,-0.24442,-0.124129,0.012524,-0.144632,0.85143));
#else
	TEST(dm4=dM4*dm4,dmat4(1.181397,-0.412888,-0.009999,-0.101505,-0.007575,0.914959,0.020503,0.222705,-0.059085,0.383093,1.412586,-0.24442,-0.124129,0.012524,-0.144632,0.85143));
#endif
	TEST(im4=(imat4)matrixCompMult(dmat4(im4),1./dmat4(iM4)),imat4(1,5,-5,-1,0,1,0,5,-2,1,0,2,0,-6,6,0));
	TEST(-dm4,dmat4(-1.181397,0.412888,0.009999,0.101505,0.007575,-0.914959,-0.020503,-0.222705,0.059085,-0.383093,-1.412586,0.24442,0.124129,-0.012524,0.144632,-0.85143));
	TEST(++ldm4,ldmat4(3.16,0.59,1,0.91,0.99,2.93,1.04,1.23,0.98,1.38,3.33,0.7,0.91,0.99,0.87,2.85));
	TEST(-- m4,mat4(-0.96f,-0.99f,-0.8f,-0.61f,-1.09f,-1.03f,-0.9f,-0.83f,-1.32f,-1.02f,-0.93f,-0.9f,-0.79f,-1.23f,-1.13f,-1.05f));
	TEST(dm4++,dmat4(1.181397,-0.412888,-0.009999,-0.101505,-0.007575,0.914959,0.020503,0.222705,-0.059085,0.383093,1.412586,-0.24442,-0.124129,0.012524,-0.144632,0.85143));
	TEST(dm4,dmat4(2.181397,0.587112,0.990001,0.898495,0.992425,1.914959,1.020503,1.222705,0.940915,1.383093,2.412586,0.75558,0.875871,1.012524,0.855368,1.85143));
	TEST(im4--,imat4(1,5,-5,-1,0,1,0,5,-2,1,0,2,0,-6,6,0));
	TEST(im4,imat4(0,4,-6,-2,-1,0,-1,4,-3,0,-1,1,-1,-7,5,-1));

	cout<<"\n=================Testing \"quat.h\"=================\n\n";

	quat q=axisAngleToQuat(vec3(1,1,1),radians(75.0f));
	TEST(q==quat(0.35147f,0.35147f,0.35147f,0.79335f),HMtrue);
	TEST(q!=quat(0.35247f,0.35147f,0.35147f,0.79335f),HMtrue);
	TEST(q=quat(0.35147f,0.35147f,0.35147f,0.79335f),quat(0.35147f,0.35147f,0.35147f,0.79335f));
	TEST(quat(m3=mat3(q),HMtrue),q);
	TEST(quat(m4=mat4(q),HMtrue),q);
	TEST(normalize(quat(mat3(q),HMfalse)),q);
	TEST(normalize(quat(mat4(q),HMfalse)),q);
	TEST(quat(mat3(q,HMtrue),HMtrue),q);
	TEST(quat(mat4(q,HMtrue),HMtrue),q);
	TEST(q*2.0f,quat(0.70294f,0.70294f,0.70294f,1.5867f));
	TEST(q/2.0f,quat(0.17573f,0.17573f,0.17573f,0.39668f));
	TEST(2.0f*q,quat(0.70294f,0.70294f,0.70294f,1.5867f));
	quat Q=axisAngleToQuat(vec3(1,2,3),radians(145.0f));
	TEST(q+Q,quat(0.60636f,0.86125f,1.1161f,1.0941f));
	TEST(q-Q,quat(0.096577f,-0.15831f,-0.41321f,0.49265f));
	TEST(QUAT_MUL(q,Q),quat(0.39749f,0.33095f,0.80193f,-0.29895f));
	TEST(q*=1.5f,quat(0.5272f,0.5272f,0.5272f,1.19f));
	TEST(Q/=1.5f,quat(0.16993f,0.33986f,0.50978f,0.20047f));
	v3=vec3(3,2,1);
	TEST(QUAT_MUL(normalize(q),v3),vec3(1.7011f,3.1154f,1.1835f));
	TEST(q+=Q,quat(0.69713f,0.86706f,1.037f,1.3905f));
	TEST(Q-=q,quat(-0.5272f,-0.5272f,-0.5272f,-1.19f));
#ifndef QUAT_MUL_REVERSE_ORDER
	TEST(q*=Q,quat(-1.4731f,-1.9441f,-1.8775f,-0.28339f));
#else
	TEST(q=Q*q,quat(-1.4731f,-1.9441f,-1.8775f,-0.28339f));
#endif
	TEST(-q,quat(1.4731f,1.9441f,1.8775f,0.28339f));

	cout<<"\n=================Testing \"aabb.h\"=================\n\n";

	aabb b;
	vec3 vert[3]={vec3(2,1,-1),vec3(4,2,3),vec3(3,0,-2)};
	TEST(aabb(2.5f),aabb(vec3(-2.5f,-2.5f,-2.5f),vec3(2.5f,2.5f,2.5f)));
	TEST(aabb(-.5f,3),aabb(vec3(-0.5f,-0.5f,-0.5f),vec3(3,3,3)));
	TEST(b=aabb(3,vert),aabb(vec3(2,0,-2),vec3(4,2,3)));
	TEST(b|vec3(3,3,3),aabb(vec3(2,0,-2),vec3(4,3,3)));
	TEST(vec3(3,3,3)|b,aabb(vec3(2,0,-2),vec3(4,3,3)));
	TEST(aabb(0)|vec3(3,3,3),aabb(vec3(3,3,3),vec3(3,3,3)));
	TEST(vec3(3,3,3)|aabb(0),aabb(vec3(3,3,3),vec3(3,3,3)));
	TEST(aabb(0)|b,aabb(vec3(2,0,-2),vec3(4,2,3)));
	TEST(b|aabb(1),aabb(vec3(-1,-1,-2),vec3(4,2,3)));
	TEST(b*=vec3(.5f,2,1),aabb(vec3(1,0,-2),vec3(2,4,3)));
	TEST(b/=vec3(.5f,2,1),aabb(vec3(2,0,-2),vec3(4,2,3)));
	TEST(b=aabb(vec3(1,2,3),vec3(4,5,6))+vec3(3,2,1),aabb(vec3(4,4,4),vec3(7,7,7)));
	irect ir=irect(ivec2(-3,-2),ivec2(-1,-1));
	TEST(ir&irect(0),irect(ivec2(2147483647,2147483647),ivec2(-2147483647,-2147483647)));
	TEST(ir&irect(1),irect(ivec2(-1,-1),ivec2(-1,-1)));
	TEST(ir&irect(2),irect(ivec2(-2,-2),ivec2(-1,-1)));
	TEST(ir&=irect(2),irect(ivec2(-2,-2),ivec2(-1,-1)));
	TEST(ir&=irect(1),irect(ivec2(-1,-1),ivec2(-1,-1)));
	TEST(ir|=ivec2(2,3),irect(ivec2(-1,-1),ivec2(2,3)));
	TEST(ir|=irect(1),irect(ivec2(-1,-1),ivec2(2,3)));
	TEST(ir*5,irect(ivec2(-5,-5),ivec2(10,15)));
	TEST(ir*=5,irect(ivec2(-5,-5),ivec2(10,15)));
	TEST(ir/5,irect(ivec2(-1,-1),ivec2(2,3)));
	TEST(ir/=5,irect(ivec2(-1,-1),ivec2(2,3)));
	drange dr=drange(-3.5,1.25);
	TEST(dr&drange(0),drange(1.7976931348623158e+308,-1.7976931348623158e+308));
	TEST(dr&drange(1),drange(-1,1));
	TEST(dr|drange(-2,2),drange(-3.5,2));
	TEST(dr&drange(2),drange(-2,1.25));
	TEST(dr&=drange(2),drange(-2,1.25));
	TEST(dr&=drange(1),drange(-1,1));
	TEST(dr|=2,drange(-1,2));
	TEST(dr|=drange(0),drange(-1,2));
	TEST(dr|=drange(2),drange(-2,2));
	TEST(dr*=1.5,drange(-3,3));
	TEST(dr*2.,drange(-6,6));
	TEST(dr/3.,drange(-1,1));
	TEST(dr/=2.,drange(-1.5,1.5));

	cout<<"\n=================Testing \"func.h\"=================\n\n";

	v2=vec2(1,2);
	TEST(vec_cast<ivec2>(v2),ivec2(1,2));
	ivec3 iv3=ivec3(1,2,3);
	TEST(vec_cast<ldvec3>(iv3),ldvec3(1,2,3));
	dvec4 dv4=dvec4(1,2,3,4);
	TEST(vec_cast<vec4>(dv4),vec4(1,2,3,4));
	bv2=bvec2(HMfalse,HMtrue);
	bv3=bvec3(HMfalse,HMtrue,HMfalse);
	bv4=bvec4(HMfalse,HMtrue,HMfalse,HMtrue);
	TEST(vec_cast<ldvec2>(bv2),ldvec2(0,1));
	TEST(vec_cast<vec3>(bv3),vec3(0,1,0));
	TEST(vec_cast<ivec4>(bv4),ivec4(0,1,0,1));
	TEST(mat_cast<ldmat2>(mat2(1.5)),ldmat2(1.5));
	TEST(mat_cast<imat3>(dmat3(2.7)),imat3(2));
	TEST(mat_cast<mat4>(imat4(-7)),mat4(-7));
	TEST(quat_cast<dquat>(quat(4,-2,1,3)),dquat(4,-2,1,3));
	TEST(aabb_cast<aabb>(daabb(1.5)),aabb(1.5f));
	TEST(rect_cast<srect>(rect(23.4f,30.7f)),srect(23,30));
	TEST(range_cast<irange>(ldrange(-.5l,4.3l)),irange(0,4));
	TEST(equal(radians(180.0f),3.14159f),HMtrue);
	TEST(equal(degrees(1.57),89.95437383553),HMtrue);
	TEST(inversesqrt(4.0),0.5);
/*	rseed(123);
	TEST(random(20)!=random(20) && random(20)!=random(20),true);
	TEST(random(2,20)!=random(2,20) && random(2,20)!=random(2,20),true);
	TEST(random(10.0)!=random(10.0) && random(10.0)!=random(10.0),true);
	TEST(random(5.0,10.0)!=random(5.0,10.0) && random(5.0,10.0)!=random(5.0,10.0),true);*/
	TEST(square(-23),529);
	TEST(cube(-12.0l),-1728.0l);
	TEST(abs(dvec3(-1.5,2.4,-3.)),dvec3(1.5,2.4,3.));
	TEST(equal(sign(-.8l),-1.l),HMtrue);
	TEST(equal(sign(0.01),1.),HMtrue);
	TEST(equal(sign(0.f),0.f),HMtrue);
	TEST(equal(fract(-.9f),0.1f),HMtrue);
	TEST(mod(400.0,360.0),40);
	TEST(wrap(-40.0f,360.0f),320);
	TEST(wrap(220,100,200),120);
	TEST(min(3.0,-2.0),-2);
	TEST(max(8,10),10);
	TEST(min(vec3(1,2,3)),1);
	TEST(max(ivec3(1,2,3)),3);
	TEST(mincw(dvec3(1,2,3),dvec3(3,2,1)),dvec3(1,2,1));
	TEST(maxcw(ldvec3(1,2,3),ldvec3(3,2,1)),ldvec3(3,2,3));
	TEST(clamp(-1,4,6),4);
	TEST(clamp(5.0,4.0,6.0),5);
	TEST(clamp(7.0f,4.0f,6.0f),6);
	TEST(mix(-1.,7.,.25),1.);
	TEST(lerp(vec2(1,3),vec2(3,0),.5f),vec2(2,1.5f));
	TEST(lerp(vec2(1,3),vec2(3,0),.75f),vec2(2.5f,0.75f));
	TEST(cerp(vec2(1,3),vec2(3,0),.75f),vec2(2.70711f,0.43934f));
	TEST(herp3(vec2(1,3),vec2(3,0),.75f),vec2(2.6875f,0.46875f));
	TEST(herp5(vec2(1,3),vec2(3,0),.75f),vec2(2.79297f,0.310547f));
//	TEST(step(1,2),1);
//	TEST(step(3,3),1);
//	TEST(step(4,0),0);
	TEST(equal(smoothstep(10.0,20.0,12.0),0.104),HMtrue);
	TEST(equal(smoothstep(10.0,20.0,2.0),0.0),HMtrue);
	TEST(equal(smoothstep(10.0,20.0,22.0),1.0),HMtrue);
	TEST(equal(dot(vec2(.5f,3.f),vec2(2.f,.5f)),2.5f),HMtrue);
	TEST(equal(sqlen(dvec3(1,2,3)),14.0),HMtrue);
	TEST(equal(length(ldvec2(3,4)),5.0l),HMtrue);
	TEST(equal(distance(vec4(1,2,3,4),vec4(4,3,2,1)),4.47214f),HMtrue);
	TEST(cross(ivec3(1,0,4),ivec3(-1,3,2)),ivec3(-12,-6,3));
	TEST(normalize(vec4(2,2,0,1)),vec4(0.66667f,0.66667f,0,0.33333f));
	TEST(isZero(dot(perp(vec2(1,0)),vec2(1,0))),HMtrue);
	TEST(isZero(dot(perp(vec2(0,-2)),vec2(0,-2))),HMtrue);
	TEST(isZero(dot(perp(vec2(1.5,3)),vec2(1.5,3))),HMtrue);
	TEST(isZero(dot(perp(vec3(-1,0,0)),vec3(-1,0,0))),HMtrue);
	TEST(isZero(dot(perp(vec3(0,1,0)),vec3(0,1,0))),HMtrue);
	TEST(isZero(dot(perp(vec3(0,0,1)),vec3(0,0,1))),HMtrue);
	TEST(isZero(dot(perp(vec3(0,0,-2)),vec3(0,0,-2))),HMtrue);
	TEST(isZero(dot(perp(vec3(-1,1,2)),vec3(-1,1,2))),HMtrue);
	TEST(isZero(dot(perp(vec3(2,1,-3)),vec3(2,1,-3))),HMtrue);
	TEST(slerp(normalize(vec3(0,1,1)),normalize(vec3(2,0,0)),.5f),vec3(0.707107f,0.5f,0.5f));
	TEST(faceforward(vec3(0,0,1),vec3(-1,-1,-1),vec3(1,2,3)),vec3(0,0,1));
	TEST(reflect(vec2(2,2),vec2(1,0)),vec2(-2,2));
	TEST(reflect(vec3(2,2,2),vec3(0,1,0)),vec3(2,-2,2));
	TEST(reflect(vec3(1,.5f,3),unitPlane(vec3(1,1,-1),vec3(-2,.7f,1))),vec3(0.46667f,-0.033333f,3.5333f));
	TEST(reflect(vec4(2,1,6,2),unitPlane(vec3(1,1,-1),vec3(-2,.7f,1))),vec4(0.93333f,-0.066667f,7.0667f,2));
	TEST(refract(normalize(vec3(1,.5,3)),normalize(vec3(1,1,-1)),1.5f),vec3(0,0,0));
	TEST(matrixCompMult(imat2(2,3,-2,1),imat2(4,-1,-3,2)),imat2(8,-3,6,2));
	m2=mat2(1)+.1f; m2[0][1]-=.2f;
	TEST(inverse(m2)*m2,mat2(1));
	TEST(inverse(inverse(m2)),m2);
	TEST(transpose(m2),mat2(1.1f,0.1f,-0.1f,1.1f));
	TEST(transpose(transpose(m2)),m2);
	m3=mat3(1)+.1f; m3[0][1]-=.2f;
	TEST(inverse(m3)*m3,mat3(1));
	TEST(inverse(inverse(m3)),m3);
	TEST(transpose(m3),mat3(1.1f,0.1f,0.1f,-0.1f,1.1f,0.1f,0.1f,0.1f,1.1f));
	TEST(transpose(transpose(m3)),m3);
	m4=mat4(1)+.1f; m4[2][1]-=.2f;
	TEST(inverse(m4)*m4,mat4(1));
	TEST(inverse(inverse(m4)),m4);
	TEST(transpose(m4),mat4(1.1f,0.1f,0.1f,0.1f,0.1f,1.1f,-0.1f,0.1f,0.1f,0.1f,1.1f,0.1f,0.1f,0.1f,0.1f,1.1f));
	TEST(transpose(transpose(m4)),m4);
	TEST(orthonormalize(m3),mat3(0.99184f,-0.090167f,0.090167f,0.099015f,0.99015f,-0.099015f,-0.080351f,0.10713f,0.99099f));
	v3=vec3(1,-2,4);
	TEST(scalingMat4(vec3(7,8,9))*vec4(v3,1.0f),vec4(vec3(7,8,9)*v3,1.0f));
	TEST(scalingMat3(vec3(7,8,9))*v3,vec3(7,8,9)*v3);
#ifndef D3D_MAT_STYLE
	TEST(translationMat4(vec3(7,8,9))*vec4(v3,1.0f),vec4(vec3(7,8,9)+v3,1.0f));
	TEST(crossMat3(vec3(7,8,9))*v3,cross(vec3(7,8,9),v3));
	TEST(mat3(crossMat4(vec3(7,8,9)))*v3,cross(vec3(7,8,9),v3));
	v2=vec2(v3);
	TEST(scalingMat3(vec2(7,8))*vec3(v2,1.0f),vec3(vec2(7,8)*v2,1.0f));
	TEST(scalingMat2(vec2(7,8))*v2,vec2(7,8)*v2);
	TEST(translationMat3(vec2(7,8))*vec3(v2,1.0f),vec3(vec2(7,8)+v2,1.0f));
	TEST(rotationXMat(.345f)*v3,rotateX(.345f,v3));
	TEST(rotationYMat(.34f)*v3,rotateY(.34f,v3));
	TEST(rotationZMat(.3f)*v3,rotateZ(.3f,v3));
#else
	TEST(vec4(v3,1.0f)*translationMat4(vec3(7,8,9)),vec4(vec3(7,8,9)+v3,1.0f));
	TEST(v3*crossMat3(vec3(7,8,9)),cross(vec3(7,8,9),v3));
	TEST(v3*mat3(crossMat4(vec3(7,8,9))),cross(vec3(7,8,9),v3));
	v2=vec2(v3);
	TEST(vec3(v2,1.0f)*scalingMat3(vec2(7,8)),vec3(vec2(7,8)*v2,1.0f));
	TEST(v2*scalingMat2(vec2(7,8)),vec2(7,8)*v2);
	TEST(vec3(v2,1.0f)*translationMat3(vec2(7,8)),vec3(vec2(7,8)+v2,1.0f));
	TEST(v3*rotationXMat(.345f),rotateX(.345f,v3));
	TEST(v3*rotationYMat(.34f),rotateY(.34f,v3));
	TEST(v3*rotationZMat(.3f),rotateZ(.3f,v3));
#endif
	TEST(rotationXMat4(.345f),rotationMat4(.345f,vec3(1,0,0)));
	TEST(rotationYMat3(.34f),rotationMat3(.34f,vec3(0,1,0)));
	TEST(rotationZMat(.3f),rotationMat(.3f,vec3(0,0,1)));
	vec4 pl=unitPlane(vec3(1,2,3),vec3(3,2,1)); v4=vec4(10,20,30,40);
	TEST(reflect(v4,pl),MAT_MUL(reflectionMat(pl),v4));
	TEST(project(v4,pl),MAT_MUL(projectionMat(pl),v4));
	v3=vec3(v4);
	TEST(vec4(reflect(v3,pl),1),MAT_MUL(reflectionMat(pl),vec4(v3,1)));
	TEST(vec4(project(v3,pl),1),MAT_MUL(projectionMat(pl),vec4(v3,1)));
	TEST(any(bvec2(HMtrue,HMfalse)),HMtrue);
	TEST(any(bvec3(HMfalse,HMfalse,HMfalse)),HMfalse);
	TEST(any(bvec4(HMfalse,HMfalse,HMfalse,HMtrue)),HMtrue);
	TEST(all(bvec2(HMtrue,HMfalse)),HMfalse);
	TEST(all(bvec3(HMtrue,HMtrue,HMtrue)),HMtrue);
	TEST(all(bvec4(HMtrue,HMtrue,HMfalse,HMtrue)),HMfalse);
	TEST(not(bvec3(HMtrue,HMtrue,HMfalse)),bvec3(HMfalse,HMfalse,HMtrue));
	TEST(planeNormal(vec3(1,2,3),vec3(-3,4,.5f),vec3(-2,.7f,1)),vec3(-7.25f,-0.5f,11.2f));
	TEST(v4=plane(vec3(1,2,3),vec3(-3,4,.5f),vec3(-2,.7f,1)),vec4(-7.25f,-0.5f,11.2f,-25.35f));
	TEST(plane(planeNormal(vec3(1,2,3),vec3(-3,4,.5),vec3(-2,.7f,1)),vec3(1,2,3)),vec4(-7.25f,-0.5f,11.2f,-25.35f));
	TEST(unitPlane(vec3(1,2,3),vec3(-3,4,.5f),vec3(-2,.7f,1)),vec4(-0.54303f,-0.03745f,0.83888f,-1.8987f));
	TEST(unitPlane(planeNormal(vec3(1,2,3),vec3(-3,4,.5f),vec3(-2,.7f,1)),vec3(1,2,3)),vec4(-0.54303f,-0.03745f,0.83888f,-1.8987f));
	TEST(normalizePlane(v4),vec4(-0.54303f,-0.03745f,0.83888f,-1.8987f));
	TEST(translatePlane(v4,vec3(1,-1,2)),vec4(-7.25f,-0.5f,11.2f,-41));
	TEST(planePoint(v4),vec3(-1.0311f,-0.071107f,1.5928f));
	TEST(unitPlanePoint(normalizePlane(v4)),vec3(-1.0311f,-0.071107f,1.5928f));
	TEST(reflectPlane(vec4(2,1,6,2),normalizePlane(v4)),vec4(6.24622f,1.29284f,-0.559683f,16.8471f));
	TEST(equal(planeDist(v4,vec3(6,8,10)),2.9323f),HMtrue);
	TEST(equal(planeDist(v4,vec4(3,4,5,.5f)),2.9323f),HMtrue);
	TEST(isZero(planeDist(v4,planePoint(v4))),HMtrue);
	TEST(notZero(planeDist(v4,unitPlanePoint(normalizePlane(v4)))),HMfalse);
	TEST(equal(unitPlaneDist(normalizePlane(v4),vec3(6,8,10)),2.9323f),HMtrue);
	TEST(equal(unitPlaneDist(normalizePlane(v4),vec4(3,4,5,.5)),2.9323f),HMtrue);
	TEST(onPlane(v4,vec3(-3,4,.5)),HMtrue);
	TEST(rayPlaneIntn(v3,vec3(0,1,1),vec3(-1,1,2),v4)==HMtrue && v3==vec3(-0.50257f,1.5026f,2.0051f),true);
	TEST(segPlaneIntn(v3,vec3(0,1,1),vec3(-1,2,3),v4)==HMtrue && v3==vec3(-0.50257f,1.5026f,2.0051f),true);
	TEST(lookAtDirnMat(vec3(10,-20,40),v3,vec3(0,0,1)),mat4(0.94836f,0.24888f,0.19666f,0,0.3172f,-0.7441f,-0.58796f,0,0,0.61998f,-0.78462f,0,-3.1395f,-42.17f,17.659f,1));
	TEST(orthoMatGL(-10.,100.,-20.,200.,0.),dmat4(0.018181818181818,0,0,0,0,0.009090909090909,0,0,0,0,-2,0,-0.8181818181818,-0.818181818181818,-1,1));
	TEST(frustumMatGL<float>(-100,50,-120,200,1,100),mat4(0.0133333f,0,0,0,0,0.00625f,0,0,-0.333333f,0.25f,-1.0202f,-1,0,0,-2.0202f,0));
	TEST(perspectiveMatGL(radians(50.f),1.5f,.1f,50.f),mat4(1.42967124f,0,0,0,0,2.14450687f,0,0,0,0,-1.00400802f,-1,0,0,-0.200400802f,0));
	TEST(shortestArcQuat(vec3(1,2,3),vec3(-3,4,.5f)),quat(-0.35662061f,-0.30799049f,0.32420054f,0.82027900f));
	TEST(normalize(shortestArcQuat(vec3(1,2,3),vec3(-3,4,.5f))),quat(-0.35662061f,-0.30799049f,0.32420054f,0.82027900f));
	q=axisAngleToQuat(vec3(1,1,1),radians(75.0f));
	vec3 r=rotate(radians(75.0f),q.xyz(),v3=vec3(2,-.4f,.9f));
	TEST(MAT_MUL(rotationMat(radians(75.0f),q.xyz()),v3), r);
	TEST(QUAT_MUL(q,v3),r);
	TEST(QUAT_MUL(QUAT_MUL(q,quat(v3)),conjugate(q)).xyz(),r);
	v4=vec4(3,2,1,.5f);
	TEST(equal(quatToAxisAngle(v3,axisAngleToQuat(v4.xyz(),v4.w)),v4.w) && v3==normalize(v4.xyz()),HMtrue);
	TEST(normalize(quatToAxisAngle(axisAngleToQuat(vec3(v4),v4.w))),normalize(vec3(v4)));
	TEST(equal(quatAngle(axisAngleToQuat(v4.xyz(),v4.w)),v4.w),HMtrue);
	TEST(slerp(axisAngleToQuat(vec3(1,-2,3),radians(65.f)),axisAngleToQuat(vec3(-2,.5,1),radians(190.f)),.5f),quat(0.69144261f,-0.34436423f,-0.0027140975f,0.63506925f));
	quat dq[2], dq2[2];
	vec3 t(1,-1,2), p(-2,.5f,1.5f);
	quatTransToDualQuat(dq, q, t);
	TEST(dualQuatGetTrans(dq), t);
	TEST(vec4(dualQuatTransformPoint (dq,p),1), MAT_MUL(dualQuatToMat(dq),vec4(p,1)));
	TEST(vec4(dualQuatTransformVector(dq,p),0), MAT_MUL(dualQuatToMat(dq),vec4(p,0)));
	inverse(dq2, dq);
	dualQuatMul(dq2, dq, dq2);
	TEST(dq2[0], quat(1.f));
	TEST(dq2[1], quat(0.f));
	matToDualQuat(dq2, dualQuatToMat(dq));
	TEST(dq[0], dq2[0]);
	TEST(dq[1], dq2[1]);

	cout<<"\n============Testing Noise Functions============\n\n";

	extern HMint testNoiseRect(HMint minX,HMint maxX,HMint minY,HMint maxY,HMint step);
	TEST(testNoiseRect(-30,-4,10,120,1),12);
	TEST(testNoiseRect(10,94,-40,150,2),10);
	TEST(testNoiseRect(0,1024,0,2048,16),2);
	TEST(noise2(vec4(2.1f,7.8f,-4.3f,1.4f)),vec2(0.443738f,-0.152736f));
	int i,j;
	HMfloat avg=0;
#define NOISE_SIZE 512
	for (i=0;i<NOISE_SIZE;i++)
		for (j=0;j<NOISE_SIZE;j++)
			avg+=noise1(vec2(i*NOISE_SIZE/(j+1),j));
	TEST(equal(avg/(NOISE_SIZE*NOISE_SIZE),-0.000898521f),HMtrue);
	avg=0;
	for (i=0;i<6;i++)
		for (j=0;j<5;j++)
			for (int k=0;k<7;k++)
				for (int l=0;l<4;l++)
					avg+=noise1(vec4(i,j,k,l));
	TEST(equal(avg/(6*5*7*4),0.00614244f),HMtrue);

	extern void tga_write(const char *fname,HMint width,HMint height,HMuint c,HMubyte *pixels);

	static ubvec3 pixelsRGB[NOISE_SIZE][NOISE_SIZE];
	static HMfloat perlinNoise[NOISE_SIZE][NOISE_SIZE];
	static HMubyte pixelsA[NOISE_SIZE][NOISE_SIZE];
	ivec3 avgubv=ivec3(0);
	//HMuint avgub=0;
	for (i=0;i<NOISE_SIZE;i++)
		for (j=0;j<NOISE_SIZE;j++)
		{
			//pixelsRGB[i][j]=vec_cast<ubvec3>((noise3(vec2(i/100000.0+.65,j/20000.0+.37))+1.f)*.5f*255.f);
//			pixelsRGB[i][j]=vec_cast<ubvec3>((noise3(vec2(i/(float)NOISE_SIZE*4.-2.,j/(float)NOISE_SIZE*4.-2.),lerp)+1.f)*.5f*255.f);
			pixelsRGB[i][j]=vec_cast<ubvec3>(noise3(ivec2(i-NOISE_SIZE/2,j-NOISE_SIZE/2)));
			avgubv+=vec_cast<ivec3>(pixelsRGB[i][j]);
			//pixelsA[i][j]=(noise1(vec4(i/10000.0+.95,.2,j/300000.0+.13,.77))+1.f)*.5f*255.f;
			//pixelsA[i][j]=(noise1(dvec4(i/(float)NOISE_SIZE*40.,.2,j/(float)NOISE_SIZE*40.,.77))+1.f)*.5f*255.f;
			//avgub+=pixelsA[i][j];
		}

	TEST(sqlen(avgubv/sizeof(pixelsA)-ivec3(127)),0);
	//TEST(avgub/sizeof(pixelsA),127);
	tga_write("noiseRGB.tga",NOISE_SIZE,NOISE_SIZE,3,(HMubyte*)pixelsRGB[0]);
	cout<<"RGB Noise was written into a file \"noiseRGB.tga\"\n";

	HMfloat k,l;
	for (i=0,k=NOISE_SIZE;i<NOISE_SIZE;i++,k+=4.f/NOISE_SIZE)
		for (j=0,l=0;j<NOISE_SIZE;j++,l+=4.f/NOISE_SIZE)
			perlinNoise[i][j]=noise1(vec2(k,l),cerp);
	for (i=0/*,k=0*/;i<NOISE_SIZE;i++,k+=16.f/NOISE_SIZE)
		for (j=0,l=0;j<NOISE_SIZE;j++,l+=16.f/NOISE_SIZE)
			perlinNoise[i][j]+=.5f*noise1(vec2(k,l),herp3);
	for (i=0/*,k=0*/;i<NOISE_SIZE;i++,k+=64.f/NOISE_SIZE)
		for (j=0,l=0;j<NOISE_SIZE;j++,l+=64.f/NOISE_SIZE)
			perlinNoise[i][j]+=.25f*noise1(vec2(k,l),lerp);
	for (i=0;i<NOISE_SIZE;i++)
		for (j=0;j<NOISE_SIZE;j++)
			perlinNoise[i][j]+=.125f*((HMuint)noise1(ivec2(i,j))*(2.f/hm_limits<HMuint>::max())-1.f);

	for (i=0;i<NOISE_SIZE;i++)
		for (j=0;j<NOISE_SIZE;j++)
			pixelsA[i][j]=HMubyte((perlinNoise[i][j]/3.75f+.5f)*255.f);

	tga_write("noiseA.tga",NOISE_SIZE,NOISE_SIZE,1,pixelsA[0]);
	cout<<"Perlin Noise was written into a file \"noiseA.tga\"\n";


	cout<<"\nTotal tests: "<<totalTests;

	if (totalBugs) cout<<"\nTotal bugs: "<<totalBugs<<endl;
	else           cout<<"\nStrange as it may seem, but it's working! :-)\n";

	return 0;
}

typedef unsigned char  BYTE;
typedef unsigned short WORD;

#pragma pack(1)
struct TGA_Header
{
	BYTE IDLength;
	BYTE ColorMapType;
	BYTE ImageType;
	WORD ColorMapStart;
	WORD ColorMapLength;
	BYTE ColorMapDepth;
	WORD XOffset;
	WORD YOffset;
	WORD Width;
	WORD Height;
	BYTE Depth;
	BYTE ImageDescriptor;
};

#define TARGA_EMPTY_IMAGE			0
#define TARGA_COLORMAP				1
#define TARGA_TRUECOLOR_IMAGE		2
#define TARGA_BW_IMAGE				3
#define TARGA_COLORMAP_RLE_IMAGE	9
#define TARGA_TRUECOLOR_RLE_IMAGE	10
#define TARGA_BW_RLE_IMAGE			11

void tga_write(const char *fname,HMint width,HMint height,HMuint c,HMubyte *pixels)
{
	TGA_Header header={0};
	header.ImageType=(c==1) ? TARGA_BW_IMAGE : TARGA_TRUECOLOR_IMAGE;
	header.Width=width;
	header.Height=height;
	header.Depth=8*c;

	FILE *f=fopen(fname,"wb");
	fwrite(&header,sizeof(TGA_Header),1,f);
	fwrite(pixels,width*c,height,f);
	fclose(f);
}

HMint testNoiseRect(HMint minX,HMint maxX,HMint minY,HMint maxY,HMint step)
{
	ivec4 sum=ivec4(0);
	for (HMint x=minX;x<maxX;x+=step)
		for (HMint y=minY;y<maxY;y+=step)
			sum+=vec_cast<ivec4>(vec_cast<Tvec<HMuint,4> >(noise4(ivec2(x,y)))/16777216U);
	return sqlen((sum*step*step)/((maxX-minX)*(maxY-minY))-ivec4(127));
}
