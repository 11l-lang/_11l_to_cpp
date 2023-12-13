namespace csv
{
class Reader
{
	File file;
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
			return reader != nullptr;
		}
		void operator++() {reader->read_row(); if (reader->row_data.empty()) reader = nullptr;}
		Row operator*() const {return Row(reader);}
	};

	class Error {};

	void read_row()
	{
		row_data.clear();
		delim_positions.clear();

		for (int c; (c = fgetc(file.file)) != EOF;) {
			if (c == '"') {
				while (true) {
					c = fgetc(file.file);
					if (c == EOF)
						throw Error();
					if (c == '"') {
						c = fgetc(file.file);
						if (c != '"')
							break;
					}
					row_data.append(c);
				}
			}

			if (c == '\n' || c == EOF)
				break;

			if (c == delimiter.code) {
				delim_positions.append(row_data.len());
			}
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

public:
	Reader(const String &file_name, const String &encoding = u"utf-8"_S, const String &delimiter = u","_S) : file(file_name), delimiter(delimiter)
	{
		assert(unsigned(this->delimiter.code) < 128);
	}

	Iterator begin()
	{
		read_row();
		return Iterator(this);
	}

	Iterator end()
	{
		return Iterator(nullptr);
	}
};

Reader read(const String &file_name, const String &encoding = u"utf-8"_S, const String &delimiter = u","_S)
{
	return Reader(file_name, encoding, delimiter);
}
}
