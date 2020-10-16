#include <deque>

template <typename Type> class Deque : public std::deque<Type>
{
public:
	Deque() {}

	using std::deque<Type>::begin,
	      std::deque<Type>::end,
	      std::deque<Type>::clear;

	int len() const {return (int)std::deque<Type>::size();}

	void append(Type &&v) {std::deque<Type>::push_back(std::move(v));}
	void append(const Type &v) {std::deque<Type>::push_back(v);}

	void append_left(Type &&v) {std::deque<Type>::push_front(std::move(v));}
	void append_left(const Type &v) {std::deque<Type>::push_front(v);}

	template <typename Ty> void extend(const Ty &iterable) { for (auto &&el : iterable) append(el); }
	template <typename Ty> void extend_left(const Ty &iterable) { for (auto &&el : iterable) append_left(el); }

	int index(const Type &v, int start = 0) const
	{
		for (int i=start, l=len(); i<l; i++)
			if (std::deque<Type>::at(i) == v) return i;
		throw ValueError(v);
	}

	void insert(int index, const Type &v)
	{
		std::deque<Type>::insert(begin() + index, v);
	}

	Type pop()
	{
		if (std::deque<Type>::empty())
			throw IndexError(0);
		Type r(std::move(std::deque<Type>::at(len()-1)));
		std::deque<Type>::pop_back();
		return r;
	}

	Type pop_left()
	{
		if (std::deque<Type>::empty())
			throw IndexError(0);
		Type r(std::move(std::deque<Type>::at(0)));
		std::deque<Type>::pop_front();
		return r;
	}

	void remove(const Type &val)
	{
		auto it = std::find(begin(), end(), val);
		if (it == end())
			throw ValueError(val);
		std::deque<Type>::erase(it);
	}
};

template <typename Type> inline bool in(const Type &val, const Deque<Type> &d)
{
	return std::find(d.begin(), d.end(), val) != d.end();
}
