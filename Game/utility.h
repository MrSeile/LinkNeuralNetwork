#pragma once

#include <iostream>
#include <ctime>
#include <chrono>

#define RandINIT() srand((unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());

typedef unsigned int uint;
typedef unsigned char byte;

class Timer
{
private:
	std::chrono::steady_clock::time_point start;

public:
	Timer()
	{
		start = std::chrono::high_resolution_clock::now();
	}

	using nanoseconds = std::chrono::duration<double, std::nano>;
	using microseconds = std::chrono::duration<double, std::micro>;
	using milliseconds = std::chrono::duration<double, std::milli>;
	using seconds = std::chrono::duration<double>;
	using minutes = std::chrono::duration<double, std::ratio<60>>;
	using hours = std::chrono::duration<double, std::ratio<3600>>;

	void Restart()
	{
		start = std::chrono::high_resolution_clock::now();
	}

	template<class T>
	double GetElapsedTime()
	{
		return std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now() - start).count();
	}
};

inline double map(const double& value, const double& inputMin, const double& inputMax, const double& outputMin, const double& outputMax)
{
	return outputMin + ((outputMax - outputMin) / (inputMax - inputMin)) * (value - inputMin);
}

inline double randRange(const double& min, const double& max)
{
	return min + (rand() / (RAND_MAX / ((max + 0) - min)));
}