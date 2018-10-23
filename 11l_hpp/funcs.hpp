template <class T, class Ta> inline const T snap(const T x, const Ta grid_size) {return floor(x/grid_size + T(0.5)) * grid_size;}

template <class T, class Ta> inline const T round(const T number, const Ta ndigits) {return snap(number, pow(0.1, ndigits));}
