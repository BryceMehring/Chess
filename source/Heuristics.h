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

	int GetMaterialValue(const Board&, const ivec2& pos, int type, int owner) const;

	static const int m_pawnMoveTable[8][8];
	static const int m_pawnEndGameMoveTable[8][8];
	static const int m_knightMoveTable[8][8];
	static const int m_bishopMoveTable[8][8];
	static const int m_rookMoveTable[8][8];
	static const int m_queenMoveTable[8][8];
	static const int m_kingMiddleGameTable[8][8];
	static const int m_kingEndGameMoveTable[8][8];

};

#endif // _HEURISTICS_
