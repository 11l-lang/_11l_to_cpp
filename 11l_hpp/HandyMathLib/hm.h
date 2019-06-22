/*
===================================================================================================
================================================ENG================================================
===================================================================================================

Handy Math Library
Copyright (C) 2005-2019  Alexander Tretyak

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA



BRIEF DOCUMENTATION:


Configuring Library:

Initially, library represents matrices in column major order.
To change this behaviour define some of following constants before including "hm.h":

CONSTANT                         DESCRIPTION
==============================   ==================================================================
#define D3D_MAT_STYLE            Represents matrices in row major order. But because of particular
                                 treatment of matrices in D3D it affects just on matrix-matrix,
                                 matrix-vector and vector-matrix multiplications.
#define QUAT_MUL_REVERSE_ORDER   If this constant is defined, multiplications of quaternions and
                                 quaternion by 3D-vector are performed in inverse order, so
                                 for the multiply operator the left argument of expression should
                                 be a 3D-vector and the right should be a quaternion.


Supported Types:

TvecN - vector,

  where T - type of vector's components:

    T       Corresponding HM type
    --      ---------------------
    b       HMbool
    i       HMint
    d       HMdouble
    ld      HMlong_double
    s       HMshort
    us      HMushort
    ub      HMubyte

  If T is missing, then vector's components are of type HMfloat.

  N represent the dimension of the vector.

  The names of the components of a vector are denoted by a single letter. As a notational
  convenience, several letters are associated with each component based on common usage of
  position, color or texture coordinate vectors. The individual components of a vector can be
  selected by following the variable name with period ( . ) and then the component name.
  The component names supported are:
  {x, y, z, w} useful when accessing vectors that represent points or normals
  {r, g, b, a} useful when accessing vectors that represent colors
  {s, t, p, q} useful when accessing vectors that represent texture coordinates
  The component names x, r, and s are, for example, synonyms for the same (first) component in a
  vector.
  Note that the third component of a texture, r in OpenGL, has been renamed p so as to avoid the
  confusion with r (for red) in a color.
  The component selection syntax allows multiple components to be selected by appending their names
  (from the same name set) with parentheses after the period ( . ). Example: vec4 v4; v4.yzw();
  Array subscripting syntax can also be applied to vectors to provide numeric indexing. So in
  pos[2] refers to the third element of pos and is equivalent to pos.z.

TmatN or TmatCxR - matrix, where T (one of 'i','d' and 'ld') has the same meaning that in TvecN,
  C is the number of columns, R is the number of rows.
  TmatN represent square matrix with number of columns and rows both equal to N.

  If T is missing, then matrix's components are of type HMfloat.

  The components of a matrix can be accessed using array subscripting syntax. Applying a single
  subscript to a matrix treats the matrix as an array of column vectors, and selects a single
  column, whose type is a vector of the same size as the matrix. The leftmost column is column 0.
  A second subscript would then operate on the column vector, as defined earlier for vectors.
  Hence, two subscripts select a column and then a row.

Tquat - quaternion, where T (either 'd' or 'ld') has the same meaning that in Tvec4.

  If T is missing, then quaternion's components are of type HMfloat.

  The component names of a quaternion supported are x, y, z, and w. Also .xyz() can be used to
  select first 3 components of the quaternion (i.e. vector of quaternion) as a separate 3D-vector.

Taabb - axis aligned bounding box, where T (one of 'i','s','d' and 'ld') has the same meaning that
        in Tvec3.

  If T is missing, then box's components are of type HMfloat.

  Box consists of two bounding 3D-vectors of corresponding type: min and max. Each of them can be
  accessed by appending its name after the period ( . ). Example: aabb b; b.min;

Trect - rectangle, where T (one of 'i','s','d' and 'ld') has the same meaning that in Tvec2.

  If T is missing, then rectangle's components are of type HMfloat.

  Rectangle consists of two bounding 2D-vectors of corresponding type: min and max. Each of them
  can be accessed by appending its name after the period ( . ). Example: rect r; r.max;

Trange - range, where T (one of 'i','s','d' and 'ld') has the same meaning that in Tvec2.

  If T is missing, then range's components are of type HMfloat.

  Range consists of two bounding values of corresponding type: min and max. Each of them
  can be accessed by appending its name after the period ( . ). Example: range r; r.min;


Constructors:

  Constructors can be used to create vectors, matrices, or quaternions from a set of scalars,
vectors, matrices, or quaternions.
  If there is a single scalar parameter to a vector constructor, it is used to initialize all
components of the constructed vector to that scalar's value. If there is a single scalar parameter
to a matrix constructor, it is used to initialize all the components on the matrix's diagonal, with
the remaining components initialized to 0.0. If there is a single vector parameter to a matrix
constructor, it is used to initialize all the components on the matrix's diagonal, with the
remaining components initialized to 0.0. If there is a single scalar parameter to a quaternion
constructor, it is used to initialize the scalar of quaternion (i.e. value of component w), with
the remaining components (x,y,z) initialized to 0.0. If there is a single 3D-vector parameter to a
quaternion constructor, it is used to initialize the vector of quaternion (i.e. first 3 components
of quaternion), with the remaining component w initialized to 0.0. If there are 4 scalar parameters
to a quaternion constructor, they are used to initialize all the components of the quaternion. If
there is a single scalar parameter to an aabb/rect constructor it is used to initialize vector min
to the negative of that scalar's value and max to that scalar's value; but if scalar's value is
zero, constructor creates a null aabb/rect, i.e. such aabb/rect that contains empty volume. If
there are 2 scalar or 2 vector parameters to an aabb/rect constructor, vector construction rules
are used to initialize vector min to the first parameter and vector max to the second parameter.
  Vectors can be constructed from multiple vector parameters, and/or multiple scalar parameters,
than they will be assigned in order, from left to right, to the components of the constructed
vector. In this case, there must be enough (but no more) components provided in the parameters to
provide an initializer for every component in the constructed vector. Matrices will be constructed
in column major order, and can be constructed from multiple scalar parameters with number of
matrix's components or from multiple vector parameters. In last case, number of vectors to
initialize the matrix and their dimension must be equal to the dimension of the constructed matrix.
  Matrices can be constructed from other matrices. If matrix is constructed from greater dimension
matrix, then its submatrix starting with first element is used to initialize all components of the
constructed matrix. If matrix is constructed from less dimension matrix, then it is used to
initialize submatrix of the constructed matrix starting with first element. The remaining
components initialized to 1.0 if they are on the matrix's diagonal, and 0.0 otherwise.
  Also matrices can be constructed by the rules used to construct matrices with less dimension,
i.e. 4D-matrix can be constructed from two 2D-vectors, or three 3D-vectors, or 9 scalar parameters
(as to construct 3D-matrix) and etc. In this case, rules of matrix construction from less dimension
matrix are used.
  Remaining allowable constructors are listed below:

CONSTRUCTOR                     DESCRIPTION
=============================   ===================================================================
mat3(quat q,                    Constructs a 3D-matrix from quaternion q. If quatIsUnit==true,
     bool quatIsUnit=true)      q must be unit quaternion.

mat4(quat q,                    Constructs a 4D-matrix from quaternion q. If quatIsUnit==true,
     bool quatIsUnit=true)      q must be unit quaternion.

quat(mat3 M,                    Constructs a quaternion from the 3D-matrix M. If quatIsUnit==true
     bool quatIsUnit=true)      constructed quaternion is unit. Matrix must contain only rotation
                                transform in order to achieve the desired result.

quat(mat4 M,                    Constructs a quaternion from the 4D-matrix M. If quatIsUnit==true
     bool quatIsUnit=true)      constructed quaternion is unit. Matrix must contain only rotation
                                transform in order to achieve the desired result.

aabb(uint count,                Constructs an aabb that bounds count points from array vertices[].
     vec3 vertices[])

rect(uint count,                Constructs a rectangle that bounds count points from array
     vec2 vertices[])           vertices[].

range(uint count,               Constructs a range that bounds count values from array vertices[].
      float vertices[])


Operations:

  Assignments of values to variable names are done with the assignment operator ( = ), like
  lvalue = expression
  The assignment operator stores the value of expression into lvalue. Expression and lvalue must
have the same type. All desired type-conversions must be specified explicitly via a constructor.
  Other assignment operators are the arithmetic assignments add into (+=), subtract from (-=),
multiply into (*=), and divide into (/=). The expression "lvalue op= expression" is equivalent to
"lvalue = lvalue op expression" and the lvalue and expression must satisfy the semantic
requirements of both op and equals (=).
  The arithmetic binary operators supported are add (+), subtract (-), multiply (*), and
divide (/). The two operands must be the same type, or one can be a scalar and the other a vector,
matrix, or quaternion. Additionally, for multiply (*), one can be a vector and the other a matrix
with the same dimensional size of the vector, or one can be a quaternion and the other a 3D-vector.
If one operand is scalar and the other is a vector, matrix, or quaternion the scalar is applied
component-wise to the vector, matrix, or quaternion, resulting in the same type as the vector,
matrix, or quaternion.
  Multiply (*) applied to two vectors yields a component-wise multiply. Multiply (*) applied to two
matrices yields a linear algebraic matrix multiply, not a component-wise multiply. Multiply (*)
applied to two quaternions yields an algebraic quaternion multiply, not a component-wise multiply.
  If the left argument of expression is a vector and the right is a matrix with a compatible
dimension, then multiply operator (*) will do a row vector matrix multiplication, and will result
in the same type as the vector.
  If the left argument of expression is a matrix and the right is a vector with a compatible
dimension, then multiply operator (*) will do a column vector matrix multiplication, and will
result in the same type as the vector.
  If the left argument of expression is a quaternion and the right is a 3D-vector, then multiply
operator (*) will rotate the vector by quaternion, and will result in the same type as the vector.
  Use functions dot, cross, and matrixCompMult to get, respectively, vector dot product, vector
cross product, and matrix component-wise multiplication.
  The arithmetic unary operators supported are negate (-), post- and pre-increment and decrement
(-- and ++) that operate on vectors and matrices. These result with the same type they operated on.
For post- and pre-increment and decrement, the expression must be one that could be assigned to (an
l-value). Pre-increment and predecrement add or subtract 1 or 1.0 to the contents of the expression
they operate on, and the value of the pre-increment or pre-decrement expression is the resulting
value of that modification. Post-increment and post-decrement expressions add or subtract 1 or 1.0
to the contents of the expression they operate on, but the resulting expression has the
expression's value before the post-increment or post-decrement was executed.
  The equality operators equal (==) and not equal (!=) operate on all supported types. They result
in a scalar HMbool. All components of the operands must be equal for the operands to be considered
equal. To get component-wise equality results for vectors, use functions equal and notEqual.

Vector and Matrix Operations:

  With a few exceptions, operations are component-wise. When an operator operates on a vector or
matrix, it is operating independently on each component of the vector or matrix, in a
component-wise fashion. For example,
  vec3 v, u;
  HMfloat f;

  v = u + f;
  will be equivalent to
  v.x = u.x + f;
  v.y = u.y + f;
  v.z = u.z + f;

  And

  vec3 v, u, w;
  w = v * u;
  will be equivalent to
  w.x = v.x * u.x;
  w.y = v.y * u.y;
  w.z = v.z * u.z;

  and likewise for most operators. The exceptions are matrix multiplied by vector, vector
multiplied by matrix, and matrix multiplied by matrix. These do not operate component-wise, but
rather perform the correct linear algebraic multiply. They require the size of the operands match.
  All unary operations works component-wise on their operands. For binary arithmetic operations, if
the two operands are the same type, then the operation is done component-wise and produces a result
that is the same type as the operands. If one operand is a scalar and the other operand is a vector
or matrix, then the operation proceeds as if the scalar value was replicated to form a matching
vector or matrix operand.

Quaternion Operations:

  Multiply (*) and divide (/) applied to a quaternion and a scalar yields a component-wise multiply
or divide, respectively. Add (+) and subtract (-) applied to two quaternions yields a
component-wise add or subtract, respectively. Multiply (*) applied to two quaternions perform an
algebraic quaternion multiply. Multiply (*) applied to a quaternion as the left argument and a
3D-vector as the right will rotate the vector by quaternion, and will result the rotated vector.
  Other arguments for binary operators add (+), subtract (-), multiply (*), and divide (/) are not
defined and using them will result in an error.
  Unary negation operator (-) works component-wise on quaternions.

AABB/Rectangle Operations:

  Bitwise OR (|) applied to two aabbs/rectangles will result in union of the operands, i.e. such
aabb/rectangle that contains both of the operands. Bitwise AND (&) applied to two aabbs/rectangles
will result in intersection of the operands, which has the same type as they are. Bitwise OR (|)
applied to an aabb/rectangle and a 3D/2D-vector will expand aabb/rectangle so that it will be
contain this vector, and returns the resulting aabb/rectangle. Add (+) or subtract (-) applied
to an aabb/rectangle and a 3D/2D-vector yields a component-wise add or subtract, respectively,
applied to both min and max values of the aabb/rectangle, and returns the resulting aabb/rectangle.
  Bitwise XOR (^) applied to a aabb/rectangle and a scalar/vector will expand aabb/rectangle by
subtracting and adding that scalar/vector value to min and max of aabb/rectangle correspondingly,
and returns the resulting aabb/rectangle.


Supported Functions:

When functions are specified below, where the input arguments (and corresponding output) can be
HMfloat, vec2, vec3, vec4, HMdouble, dvec2, dvec3, dvec4, HMlong_double, ldvec2, ldvec3, or ldvec4,
genType is used as the argument. Similarly, vec is used to denote any type vectors, mat - to denote
any type matrices and so on for all library types.
Also float is used to denote any supported floating point type: HMfloat, HMdouble or HMlong_double.
And genTypeInt is used to denote HMint, ivec2, ivec3, or ivec4.

SYNTAX                                    DESCRIPTION

=======================================   =========================================================

                                    ------TYPE CAST FUNCTIONS------

#vec vec_cast<#vec>(vec v)                Converts vector v to the type of #vec. Example:
                                            ivec3 a,b;
                                            ldvec3 c=vec_cast<ldvec3>(a+b);

#mat mat_cast<#mat>(mat m)                Converts matrix m to the type of #mat. Example:
                                            mat3 a=mat3(1);
                                            dmat4 b=mat_cast<dmat4>(a);//mat_cast<dmat4>(a) returns
                                                                       //the identity 4D matrix

#quat quat_cast<#quat>(quat q)            Converts quaternion q to the type of #quat. Example:
                                            dvec3 v; dquat q;
                                            quat r=quat_cast<quat>(q*dquat(v)*conjugate(q));

#aabb aabb_cast<#aabb>(aabb b)            Converts aabb b to the type of #aabb.

#rect rect_cast<#rect>(rect r)            Converts rectangle r to the type of #rect.

#range range_cast<#range>(range r)        Converts range r to the type of #range.

                                    ------ANGLE FUNCTIONS------

genType radians(genType deg)              Converts degrees to radians and returns the result.
genType degrees(genType rad)              Converts radians to degrees and returns the result.

                                    ------EXPONENTIAL FUNCTIONS------

genType exp2(genType x)                   Returns 2 raised to the x power.

genType log2(genType x)                   Returns the base 2 logarithm of x, i.e., returns the
                                          value y which satisfies the equation x = 2^y.
                                          Result is undefined if x <= 0.

genType sqrt(genType x)                   Returns the positive square root of x.
                                          Result is undefined if x < 0.

genType inversesqrt(genType x)            Returns the reciprocal of the positive square root of x.
                                          Result is undefined if x <= 0.

                                    ------PSEUDORANDOM NUMBER GENERATOR FUNCTIONS------

uint random(uint randMax)                 Returns a pseudorandom integer in the range 0 to randMax.

uint random(uint randMin,                 Returns a pseudorandom integer in the range randMin to
            uint randMax)                 randMax.

long_double random(long_double randMax)   Returns a pseudorandom float in the range 0.0 to randMax.

long_double random(long_double randMin,   Returns a pseudorandom float in the range randMin to
                   long_double randMax)   randMax.

void rseed(uint seed)                     Sets a seed for random-number generation, i.e. the
                                          starting point for generating a series of pseudorandom
                                          integers or floats.

                                    ------COMMON FUNCTIONS------

genType square(genType x)                 Returns the square of x.

genType cube(genType x)                   Returns the cube of x.

genType abs(genType x)                    Returns x if x >= 0, otherwise it returns –x.

genType sign(genType x)                   Returns 1.0 if x > 0, 0.0 if x = 0, and –1.0 if x < 0.

genType floor(genType x)                  Returns a value equal to the nearest integer that is
                                          less than or equal to x.

genType fract(genType x)                  Returns x - floor (x).

genType mod(genType x, genType y)         Modulus. Returns x - y*floor(x/y).
                                          // ((
genType wrap(genType x,                   Wraps x at range [0,maxVal). Returns mod(x,y).
             genType maxVal)

genType wrap(genType x,                   Wraps x at range [minVal,maxVal). // ]]
             genType minVal,              Returns mod(x-minVal, maxVal-minVal) + minVal.
             genType maxVal)

genType snap(genType x,                   Returns floor(x/gridSize + .5) * gridSize.
             genType2 gridSize)

genType min(genType v)                    Returns the minimal of the v components.

genType min(genType x, genType y)         Returns y if y < x, otherwise it returns x.

genType min(genType x,                    Returns the minimal value of x, y and z.
            genType y,
            genType z)

genType min(genType x,                    Returns the minimal value of x, y, z and w.
            genType y,
            genType z,
            genType w)

genType max(genType v)                    Returns the maximal of the v components.

genType max(genType x, genType y)         Returns y if x < y, otherwise it returns x.

genType max(genType x,                    Returns the maximal value of x, y and z.
            genType y,
            genType z)

genType max(genType x,                    Returns the maximal value of x, y, z and w.
            genType y,
            genType z,
            genType w)

genType clamp(genType x,                  Returns max(minVal, min(x, maxVal)).
              genType minVal,
              genType maxVal)

genType mix (genType x,                   Returns x * (1 - a) + y * a, i.e., the linear blend of
             genType y,                   x and y.
             genType2 a)

genType lerp(genType x,                   The same as mix(x,y,a).
             genType y,
             genType2 a)

genType cerp(genType x,                   Performs Cosine intERPolation (CERP) between x and y.
             genType y,
             genType2 a)

genType herp3(genType x,                  Performs Hermite intERPolation (HERP) between x and y
              genType y,                  using polynomial of degree 3.
              genType2 a)

genType herp5(genType x,                  Performs Hermite intERPolation (HERP) between x and y
              genType y,                  using polynomial of degree 5.
              genType2 a)

genType step(genType edge, genType x)     Returns 0.0 if x < edge, otherwise it returns 1.0.

genType smoothstep(genType edge0,         Returns 0.0 if x <= edge0 and 1.0 if x >= edge1 and
                   genType edge1,         performs smooth Hermite interpolation between 0 and 1
                   genType x)             when edge0 < x < edge1. This is useful in cases where you
                                          would want a threshold function with a smooth transition.
										  This is equivalent to:
                                            genType t;
                                            t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
                                            return t * t * (3 - 2 * t);

                                    ------GEOMETRIC FUNCTIONS------

float dot(genType v0, genType v1)         Returns the dot product of v0 and v1.

float length(genType v)                   Returns the length of vector v.

float sqlen(genType v)                    Returns the squared length of vector v.

float distance(genType p0, genType p1)    Returns the distance between p0 and p1, i.e.
                                          length (p0 – p1).

vec2 cross(vec2 v0, vec2 v1)              Returns the exterior (or wedge) product of v0 and v1.

vec3 cross(vec3 v0, vec3 v1)              Returns the cross product of v0 and v1.

genType normalize(genType v)              Returns a vector in the same direction as v but with a
                                          length of 1.

vec2 perp(vec2 v)                         Returns the perpendicular of vector v, i.e.
                                          vec2(-v.y, v.x).

vec3 perp(vec3 v)                         Returns the perpendicular of vector v.

vec3 slerp(vec3 v1,vec3 v2,float t)       Returns Spherical Linear intERPolation (SLERP) of vectors
                                          v1 and v2.

genType faceforward(genType N,            If dot (Nref, I) < 0 return N otherwise return –N.
                    genType I,
                    genType Nref)

genType reflect(genType V, genType N)     For the incident vector V and surface orientation N,
                                          returns the reflection direction:
                                            result = V - 2*dot(N,V)*N;
                                          N must already be normalized in order to achieve the
                                          desired result.
genType reflect(genType V, vec4 Plane)    For the incident vector V and plane Plane, returns the
                                          reflection direction.
                                          Plane must already be normalized in order to achieve the
                                          desired result.

genType refract(genType V,                For the incident vector V and surface normal N, and the
                genType N,                ratio of indices of refraction eta, return the refraction
                float eta)                vector. The returned result is computed by
                                            k = 1.0 - eta * eta * (1.0 - dot(N, V) * dot(N, V))
                                            if (k < 0.0)
                                                result = genType(0.0)
                                            else
                                                result = eta * V - (eta * dot(N, V) + sqrt(k)) * N
                                          The input parameters for the incident vector V and
                                          the surface normal N must already be normalized
                                          to get the desired results.

                                    ------MATRIX FUNCTIONS------

mat matrixCompMult(mat x, mat y)          Multiply matrix x by matrix y component-wise, i.e.,
                                          result[i][j] is the scalar product of x[i][j] and
                                          y[i][j].
                                          Note: to get linear algebraic matrix multiplication, use
                                          the multiply operator (*).

float fnorm(mat m)                        Returns the Frobenius norm of matrix m.

float det(mat m)                          Returns the determinant of matrix m.

mat inverse(mat m)                        Returns the inverse of matrix m.

mat transpose(mat m)                      Returns the transpose of matrix m.

mat3 orthonormalize(mat3 m)               Returns the orthonormalized matrix m.

mat4 scalingMat(vec3 s)                   Returns the matrix m which satisfies the equation
mat4 scalingMat4(vec3 s)                  m*vec4(v,1.0) = vec4(s*v,1.0), where v - arbitrary three-
                                          dimensional vector.

mat3 scalingMat3(vec3 s)                  Returns the matrix m which satisfies the equation
                                          m*v = s*v, where v - arbitrary three-dimensional vector.

mat3 scalingMat(vec2 s)                   Returns the matrix m which satisfies the equation
mat3 scalingMat3(vec2 s)                  m*vec3(v,1.0) = vec3(s*v,1.0), where v - arbitrary two-
                                          dimensional vector.

mat2 scalingMat2(vec2 s)                  Returns the matrix m which satisfies the equation
                                          m*v = s*v, where v - arbitrary two-dimensional vector.

mat4 translationMat(vec3 tr)              Returns the matrix m which satisfies the equation
mat4 translationMat4(vec3 tr)             m*vec4(v,1.0) = vec4(tr+v,1.0), where v - arbitrary
                                          three-dimensional vector.

mat3 translationMat(vec2 tr)              Returns the matrix m which satisfies the equation
mat3 translationMat3(vec2 tr)             m*vec3(v,1.0) = vec3(tr+v,1.0), where v - arbitrary two-
                                          dimensional vector.

mat3 crossMat(vec3 n)                     Returns the matrix m which satisfies the equation
mat3 crossMat3(vec3 n)                    m*v = cross(n,v), where v - arbitrary three-dimensional
                                          vector.

mat4 crossMat4(vec3 n)                    Returns the matrix m which satisfies the equation
                                          m*vec4(v,1.0) = vec4(cross(n,v),1.0), where v - arbitrary
                                          three-dimensional vector.

                                    ------VECTOR RELATIONAL FUNCTIONS------

bvec isZero(vec v)                        Returns the component-wise compare of v == 0.
bvec notZero(vec v)                       Returns the component-wise compare of v != 0.
bvec lessThan(vec a, vec b)               Returns the component-wise compare of a < b.
bvec lessThanEqual(vec a, vec b)          Returns the component-wise compare of a <= b.
bvec greaterThan(vec a, vec b)            Returns the component-wise compare of a > b.
bvec greaterThanEqual(vec a, vec b)       Returns the component-wise compare of a >= b.
bvec equal(vec a, vec b)                  Returns the component-wise compare of a == b.
bvec notEqual(vec a, vec b)               Returns the component-wise compare of a != b.
bool any(bvec v)                          Returns true if any component of v is true.
bool all(bvec v)                          Returns true only if all components of v are true.
bvec not(bvec v)                          Returns the component-wise logical complement of v.

                                    ------NOISE FUNCTIONS------

  The noise functions below are defined to have the following characteristics:
• The return value(s) are uniform distrubuted in the range [-1.0,1.0].
• The return value(s) have an overall average of 0.0.
• They are repeatable, in that a particular input value will always produce the same return value.
• The spatial frequency is 1.0.
• Their period is 2.0^32.

float noise1(genType v,ferp=herp5)        Returns a 1D noise value based on the input value v.
 vec2 noise2(genType v,ferp=herp5)        Returns a 2D noise value based on the input value v.
 vec3 noise3(genType v,ferp=herp5)        Returns a 3D noise value based on the input value v.
 vec4 noise4(genType v,ferp=herp5)        Returns a 4D noise value based on the input value v.

  int noise1(genTypeInt v)                Returns a 1D noise value based on the input value v.
ivec2 noise2(genTypeInt v)                Returns a 2D noise value based on the input value v.
ivec3 noise3(genTypeInt v)                Returns a 3D noise value based on the input value v.
ivec4 noise4(genTypeInt v)                Returns a 4D noise value based on the input value v.

                                    ------PSEUDO-PLANE FUNCTIONS------

vec3 planeNormal(vec3 p0,                 Returns the normal of plane containing points p0, p1 and
                 vec3 p1,                 p2.
				 vec3 p2)

vec4 plane(vec3 p0, vec3 p1, vec3 p2)     Constructs a plane from 3 point lying in that plane.

vec4 plane(vec3 n, vec3 p)                Constructs a plane from normal and point lying in this
                                          plane.

vec4 unitPlane(vec3 p0,                   Constructs a unit plane from 3 point lying in that plane.
               vec3 p1,
			   vec3 p2)

vec4 unitPlane(vec3 n, vec3 p)            Constructs a unit plane from normal and point lying in
                                          this plane.

vec4 normalizePlane(vec4 Plane)           Converts Plane to the unit plane and returns the result.

vec4 translatePlane(vec4 Plane,vec3 tr)   Move Plane on the vector tr and returns the result.

vec3 planePoint(vec4 Plane)               Returns a point lying in the specified plane.

vec3 unitPlanePoint(vec4 Plane)           Returns a point lying in the specified unit plane.

vec4 reflectPlane(vec4 Plane,             For the incident plane Plane and unit plane R, returns
                  vec4 R)                 the Plane reflected relative to R.
                                          Plane R must already be normalized in order to achieve
                                          the desired result. But Plane mustn't be normalized.

float planeDist(vec4 Plane, vec3 point)   Returns the distance between plane Plane and point.

float planeDist(vec4 Plane, vec4 point)   Returns the distance between plane Plane and point in
                                          homogeneous coordinates.

float unitPlaneDist(vec4 Plane,           Returns the distance between unit plane Plane and point.
                    vec3 point)

float unitPlaneDist(vec4 Plane,           Returns the distance between unit plane Plane and point
                    vec4 point)           in homogeneous coordinates.

bool onPlane(vec4 Plane, vec3 point)      Returns true if point lying on Plane, otherwise it
                                          returns false.

bool outOfPlane(vec4 Plane,               Returns true if point not lying on Plane, otherwise it
                vec3 point)               returns false.

bool abovePlane(vec4 Plane,               Returns true if point is above Plane, otherwise it
                vec3 point)               returns false.

bool onPlaneOrAbove(vec4 Plane,           Returns true if point is above Plane or lying on it,
                    vec3 point)           otherwise function returns false.

bool belowPlane(vec4 Plane,               Returns true if point is below Plane, otherwise it
                vec3 point)               returns false.

bool onPlaneOrBelow(vec4 Plane,           Returns true if point is below Plane or lying on it,
                    vec3 point)           otherwise function returns false.

bool rayPlaneIntn(out float t,            Returns true if line (rayStart,rayStart+rayDirn) is not
                  vec3 rayStart,          parallel to Plane. In that case t sets to the parameter
                  vec3 rayDirn,           value in point of intersection from parametric
                  vec4 Plane)             representation of line. Otherwise function returns false.

bool rayPlaneIntn(out vec3 intn,          Returns true if ray (rayStart,rayStart+rayDirn)
                  vec3 rayStart,          intersects Plane in point between min (and max if
                  vec3 rayDirn,           specified). In that case the point of intersection puts
                  vec4 Plane,             into the vector intn.
                  float min=0[,max])      Otherwise function returns false.

bool segPlaneIntn(out float t,            Returns true if line (segStart,segEnd) is not parallel
                  vec3 segStart,          to Plane. In that case t sets to the parameter value in
                  vec3 segEnd,            point of intersection from parametric representation of
                  vec4 Plane)             line. Otherwise function returns false.

bool segPlaneIntn(out vec3 intn,          Returns true if segment (segStart,segEnd) intersects the
                  vec3 segStart,          Plane in point between min (and max if specified). In
                  vec3 segEnd,            that case the point of intersection puts into the vector
                  vec4 Plane,             intn.
                  float min=0[,max])      Otherwise function returns false.

                                    ------LOOKAT FUNCTIONS------

mat4 lookAtDirnMat(vec3 eye,              Creates a viewing matrix derived from an eye point, a
                   vec3 dirn,             look-at direction, and an UP vector.
                   vec3 up)

mat4 lookAtMat(vec3 eye,                  Creates a viewing matrix derived from an eye point, a
               vec3 center,               reference point indicating the center of the scene, and
               vec3 up)                   an UP vector.

mat4 lookAtDirnMat(vec3 eye,              Creates a viewing matrix derived from an eye point and a
                   vec3 dirn)             look-at direction.

mat4 lookAtMat(vec3 eye,                  Creates a viewing matrix derived from an eye point and a
               vec3 center)               reference point indicating the center of the scene.

mat4 lookAtDirnMat(vec3 dirn)             Creates a viewing matrix derived from only a look-at
                                          direction.

                                    ------PROJECTIVE FUNCTIONS------

mat4 orthoMat#(float left, float right,   Creates a projective matrix that produces an
               float bottom, float top,   orthographical projection. # is either GL or D3D, depends
               float znear = -1,          on API.
               float zfar = 1)

mat4 frustumMat#(float left, float right, Creates a projective matrix that produces a perspective
                 float bottom, float top, projection. # is either GL or D3D, depends on API.
                 float znear, float zfar)

mat4 perspectiveMat#(float fovy,          Creates a projective matrix that specifies a viewing
                     float aspect         frustum into the world coordinate system. # is either GL
                     float znear,         or D3D, depends on API.
                     float zfar)

                                    ------AUXILIARY FUNCTIONS------

vec3 rotateX(float angle,vec3 vec)        Rotates vec around the OX on angle radians.

mat3 rotationXMat(float angle)            Produces a rotation matrix of angle radians around the
mat3 rotationXMat3(float angle)           OX.
mat4 rotationXMat4(float angle)

vec3 rotateY(float angle,vec3 vec)        Rotates vec around the OY on angle radians.

mat3 rotationYMat(float angle)            Produces a rotation matrix of angle radians around the
mat3 rotationYMat3(float angle)           OY.
mat4 rotationYMat4(float angle)

vec3 rotateZ(float angle,vec3 vec)        Rotates vec around the OZ on angle radians.

mat3 rotationZMat(float angle)            Produces a rotation matrix of angle radians around the
mat3 rotationZMat3(float angle)           OZ.
mat4 rotationZMat4(float angle)

vec3 rotate(float angle,                  Rotates vec around the vector axis on angle radians.
            vec3 axis,
            vec3 vec)

mat3 rotationMat(float angle,vec3 axis)   Produces a rotation matrix of angle radians around the
mat3 rotationMat3(float angle,vec3 axis)  vector axis.
mat4 rotationMat4(float angle,vec3 axis)

mat4 reflectionMat(vec4 Plane)            Produces a matrix reflected relative to Plane.

vec3 project(vec3 V,vec4 Plane)           For the incident vector V and plane Plane, returns the
vec4 project(vec4 V,vec4 Plane)           projection of V on Plane.
                                          Plane must already be normalized in order to achieve the
                                          desired result.

mat4 projectionMat(vec4 Plane)            Produces an on Plane projection matrix.

bool intersect(aabb b0,aabb b1)           Returns true if b0 and b1 intersect, otherwise it returns
bool intersect(rect r0,rect r1)           false.
bool intersect(range r0,range r1)

                                    ------QUATERNION FUNCTIONS------

quat conjugate(quat q)                    Returns the conjugated of quaternion q.
float dot(quat q0,quat q1)                Returns the dot product of q0 and q1.
float sqlen(quat q)                       Returns the squared magnitude of quaternion q.
float length(quat q)                      Returns the magnitude of quaternion q.
quat normalize(quat q)                    Returns the normalized of quaternion q.
quat inverse(quat q)                      Returns the inverse of quaternion q.

quat shortestArcQuat(vec3 from,           Returns an unit quaternion that rotates vector from to
                     vec3 to)             vector to by the shortest path.

quat axisAngleToQuat(vec3 v,              Constructs a unit quaternion that rotates around v on
                     float alpha)         alpha radians. Vector v can be not unit.

float quatToAxisAngle(out vec3 axis,      Converts unit quaternion q to "angle and axis" notation.
                      quat q)             Function returns angle and puts unit axis to the
                                          corresponding vector variable.

vec3 quatToAxisAngle(quat q)              Converts unit quaternion q to "angle and axis" notation.
                                          Function returns an axis-vector with a length of angle.
                                          // (
float quatAngle(quat q)                   Returns the rotation angle of unit quaternion in range
                                          [0; 2*PI). // ]

vec3 unitQuatTransformVector(quat q,      Transforms a given vector v by unit quaternion q and
                             vec3 v)      returns the result.

quat nlerp(quat from,quat to,float t)     Returns normalized quaternion lerp, result is almost
                                          identical as for slerp, but this function is much faster.

quat slerp(quat from,quat to,float t)     Interpolate between from and to quaternion rotations
                                          along the shortest arc, i.e. returns Spherical Linear
                                          intERPolation (SLERP) of from and to quaternions.

quat squad(quat p,                        Returns slerp(slerp(p,q,t), slerp(a,b,t), 2*t*(1-t)),
           quat a,                        i.e. spherical cubic blend of p, a, b and q.
           quat b,
           quat q,
           float t)

                                    ------DUAL QUATERNION FUNCTIONS------

conjugate(out quat r[2], quat dq[2])      Writes to r the conjugated of dual quaternion dq.
float dot(quat q0[2], quat q1[2])         Returns the dot product of dual quaternions q0 and q1.
fastnormalize(out quat r[2], quat dq[2])  Writes to r the normalized of dual quaternion dq.
inverse(out quat r[2], quat dq[2])        Writes to r the inverse of dual quaternion dq.

quat quatTransGetDualQuatPart(quat q,     Returns the dual part of unit dual quaternion constructed
                              vec3 t)     from unit quaternion q and translation vector t.

mat4 dualQuatToMat(quat dq[2])            Converts unit dual quaternion dq to 4D matrix and
                                          returns the result.

matToDualQuat(out quat dq[2], mat4 M)     Converts matrix M to unit dual quaternion and writes
                                          result to dq.

quatTransToDualQuat(out quat dq[2],       Constructs a unit dual quaternion from unit quaternion q
                    quat q, vec3 t)       and translation vector t. Result is stored in dq.

vec3 dualQuatToQuatTrans(                 Unpacks a unit dual quaternion to unit quaternion and
                  out quat q, quat dq[2]) translation vector. Obtained quaternion is stored in q.
                                          Function returns the translation vector.

vec3 dualQuatGetTrans(quat dq[2])         Returns the translation vector of unit dual quaternion.

vec3 dualQuatTransformPoint(              Transforms a given point by unit dual quaternion dq and
                  quat dq[2], vec3 point) returns the result.

vec3 dualQuatTransformVector(             Transforms a given vector v by unit dual quaternion dq
                  quat dq[2], vec3 v)     and returns the result.

dualQuatMul(out quat r[2], quat q1[2],    Computes a multiplication of q1 and q2 and writes result
            quat q2[2])                   to r.

nlerp(out quat r[2], quat from[2],        Computes a normalized lerp of dual quaternions and writes
      quat to[2], float t)                result to r.



===================================================================================================
================================================RUS================================================
===================================================================================================

Handy Math Library
Copyright (C) 2005-2009  Александр Третьяк

Данная библиотека является бесплатным программным обеспечением;
вы можете распространять её и/или модифицировать по условиям
Стандартной Общественной Лицензии Ограниченного Применения GNU,
изданной Free Software Foundation, версии 2.1 или (по вашему
усмотрению) любой более поздней версии.

Эта библиотека распространяется в надежде, что она будет полезна,
однако БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ; даже без гарантии ПРИГОДНОСТИ
ИСПОЛЬЗОВАНИЯ В КОММЕРЧЕСКИХ ЦЕЛЯХ или ГОДНОСТИ ДЛЯ КОНКРЕТНОГО
ПРИМЕНЕНИЯ. Для более подробной информации смотрите Стандартную
Общественную Лицензию Ограниченного Применения GNU.

Вы должны были получить копию Стандартной Общественной Лицензии
Ограниченного Применения GNU вместе с этой библиотекой.
В противном случае, напишите в Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA



КРАТКАЯ ДОКУМЕНТАЦИЯ:


Настройка библиотеки:

По умолчанию, библиотека представляет матрицы в порядке следования столбцов.
Для изменения этих настроек определите необходимые константы перед включением "hm.h":

КОНСТАНТА                        ОПИСАНИЕ
==============================   ==================================================================
#define D3D_MAT_STYLE            Матрицы представляются в порядке следования строк. Но из-за
                                 особого представления матриц в D3D параметр влияет только на
                                 операции умножения матрицы на матрицу, матрицы на вектор и вектора
                                 на матрицу.
#define QUAT_MUL_REVERSE_ORDER   Если эта константа определена, умножения двух кватернионов и
                                 кватерниона на 3D-вектор выполняются в обратном порядке, таким
                                 образом при умножении левый аргумент должен быть 3D-вектором, а
                                 правый - кватернионом.


Поддерживаемые типы:

TvecN - вектор,

  где T - тип компонент вектора:

    T       Соответствующий тип
    --      ---------------------
    b       HMbool
    i       HMint
    d       HMdouble
    ld      HMlong_double
    s       HMshort
    us      HMushort
    ub      HMubyte

  Если T пропущено в объявлении вектора, то компоненты такого вектора имеют тип HMfloat.

  N представляет размерность вектора.

  Названия компонент вектора обозначаются одной буквой. Для удобства, с каждым компонентом
  ассоциировано несколько букв для использования одного типа для обозначения положения, цвета или
  текстурных координат. Отдельные компоненты вектора могут быть выделены указанием после имени
  переменной точки ( . ) и следующим за ней названием компоненты.
  Поддерживаемые названия компонент:
  {x, y, z, w} полезно при использовании векторов обозначающих точки или нормали
  {r, g, b, a} полезно при использовании векторов обозначающих цвета
  {s, t, p, q} полезно при использовании векторов обозначающих текстурные координаты
  Названия компонент x, r и s, к примеру, обозначают одну и ту же (первую) компоненту вектора.
  Обратите внимание, что третья компонента текстуры, называемая r в OpenGL, была переименована в p
  из-за пересечением с компонентой r для (красного) цвета.
  Синтаксис выбора компонент позволяет выбирать несколько компонент вектора посредством добавления
  их названий (из одного множества имён) со скобками после точки ( . ). Напр.: vec4 v4; v4.yzw();
  Для доступа к элементам вектора можно использовать также оператор индексирования ( [] ). Таким
  образом, pos[2] обозначает третий элемент вектора pos, и эта запись эквивалентна pos.z.

TmatN или TmatCxR - матрица, где T (может быть 'i','d' или 'ld') имеет тот же смысл, что и в TvecN.
  C задает количество столбцов матрицы, а R - количество строк.
  TmatN представляет квадратную матрицу размерности N.

  Если T пропущено в объявлении матрицы, то компоненты такой матрицы имеют тип HMfloat.

  Компоненты матрицы доступны через оператор индексирования ( [] ). При одиночном индексировании
  матрица рассматривается как массив вектор-столбцов, и при использовании этого оператора
  возвращается столбец, который имеет тип вектора той же размерности что и матрица. Индексирование
  столбцов начинается с нуля. Второй оператор индексирования будет уже применяться к вектору, как
  было описано выше для векторов. Отсюда, 2 оператора индексирования выбирают сначала столбец, а
  затем строку, и таким образом возвращают один элемент матрицы.

Tquat - кватернион, где T (может быть 'd' или 'ld') имеет тот же смысл, что и в Tvec4.

  Если T пропущено в объявлении кватерниона, то его компоненты имеют тип HMfloat.

  Поддерживаемые названия компонент кватерниона: x, y, z и w. Также можно использовать .xyz() для
  выбора первых трёх компонент кватерниона (т.е. вектор кватерниона) как отдельный 3D-вектор.

Taabb - выровненный по осям ограничивающий объём ("бокс"), где T (может быть 'i','s','d' или 'ld')
        имеет тот же смысл, что и в Tvec3.

  Если T пропущено в объявлении бокса, то его компоненты имеют тип HMfloat.

  Бокс состоит из двух ограничивающих 3D-векторов соответствующего типа: min и max. К каждому из
  них можно обратиться, указав его название после точки ( . ). Например: aabb b; b.min;

Trect - прямоугольник, где T (может быть 'i','s','d' или 'ld') имеет тот же смысл, что и в Tvec2.

  Если T пропущено в объявлении прямоугольника, то его компоненты имеют тип HMfloat.

  Прямоугольник состоит из двух ограничивающих 2D-векторов соответствующего типа: min и max. К
  каждому из них можно обратиться, указав его название после точки ( . ). Например: rect r; r.max;

Trange - диапазон, где T (может быть 'i','s','d' или 'ld') имеет тот же смысл, что и в Tvec2.

  Если T пропущено в объявлении прямоугольника, то его компоненты имеют тип HMfloat.

  Диапазон состоит из двух ограничивающих значений соответствующего типа: min и max. К каждому
  из них можно обратиться, указав его название после точки ( . ). Например: range r; r.min;


Конструкторы:

  Конструкторы используются для создания векторов, матриц и кватернионов из множества скаляров,
векторов, матриц или кватернионов.
  Если в конструктор вектора передаётся один скалярный параметр, он используется для инициализации
всех компонент нового вектора и установки их в это скалярное значение. Если в конструктор матрицы
передаётся один скалярный параметр, он используется для инициализации всех элементов на диагонали
матрицы, а остальные элементы устанавливаются в 0.0. Если в конструктор матрицы передаётся один
векторный параметр, он используется для инициализации всех элементов на диагонали матрицы, а
остальные элементы матрицы устанавливаются в 0.0. Если в конструктор кватерниона передаётся
один скалярный параметр, он используется для инициализации скаляра кватерниона (т.е. значения
компоненты w), а остальные компоненты кватерниона (x,y,z) устанавливаются в 0.0. Если в конструктор
кватерниона передаётся один 3D-вектор, он используется для инициализации вектора кватерниона (т.е.
первых трёх компонент кватерниона), а оставшаяся компонента w устанавливается в 0.0. Если в
конструктор кватерниона передаются 4 скалярных параметра, они используются для инициализации всех
компонент кватерниона. Если в конструктор бокса/прямоугольника передаётся один скалярный параметр,
он используется для инициализации вектора min в противоположное значение параметра, и вектора max в
это скалярное значение; однако если значение этого параметра равно нулю, то конструктор создаёт
пустой бокс/прямоугольник, т.е. такой бокс/прямоугольник, который представляет пустой объём. Если
в конструктор бокса/прямоугольника передаются 2 скалярных или 2 векторных параметра, векторы min и
max инициализируются по правилам создания векторов, причём вектор min инициализируется первым
параметром, а max - вторым.
  Векторы могут быть созданы из множества векторных параметров, и/или множества скалярных, тогда
они назначаются компонентам нового вектора в порядке слева направо. В этом случае, совокупности
компонент параметров должно быть достаточно (но не больше) для инициализации всех компонент
вектора. Матрицы создаются в порядке следования столбцов, и могут быть созданы из множества
скалярных параметров по числу элементов матрицы или из множества векторных параметров. В последнем
случае, число векторов для инициализации матрицы и их размерность должны быть равны размерности
создаваемой матрицы.
  Матрицы можно создавать из других матриц. Если матрица создаётся из матрицы большей размерности,
то её подматрица, начинающаяся с первого элемента, используется для инициализации всех элементов
создаваемой матрицы. Если матрица создаётся из матрицы меньшей размерности, то она используется для
инициализации подматрицы (начинающейся с первого элемента) создаваемой матрицы. Остальные элементы
новой матрицы устанавливаются в 1.0, если они лежат на диагонали матрицы и в 0.0 в противном
случае.
  Также матрицы можно создавать по правилам создания матриц меньшей размерности, т.е. 4D-матрица
может быть создана из двух 2D-векторов, трёх 3D-векторов, 9 скалярных параметров (как при создании
3D-матрицы) и т.д. В этом случае, применяюся правила создания матриц из матриц меньшей размерности.
  Остальные допустимые конструкторы перечислены ниже:

КОНСТРУКТОР                     ОПИСАНИЕ
=============================   ===================================================================
mat3(quat q,                    Создаёт 3D-матрицу из кватерниона q. Если quatIsUnit==true,
     bool quatIsUnit=true)      q должен быть единичным кватернионом.

mat4(quat q,                    Создаёт 4D-матрицу из кватерниона q. Если quatIsUnit==true,
     bool quatIsUnit=true)      q должен быть единичным кватернионом.

quat(mat3 M,                    Создаёт кватернион из 3D-матрицы M. Если quatIsUnit==true, создаётся
     bool quatIsUnit=true)      единичный кватернион. Матрица должна содержать только вращение для
                                достижения желаемого результата.

quat(mat4 M,                    Создаёт кватернион из 4D-матрицы M. Если quatIsUnit==true, создаётся
     bool quatIsUnit=true)      единичный кватернион. Матрица должна содержать только вращение для
                                достижения желаемого результата.

aabb(uint count,                Создаёт бокс, ограничивающий count точек из массива vertices[].
     vec3 vertices[])

rect(uint count,                Создаёт прямоугольник, ограничивающий count точек из массива
     vec2 vertices[])           vertices[].

range(uint count,               Создаёт диапазон, ограничивающий count значений из массива
      float vertices[])         vertices[].


Операции:

  Присваивание значений переменным (объектам) производится оператором присваивания ( = ):
  lvalue = expression
  Оператор присваивания помещает значение выражения expression в lvalue. Expression и lvalue должны
быть одного типа. Все необходимые преобразования типов должны быть явно заданы через конструкторы.
  Другие операторы присваивания: прибавить к (+=), вычесть из (-=), умножить на (*=) и
разделить на (/=). Выражение "lvalue op= expression" эквивалентно "lvalue = lvalue op expression",
причём lvalue и expression дожны удовлетворять семантическим требования как оператора op, так и
присваивания (=).
  Поддерживаемые арифметические бинарные операторы: сложение (+), вычитание (-), умножение (*) и
деление (/). Два операнда должны быть одного типа или один может быть числом, а другой вектором,
матрицей или кватернионом. Кроме того, для умножения (*), один может быть вектором, а другой -
матрицей той же размерности, что и вектор, или же один может быть кватернионом, а другой -
3D-вектором. Если один операнд - число, а другой - вектор, матрица или кватернион, число
покомпонентно применяется к вектору, матрице или кватерниону. Результат имеет тот же тип, что и
вектор, матрица или кватернион.
  Произведение (*) двух векторов даёт покомпонентное умножение. Произведение (*) двух матриц даёт
матричное произведение по правилам линейной алгебры, а не покомпонентное умножение. Произведе-
ние (*) двух кватернионов даёт алгебраическое произведение кватернионов, а не покомпонентное
умножение.
  Если левый аргумент выражения - вектор, а правый - матрица той же размерности, то оператор
умножения (*) произведёт умножение вектор-строки на матрицу. Результат имеет тот же тип, что и
вектор.
  Если левый аргумент выражения - матрица, а правый - вектор той же размерности, то оператор
умножения (*) произведёт умножение матрицы на вектор-столбец. Результат имеет тот же тип, что и
вектор.
  Если левый аргумент выражения - кватернион, а правый - 3D-вектор, то оператор умножения (*)
повернёт этот вектор кватернионом. Результат имеет тот же тип, что и вектор.
  Используйте функции dot, cross и matrixCompMult для вычисления соответственно скалярного
произведения векторов, векторного произведения векторов и покомпонентного умножения матриц.
  Поддерживаемые унарные операторы: отрицание (-), пост- и пре-инкремент и декремент (-- и ++),
которые оперируют над векторами и матрицами. Результат имеет тот же тип, что и операнд этих
операторов. Для пост- и пре-инкремента и декремента выражение должно быть l-value. Пре-инкремент и
декремент прибавляют или вычитают 1 или 1.0 к каждому элементу выражения, с которым они оперируют,
и значение пре-инкремента и декремента - результат этого изменения. Пост-инкремент и пост-декремент
прибавляют или вычитают 1 или 1.0 к каждому элементу выражения, с которым они оперируют, но
результатом будет значение выражения до выполнения оператора пост-инкремента или пост-декремента.
  Операторы сравнения - равно (==) и не равно (!=) работают со всеми поддерживаемыми типами.
Результат имеет тип HMbool. Чтобы операнды считались равными, все их компоненты должны быть равны.
Для получения результатов покомпонентного сравнения векторов, используйте функции equal и notEqual.

Векторные и матричные операции:

  За небольшим исключением, эти операции покомпонентные. Когда оператор применяется к вектору или
матрице, он применяется независимо к каждому элементу вектора или матрицы, покомпонентно. Например:
  vec3 v, u;
  HMfloat f;

  v = u + f;
  будет эквивалентно
  v.x = u.x + f;
  v.y = u.y + f;
  v.z = u.z + f;

  И

  vec3 v, u, w;
  w = v * u;
  будет эквивалентно
  w.x = v.x * u.x;
  w.y = v.y * u.y;
  w.z = v.z * u.z;

  и так для большинства операторов. Исключения составляют умножение матрицы на вектор, вектора на
матрицу и матрицы на матрицу. В этом случае выполняется не покомпонетное умножение, а соответ-
ствующее алгебраическое умножение. Эти операторы требуют соответствия размерности операндов.
  Все унарные операторы работают покомпонентно с их операндами. Для бинарных арифметических
операторов, если два операнда одного типа, то операция выполняется покомпонентно и результат имеет
тот же тип, что и операнды. Если один операнд - число, а другой - вектор или матрица, то операция
выполняется так, как если бы число было размножено для образования операнда вектора или матрицы.

Операции над кватернионами:

  Произведение (*) и частное (/) кватерниона и числа даёт покомпонентное умножение или деление
соответственно. Сложение (+) и вычитание (-) двух кватернионов даёт покомпонентное сложение или
вычитание соответственно. Умножение (*) двух кватернионов даёт алгебраическое произведение
кватернионов. Если левый аргумент выражения - кватернион, а правый - 3D-вектор, то оператор
умножения (*) повернёт этот вектор кватернионом и вернёт повёрнутый вектор.
  Другие аргументы для бинарных операторов сложение (+), вычитание (-), умножение (*) и деление (/)
не определены и их использование приведёт к возникновению ошибки.
  Унарный оператор отрицания (-) над кватернионами работает покомпонентно.

Операции над боксами и прямоугольниками:

  Логическое ИЛИ (|) двух боксов/прямоугольников даёт объединение операндов, т.е. такой бокс/прямо-
угольник, который содержит оба операнда. Логическое И (&) двух боксов/прямоугольников даёт
пересечение операндов, результат которого имеет тот же тип, что и операнды. Логическое ИЛИ (|)
бокса/прямоугольника и 3D/2D-вектора расширяет бокс/прямоугольник таким образом, чтобы он содержал
этот вектор, результатом операции является полученный бокс/прямоугольник. Сложение (+) или
вычитание (-) бокса/прямоугольника и 3D/2D-вектора дает покомпонентное сложение или вычитание
соответственно, применённое к значениям min и max бокса/прямоугольника, результатом операции
является полученный бокс/прямоугольник.
  Логическое ИСКЛЮЧАЮЩЕЕ ИЛИ (^) бокса/прямоугольника и числа/вектора расширяет бокс/прямоугольник
посредством уменьшения/увеличения его компонентов min/max соответственно на значение этого числа/
вектора, результатом операции является полученный бокс/прямоугольник.


Поддерживаемые функции:

В объявлении нижеследующих функций, где входные параметры (и соответствующие выходные) могут быть
HMfloat, vec2, vec3, vec4, HMdouble, dvec2, dvec3, dvec4, HMlong_double, ldvec2, ldvec3 или ldvec4,
genType используется в качестве аргумента. Аналогично, vec используется для обозначения вектора
любого типа, mat - для обозначения матрицы любого типа и т.д. для всех типов библиотеки.
Также float используется для обозначения любого поддерживаемого вещественного типа: HMfloat,
HMdouble или HMlong_double.
И genTypeInt используется для обозначения HMint, ivec2, ivec3 или ivec4.

СИНТАКСИС                                 ОПИСАНИЕ

=======================================   =========================================================

                                    ------ФУНКЦИИ ПРЕОБРАЗОВАНИЯ ТИПОВ------

#vec vec_cast<#vec>(vec v)                Преобразует вектор v к типу #vec. Пример:
                                            ivec3 a,b;
                                            ldvec3 c=vec_cast<ldvec3>(a+b);

#mat mat_cast<#mat>(mat m)                Преобразует матрицу m к типу #mat. Пример:
                                            mat3 a=mat3(1);
                                            dmat4 b=mat_cast<dmat4>(a);//mat_cast<dmat4>(a) возвра-
                                                                       //щает единичную 4D матрицу

#quat quat_cast<#quat>(quat q)            Преобразует кватернион q к типу #quat. Пример:
                                            dvec3 v; dquat q;
                                            quat r=quat_cast<quat>(q*dquat(v)*conjugate(q));

#aabb aabb_cast<#aabb>(aabb b)            Преобразует бокс b к типу #aabb.

#rect rect_cast<#rect>(rect r)            Преобразует прямоугольник r к типу #rect.

#range range_cast<#range>(range r)        Преобразует диапазон r к типу #range.

                                    ------ФУНКЦИИ ДЛЯ РАБОТЫ С УГЛАМИ------

genType radians(genType deg)              Преобразует градусы в радианы и возвращает результат.
genType degrees(genType rad)              Преобразует радианы в градусы и возвращает результат.

                                    ------ЭКСПОНЕНЦИАЛЬНЫЕ ФУНКЦИИ------

genType exp2(genType x)                   Возвращает 2 в степени x.

genType log2(genType x)                   Возвращает логарифм по основанию 2 от x, т.е., возвращает
                                          значение y, которое удовлетворяет условию x = 2^y.
                                          Результат неопределён если x <= 0.

genType sqrt(genType x)                   Возвращает положительный квадратный корень x.
                                          Результат неопределён если x < 0.

genType inversesqrt(genType x)            Возвращает обратный квадратный корень x.
                                          Результат неопределён если x <= 0.

                                    ------ФУНКЦИИ ГЕНЕРАТОРА ПСЕВДОСЛУЧАЙНЫХ ЧИСЕЛ------

uint random(uint randMax)                 Возвращает псевдослучайное целое число в диапазоне от 0
                                          до randMax.

uint random(uint randMin,                 Возвращает псевдослучайное целое число в диапазоне от
            uint randMax)                 randMin до randMax.

long_double random(long_double randMax)   Возвращает псевдослучайное вещественное число в диапазоне
                                          от 0.0 до randMax.

long_double random(long_double randMin,   Возвращает псевдослучайное вещественное число в диапазоне
                   long_double randMax)   от randMin до randMax.

void rseed(uint seed)                     Задаёт начальное число для генерации случайных чисел, т.е.
                                          точку отсчёта для генерации серии псевдослучайных целых
                                          или вещественных значений.

                                    ------ОБЩИЕ ФУНКЦИИ------

genType square(genType x)                 Возвращает квадрат числа x.

genType cube(genType x)                   Возвращает куб числа x.

genType abs(genType x)                    Возвращает x если x >= 0, иначе возвращает –x.

genType sign(genType x)                   Возвращает 1.0 если x>0, 0.0 если x==0, и –1.0 если x<0.

genType floor(genType x)                  Возвращает ближайшее целое, которое не превышает x.

genType fract(genType x)                  Возвращает x - floor (x).

genType mod(genType x, genType y)         Остаток от деления. Возвращает x - y*floor(x/y).
                                          // ((
genType wrap(genType x,                   Сворачивает x в интервале [0,maxVal). Возвращает mod(x,y).
             genType maxVal)

genType wrap(genType x,                   Сворачивает x в интервале [minVal,maxVal). // ]]
             genType minVal,              Возвращает mod(x-minVal, maxVal-minVal) + minVal.
             genType maxVal)

genType snap(genType x,                   Возвращает floor(x/gridSize + .5) * gridSize.
             genType2 gridSize)

genType min(genType v)                    Возвращает минимальную компоненту вектора v.

genType min(genType x, genType y)         Возвращает y если y < x, иначе возвращает x.

genType min(genType x,                    Возвращает минимальное из значений x, y и z.
            genType y,
            genType z)

genType min(genType x,                    Возвращает минимальное из значений x, y, z и w.
            genType y,
            genType z,
            genType w)

genType max(genType v)                    Возвращает максимальную компоненту вектора v.

genType max(genType x, genType y)         Возвращает y если x < y, иначе возвращает x.

genType max(genType x,                    Возвращает максимальное из значений x, y и z.
            genType y,
            genType z)

genType max(genType x,                    Возвращает максимальное из значений x, y, z и w.
            genType y,
            genType z,
            genType w)

genType clamp(genType x,                  Возвращает max(minVal, min(x, maxVal)).
              genType minVal,
              genType maxVal)

genType mix (genType x,                   Возвращает x * (1 - a) + y * a, т.е. линейную
             genType y,                   интерполяцию x и y.
             genType2 a)

genType lerp(genType x,                   То же что и mix(x,y,a).
             genType y,
             genType2 a)

genType cerp(genType x,                   Возвращает косинусоидальную интерполяцию (CERP) x и y.
             genType y,
             genType2 a)

genType herp3(genType x,                  Возвращает интерполяцию Эрмита (HERP) между x и y с
              genType y,                  помощью многочлена 3 степени.
              genType2 a)

genType herp5(genType x,                  Возвращает интерполяцию Эрмита (HERP) между x и y с
              genType y,                  помощью многочлена 5 степени.
              genType2 a)

genType step(genType edge, genType x)     Возвращает 0.0 если x < edge, иначе возвращает 1.0.

genType smoothstep(genType edge0,         Возвращает 0.0 если x <= edge0 и 1.0 если x >= edge1 и
                   genType edge1,         производит гладкое интерполирование Эрмита между 0 и 1
                   genType x)             когда edge0 < x < edge1. Это может быть полезно когда
                                          требуется пороговая функция с плавным переходом.
										  Вызов функции эквивалентен:
                                            genType t;
                                            t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
                                            return t * t * (3 - 2 * t);

                                    ------ГЕОМЕТРИЧЕСКИЕ ФУНКЦИИ------

float dot(genType v0, genType v1)         Возвращает скалярное произведение векторов v0 и v1.

float length(genType v)                   Возвращает длину вектора v.

float sqlen(genType v)                    Возвращает квадрат длины вектора v.

float distance(genType p0, genType p1)    Возвращает расстояние между точками p0 и p1, т.е.
                                          length (p0 – p1).

vec2 cross(vec2 v0, vec2 v1)              Возвращает псевдоскалярное произведение векторов v0 и v1.

vec3 cross(vec3 v0, vec3 v1)              Возвращает векторное произведение векторов v0 и v1.

genType normalize(genType v)              Возвращает вектор, имеющий то же направление что и v, но
                                          длина которого равна 1.

vec2 perp(vec2 v)                         Возвращает вектор, перпендикулярный v, т.е.
                                          vec2(-v.y, v.x).

vec3 perp(vec3 v)                         Возвращает вектор, перпендикулярный v.

vec3 slerp(vec3 v1,vec3 v2,float t)       Возвращает сферическую линейную интерполяцию (SLERP)
                                          векторов v1 и v2.

genType faceforward(genType N,            Если dot (Nref, I) < 0 возвращает N, в противном случае
                    genType I,            возвращает –N.
                    genType Nref)

genType reflect(genType V, genType N)     Для произвольного вектора V и ориентации поверхности N,
                                          возвращает отражённое направление:
                                            result = V - 2*dot(N,V)*N;
                                          N должен быть нормирован для достижения желаемого
                                          результата.
genType reflect(genType V, vec4 Plane)    Для произвольного вектора V и плоскости Plane, возвращает
                                          отражённое направление вектора V.
                                          Плоскость должна быть нормализована для достижения
                                          желаемого результата.

genType refract(genType V,                Для произвольного вектора V, нормали к поверхности N и
                genType N,                отношения коэффициентов преломления eta, возвращает
                float eta)                преломлённый вектор. Возвращаемый результат рассчитыва-
                                          ется следующим образом:
                                            k = 1.0 - eta * eta * (1.0 - dot(N, V) * dot(N, V))
                                            if (k < 0.0)
                                                result = genType(0.0)
                                            else
                                                result = eta * V - (eta * dot(N, V) + sqrt(k)) * N
                                          Входные значения - вектор V и нормаль к поверхности N -
                                          должны быть нормализована для достижения желаемого
                                          результата.

                                    ------ФУНКЦИИ ДЛЯ РАБОТЫ С МАТРИЦАМИ------

mat matrixCompMult(mat x, mat y)          Покомпонентно умножает матрицу x на y, т.е., result[i][j]
                                          вычисляется как произведение x[i][j] и y[i][j].
                                          Примечание: для вычисления результата алгебраического
                                          умножения матриц используйте оператор умножения (*).

float fnorm(mat m)                        Возвращает фробениусову норму матицы m.

float det(mat m)                          Возвращает определитель матрицы m.

mat inverse(mat m)                        Возвращает матрицу, обратную m.

mat transpose(mat m)                      Возвращает транспонированную матрицу m.

mat3 orthonormalize(mat3 m)               Возвращает матрицу m после ортонормализации.

mat4 scalingMat(vec3 s)                   Возвращает матрицу m которая удовлетворяет условию
mat4 scalingMat4(vec3 s)                  m*vec4(v,1.0) = vec4(s*v,1.0), где v - любой трёхмерный
                                          вектор.

mat3 scalingMat3(vec3 s)                  Возвращает матрицу m которая удовлетворяет условию
                                          m*v = s*v, где v - любой трёхмерный вектор.

mat3 scalingMat(vec2 s)                   Возвращает матрицу m которая удовлетворяет условию
mat3 scalingMat3(vec2 s)                  m*vec3(v,1.0) = vec3(s*v,1.0), где v - любой двухмерный
                                          вектор.

mat2 scalingMat2(vec2 s)                  Возвращает матрицу m которая удовлетворяет условию
                                          m*v = s*v, где v - любой двухмерный вектор.

mat4 translationMat(vec3 tr)              Возвращает матрицу m которая удовлетворяет условию
mat4 translationMat4(vec3 tr)             m*vec4(v,1.0) = vec4(tr+v,1.0), где v - любой трёхмерный
                                          вектор.

mat3 translationMat(vec2 tr)              Возвращает матрицу m которая удовлетворяет условию
mat3 translationMat3(vec2 tr)             m*vec3(v,1.0) = vec3(tr+v,1.0), где v - любой двухмерный
                                          вектор.

mat3 crossMat(vec3 n)                     Возвращает матрицу m которая удовлетворяет условию
mat3 crossMat3(vec3 n)                    m*v = cross(n,v), где v - любой трёхмерный вектор.

mat4 crossMat4(vec3 n)                    Возвращает матрицу m которая удовлетворяет условию
                                          m*vec4(v,1.0) = vec4(cross(n,v),1.0), где v - любой
                                          трёхмерный вектор.

                                    ------ФУНКЦИИ ОТНОШЕНИЙ ВЕКТОРОВ------

bvec isZero(vec v)                        Возвращает покомпонентное сравнение a == 0.
bvec notZero(vec v)                       Возвращает покомпонентное сравнение a != 0.
bvec lessThan(vec a, vec b)               Возвращает покомпонентное сравнение a < b.
bvec lessThanEqual(vec a, vec b)          Возвращает покомпонентное сравнение a <= b.
bvec greaterThan(vec a, vec b)            Возвращает покомпонентное сравнение a > b.
bvec greaterThanEqual(vec a, vec b)       Возвращает покомпонентное сравнение a >= b.
bvec equal(vec a, vec b)                  Возвращает покомпонентное сравнение a == b.
bvec notEqual(vec a, vec b)               Возвращает покомпонентное сравнение a != b.
bool any(bvec v)                          Возвращает true если любая компонента v истинна.
bool all(bvec v)                          Возвращает true только когда все компоненты v истинны.
bvec not(bvec v)                          Возвращает покомпонентное логическое отрицание v.

                                    ------ФУНКЦИИ ШУМА------

  Перечисленные ниже функции шума имеют следующие характеристики:
• Возвращаемое значение(я) равномерно распределено(ы) на интервале [-1.0,1.0].
• Возвращаемое значение(я) имеет общее среднее значение 0.0.
• Они повторяемы, т.е. конкретное входное значение всегда даёт одно и то же выходное значение.
• Пространственная частота - 1.0.
• Период функций - 2.0^32.

float noise1(genType v,ferp=herp5)        Возвращает 1D значение шума, основанное на v.
 vec2 noise2(genType v,ferp=herp5)        Возвращает 2D значение шума, основанное на v.
 vec3 noise3(genType v,ferp=herp5)        Возвращает 3D значение шума, основанное на v.
 vec4 noise4(genType v,ferp=herp5)        Возвращает 4D значение шума, основанное на v.

  int noise1(genTypeInt v)                Возвращает 1D значение шума, основанное на v.
ivec2 noise2(genTypeInt v)                Возвращает 2D значение шума, основанное на v.
ivec3 noise3(genTypeInt v)                Возвращает 3D значение шума, основанное на v.
ivec4 noise4(genTypeInt v)                Возвращает 4D значение шума, основанное на v.

                                    ------ФУНКЦИИ ДЛЯ РАБОТЫ С ПЛОСКОСТЯМИ------

vec3 planeNormal(vec3 p0,                 Возвращает нормаль к плоскости, в которой лежат точки p0,
                 vec3 p1,                 p1 и p2.
				 vec3 p2)

vec4 plane(vec3 p0, vec3 p1, vec3 p2)     Создаёт плоскость по 3-м лежащим в ней точкам.

vec4 plane(vec3 n, vec3 p)                Создаёт плоскость по нормали и точке, принадлежащей
                                          этой плоскости.

vec4 unitPlane(vec3 p0,                   Создаёт единичную плоскость по 3-м лежащим в ней точкам.
               vec3 p1,                   Примечание: под единичной здесь понимается плоскость с
			   vec3 p2)                   единичной нормалью.

vec4 unitPlane(vec3 n, vec3 p)            Создаёт единичную плоскость по нормали и точке,
                                          принадлежащей этой плоскости.

vec4 normalizePlane(vec4 Plane)           Преобразует Plane в единичную плоскость и возвращает
                                          результат.

vec4 translatePlane(vec4 Plane,vec3 tr)   Перемещает Plane на вектор tr и возвращает результат.

vec3 planePoint(vec4 Plane)               Возвращает точку, лежащую в заданной плоскости.

vec3 unitPlanePoint(vec4 Plane)           Возвращает точку, лежащую в заданной единичной плоскости.

vec4 reflectPlane(vec4 Plane,             Для произвольной плоскости Plane и единичной плоскости R,
                  vec4 R)                 возвращает отражение плоскости Plane относительно R.
                                          Плоскость R должна быть единичной для достижения
                                          желаемого результата. Но Plane не обязана быть единичной.

float planeDist(vec4 Plane, vec3 point)   Возвращает расстояние от плоскости Plane до точки point.

float planeDist(vec4 Plane, vec4 point)   Возвращает расстояние от плоскости Plane до точки point,
                                          заданной в однородных координатах.

float unitPlaneDist(vec4 Plane,           Возвращает расстояние от единичной плоскости Plane до
                    vec3 point)           точки point.

float unitPlaneDist(vec4 Plane,           Возвращает расстояние от единичной плоскости Plane до
                    vec4 point)           точки point, заданной в однородных координатах.

bool onPlane(vec4 Plane, vec3 point)      Возвращает true если точка лежит на плоскости Plane,
                                          иначе функция возвращает false.

bool outOfPlane(vec4 Plane,               Возвращает true если точка не лежит на плоскости Plane,
                vec3 point)               иначе функция возвращает false.

bool abovePlane(vec4 Plane,               Возвращает true если точка лежит выше плоскости Plane,
                vec3 point)               иначе функция возвращает false.

bool onPlaneOrAbove(vec4 Plane,           Возвращает true если точка лежит выше или на плоскости
                    vec3 point)           Plane, иначе функция возвращает false.

bool belowPlane(vec4 Plane,               Возвращает true если точка лежит ниже плоскости Plane,
                vec3 point)               иначе функция возвращает false.

bool onPlaneOrBelow(vec4 Plane,           Возвращает true если точка лежит ниже или на плоскости
                    vec3 point)           Plane, иначе функция возвращает false.

bool rayPlaneIntn(out float t,            Возвращает true если прямая (rayStart,rayStart+rayDirn)
                  vec3 rayStart,          не параллельна Plane. В этом случае в t запишется зна-
                  vec3 rayDirn,           чение параметра в точке пересечения из параметрического
                  vec4 Plane)             представления прямой. Иначе функция возвращает false.

bool rayPlaneIntn(out vec3 intn,          Возвращает true если луч (rayStart,rayStart+rayDirn)
                  vec3 rayStart,          пересекает плоскость Plane в точке между min (и max если
                  vec3 rayDirn,           значение указано). В этом случае точка пересечения
                  vec4 Plane,             запишется в радиус-вектор intn.
                  float min=0[,max])      Иначе функция возвращает false.

bool segPlaneIntn(out float t,            Возвращает true если прямая (segStart,segEnd) не парал-
                  vec3 segStart,          лельна Plane. В этом случае в t запишется значение пара-
                  vec3 segEnd,            метра в точке пересечения из параметрического представле-
                  vec4 Plane)             ния прямой. Иначе функция возвращает false.

bool segPlaneIntn(out vec3 intn,          Возвращает true если отрезок (segStart,segEnd) пересекает
                  vec3 segStart,          плоскость Plane в точке между min (и max если значение
                  vec3 segEnd,            указано). В этом случае точка пересечения запишется в
                  vec4 Plane,             радиус-вектор intn.
                  float min=0[,max])      Иначе функция возвращает false.

                                    ------ФУНКЦИИ ВИДА------

mat4 lookAtDirnMat(vec3 eye,              Создаёт матрицу вида на основании положения камеры eye,
                   vec3 dirn,             направления взгляда dirn и вектора вверх up.
                   vec3 up)

mat4 lookAtMat(vec3 eye,                  Создаёт матрицу вида на основании положения камеры eye,
               vec3 center,               указанной точки, задающей центр сцены, и вектора вверх
               vec3 up)                   up.

mat4 lookAtDirnMat(vec3 eye,              Создаёт матрицу вида на основании положения камеры eye и
                   vec3 dirn)             направления взгляда dirn.

mat4 lookAtMat(vec3 eye,                  Создаёт матрицу вида на основании положения камеры eye и
               vec3 center)               указанной точки, задающей центр сцены.

mat4 lookAtDirnMat(vec3 dirn)             Создаёт матрицу вида на основании только направления
                                          взгляда dirn.

                                    ------ФУНКЦИИ ПРОЕЦИРОВАНИЯ------

mat4 orthoMat#(float left, float right,   Создаёт проекционную матрицу для ортографической
               float bottom, float top,   проекции. # может быть GL либо D3D, в зависимости от API,
               float znear = -1,          для которого будет использована матрица.
               float zfar = 1)

mat4 frustumMat#(float left, float right, Создаёт проекционную матрицу, выполняющую перспективное
                 float bottom, float top, преобразование. # может быть GL либо D3D, в зависимости
                 float znear, float zfar) от API, для которого будет использована матрица.

mat4 perspectiveMat#(float fovy,          Создаёт проекционную матрицу, задающую пирамиду видимости
                     float aspect         в мировой системе координат. # может быть GL либо D3D, в
                     float znear,         зависимости от API, для которого будет использована
                     float zfar)          матрица.

                                    ------ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ------

vec3 rotateX(float angle,vec3 vec)        Поворачивает vec вокруг OX на angle в радианах.

mat3 rotationXMat(float angle)            Создаёт матрицу вращения на угол angle в радианах вокруг
mat3 rotationXMat3(float angle)           OX.
mat4 rotationXMat4(float angle)

vec3 rotateY(float angle,vec3 vec)        Поворачивает vec вокруг OY на angle в радианах.

mat3 rotationYMat(float angle)            Создаёт матрицу вращения на угол angle в радианах вокруг
mat3 rotationYMat3(float angle)           OY.
mat4 rotationYMat4(float angle)

vec3 rotateZ(float angle,vec3 vec)        Поворачивает vec вокруг OZ на angle в радианах.

mat3 rotationZMat(float angle)            Создаёт матрицу вращения на угол angle в радианах вокруг
mat3 rotationZMat3(float angle)           OZ.
mat4 rotationZMat4(float angle)

vec3 rotate(float angle,                  Поворачивает vec вокруг вектора axis на angle в радианах.
            vec3 axis,
            vec3 vec)

mat3 rotationMat(float angle,vec3 axis)   Создаёт матрицу вращения на угол angle в радианах вокруг
mat3 rotationMat3(float angle,vec3 axis)  вектора axis.
mat4 rotationMat4(float angle,vec3 axis)

mat4 reflectionMat(vec4 Plane)            Создаёт матрицу отражения относительно плоскости Plane.

vec3 project(vec3 V,vec4 Plane)           Для произвольного вектора V и плоскости Plane, возвращает
vec4 project(vec4 V,vec4 Plane)           проекцию вектора V на Plane.
                                          Плоскость должна быть нормализована для достижения
                                          желаемого результата.

mat4 projectionMat(vec4 Plane)            Создаёт матрицу проецирования на плоскость Plane.

bool intersect(aabb b0,aabb b1)           Возвращает true если b0 и b1 пересекаются, иначе функция
bool intersect(rect r0,rect r1)           возвращает false.
bool intersect(range r0,range r1)

                                    ------ФУНКЦИИ ДЛЯ РАБОТЫ С КВАТЕРНИОНАМИ------

quat conjugate(quat q)                    Возвращает кватернион, сопряженный q.
float dot(quat q0,quat q1)                Возвращает скалярное произведение кватернионов q0 и q1.
float sqlen(quat q)                       Возвращает квадрат длины кватерниона q.
float length(quat q)                      Возвращает модуль (длину) кватерниона q.
quat normalize(quat q)                    Нормализует кватернион q и возвращает результат.
quat inverse(quat q)                      Возвращает кватернион, обратный q.

quat shortestArcQuat(vec3 from,           Возвращает единичный кватернион, который вращает вектор
                     vec3 to)             from к вектору to по кратчайшему пути.

quat axisAngleToQuat(vec3 v,              Создаёт единичный кватернион вращения вокруг вектора v на
                     float alpha)         alpha радиан. Вектор v не обязан быть единичным.

float quatToAxisAngle(out vec3 axis,      Функция возвращает угол поворота единичного кватерниона q
                      quat q)             и в axis помещает единичный вектор-ось, вокруг которого
                                          он производит вращение.

vec3 quatToAxisAngle(quat q)              Функция возвращает вектор-ось, вокруг которого производит
                                          вращение единичный кватернион q. Длина этого вектора
                                          равна углу вращения в радианах.
                                          // (
float quatAngle(quat q)                   Возвращает угол вращения единичного кватерниона q в
                                          интервале [0; 2*PI). // ]

vec3 unitQuatTransformVector(quat q,      Трансформирует заданный вектор v посредством единичного
                             vec3 v)      кватерниона q и возвращает результат.

quat nlerp(quat from,quat to,float t)     Возвращает нормализованную интерполяцию кватернионов from
                                          и to, что практически эквивалентно slerp, но гораздо
                                          быстрее.

quat slerp(quat from,quat to,float t)     Производит интерполяцию кватернионов from и to по
                                          кратчайшей дуге, т.е. возвращает сферическую линейную
                                          интерполяцию (SLERP) кватернионов from и to.

quat squad(quat p,                        Возвращает slerp(slerp(p,q,t), slerp(a,b,t), 2*t*(1-t)),
           quat a,                        т.е. сферическую кубическую интерполяцию p, a, b и q.
           quat b,
           quat q,
           float t)

                                    ------ФУНКЦИИ ДЛЯ РАБОТЫ С ДУАЛ КВАТЕРНИОНАМИ------

conjugate(out quat r[2], quat dq[2])      Записывает в r дуал кватернион, сопряженный dq.
float dot(quat q0[2], quat q1[2])         Возвращает скалярное произведение дуал кватернионов q0 и q1.
fastnormalize(out quat r[2], quat dq[2])  Нормализует дуал кватернион dq и сохраняет результат в r.
inverse(out quat r[2], quat dq[2])        Записывает в r дуал кватернион, обратный dq.

quat quatTransGetDualQuatPart(quat q,     Возвращает мнимую часть дуал кватерниона, построенного на
                              vec3 t)     основе единичного кватерниона q и вектора переноса t.

mat4 dualQuatToMat(quat dq[2])            Преобразует единичный дуал кватернион dq в 4D матрицу и
                                          возвращает результат.

matToDualQuat(out quat dq[2], mat4 M)     Преобразует матрицу M в единичный дуал кватернион и
                                          сохраняет результат в dq.

quatTransToDualQuat(out quat dq[2],       Создаёт единичный дуал кватернион на основе единичного
                    quat q, vec3 t)       кватерниона и вектора переноса. Результат сохраняет в dq.

vec3 dualQuatToQuatTrans(                 Раскладывает единичный дуал кватернион dq на кватернион и
                  out quat q, quat dq[2]) вектор переноса. Полученный кватернион сохраняется в q,
                                          а вектор переноса возвращается функцией напрямую.

vec3 dualQuatGetTrans(quat dq[2])         Извлекает вектор переноса из единичного дуал кватерниона.

vec3 dualQuatTransformPoint(              Трансформирует заданную точку point посредством
                  quat dq[2], vec3 point) единичного дуал кватерниона dq и возвращает результат.

vec3 dualQuatTransformVector(             Трансформирует заданный вектор v посредством единичного
                  quat dq[2], vec3 v)     дуал кватерниона dq и возвращает результат.

dualQuatMul(out quat r[2], quat q1[2],    Вычисляет произведение дуал кватернионов q1 и q2 и
            quat q2[2])                   сохраняет результат в r.

nlerp(out quat r[2], quat from[2],        Считает нормализованную интерполяцию дуал кватернионов
      quat to[2], float t)                from и to и сохраняет результат в r.
*/

#ifndef HM_H
#define HM_H

#include "typedef.h"
#include "vec.h"
#include "mat.h"
#include "quat.h"
#include "aabb.h"
#include "func.h"

#endif //HM_H
