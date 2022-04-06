// [https://github.com/python/cpython/blob/master/Lib/heapq.py <- google:‘heapq cpython’]

namespace minheap
{
template <typename Type> void _siftdown(Array<Type> &heap, Int startpos, Int pos)
{
	auto newitem = heap[pos];
	while (pos > startpos) {
		Int parentpos = (pos - 1) >> 1;
		auto parent = heap[parentpos];
		if (newitem < parent) {
			heap[pos] = parent;
			pos = parentpos;
			continue;
		}
		break;
	}
	heap[pos] = newitem;
}

template <typename Type> void _siftup(Array<Type> &heap, Int pos)
{
	Int endpos = heap.len();
	Int startpos = pos;
	auto newitem = heap[pos];
	Int childpos = 2*pos + 1;
	while (childpos < endpos) {
		Int rightpos = childpos + 1;
		if (rightpos < endpos && !(heap[childpos] < heap[rightpos]))
			childpos = rightpos;
		heap[pos] = heap[childpos];
		pos = childpos;
		childpos = 2*pos + 1;
	}
	heap[pos] = newitem;
	_siftdown(heap, startpos, pos);
}

template <typename Type> void push(Array<Type> &heap, const Type &item)
{
	heap.append(item);
	_siftdown(heap, 0, heap.len()-1);
}

template <typename Type> Type pop(Array<Type> &heap)
{
	auto lastelt = heap.pop();
	if (!heap.empty()) {
		auto returnitem = heap[0];
		heap[0] = lastelt;
		_siftup(heap, 0);
		return returnitem;
	}
	return lastelt;
}

template <typename Type> void heapify(Array<Type> &array)
{
	for (Int i = array.len()/2-1; i >= 0; i--)
		_siftup(array, i);
}
}

namespace maxheap
{
template <typename Type> void _siftdown(Array<Type> &heap, Int startpos, Int pos)
{
	auto newitem = heap[pos];
	while (pos > startpos) {
		Int parentpos = (pos - 1) >> 1;
		auto parent = heap[parentpos];
		if (parent < newitem) {
			heap[pos] = parent;
			pos = parentpos;
			continue;
		}
		break;
	}
	heap[pos] = newitem;
}

template <typename Type> void _siftup(Array<Type> &heap, Int pos)
{
	Int endpos = heap.len();
	Int startpos = pos;
	auto newitem = heap[pos];
	Int childpos = 2*pos + 1;
	while (childpos < endpos) {
		Int rightpos = childpos + 1;
		if (rightpos < endpos && !(heap[rightpos] < heap[childpos]))
			childpos = rightpos;
		heap[pos] = heap[childpos];
		pos = childpos;
		childpos = 2*pos + 1;
	}
	heap[pos] = newitem;
	_siftdown(heap, startpos, pos);
}

template <typename Type> void push(Array<Type> &heap, const Type &item)
{
	heap.append(item);
	_siftdown(heap, 0, heap.len()-1);
}

template <typename Type> Type pop(Array<Type> &heap)
{
	auto lastelt = heap.pop();
	if (!heap.empty()) {
		auto returnitem = heap[0];
		heap[0] = lastelt;
		_siftup(heap, 0);
		return returnitem;
	}
	return lastelt;
}

template <typename Type> void heapify(Array<Type> &array)
{
	for (Int i = array.len()/2-1; i >= 0; i--)
		_siftup(array, i);
}
}
