#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

struct CodeBlock1
{
    CodeBlock1()
    {
        uR"(
timsort

converted from java version by dan stromberg

modified by mark.dufour@gmail.com to work with shedskin

)"_S;
        randomns::seed(1);
    }
} code_block_1;
auto cmp = [](const auto &a, const auto &b){return a - b;};

template <typename T1, typename T2, typename T4, typename T5> auto array_copy(const T1 &list1, const T2 &base1, Array<int> &list2, const T4 &base2, const T5 &length)
{
    uR"(
	Copy from list1 to list2 at offsets base1 and base2, for length elements.
	Like Java's System.arraycopy.
	)"_S;
    bool copy_forward;
    if (&list1 == &list2) {
        if (base1 < base2)
            copy_forward = false;
        else
            copy_forward = true;
    }
    else
        copy_forward = true;
    if (copy_forward)
        for (auto offset : range_el(0, length))
            list2.set(base2 + offset, list1[base1 + offset]);
    else
        for (auto offset : range_el(length - 1, -1).step(-1))
            list2.set(base2 + offset, list1[base1 + offset]);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> auto gallop_right(const T1 &key, const T2 &list_, const T3 &base, const T4 &length, const T5 &hint, const T6 &comparefn)
{
    uR"(
	Like gallop_left, except that if the range contains an element equal to
	key, gallop_right returns the index after the rightmost equal element.

	@param key the key whose insertion point to search for
	@param list_ the array in which to search
	@param base the index of the first element in the range
	@param length the length of the range; must be > 0
	@param hint the index at which to begin the search, 0 <= hint < number.
	 The closer hint is to the result, the faster this method will run.
	@param comparefn the comparator used to order the range, and to search
	@return the int k,  0 <= k <= number such that list_[b + k - 1] <= key < list_[b + k]
	)"_S;
    assert(length > 0 && hint >= 0 && hint < length);
    auto offset = 1;
    auto last_offset = 0;
    if (comparefn(key, list_[base + hint]) < 0) {
        auto max_offset = hint + 1;
        while (offset < max_offset && comparefn(key, list_[base + hint - offset]) < 0) {
            last_offset = offset;
            offset = (offset << 1) + 1;
            if (offset <= 0)
                offset = max_offset;
        }
        if (offset > max_offset)
            offset = max_offset;
        auto tmp = last_offset;
        last_offset = hint - offset;
        offset = hint - tmp;
    }
    else {
        auto max_offset = length - hint;
        while (offset < max_offset && comparefn(key, list_[base + hint + offset]) >= 0) {
            last_offset = offset;
            offset = (offset << 1) + 1;
            if (offset <= 0)
                offset = max_offset;
        }
        if (offset > max_offset)
            offset = max_offset;
        last_offset += hint;
        offset += hint;
    }
    assert(-1 <= last_offset && last_offset < offset && offset <= length);
    last_offset++;
    while (last_offset < offset) {
        auto midpoint = last_offset + (idiv((offset - last_offset), 2));
        if (comparefn(key, list_[base + midpoint]) < 0)
            offset = midpoint;
        else
            last_offset = midpoint + 1;
    }
    assert(last_offset == offset);
    return offset;
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> auto gallop_left(const T1 &key, const T2 &list_, const T3 &base, const T4 &length, const T5 &hint, const T6 &comparefn)
{
    uR"(
	Locates the position at which to insert the specified key into the
	specified sorted range; if the range contains an element equal to key,
	returns the index of the leftmost equal element.

	@param key the key whose insertion point to search for
	@param list_ the array in which to search
	@param base the index of the first element in the range
	@param length the length of the range; must be > 0
	@param hint the index at which to begin the search, 0 <= hint < number.
	 The closer hint is to the result, the faster this method will run.
	@param c the comparator used to order the range, and to search
	@return the int k,  0 <= k <= number such that list_[b + k - 1] < key <= list_[b + k],
	pretending that list_[b - 1] is minus infinity and list_[b + number] is infinity.
	In other words, key belongs at index b + k; or in other words,
	the first k elements of list_ should precede key, and the last number - k
	should follow it.
	)"_S;
    assert(length > 0 && hint >= 0 && hint < length);
    auto last_offset = 0;
    auto offset = 1;
    if (comparefn(key, list_[base + hint]) > 0) {
        auto max_offset = length - hint;
        while (offset < max_offset && comparefn(key, list_[base + hint + offset]) > 0) {
            last_offset = offset;
            offset = (offset << 1) + 1;
            if (offset <= 0)
                offset = max_offset;
        }
        if (offset > max_offset)
            offset = max_offset;
        last_offset += hint;
        offset += hint;
    }
    else {
        auto max_offset = hint + 1;
        while (offset < max_offset && comparefn(key, list_[base + hint - offset]) <= 0) {
            last_offset = offset;
            offset = (offset << 1) + 1;
            if (offset <= 0)
                offset = max_offset;
        }
        if (offset > max_offset)
            offset = max_offset;
        auto tmp = last_offset;
        last_offset = hint - offset;
        offset = hint - tmp;
    }
    assert(-1 <= last_offset && last_offset < offset && offset <= length);
    last_offset++;
    while (last_offset < offset) {
        auto midpoint = last_offset + (idiv((offset - last_offset), 2));
        if (comparefn(key, list_[base + midpoint]) > 0)
            last_offset = midpoint + 1;
        else
            offset = midpoint;
    }
    assert(last_offset == offset);
    return offset;
}

template <typename T2, typename T3, typename T4, typename T5> auto binary_sort(Array<int> &list_, const T2 &low, const T3 &high, T4 start, const T5 &comparefn)
{
    uR"(
	Sorts the specified portion of the specified array using a binary
	insertion sort.  This is the best method for sorting small numbers
	of elements.  It requires O(n log n) compares, but O(n^2) data
	movement (worst case).

	If the initial part of the specified range is already sorted,
	this method can take advantage of it: the method assumes that the
	elements from index {@code low}, inclusive, to {@code start},
	exclusive are already sorted.

	@param list_ the array in which a range is to be sorted
	@param low the index of the first element in the range to be sorted
	@param high the index after the last element in the range to be sorted
	@param start the index of the first element in the range that is
		not already known to be sorted (@code low <= start <= high)
	@param comparefn comparator to used for the sort
	)"_S;
    assert(low <= start && start <= high);
    if (start == low)
        start++;
    for (auto start : range_el(start, high)) {
        auto pivot = list_[start];
        auto left = low;
        auto right = start;
        assert(left <= right);

        while (left < right) {
            auto mid = idiv((left + right), 2);
            if (comparefn(pivot, list_[mid]) < 0)
                right = mid;
            else
                left = mid + 1;
        }
        assert(left == right);
        auto number = start - left;
        array_copy(list_, left, list_, left + 1, number);
        list_.set(left, pivot);
    }
}

template <typename T2, typename T3> auto reverse_range(Array<int> &list_, T2 low, T3 high)
{
    uR"(
	Reverse the specified range of the specified array.

	@param list_ the array in which a range is to be reversed
	@param low the index of the first element in the range to be reversed
	@param high the index after the last element in the range to be reversed
	)"_S;
    high--;
    while (low < high) {
        assign_from_tuple(list_[low], list_[high], make_tuple(list_[high], list_[low]));
        low++;
        high--;
    }
}

template <typename T2, typename T3, typename T4> auto count_run_and_make_ascending(Array<int> &list_, const T2 &low, const T3 &high, const T4 &comparefn)
{
    uR"(
	Returns the length of the run beginning at the specified position in
	the specified array and reverses the run if it is descending (ensuring
	that the run will always be ascending when the method returns).

	A run is the longest ascending sequence with:

		list_[low] <= list_[low + 1] <= list_[low + 2] <= ...

	or the longest descending sequence with:

		list_[low] >  list_[low + 1] >  list_[low + 2] >  ...

	For its intended use in a stable mergesort, the strictness of the
	definition of "descending" is needed so that the call can safely
	reverse a descending sequence without violating stability.

	@param list_ the array in which a run is to be counted and possibly reversed
	@param low index of the first element in the run
	@param high index after the last element that may be contained in the run.
	 It is required that @code{low < high}.
	@param comparefn the comparator to used for the sort
	@return  the length of the run beginning at the specified position in
			  the specified array
	)"_S;
    assert(low < high);
    auto run_high = low + 1;
    if (run_high == high)
        return 1;

    if (comparefn(list_[run_high], list_[low]) < 0) {
        run_high++;
        while (run_high < high && comparefn(list_[run_high], list_[run_high - 1]) < 0)
            run_high++;
        reverse_range(list_, low, run_high);
    }
    else {
        run_high++;
        while (run_high < high && comparefn(list_[run_high], list_[run_high - 1]) >= 0)
            run_high++;
    }
    return run_high - low;
}

class Timsort
{
public:
    Array<int> list_;
    Array<int> tmp;
    std::function<int(const int, const int)> comparefn;
    int min_gallop;
    decltype(32) min_merge = 32;
    decltype(7) initial_min_gallop = 7;
    decltype(0) stack_size = 0;
    decltype(256) initial_tmp_storage_length = 256;
    Array<int> run_base;
    Array<int> run_len;
    int stack_len;

    template <typename T1, typename T2 = decltype(cmp)> Timsort(const T1 &list_, const T2 &comparefn = cmp) :
        list_(list_),
        comparefn(comparefn)
    {
        min_gallop = initial_min_gallop;
        auto length = list_.len();
        int ternary;
        if (length < initial_tmp_storage_length * 2)
            ternary = idiv(length, 2);
        else
            ternary = initial_tmp_storage_length;
        tmp = create_array(range_el(0, ternary));
        if (length < 120)
            stack_len = 5;
        else if (length < 1542)
            stack_len = 10;
        else if (length < 119151)
            stack_len = 19;
        else
            stack_len = 40;
        run_base = create_array(range_el(0, stack_len));
        run_len = create_array(range_el(0, stack_len));
    }

    template <typename T1, typename T2, typename T3 = decltype(cmp)> auto sort(T1 low, const T2 &high, const T3 &comparefn = cmp)
    {
        u"sort method - perform a sort :)"_S;
        range_check(list_.len(), low, high);
        auto num_remaining = high - low;
        if (num_remaining < 2)
            return;

        if (num_remaining < min_merge) {
            auto initial_run_len = count_run_and_make_ascending(list_, low, high, comparefn);
            binary_sort(list_, low, high, low + initial_run_len, comparefn);
            return;
        }
        auto min_run = min_run_length(num_remaining);
        while (true) {
            auto run_len = count_run_and_make_ascending(list_, low, high, comparefn);

            if (run_len < min_run) {
                int ternary;
                if (num_remaining <= min_run)
                    ternary = num_remaining;
                else
                    ternary = min_run;
                auto force = ternary;
                binary_sort(list_, low, low + force, low + run_len, comparefn);
                run_len = force;
            }
            push_run(low, run_len);
            merge_collapse();
            low += run_len;
            num_remaining -= run_len;
            if (num_remaining == 0)
                break;
        }
        assert(low == high);
        merge_force_collapse();
        assert(stack_size == 1);
    }

    template <typename T1> auto min_run_length(T1 number)
    {
        uR"(
		Returns the minimum acceptable run length for an array of the specified
		length. Natural runs shorter than this will be extended with
		{@link #binary_sort}.

		Roughly speaking, the computation is:

		 If number < self.min_merge, return number (it's too small to bother with fancy stuff).
		 Else if number is an exact power of 2, return self.min_merge/2.
		 Else return an int k, self.min_merge/2 <= k <= self.min_merge, such that number/k
		  is close to, but strictly less than, an exact power of 2.

		For the rationale, see listsort.txt.

		@param number the length of the array to be sorted
		@return the length of the minimum run to be merged
		)"_S;
        assert(number >= 0);
        auto low_bit = 0;

        while (number >= min_merge) {
            low_bit |= (number & 1);
            number >>= 1;
        }
        return number + low_bit;
    }

    template <typename T1, typename T2> auto push_run(const T1 &run_base, const T2 &run_len)
    {
        uR"(
		Pushes the specified run onto the pending-run stack.

		@param self.run_base index of the first element in the run
		@param self.run_len  the number of elements in the run
		)"_S;
        this->run_base.set(stack_size, run_base);
        this->run_len.set(stack_size, run_len);
        stack_size++;
    }

    auto merge_collapse()
    {
        uR"(
		Examines the stack of runs waiting to be merged and merges adjacent runs
		until the stack invariants are reestablished:

		 1. self.run_len[i - 3] > self.run_len[i - 2] + self.run_len[i - 1]
		 2. self.run_len[i - 2] > self.run_len[i - 1]

		This method is called each time a new run is pushed onto the stack,
		so the invariants are guaranteed to hold for i < self.stack_size upon
		entry to the method.
		)"_S;
        while (stack_size > 1) {
            auto number = stack_size - 2;
            if (number > 0 && run_len[number - 1] <= run_len[number] + run_len[number + 1]) {
                if (run_len[number - 1] < run_len[number + 1])
                    number--;
                merge_at(number);
            }
            else if (run_len[number] <= run_len[number + 1])
                merge_at(number);
            else
                break;
        }
    }

    auto merge_force_collapse()
    {
        uR"(
		Merges all runs on the stack until only one remains.  This method is
		called once, to complete the sort.
		)"_S;
        while (stack_size > 1) {
            auto number = stack_size - 2;
            if (number > 0 && run_len[number - 1] < run_len[number + 1])
                number--;
            merge_at(number);
        }
    }

    template <typename T1> auto merge_at(const T1 &i)
    {
        uR"(
		Merges the two runs at stack indices i and i+1.  Run i must be
		the penultimate or antepenultimate run on the stack.  In other words,
		i must be equal to self.stack_size-2 or self.stack_size-3.

		@param i stack index of the first of the two runs to merge
		)"_S;
        assert(stack_size >= 2);
        assert(i >= 0);
        assert(i == stack_size - 2 || i == stack_size - 3);
        auto base1 = run_base[i];
        auto len1 = run_len[i];
        auto base2 = run_base[i + 1];
        auto len2 = run_len[i + 1];
        assert(len1 > 0 && len2 > 0);
        assert(base1 + len1 == base2);
        run_len.set(i, len1 + len2);
        if (i == stack_size - 3) {
            run_base.set(i + 1, run_base[i + 2]);
            run_len.set(i + 1, run_len[i + 2]);
        }
        stack_size--;
        auto k = gallop_right(list_[base2], list_, base1, len1, 0, comparefn);
        assert(k >= 0);
        base1 += k;
        len1 -= k;
        if (len1 == 0)
            return;
        len2 = gallop_left(list_[base1 + len1 - 1], list_, base2, len2, len2 - 1, comparefn);
        assert(len2 >= 0);
        if (len2 == 0)
            return;
        if (len1 <= len2)
            merge_low(base1, len1, base2, len2);
        else
            merge_high(base1, len1, base2, len2);
    }

    template <typename T1, typename T2, typename T3, typename T4> auto merge_low(const T1 &base1, T2 len1, const T3 &base2, T4 len2)
    {
        uR"(
		Merges two adjacent runs in place, in a stable fashion.  The first
		element of the first run must be greater than the first element of the
		second run (list_[base1] > list_[base2]), and the last element of the first run
		(list_[base1 + len1-1]) must be greater than all elements of the second run.

		For performance, this method should be called only when len1 <= len2
		its twin, merge_high should be called if len1 >= len2.  (Either method
		may be called if len1 == len2.)

		@param base1 index of first element in first run to be merged
		@param len1  length of first run to be merged (must be > 0)
		@param base2 index of first element in second run to be merged
			(must be aBase + aLen)
		@param len2  length of second run to be merged (must be > 0)
		)"_S;
        assert(len1 > 0 && len2 > 0 && base1 + len1 == base2);
        auto tmp = ensure_capacity(len1);
        array_copy(list_, base1, tmp, 0, len1);
        auto cursor1 = 0;
        auto cursor2 = base2;
        auto dest = base1;
        list_.set(dest, list_[cursor2]);
        dest++;
        cursor2++;
        if (--len2 == 0) {
            array_copy(tmp, cursor1, list_, dest, len1);
            return;
        }
        if (len1 == 1) {
            array_copy(list_, cursor2, list_, dest, len2);
            list_.set(dest + len2, tmp[cursor1]);
            return;
        }
        auto comparefn = this->comparefn;
        auto min_gallop = this->min_gallop;
        auto loops_done = false;
        while (true) {
            auto count1 = 0;
            auto count2 = 0;

            while (true) {
                assert(len1 > 1 && len2 > 0);
                if (comparefn(list_[cursor2], tmp[cursor1]) < 0) {
                    list_.set(dest, list_[cursor2]);
                    dest++;
                    cursor2++;
                    count2++;
                    count1 = 0;
                    if (--len2 == 0) {
                        loops_done = true;
                        break;
                    }
                }
                else {
                    list_.set(dest, tmp[cursor1]);
                    dest++;
                    cursor1++;
                    count1++;
                    count2 = 0;
                    len1--;
                    if (len1 == 1) {
                        loops_done = true;
                        break;
                    }
                }
                if ((count1 | count2) >= min_gallop)
                    break;
            }
            if (loops_done)
                break;

            while (true) {
                assert(len1 > 1 && len2 > 0);
                count1 = gallop_right(list_[cursor2], tmp, cursor1, len1, 0, comparefn);
                if (count1 != 0) {
                    array_copy(tmp, cursor1, list_, dest, count1);
                    dest += count1;
                    cursor1 += count1;
                    len1 -= count1;
                    if (len1 <= 1) {
                        loops_done = true;
                        break;
                    }
                }
                list_.set(dest, list_[cursor2]);
                dest++;
                cursor2++;
                if (--len2 == 0) {
                    loops_done = true;
                    break;
                }
                count2 = gallop_left(tmp[cursor1], list_, cursor2, len2, 0, comparefn);
                if (count2 != 0) {
                    array_copy(list_, cursor2, list_, dest, count2);
                    dest += count2;
                    cursor2 += count2;
                    len2 -= count2;
                    if (len2 == 0) {
                        loops_done = true;
                        break;
                    }
                }
                list_.set(dest, tmp[cursor1]);
                dest++;
                cursor1++;
                if (--len1 == 1) {
                    loops_done = true;
                    break;
                }
                min_gallop--;
                if (!(count1 >= initial_min_gallop || count2 >= initial_min_gallop))
                    break;
            }
            if (loops_done)
                break;
            if (min_gallop < 0)
                min_gallop = 0;
            min_gallop += 2;
        }
        int ternary;
        if (min_gallop < 1)
            ternary = 1;
        else
            ternary = min_gallop;
        this->min_gallop = ternary;

        if (len1 == 1) {
            assert(len2 > 0);
            array_copy(list_, cursor2, list_, dest, len2);
            list_.set(dest + len2, tmp[cursor1]);
        }
        else if (len1 == 0)
            throw ValueError(u"Comparison function violates its general contract!"_S);
        else {
            assert(len2 == 0);
            assert(len1 > 1);
            array_copy(tmp, cursor1, list_, dest, len1);
        }
    }

    template <typename T1, typename T2, typename T3, typename T4> auto merge_high(const T1 &base1, T2 len1, const T3 &base2, T4 len2)
    {
        uR"(
		Like merge_low, except that this method should be called only if
		len1 >= len2; merge_low should be called if len1 <= len2.  (Either method
		may be called if len1 == len2.)

		@param base1 index of first element in first run to be merged
		@param len1  length of first run to be merged (must be > 0)
		@param base2 index of first element in second run to be merged
			(must be aBase + aLen)
		@param len2  length of second run to be merged (must be > 0)
		)"_S;
        assert(len1 > 0 && len2 > 0 && base1 + len1 == base2);
        auto tmp = ensure_capacity(len2);
        array_copy(list_, base2, tmp, 0, len2);
        auto cursor1 = base1 + len1 - 1;
        auto cursor2 = len2 - 1;
        auto dest = base2 + len2 - 1;
        list_.set(dest, list_[cursor1]);
        dest--;
        cursor1--;
        if (--len1 == 0) {
            array_copy(tmp, 0, list_, dest - (len2 - 1), len2);
            return;
        }
        if (len2 == 1) {
            dest -= len1;
            cursor1 -= len1;
            array_copy(list_, cursor1 + 1, list_, dest + 1, len1);
            list_.set(dest, tmp[cursor2]);
            return;
        }
        auto comparefn = this->comparefn;
        auto min_gallop = this->min_gallop;
        auto loops_done = false;
        while (true) {
            auto count1 = 0;
            auto count2 = 0;

            while (true) {
                assert(len1 > 0 && len2 > 1);
                if (comparefn(tmp[cursor2], list_[cursor1]) < 0) {
                    list_.set(dest, list_[cursor1]);
                    dest--;
                    cursor1--;
                    count1++;
                    count2 = 0;
                    if (--len1 == 0) {
                        loops_done = true;
                        break;
                    }
                }
                else {
                    list_.set(dest, tmp[cursor2]);
                    dest--;
                    cursor2--;
                    count2++;
                    count1 = 0;
                    len2--;
                    if (len2 == 1) {
                        loops_done = true;
                        break;
                    }
                }
                if (!((count1 | count2) < min_gallop))
                    break;
            }
            if (loops_done)
                break;

            while (true) {
                assert(len1 > 0 && len2 > 1);
                count1 = len1 - gallop_right(tmp[cursor2], list_, base1, len1, len1 - 1, comparefn);
                if (count1 != 0) {
                    dest -= count1;
                    cursor1 -= count1;
                    len1 -= count1;
                    array_copy(list_, cursor1 + 1, list_, dest + 1, count1);
                    if (len1 == 0) {
                        loops_done = true;
                        break;
                    }
                }
                list_.set(dest, tmp[cursor2]);
                dest--;
                cursor2--;
                if (--len2 == 1) {
                    loops_done = true;
                    break;
                }
                count2 = len2 - gallop_left(list_[cursor1], tmp, 0, len2, len2 - 1, comparefn);
                if (count2 != 0) {
                    dest -= count2;
                    cursor2 -= count2;
                    len2 -= count2;
                    array_copy(tmp, cursor2 + 1, list_, dest + 1, count2);
                    if (len2 <= 1) {
                        loops_done = true;
                        break;
                    }
                }
                list_.set(dest, list_[cursor1]);
                dest--;
                cursor1--;
                if (--len1 == 0) {
                    loops_done = true;
                    break;
                }
                min_gallop--;
                if (!(count1 >= initial_min_gallop || count2 >= initial_min_gallop))
                    break;
            }
            if (loops_done)
                break;
            if (min_gallop < 0)
                min_gallop = 0;
            min_gallop += 2;
        }
        int ternary;
        if (min_gallop < 1)
            ternary = 1;
        else
            ternary = min_gallop;
        this->min_gallop = ternary;

        if (len2 == 1) {
            assert(len1 > 0);
            dest -= len1;
            cursor1 -= len1;
            array_copy(list_, cursor1 + 1, list_, dest + 1, len1);
            list_.set(dest, tmp[cursor2]);
        }
        else if (len2 == 0)
            throw ValueError(u"Comparison function violates its general contract!"_S);
        else {
            assert(len1 == 0);
            assert(len2 > 0);
            array_copy(tmp, 0, list_, dest - (len2 - 1), len2);
        }
    }

    template <typename T1> auto ensure_capacity(const T1 &min_capacity)
    {
        uR"(
		Ensures that the external array tmp has at least the specified
		number of elements, increasing its size if necessary.  The size
		increases exponentially to ensure amortized linear time complexity.

		@param min_capacity the minimum required capacity of the tmp array
		@return tmp, whether or not it grew
		)"_S;
        if (tmp.len() < min_capacity) {
            auto new_size = min_capacity;
            new_size |= new_size >> 1;
            new_size |= new_size >> 2;
            new_size |= new_size >> 4;
            new_size |= new_size >> 8;
            new_size |= new_size >> 16;
            new_size++;
            if (new_size < 0)
                new_size = min_capacity;
            else
                new_size = min(new_size, idiv(list_.len(), 2));
            tmp = create_array(range_el(0, new_size));
        }
        return tmp;
    }
};

template <typename T1, typename T2 = decltype(cmp)> auto timsort(const T1 &list_, const T2 &comparefn = cmp)
{
    u"Sort function for timsort"_S;
    auto timsort_object = Timsort(list_, comparefn);
    timsort_object.sort(0, list_.len());
    assert(timsort_object.list_ == sorted(timsort_object.list_));
    return timsort_object.list_;
}

template <typename T1, typename T2, typename T3> auto range_check(const T1 &array_len, const T2 &from_index, const T3 &to_index)
{
    uR"(
	Checks that from_index and to_index are in range, and throws an
	appropriate exception if they aren't.

	@param array_len the length of the array
	@param from_index the index of the first element of the range
	@param to_index the index after the last element of the range
	@throws IllegalArgumentException if from_index > to_index
	@throws ArrayIndexOutOfBoundsException if from_index < 0
		 or to_index > array_len
	)"_S;
    if (from_index > to_index)
        throw ValueError(u"from_index("_S + String(from_index) + u") > to_index("_S + String(to_index) + u")"_S);
    if (from_index < 0)
        throw IndexError(from_index);
    if (to_index > array_len)
        throw IndexError(to_index);
}

int main()
{
    auto l = create_array(range_el(0, 1000));
    randomns::shuffle(l);
    print(l);
    l = timsort(l);
    print(l);
}
