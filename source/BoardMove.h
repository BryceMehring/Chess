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

	// Starting position of the piece movement
	ivec2 from;

	// Tile that the piece is moving to
	ivec2 to;

	// Piece that is moving
	BoardPiece* pFrom;

	// Piece that is being attacked, else it points to nothing
	BoardPiece* pTo;

	// Promotion type if the moving piece is a pawn
	int promotion;

	// Specifies what kind of move is being made
	SpecialMove specialMove;

	// The value of the game state to the owner after piece movement
	float worth;
};

std::ostream& operator<<(std::ostream& stream, const BoardMove& move);

#endif // _BOARDMOVE_
