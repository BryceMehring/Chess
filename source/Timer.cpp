
#include "Timer.h"

using namespace std;

Timer::Timer() : m_bActive(false)
{
}

void Timer::Start()
{
	m_bActive = true;
	PollCounter(m_start);
}

void Timer::Stop()
{
	m_bActive = false;
	PollCounter(m_end);
}

void Timer::Reset()
{
	if (m_bActive)
	{
		Stop();
	}

	m_start = m_end;
	m_bActive = false;
}

std::uint64_t Timer::GetTime()
{
	if (m_bActive)
	{
		PollCounter(m_end);
	}

	std::chrono::nanoseconds time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(m_end - m_start);
	return time_span.count();
}

bool Timer::IsActive() const
{
	return m_bActive;
}

void Timer::PollCounter(chrono::high_resolution_clock::time_point& Out)
{
	Out = chrono::high_resolution_clock::now();
}

