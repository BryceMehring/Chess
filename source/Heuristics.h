#ifndef _HEURISTICS_
#define _HEURISTICS_

#include "Board.h"
#include <vector>

// todo: rename this class
// Defines a state heuristic for a single chess piece
class ChessHeuristic
{
public:

	float operator ()(const Board&, const std::vector<BoardMove>&, const BoardPiece&) const;
};

#endif // _HEURISTICS_
