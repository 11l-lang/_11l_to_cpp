#include <initializer_list>
#include <vector>

template <typename Ty> class Array : public std::vector<Ty>
{
public:
    Array(std::initializer_list<Ty> il)
    {}
};

template <class Ty> void create_array(std::initializer_list<Ty> il)
{
    return Array<Ty>(il);
}
