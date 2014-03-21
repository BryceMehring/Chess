#ifndef _HEURISTICS_
#define _HEURISTICS_

#include "Board.h"
#include <vector>

class ChessHeuristic
{
public:

	float operator ()(const Board&, const std::vector<BoardMove>&, const BoardPiece&) const;
};

#endif // _HEURISTICS_
