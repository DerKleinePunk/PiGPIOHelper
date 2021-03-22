#pragma once
#include <chrono>
#include <ostream>

class ElapsedTimer {
	std::chrono::high_resolution_clock::time_point _start;
public:
	explicit ElapsedTimer(bool run = false)
	{
		if (run)
			Reset();
	}

	void Reset()
	{
		_start = std::chrono::high_resolution_clock::now();
	}

	std::chrono::microseconds Elapsed() const
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _start);
	}

	template <typename T, typename Traits>
	friend std::basic_ostream<T, Traits>& operator<<(std::basic_ostream<T, Traits>& out, const ElapsedTimer& timer)
	{
		return out << timer.Elapsed().count();
	}
};
