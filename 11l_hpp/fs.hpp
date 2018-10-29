#include <filesystem>

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

#if _MSC_VER // `disable_recursion_pending()` is broken[‘after its call `recursion_pending()` is always false’] in MSVC 2017 under std::filesystem, but works correctly under std::experimental::filesystem
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
					if (std_filesystem::is_directory(it->status()))
						do
							++it;
						while (it != std_filesystem::recursive_directory_iterator() && std_filesystem::is_directory(it->status()));
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

inline Walker walk(const String &path = u".", Nullable<std::function<bool(const String&)>> dir_filter = nullptr, bool files_only = true)
{
	return Walker(path, dir_filter, files_only);
}

inline Walker walk(const String &path, const std::function<bool(const String&)> &dir_filter, bool files_only = true)
{
	return Walker(path, dir_filter, files_only);
}

inline bool is_directory(const String &path)
{
	return std::filesystem::is_directory((std::u16string&)path);
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
}
}
