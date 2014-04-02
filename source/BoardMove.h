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
	BoardMove() : promotion('Q'), capturedType(0), specialMove(SpecialMove::None)
	{
	}

	BoardMove(const ivec2& f, const ivec2& t, int capType = 0, int p = 'Q', SpecialMove m = SpecialMove::None) :
	from(f), to(t), promotion(p), capturedType(capType), specialMove(m)
	{
	}

	// Starting position of the piece movement
	ivec2 from;

	// Position that the piece is moving to
	ivec2 to;

	// Promotion type if the moving piece is a pawn
	int promotion;

	// Type of the piece that is being captured
	int capturedType;

	// Specifies what kind of move is being made
	SpecialMove specialMove;
};

std::ostream& operator<<(std::ostream& stream, const BoardMove& move);

#endif // _BOARDMOVE_
