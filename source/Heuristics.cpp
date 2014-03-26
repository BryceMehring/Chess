#include "Heuristics.h"
#include <cassert>

float ChessHeuristic::operator ()(const Board& board, const std::vector<BoardMove>& moves, const BoardPiece& piece) const
{
	float worth = 0.0f;
	switch(piece.type)
	{
		case 'P':
		{
			float scalar = 1.0f;
			if(piece.rank >= 3 && piece.rank <= 6)
			{
				scalar = 2.0f;
			}

			if(!board.IsTileEmpty(piece.file, piece.rank + (piece.owner == 1 ? 1 : -1)))
			{
				worth += scalar*0.5f;
			}
			else
			{
				worth += scalar*1.2f;
			}

			break;
		}
		case 'N':
			worth += 3.0f;
			break;
		case 'B':
			worth += 3.0f;
			for(auto iter : moves)
			{
				if(iter.pFrom->piece.id() == piece.piece.id())
				{
					if(iter.pTo != nullptr)
					{
						worth += 2.6f;
					}
					else
					{
						worth += 0.1f;
					}
				}
			}
			break;
		case 'R':
			worth += 5.0f;

			for(auto iter : moves)
			{
				if(iter.pFrom->piece.id() == piece.piece.id())
				{
					if(iter.pTo != nullptr)
					{
						worth += 2.5f;
					}
					else
					{
						worth += 0.1f;
					}
				}
			}

			break;
		case 'Q':
			worth += 9.0f;
			for(auto iter : moves)
			{
				if(iter.pFrom->piece.id() == piece.piece.id())
				{
					if(iter.pTo != nullptr)
					{
						worth += 2.5f;
					}
					else
					{
						worth += 0.1f;
					}
				}
			}
			break;
		case 'K':
			//worth += 900.0f;
			break;
		default:
			assert("Invalid piece type" && false);
			break;
	}

	return worth;
}
