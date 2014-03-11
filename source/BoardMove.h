#ifndef _BOARDMOVE_
#define _BOARDMOVE_

#include "vec2.h"
#include <ostream>

enum class SpecialMove
{
	EnPassant,
	Castle,
	Promotion,
	None
};

class BoardPiece;

struct BoardMove
{
	BoardMove() : promotion('Q'), specialMove(SpecialMove::None)
	{
	}

	BoardMove(const ivec2& f, const ivec2& t, BoardPiece* pF = nullptr, BoardPiece* pT = nullptr, int p = 'Q', SpecialMove m = SpecialMove::None) :
	from(f), to(t), pFrom(pF), pTo(pT), promotion(p), specialMove(m)
	{
	}

	ivec2 from;
	ivec2 to;
	BoardPiece* pFrom;
	BoardPiece* pTo;
	int promotion;
	SpecialMove specialMove;
	float worth;
};

std::ostream& operator<<(std::ostream& stream, const BoardMove& move);

#endif // _BOARDMOVE_
