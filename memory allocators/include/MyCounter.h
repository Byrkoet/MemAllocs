#pragma once

#include <Windows.h>
#include <cstdint>
#include <iostream>

class MyCounter
{
	double m_pc_freq;
	int64_t m_ticks_start;
public:
	MyCounter() : m_pc_freq(0), m_ticks_start(0) {}
	~MyCounter() {}

	// Set starting point.
	void Start()
	{
		LARGE_INTEGER ticks_per_second;

		if (!QueryPerformanceFrequency(&ticks_per_second))
		{
			std::cerr << "QueryPerformanceFrequency failed\n";
		}

		// Note: per millisecond. Leave out division for seconds.
		m_pc_freq = double(ticks_per_second.QuadPart / 1000.0);

		// Recycle ticks_per_second to hold starting ticks.
		QueryPerformanceCounter(&ticks_per_second);
		m_ticks_start = ticks_per_second.QuadPart;
	}

	// Retrieve (start ticks - ticks now) / ticks_per_sec.
	double Elapsed() const
	{
		LARGE_INTEGER ticks_end;

		QueryPerformanceCounter(&ticks_end);

		return double(ticks_end.QuadPart - m_ticks_start) / m_pc_freq;
	}
};
