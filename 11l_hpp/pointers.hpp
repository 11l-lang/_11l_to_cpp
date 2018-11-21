template <class T> class SharedPtr // T must be a class derived from SharedObject
{
	T *object;

	void unconnect()
	{
		if (object) T::dec_ref(object);
	}

	void connect(T *p)
	{
		object = p;
		if (object) object->add_ref();
	}

public:
	SharedPtr():object(nullptr) {}
	SharedPtr(T *p) {connect(p);}
	SharedPtr(const SharedPtr &p) {connect(p.object);}
	SharedPtr &operator=(T *p)
	{
		if (p) p->add_ref();//add_ref() at the very beginning to correctly working assignment to self
		unconnect();
		object = p;
		return *this;
	}
	SharedPtr &operator=(const SharedPtr &p)
	{
		return *this = p.object;
	}

	~SharedPtr() {unconnect();}

	operator T *() const  {return object;}
	T *operator->() const {return object;}
};

class SharedObject
{
	int ref_count;

	SharedObject(const SharedObject &);//forbid copy
	void operator=(const SharedObject &);//and assignment

public:
	SharedObject() : ref_count(0) {}

	void add_ref() {ref_count++;}
	template <class T> static void dec_ref(T *object)
	{
		if (--object->ref_count == 0) delete object;
	}
};

template <class Ty, class... Types> inline SharedPtr<Ty> make_SharedPtr(Types&&... args)
{
	return SharedPtr<Ty>(new Ty(std::forward<Types>(args)...));
}
