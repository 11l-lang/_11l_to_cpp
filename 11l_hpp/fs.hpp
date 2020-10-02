#include <filesystem>
#ifndef _WIN32
#include <unistd.h>
#endif

namespace fs
{
inline String get_temp_dir()
{
	return os::getenv(u"TMPDIR", os::getenv(u"TEMP", os::getenv(u"TMP", u".")));
}

inline Array<String> list_dir(const String &path = u".")
{
	Array<String> r;
	for (auto &&p: std::filesystem::directory_iterator((std::u16string&)path))
		r.append(p.path().filename().u16string());
	return r;
}

#if defined(_MSC_VER) && _MSC_VER < 1920 // `disable_recursion_pending()` is broken[‘after its call `recursion_pending()` is always false’] in MSVC 2017 under std::filesystem, but works correctly under std::experimental::filesystem
namespace std_filesystem = std::experimental::filesystem;
#else
namespace std_filesystem = std::filesystem;
#endif

class Walker
{
	class Iterator
	{
		std_filesystem::recursive_directory_iterator it;
		const Walker *walker;
		friend class Walker;
		void next()
		{
			if (walker->dir_filter == nullptr) {
				if (walker->files_only)
					while (it != std_filesystem::recursive_directory_iterator() && std_filesystem::is_directory(it->status()))
						++it;
			}
			else
				if (walker->files_only)
					for (; it != std_filesystem::recursive_directory_iterator(); ++it) {
						if (std_filesystem::is_directory(it->status())) {
							if (!(*walker->dir_filter)(it->path().filename().u16string()))
								it.disable_recursion_pending();
							continue;
						}
						break;
					}
				else
					for (; it != std_filesystem::recursive_directory_iterator(); ++it) {
						if (std_filesystem::is_directory(it->status()))
							if (!(*walker->dir_filter)(it->path().filename().u16string())) {
								it.disable_recursion_pending();
								continue;
							}
						break;
					}
		}

	public:
		explicit Iterator(const Walker *walker, const std_filesystem::recursive_directory_iterator &it) : walker(walker), it(it) {}
		bool operator!=(Iterator i) const {return it != i.it;}
		void operator++() {it++; next();}
		String operator*() const {return it->path().u16string();}
	};
	std_filesystem::recursive_directory_iterator rdi;
	Nullable<std::function<bool(const String&)>> dir_filter;
	bool files_only;

public:
	Walker(const String &path, Nullable<std::function<bool(const String&)>> dir_filter, bool files_only) : rdi((const std::u16string&)path), dir_filter(dir_filter), files_only(files_only) {}

	Iterator begin() const
	{
		Iterator it(this, rdi);
		it.next();
		return it;
	}

	Iterator end() const
	{
		return Iterator(this, std_filesystem::recursive_directory_iterator());
	}
};

inline Walker walk_dir(const String &path = u".", Nullable<std::function<bool(const String&)>> dir_filter = nullptr, bool files_only = true)
{
	return Walker(path, dir_filter, files_only);
}

inline Walker walk_dir(const String &path, const std::function<bool(const String&)> &dir_filter, bool files_only = true)
{
	return Walker(path, dir_filter, files_only);
}

inline Walker walk_dir(const String &path, nullptr_t, bool files_only)
{
	return Walker(path, nullptr, files_only);
}

inline bool is_dir(const String &path)
{
	return std::filesystem::is_directory((std::u16string&)path);
}

inline bool is_file(const String &path)
{
	return std::filesystem::is_regular_file((std::u16string&)path);
}

inline bool is_symlink(const String &path)
{
	return std::filesystem::is_symlink((std::u16string&)path);
}

inline uintmax_t file_size(const String &path)
{
	return std::filesystem::file_size((std::u16string&)path);
}

inline bool create_dir(const String &path)
{
	return std::filesystem::create_directory((std::u16string&)path);
}

inline bool create_dirs(const String &path)
{
	return std::filesystem::create_directories((std::u16string&)path);
}

inline bool remove_file(const String &path)
{
#ifdef _WIN32
	return DeleteFileW((wchar_t*)path.c_str()) != 0;
#else
	return unlink(convert_utf16_to_utf8(path).c_str()) == 0;
#endif
}

inline bool remove_dir(const String &path)
{
#ifdef _WIN32
	return RemoveDirectoryW((wchar_t*)path.c_str()) != 0;
#else
	return rmdir(convert_utf16_to_utf8(path).c_str()) == 0;
#endif
}

inline uintmax_t remove_all(const String &path)
{
	return std::filesystem::remove_all((std::u16string&)path);
}

inline void rename(const String &old_path, const String &new_path)
{
	std::filesystem::rename((std::u16string&)old_path, (std::u16string&)new_path);
}

namespace path
{
static const char16_t sep =
#ifdef _WIN32
u'\\'
#else
u'/'
#endif
;
inline String join(const String &path1, const String &path2)
{
	String r(path1);
	if (!(r.ends_with(u"\\") || r.ends_with(u"/")))
		r += sep;
	return r + path2;
}

inline String dir_name(const String &path)
{
	size_t sep_pos = path.find_last_of(u"/\\");
	if (sep_pos == String::npos)
		return String();
	return String(path.c_str(), sep_pos);
}

inline String base_name(const String &path)
{
	size_t sep_pos = path.find_last_of(u"/\\");
	if (sep_pos == String::npos)
		return path;
	return String(path.c_str() + sep_pos + 1);
}

inline String absolute(const String &path)
{
	return std::filesystem::absolute((std::u16string&)path).u16string();
}

inline String relative(const String &path, const String &base)
{
	return std::filesystem::relative((std::u16string&)path, (std::u16string&)base).u16string();
}

inline Tuple<String, String> split_ext(const String &path)
{
	size_t sep_pos = path.find_last_of(u"/\\");
	if (sep_pos == String::npos)
		sep_pos = 0;
	size_t dot_pos = path.find_last_of(u".");

	if (dot_pos == String::npos || dot_pos < sep_pos)
		return Tuple<String, String>(path, String());

	return Tuple<String, String>(path[range_el(0, (int)dot_pos)], path[range_ei((int)dot_pos)]);
}
}
}
