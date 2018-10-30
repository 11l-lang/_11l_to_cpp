#include <chrono>

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
