#include <math.h>

template <class T, class Ta> inline T snap(const T x, const Ta grid_size) {return floor(x/grid_size + T(0.5)) * grid_size;}

template <class T, class Ta> inline T round(const T number, const Ta ndigits) {return snap(number, pow(0.1, ndigits));}

template <class T> inline T fract(const T x) {return x - floor(x);}

inline double log(const double x, const double base) {return log(x) / log(base);}

static const double pi = 3.1415926535897932384626433832795;

inline double radians(double deg) {return deg*0.017453292519943295769236907684886;}
inline double degrees(double rad) {return rad*57.295779513082320876798154814105;}

inline const short    mod(short    x, short    y) {return x%y;}
inline const int      mod(int      x, int      y) {return x%y;}
inline const unsigned mod(unsigned x, unsigned y) {return x%y;}
template <class T, class Ta> inline T mod(const T &x, const Ta &y) {return x - y*floor(x/y);}

template <class T> inline T sign(const T x) {return x>0 ? T(1) : (x<0 ? T(-1) : 0);}
