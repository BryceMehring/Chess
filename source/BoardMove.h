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

class Piece;

struct BoardMove
{
	BoardMove() : promotion('Q'), specialMove(SpecialMove::None)
	{
	}

	BoardMove(const ivec2& f, const ivec2& t, Piece* pF = nullptr, Piece* pT = nullptr, int p = 'Q', SpecialMove m = SpecialMove::None) : 
	from(f), to(t), pFrom(pF), pTo(pT), promotion(p), specialMove(m)
	{
	}

	ivec2 from;
	ivec2 to;
	Piece* pFrom;
	Piece* pTo;
	int promotion;
	SpecialMove specialMove;
};

std::ostream& operator<<(std::ostream& stream, const BoardMove& move);

#endif // _BOARDMOVE_
