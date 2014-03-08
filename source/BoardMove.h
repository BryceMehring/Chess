#ifndef _BOARDMOVE_
#define _BOARDMOVE_

#include "vec2.h"
#include <ostream>

enum class SpecialMove
{
	EnPassant,
	Castle,
	Promotion,
	Capture,
	None
};

struct BoardMove
{
	BoardMove() : promotion('Q'), specialMove(SpecialMove::None)
	{
	}

	BoardMove(const ivec2& f, const ivec2& t) : from(f), to(t), promotion('Q'), specialMove(SpecialMove::None)
	{
	}

	BoardMove(const ivec2& f, const ivec2& t, int p) : from(f), to(t), promotion(p), specialMove(SpecialMove::None)
	{
	}

	BoardMove(const ivec2& f, const ivec2& t, int p, SpecialMove m) : from(f), to(t), promotion(p), specialMove(m)
	{
	}

	ivec2 from;
	ivec2 to;
	int promotion;
	SpecialMove specialMove;
};

std::ostream& operator<<(std::ostream& stream, const BoardMove& move);

#endif // _BOARDMOVE_
