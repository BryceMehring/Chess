#ifndef _HEURISTICS_
#define _HEURISTICS_

#include "Board.h"
#include <vector>

// todo: rename this class
// Defines a state heuristic for a single chess piece
class ChessHeuristic
{
public:

	int operator ()(const Board&, const std::vector<BoardMove>&, const BoardPiece&) const;

private:

	static const int m_pawnMoveTable[8][8];
	static const int m_knightMoveTable[8][8];
	static const int m_bishopMoveTable[8][8];
	static const int m_rookMoveTable[8][8];
	static const int m_queenMoveTable[8][8];
	static const int m_kingMoveTable[8][8];

};

#endif // _HEURISTICS_
