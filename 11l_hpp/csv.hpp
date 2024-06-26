﻿namespace csv
{
class Reader
{
	File file; // `const File &` cannot be used here, because the lifetimes of all temporaries within range-expression `csv::read(File(...))` are extended only since C++23
	Char delimiter;
	bool check_bom = true;
	Array<char> row_data;
	Array<Int> delim_positions;

	class Row
	{
		Reader *reader;

		class Column
		{
			Reader *reader;
			Int index;

		public:
			Column(Reader *reader, Int index) : reader(reader), index(index) {}
			// explicit operator Int  () const {return to_int_t<Int  >(reader->get_column_cstr(index));} // without `explicit` `print(row[0]);` does not work
			// explicit operator Int64() const {return to_int_t<Int64>(reader->get_column_cstr(index));}
			// explicit operator double() const {return atof(reader->get_column_cstr(index));}
			friend inline Int    to_int  (Column col) {return to_int_t<Int  >(col.reader->get_column_cstr(col.index));}
			friend inline Int64  to_int64(Column col) {return to_int_t<Int64>(col.reader->get_column_cstr(col.index));}
			friend inline double to_float(Column col) {return atof(col.reader->get_column_cstr(col.index));}
			operator String() const {return reader->get_column_string(index);}
		};

	public:
		Row(Reader *reader) : reader(reader) {}
		Column operator[](Int index) {return Column(reader, index);}
	};

	class Iterator
	{
		Reader *reader;

	public:
		Iterator(Reader *reader) : reader(reader) {}
		bool operator!=(Iterator i) const {
			assert(i.reader == nullptr);
			return !reader->row_data.empty();
		}
		void operator++() {reader->read_row();}
		Row operator*() const {return Row(reader);}
	};

	class Error {};

	void read_row()
	{
		row_data.clear();
		delim_positions.clear();

		while (!file.at_eof()) {
			char c = (char)file.read_byte();
			if (c == '"') {
				while (true) {
					if (file.at_eof())
						throw Error();
					c = (char)file.read_byte();
					if (c == '"') {
						c = (char)file.read_byte();
						if (c != '"')
							break;
					}
					if (c == '\n' && !row_data.empty() && row_data.last() == '\r')
						row_data.pop_back();
					row_data.append(c);
				}
			}

			if (c == '\r') {
				row_data.append(c);
				if (file.at_eof())
					break;
				c = (char)file.read_byte();
				if (c == '\n') {
					row_data.pop_back();
					break;
				}
			}
			else if (c == '\n')
				break;

			if (c == delimiter.code)
				delim_positions.append(row_data.len());
			//else // this line can be uncommented [:‘when to_int_t() will accept string length argument’] and get_column_cstr() won't be needed [in favour to get_column_span()]
			row_data.append(c);
		}

		if (check_bom) {
			check_bom = false;
			unsigned char utf8bom[3] = {0xEF, 0xBB, 0xBF};
			if (row_data.len() >= 3 && memcmp(row_data.data(), utf8bom, 3) == 0) {
				row_data.del(range_el(0, 3));
				for (auto &dp : delim_positions)
					dp -= 3;
			}
		}
	}

public: // for `col.reader->get_column_cstr()`
	const char *get_column_cstr(Int column_index)
	{
		if (column_index == delim_positions.len()) { // this is the last column
			if (row_data.last() != 0) // this is a hack (because there is no distinction if \0 character was added here already or it was present in csv file), which can be avoided only when to_int_t() will accept string length argument
				row_data.append(0);
			return &row_data[delim_positions.last()];
		}
		row_data[delim_positions[column_index]] = 0;
		return &row_data[column_index == 0 ? 0 : delim_positions[column_index - 1] + 1];
	}
	String get_column_string(Int column_index)
	{
		Int start = column_index == 0 ? 0 : delim_positions[column_index - 1] + 1,
		    end   = column_index == delim_positions.len() ? row_data.len() : delim_positions[column_index];
		return convert_utf8_string_to_String(&row_data[start], end - start);
	}

	void init(Array<String> *header, bool skip_first_row)
	{
		assert(unsigned(delimiter.code) < 128);
		read_row();
		if (header != nullptr) {
			*header = read_current_row_as_str_array();
			assert(skip_first_row);
		}
		else if (skip_first_row)
			read_row();
	}

public:
	Reader(const String &file_name, const String &encoding = u"utf-8"_S, const String &delimiter = u","_S, Array<String> *header = nullptr, bool skip_first_row = true) : file(file_name, encoding), delimiter(delimiter)
	{
		init(header, skip_first_row);
	}
	Reader(File &&file, const String &delimiter = u","_S, Array<String> *header = nullptr, bool skip_first_row = true) : file(std::move(file)), delimiter(delimiter)
	{
		init(header, skip_first_row);
	}

	Iterator begin()
	{
		return Iterator(this);
	}

	Iterator end()
	{
		return Iterator(nullptr);
	}

	Array<String> /*get_header*/read_current_row_as_str_array()
	{
		Array<String> r;
		r.reserve(delim_positions.len() + 1);
		for (Int i = 0; i <= delim_positions.len(); i++)
			r.append(get_column_string(i));
		read_row();
		return r;
	}
};

Reader read(const String &file_name, const String &encoding = u"utf-8"_S, const String &delimiter = u","_S, Array<String> *header = nullptr, bool skip_first_row = true)
{
	return Reader(file_name, encoding, delimiter, header, skip_first_row);
}
Reader read(File &&file, const String &encoding = u"utf-8"_S, const String &delimiter = u","_S, Array<String> *header = nullptr, bool skip_first_row = true)
{
	return Reader(std::forward<File>(file), delimiter, header, skip_first_row);
}

class Writer
{
	FileWr file;
	Char delimiter;

public:
	Writer(FileWr &&file, const String &encoding = u"utf-8"_S, const String &delimiter = u","_S) : file(std::move(file)), delimiter(delimiter)
	{
		assert(unsigned(this->delimiter.code) < 128);
	}
	Writer(const String &file_name, const String &encoding = u"utf-8"_S,
		                            const String &delimiter = u","_S) : Writer(FileWr(file_name, encoding), delimiter) {}

	template <class ElemTy> void write_row(const Array<ElemTy> &arr)
	{
		for (Int i=0; i<arr.len(); i++) {
			if (i > 0)
				file.write(delimiter);
			String field(arr[i]);
			if (field.starts_with(u" ") || field.ends_with(u" ") || in(u'"', field) || in(u'\n', field) || in(delimiter, field))
				field = u'"'_C & field.replace(u"\"", u"\"\"") & u'"'_C;
			file.write(field);
		}
		file.write(u'\n'_C);
	}
};
}
