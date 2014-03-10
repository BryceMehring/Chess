#ifndef _TIMER_H_
#define _TIMER_H_

#include <chrono>
#include <cstdint>

// Defines a simple High Resolution Timer
class Timer
{
public:

	// Construct the timer initially stopped.
	Timer();

	// Activate the timer and poll the counter.
	void Start();

	// Deactivate the timer and poll the counter.
	void Stop();

	// Stops the timer if it's active and resets all
	// of the Timer's members to their initial values.
	void Reset();

	// Returns the time elapsed since Start() was called in nanoseconds.
	std::uint64_t GetTime();

	// Returns true if the Timer is currently active.
	bool IsActive() const;

private:

	// Poll the query performance counter
	void PollCounter(std::chrono::high_resolution_clock::time_point& Out);

private:
	std::chrono::high_resolution_clock::time_point m_start;
	std::chrono::high_resolution_clock::time_point m_end;
	bool m_bActive;
}; // class Timer


#endif // _TIMER_H_
