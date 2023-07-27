#include "static_vector.hpp"

template <typename Type, size_t fix_len> class ArrayMaxLen : public nstl::static_vector<Type, fix_len>
{
public:
    ArrayMaxLen() {}
    ArrayMaxLen(std::initializer_list<Type> il) : nstl::static_vector<Type, fix_len>(il) {}
    explicit ArrayMaxLen(const String &s)
    {
        nstl::static_vector<Type, fix_len>::reserve(s.len());
        for (auto c : s)
            append(c);
    }

    using nstl::static_vector<Type, fix_len>::begin,
          nstl::static_vector<Type, fix_len>::end,
          nstl::static_vector<Type, fix_len>::clear,
          nstl::static_vector<Type, fix_len>::reserve;

    void drop() {clear();}

    template <typename Func> auto map(Func &&func) const -> ArrayMaxLen<decltype(func(std::declval<Type>())), fix_len>
    {
        ArrayMaxLen<decltype(func(std::declval<Type>())), fix_len> r;
        r.reserve(len());
        for (auto &&el : *this)
            r.push_back(func(el));
        return r;
    }

    template <typename Func, typename = decltype(std::declval<Func>()(std::declval<decltype(tuple_element_f<Type, 0>())>(),
                                                                      std::declval<decltype(tuple_element_f<Type, 1>())>()))> auto map(Func &&func) const
    {
        ArrayMaxLen<decltype(func(std::declval<std::tuple_element_t<0, Type>>(), std::declval<std::tuple_element_t<1, Type>>())), fix_len> r;
        r.reserve(len());
        for (auto &&el : *this)
            r.push_back(func(_get<0>(el), _get<1>(el)));
        return r;
    }

    template <typename Func, typename Dummy = int, typename = decltype(std::declval<Func>()(std::declval<decltype(tuple_element_f<Type, 0>())>(),
                                                                                            std::declval<decltype(tuple_element_f<Type, 1>())>(),
                                                                                            std::declval<decltype(tuple_element_f<Type, 2>())>()))> auto map(Func &&func) const
    {
        ArrayMaxLen<decltype(func(std::declval<std::tuple_element_t<0, Type>>(), std::declval<std::tuple_element_t<1, Type>>(), std::declval<std::tuple_element_t<2, Type>>())), fix_len> r;
        r.reserve(len());
        for (auto &&el : *this)
            r.push_back(func(_get<0>(el), _get<1>(el), _get<2>(el)));
        return r;
    }

    String join(const String &str) const
    {
        String r;
        auto it = begin();
        if (it != end()) {
            r &= *it;
            for (++it; it != end(); ++it) {
                r &= str;
                r &= *it;
            }
        }
        return r;
    }

    template <typename Func, typename = decltype(std::declval<Func>()(std::declval<Type>()))> ArrayMaxLen filter(Func &&func) const
    {
        ArrayMaxLen r;
        for (auto &&el : *this)
            if (func(el))
                r.push_back(el);
        return r;
    }

    template <typename Func, typename Dummy = int, typename = decltype(std::declval<Func>()(std::declval<decltype(tuple_element_f<Type, 0>())>(),
                                                                                            std::declval<decltype(tuple_element_f<Type, 1>())>()))> ArrayMaxLen filter(Func &&func) const
    {
        ArrayMaxLen r;
        for (auto &&el : *this)
            if (func(_get<0>(el), _get<1>(el)))
                r.push_back(el);
        return r;
    }

    template <typename Func, typename Dummy = int, typename Dummy2 = int, typename = decltype(std::declval<Func>()(std::declval<decltype(tuple_element_f<Type, 0>())>(),
                                                                                                                   std::declval<decltype(tuple_element_f<Type, 1>())>(),
                                                                                                                   std::declval<decltype(tuple_element_f<Type, 2>())>()))> ArrayMaxLen filter(Func &&func) const
    {
        ArrayMaxLen r;
        for (auto &&el : *this)
            if (func(_get<0>(el), _get<1>(el), _get<2>(el)))
                r.push_back(el);
        return r;
    }

    template <typename Func> Type reduce(Func &&func) const
    {
        Type r = (*this)[0];
        for (auto it = begin() + 1; it != end(); ++it)
            r = func(r, *it);
        return r;
    }

    template <typename Func> Type reduce(const Type &initial, Func &&func) const
    {
        Type r = initial;
        for (auto &&el : *this)
            r = func(r, el);
        return r;
    }

    Int len() const {return (Int)nstl::static_vector<Type, fix_len>::size();}

    friend ArrayMaxLen operator*(ArrayMaxLen a, Int n)
    {
        if (n < 1) // mimic Python's behavior in which [1] * 0 = [] and [1] * -1 = []
            a.clear();
        else {
            Int len = a.len();
            a.reserve(len * n);
            for (Int i = 1; i < n; i++)
                for (Int j = 0; j < len; j++)
                    a.append(a[j]);
        }
        return std::move(a);
    }
    friend ArrayMaxLen operator*(Int n, ArrayMaxLen a)
    {
        return std::move(a) * n;
    }
/*  friend ArrayMaxLen operator*(const ArrayMaxLen &a, Int n)
    {
        ArrayMaxLen r;
        if (n >= 1) { // mimic Python's behavior in which [1] * 0 = [] and [1] * -1 = []
            Int len = a.len();
            r.reserve(len * n);
            for (Int i = 0; i < n; i++)
                for (Int j = 0; j < len; j++)
                    r.append(a[j]);
        }
        return r;
    }
    friend ArrayMaxLen operator*(Int n, const ArrayMaxLen &a)
    {
        return a * n;
    }*/

    void operator*=(Int n)
    {
        if (n < 1)
            clear();
        else {
            Int l = len();
            reserve(l * n);
            for (Int i = 1; i < n; i++)
                for (Int j = 0; j < l; j++)
                    append(nstl::static_vector<Type, fix_len>::data()[j]);
        }
    }

    ArrayMaxLen operator[](const Range<Int, true,  true > range) const {return slice_e(max(range.b    , Int(0)), min(range.e, len()));}
    ArrayMaxLen operator[](const Range<Int, true,  false> range) const {return slice  (max(range.b    , Int(0)), min(range.e, len()));}
    ArrayMaxLen operator[](const Range<Int, false, true > range) const {return slice_e(max(range.b + 1, Int(0)), min(range.e, len()));}
    ArrayMaxLen operator[](const Range<Int, false, false> range) const {return slice  (max(range.b + 1, Int(0)), min(range.e, len()));}
    ArrayMaxLen operator[](const RangeEI<Int>             range) const {return slice  (max(range.b    , Int(0)),              len() );}
    ArrayMaxLen operator[](const RangeWithStep<Int, true,  true > range) const {return slice_e(max(range.b    , Int(0)), min(range.e, len()), range.step);}
    ArrayMaxLen operator[](const RangeWithStep<Int, true,  false> range) const {return slice  (max(range.b    , Int(0)), min(range.e, len()), range.step);}
    ArrayMaxLen operator[](const RangeWithStep<Int, false, true > range) const {return slice_e(max(range.b + 1, Int(0)), min(range.e, len()), range.step);}
    ArrayMaxLen operator[](const RangeWithStep<Int, false, false> range) const {return slice  (max(range.b + 1, Int(0)), min(range.e, len()), range.step);}
    ArrayMaxLen operator[](const RangeEIWithStep<Int>             range) const {return slice  (max(range.b    , Int(0)), range.step > 0 ? len() : -1, range.step);}

    ArrayMaxLen operator[](const range_e_llen    range) const {return (*this)[range_el(        range.b, len() + range.e)];}
    ArrayMaxLen operator[](const range_elen_elen range) const {return (*this)[range_ee(len() + range.b, len() + range.e)];}
    ArrayMaxLen operator[](const range_elen_llen range) const {return (*this)[range_el(len() + range.b, len() + range.e)];}
    ArrayMaxLen operator[](const range_elen_i    range) const {return (*this)[range_ei(len() + range.b)];}
    ArrayMaxLen operator[](const range_elen_i_wstep range) const {return (*this)[range_ei(len() + range.b).step(range.s)];}
    ArrayMaxLen operator[](const range_elen_e_wstep range) const {return (*this)[range_ee(len() + range.b, range.e).step(range.s)];}

    decltype(std::declval<const nstl::static_vector<Type, fix_len>>().at(0)) operator[](Int i) const // decltype is needed for ArrayMaxLen<bool> support
    {
#ifdef PYTHON_NEGATIVE_INDEXING
        if (i < 0)
            i += len();
#endif
        if (in(i, range_el(Int(0), len())))
            return nstl::static_vector<Type, fix_len>::operator[](i);
        throw IndexError(i);
    }

    decltype(std::declval<nstl::static_vector<Type, fix_len>>().at(0)) operator[](Int i) // decltype is needed for ArrayMaxLen<bool> support
    {
#ifdef PYTHON_NEGATIVE_INDEXING
        if (i < 0)
            i += len();
#endif
        if (in(i, range_el(Int(0), len())))
            return nstl::static_vector<Type, fix_len>::operator[](i);
        throw IndexError(i);
    }

    const Type &at_plus_len(Int i) const
    {
#ifndef PYTHON_NEGATIVE_INDEXING
        i += len();
#endif
        return operator[](i);
    }

    Type &at_plus_len(Int i)
    {
#ifndef PYTHON_NEGATIVE_INDEXING
        i += len();
#endif
        return operator[](i);
    }

    const Type &set(Int i, const Type &v) // return `const Type&` for [https://www.rosettacode.org/wiki/Perlin_noise#Python]:‘p[256+i] = p[i] = permutation[i]’
    {
#ifdef PYTHON_NEGATIVE_INDEXING
        if (i < 0)
            i += len();
#endif
        if (in(i, range_el(Int(0), len())))
            return nstl::static_vector<Type, fix_len>::operator[](i) = v;
        else
            throw IndexError(i);
    }

    const Type &set_plus_len(Int i, const Type &v)
    {
#ifndef PYTHON_NEGATIVE_INDEXING
        i += len();
#endif
        return set(i, v);
    }

    decltype(std::declval<const nstl::static_vector<Type, fix_len>>().at(0)) at_ni(Int ii) const // decltype is needed for ArrayMaxLen<bool> support
    {
        Int i = ii;
        if (i < 0)
            i += len();
        if (in(i, range_el(Int(0), len())))
            return nstl::static_vector<Type, fix_len>::operator[](i);
        throw IndexError(ii);
    }

    decltype(std::declval<nstl::static_vector<Type, fix_len>>().at(0)) at_ni(Int ii) // decltype is needed for ArrayMaxLen<bool> support
    {
        Int i = ii;
        if (i < 0)
            i += len();
        if (in(i, range_el(Int(0), len())))
            return nstl::static_vector<Type, fix_len>::operator[](i);
        throw IndexError(ii);
    }

    void append(Type &&v) {nstl::static_vector<Type, fix_len>::push_back(std::move(v));}
    void append(const Type &v) {nstl::static_vector<Type, fix_len>::push_back(v);}

    void append(const ArrayMaxLen<Type, fix_len> &arr)
    {
        reserve(nstl::static_vector<Type, fix_len>::size() + arr.size());
        for (auto &&el : arr)
            append(el);
    }

    template <typename Ty, bool include_beginning, bool include_ending> void append(const Range<Ty, include_beginning, include_ending> &range)
    {
        reserve(nstl::static_vector<Type, fix_len>::size() + range.size());
        for (auto i : range)
            append(i);
    }
//  template <typename Ty, bool include_beginning, bool include_ending> void operator+=(const Range<Ty, include_beginning, include_ending> &range)
//  {
//      append(range);
//  }

    template <typename Ty> void extend(const Ty &iterable)
    {
        reserve(nstl::static_vector<Type, fix_len>::size() + iterable.len());
        for (auto &&el : iterable)
            append(el);
    }

    void insert(Int index, const Type &v)
    {
        nstl::static_vector<Type, fix_len>::insert(begin() + index, v);
    }

    Int index(const Type &v, Int start, Int end) const
    {
        for (Int i=start; i<end; i++)
            if (nstl::static_vector<Type, fix_len>::data()[i] == v) return i;
        throw ValueError(v);
    }
    Int index(const Type &v, Int start = 0) const
    {
        return index(v, start, len());
    }

    Nullable<Int> find(const Type &val, Int start = 0) const
    {
        auto it = std::find(begin() + start, end(), val);
        if (it != end())
            return Int(it - begin());
        return nullptr;
    }

    Nullable<Int> find(const decltype(make_tuple(std::declval<Type>(), std::declval<Type>())) &t, Int start = 0) const
    {
        for (auto it = begin() + start; it != end(); ++it) {
            if (*it == _get<0>(t)) return Int(it - begin());
            if (*it == _get<1>(t)) return Int(it - begin());
        }
        return nullptr;
    }

    template <typename Ty> Nullable<Int> find(const Tuple<Ty, Ty> &t, Int start = 0) const
    {
        return find(make_tuple((Type)_get<0>(t), (Type)_get<1>(t)), start);
    }
    template <typename Ty> Nullable<Int> find(const Tvec<Ty, 2> &t, Int start = 0) const
    {
        return find(make_tuple((Type)_get<0>(t), (Type)_get<1>(t)), start);
    }

    Int count(const Type &val) const
    {
        return (Int)std::count(begin(), end(), val);
    }

    void sort()
    {
        std::sort(begin(), end());
    }

    template <typename Key> void sort(Key &&key, bool reverse = false)
    {
        if (!reverse)
            std::sort(begin(), end(), [key](const Type &a, const Type &b) {return key(a) < key(b);});
        else
            std::sort(begin(), end(), [key](const Type &a, const Type &b) {return key(b) < key(a);});
    }

    void sort(nullptr_t, bool reverse)
    {
        if (!reverse)
            std::sort(begin(), end());
        else
            std::sort(begin(), end(), std::greater<>()); // [https://stackoverflow.com/a/37757410/2692494 <- google:‘std sort reverse’]
    }

    void sort_range(const Range<Int, true,  true> range) {std::sort(begin() + range.b, begin() + range.e + 1);}
    void sort_range(const Range<Int, true, false> range) {std::sort(begin() + range.b, begin() + range.e);}
    void sort_range(const RangeEI<Int>            range) {std::sort(begin() + range.b, end());}

    void sort_range(const Range<Int, true, false> range, nullptr_t, bool reverse)
    {
        if (!reverse)
            std::sort(begin() + range.b, begin() + range.e);
        else
            std::sort(begin() + range.b, begin() + range.e, std::greater<>());
    }

    void reverse()
    {
        std::reverse(begin(), end());
    }

    void reverse_range(const Range<Int, true,  true> range) {std::reverse(begin() + range.b, begin() + range.e + 1);}
    void reverse_range(const Range<Int, true, false> range) {std::reverse(begin() + range.b, begin() + range.e);}
    void reverse_range(const RangeEI<Int>            range) {std::reverse(begin() + range.b, end());}
    void reverse_range(const range_elen_i            range) {reverse_range(range_ei(len() + range.b));}

    bool next_permutation()
    {
        return std::next_permutation(begin(), end());
    }

    bool is_sorted() const
    {
        return std::is_sorted(begin(), end());
    }

    const Type &last() const
    {
        if (nstl::static_vector<Type, fix_len>::empty())
            throw IndexError(0);
        return nstl::static_vector<Type, fix_len>::operator[](len()-1);
    }

    Type &last()
    {
        if (nstl::static_vector<Type, fix_len>::empty())
            throw IndexError(0);
        return nstl::static_vector<Type, fix_len>::operator[](len()-1);
    }

    Type pop()
    {
        if (nstl::static_vector<Type, fix_len>::empty())
            throw IndexError(0);
        Type r(std::move(nstl::static_vector<Type, fix_len>::operator[](len()-1)));
        nstl::static_vector<Type, fix_len>::pop_back();
        return r;
    }

    Type pop(Int i)
    {
#ifdef PYTHON_NEGATIVE_INDEXING
        if (i < 0)
            i += len();
#endif
        if (!in(i, range_el(Int(0), len())))
            throw IndexError(i);
        Type r(std::move((*this)[i]));
        nstl::static_vector<Type, fix_len>::erase(begin() + i);
        return r;
    }

    Type pop_fast(Int i)
    {
#ifdef PYTHON_NEGATIVE_INDEXING
        if (i < 0)
            i += len();
#endif
        if (!in(i, range_el(Int(0), len())))
            throw IndexError(i);
        Type r(std::move((*this)[i]));
        std::swap((*this)[i], nstl::static_vector<Type, fix_len>::back());
        nstl::static_vector<Type, fix_len>::pop_back();
        return r;
    }

    Type pop_plus_len(Int i)
    {
#ifndef PYTHON_NEGATIVE_INDEXING
        i += len();
#endif
        return pop(i);
    }

    void remove(const Type &val)
    {
        auto it = std::find(begin(), end(), val);
        if (it == end())
            throw ValueError(val);
        nstl::static_vector<Type, fix_len>::erase(it);
    }

    void del(const Range<Int, true, false> range)
    {
        nstl::static_vector<Type, fix_len>::erase(begin() + range.b, begin() + range.e);
    }
    void del(const RangeEI<Int> range)
    {
        nstl::static_vector<Type, fix_len>::erase(begin() + range.b, end());
    }

    String decode(const String &encoding = u"utf-8"_S) const
    {
        static_assert(std::is_same_v<Type, Byte>, "`decode()` can be called only on `ArrayMaxLen<Byte>`");

        if (encoding == u"ascii") {
            String r;
            r.reserve(len());
            for (auto el : *this) {
                if (!(el < 128))
                    throw AssertionError();
                r &= Char(el);
            }
            return r;
        }
        else if (encoding == u"utf-8") {
            String convert_utf8_string_to_String(const char *s, size_t len);
            return convert_utf8_string_to_String((char*)nstl::static_vector<Type, fix_len>::data(), nstl::static_vector<Type, fix_len>::size());
        }
        else
            throw AssertionError();
    }

    String hex() const
    {
        static_assert(std::is_same_v<Type, Byte>, "`hex()` can be called only on `ArrayMaxLen<Byte>`");

        String r;
        r.reserve(len() * 2);
        for (auto el : *this) {
            r &= Char(hex_to_char(el >> 4));
            r &= Char(hex_to_char(el & 0xF));
        }
        return r;
    }

    template <typename Ty> bool operator==(const ArrayMaxLen<Ty, fix_len> &arr) const
    {
        if (len() != arr.len()) return false;
        for (Int i=0, n=len(); i<n; i++)
            if (!(nstl::static_vector<Type, fix_len>::operator[](i) == arr.operator[](i)))
                return false;
        return true;
    }
    template <typename Ty> bool operator!=(const ArrayMaxLen<Ty, fix_len> &arr) const
    {
        return !(*this == arr);
    }
};
