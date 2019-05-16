#include <chrono>
//#include <ctime>
#ifdef _WIN32
#include <sstream>
#include <iomanip>
#endif

namespace timens // 'time': a symbol with this name already exists and therefore this name cannot be used as a namespace name
{
class HRCTimePoint
{
	std::chrono::high_resolution_clock::time_point tp;
public:
	HRCTimePoint(const std::chrono::high_resolution_clock::time_point &tp) : tp(tp) {}

	double operator-(const HRCTimePoint &tp2) const
	{
		return std::chrono::duration_cast<std::chrono::duration<double>>(tp - tp2.tp).count();
	}
};

HRCTimePoint perf_counter()
{
	return std::chrono::high_resolution_clock::now();
}
}

class TimeDelta
{
	struct Uninitialized {};
	explicit TimeDelta(Uninitialized) {}
	friend class Time;

public:
	double seconds;

	explicit TimeDelta(double days = 0, double hours = 0, double minutes = 0, double seconds = 0, double milliseconds = 0, double microseconds = 0, double weeks = 0) :
		seconds(weeks * (7.0 * 24 * 3600) + days * (24.0 * 3600) + hours * 3600.0 + minutes * 60.0 + seconds + milliseconds * 0.001 + microseconds * 1e-6)
	{}

	operator String() const
	{
		double s = seconds;
		double days = floor(s / (24.0 * 3600));
		String r;
		if (days != 0)
			r = String(days) + (days == 1 ? u" day, " : u" days, ");

		s -= days * (24.0 * 3600);
		double hours = floor(s / 3600.0);
		r += String(hours);
		r += u':';

		s -= hours * 3600.0;
		double minutes = floor(s / 60.0);
		r += String(minutes).zfill(2);
		r += u':';

		s -= minutes * 60.0;
		if (s < 10)
			r += u'0';
		r += String(s);

		return r;
	}

	bool operator==(TimeDelta d) const {return seconds == d.seconds;}
	bool operator!=(TimeDelta d) const {return seconds != d.seconds;}

	double days() const {return seconds / (24.0 * 3600);}
};

class Time
{
	double seconds_since_epoch;
	struct Uninitialized {};
	explicit Time(Uninitialized) {}

public:
	Time() : seconds_since_epoch(std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::system_clock::now().time_since_epoch()).count()) {}

	explicit Time(int year, int month = 1, int day = 1, int hour = 0, int minute = 0, double second = 0)
	{
		tm tm;
		tm.tm_year = year - 1900;
		tm.tm_mon = month - 1;
		tm.tm_mday = day;
		tm.tm_hour = hour;
		tm.tm_min = minute;
		tm.tm_sec = (int)second;
		tm.tm_isdst = -1;
		seconds_since_epoch = mktime(&tm) + fract(second);
	}

	static Time from_unix_time(double s)
	{
		Time t((Uninitialized())); // `Time t(Uninitialized());` does not work in MSVC
		t.seconds_since_epoch = s;
		return t;
	}

	double unix_time() const {return seconds_since_epoch;}

	String strftime(const String &f16) const
	{
		time_t t = (time_t)seconds_since_epoch;
#ifdef _WIN32
#define GET_LOCALTIME_TM \
		tm tm_, *tm = &tm_; \
		localtime_s(tm, &t);
#else
#define GET_LOCALTIME_TM \
		tm *tm = localtime(&t);
#endif
		GET_LOCALTIME_TM
		char s[100];
		return String(s, s + ::strftime(s, sizeof(s), f16.to_string().c_str(), tm));
	}

	String format(const String &f) const
	{
		return strftime(f.replace(u"YYYY", u"%Y").replace(u"YY", u"%y").replace(u"MM", u"%m").replace(u"DD", u"%d")
		                 .replace(u"ГГГГ", u"%Y").replace(u"ГГ", u"%y").replace(u"ММ", u"%m").replace(u"ДД", u"%d")
		                 .replace(u"hh", u"%H").replace(u"mm", u"%M").replace(u"ss", u"%S")
		                 .replace(u"чч", u"%H").replace(u"мм", u"%M").replace(u"сс", u"%S"));
	}

	operator String() const
	{
		time_t t = (time_t)seconds_since_epoch;
		GET_LOCALTIME_TM
		String r = String(tm->tm_year + 1900) + u'-' + String(tm->tm_mon + 1).zfill(2) + u'-' + String(tm->tm_mday).zfill(2) + u' '
		         + String(tm->tm_hour).zfill(2) + u':'
		         + String(tm->tm_min ).zfill(2) + u':'
		         + String(tm->tm_sec ).zfill(2);
		double f = fract(seconds_since_epoch);
		if (f)
			r += String(f).c_str() + 1;
		return r;
	}

	Time operator+(TimeDelta d) const {return from_unix_time(seconds_since_epoch + d.seconds);}
	Time operator-(TimeDelta d) const {return from_unix_time(seconds_since_epoch - d.seconds);}
	TimeDelta operator-(Time t) const {TimeDelta r((TimeDelta::Uninitialized())); r.seconds = seconds_since_epoch - t.seconds_since_epoch; return r;}

	bool operator==(Time t) const {return seconds_since_epoch == t.seconds_since_epoch;}
	bool operator!=(Time t) const {return seconds_since_epoch != t.seconds_since_epoch;}
};

namespace timens
{
inline Time from_unix_time(double s)
{
	return Time::from_unix_time(s);
}

inline Time today()
{
	time_t t;
	time(&t);
	GET_LOCALTIME_TM
	struct tm ttm = {0};
	ttm.tm_year = tm->tm_year;
	ttm.tm_mon  = tm->tm_mon;
	ttm.tm_mday = tm->tm_mday;
	ttm.tm_isdst = tm->tm_isdst;
	return from_unix_time((double)mktime(&ttm));
}

inline Time strptime(const String &datetime_string, const String &format)
{
#ifdef _WIN32
// [https://stackoverflow.com/a/33542189/2692494 <- google:‘c++ strptime’]
	std::basic_istringstream<char16_t, std::char_traits<char16_t>, std::allocator<char16_t>> input(datetime_string);
	input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
	tm tm = {0};
	input >> std::get_time(&tm, format.c_str());
	if (input.fail())
		return Time(0);
#else // code above does not work in GCC for some unknown reason, so use `::strptime()` instead
	tm tm = {0};
	::strptime(convert_utf16_to_utf8(datetime_string).c_str(), convert_utf16_to_utf8(format).c_str(), &tm);
#endif
	return from_unix_time((double)mktime(&tm));
}
}
