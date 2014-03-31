#include "Heuristics.h"
#include <cassert>

const int ChessHeuristic::m_pawnMoveTable[8][8] =
{
	{0,  0,  0,  0,  0,  0,  0,  0},
	{50, 50, 50, 50, 50, 50, 50, 50},
	{10, 10, 20, 30, 30, 20, 10, 10},
	{5,  5, 10, 25, 25, 10,  5,  5},
	{0,  0,  0, 20, 20,  0,  0,  0},
	{5, -5,-10,  0,  0,-10, -5,  5},
	{5, 10, 10,-20,-20, 10, 10,  5},
	{0,  0,  0,  0,  0,  0,  0,  0}
};

const int ChessHeuristic::m_knightMoveTable[8][8] =
{
	{-50,-40,-30,-30,-30,-30,-40,-50},
	{-40,-20,  0,  0,  0,  0,-20,-40},
	{-30,  0, 10, 15, 15, 10,  0,-30},
	{-30,  5, 15, 20, 20, 15,  5,-30},
	{-30,  0, 15, 20, 20, 15,  0,-30},
	{-30,  5, 10, 15, 15, 10,  5,-30},
	{-40,-20,  0,  5,  5,  0,-20,-40},
	{-50,-40,-30,-30,-30,-30,-40,-50}
};

const int ChessHeuristic::m_bishopMoveTable[8][8] =
{
	{-20,-10,-10,-10,-10,-10,-10,-20},
	{-10,  0,  0,  0,  0,  0,  0,-10},
	{-10,  0,  5, 10, 10,  5,  0,-10},
	{-10,  5,  5, 10, 10,  5,  5,-10},
	{-10,  0, 10, 10, 10, 10,  0,-10},
	{-10, 10, 10, 10, 10, 10, 10,-10},
	{-10,  5,  0,  0,  0,  0,  5,-10},
	{-20,-10,-10,-10,-10,-10,-10,-20}
};

const int ChessHeuristic::m_rookMoveTable[8][8] =
{
	{0,  0,  0,  0,  0,  0,  0,  0},
	{ 5, 10, 10, 10, 10, 10, 10,  5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{0,  0,  0,  5,  5,  0,  0,  0}
};

const int ChessHeuristic::m_queenMoveTable[8][8] =
{
	{-20,-10,-10, -5, -5,-10,-10,-20},
	{-10,  0,  0,  0,  0,  0,  0,-10},
	{-10,  0,  5,  5,  5,  5,  0,-10},
	{-5,  0,  5,  5,  5,  5,  0, -5},
	{ 0,  0,  5,  5,  5,  5,  0, -5},
	{-10,  5,  5,  5,  5,  5,  0,-10},
	{-10,  0,  5,  0,  0,  0,  0,-10},
	{-20,-10,-10, -5, -5,-10,-10,-20}
};

const int ChessHeuristic::m_kingMiddleGameTable[8][8] =
{
	{-30,-40,-40,-50,-50,-40,-40,-30},
	{-30,-40,-40,-50,-50,-40,-40,-30},
	{-30,-40,-40,-50,-50,-40,-40,-30},
	{-30,-40,-40,-50,-50,-40,-40,-30},
	{-20,-30,-30,-40,-40,-30,-30,-20},
	{-10,-20,-20,-20,-20,-20,-20,-10},
	{ 20, 20,  0,  0,  0,  0, 20, 20},
	{ 20, 30, 10,  0,  0, 10, 30, 20}
};

const int ChessHeuristic::m_kingEndGameMoveTable[8][8] =
{
	{-50,-40,-30,-20,-20,-30,-40,-50},
	{-30,-20,-10,  0,  0,-10,-20,-30},
	{-30,-10, 20, 30, 30, 20,-10,-30},
	{-30,-10, 30, 40, 40, 30,-10,-30},
	{-30,-10, 30, 40, 40, 30,-10,-30},
	{-30,-10, 20, 30, 30, 20,-10,-30},
	{-30,-30,  0,  0,  0,  0,-30,-30},
	{-50,-30,-30,-30,-30,-30,-30,-50}
};

int ChessHeuristic::operator ()(const Board& board, const std::vector<BoardMove>& moves, const BoardPiece& piece) const
{
	int worth = 0;
	int rank = ((piece.owner == 1) ? piece.rank - 1 : 8 - piece.rank);
	unsigned int piecesCount = board.GetNumPieces();
	switch(piece.type)
	{
		case 'P':
			worth += 100;
			worth += m_pawnMoveTable[rank][piece.file - 1];
			break;
		case 'N':
			worth += 320;
			worth += m_knightMoveTable[rank][piece.file - 1];
			break;
		case 'B':
			worth += 330;
			worth += m_bishopMoveTable[rank][piece.file - 1];
			break;
		case 'R':
			worth += 550;
			worth += m_rookMoveTable[rank][piece.file - 1];
			break;
		case 'Q':
			worth += 900;
			worth += m_queenMoveTable[rank][piece.file - 1];
			break;
		case 'K':
			if(piecesCount > 16)
			{
				worth += m_kingMiddleGameTable[rank][piece.file - 1];
			}
			else
			{
				worth += m_kingEndGameMoveTable[rank][piece.file - 1];
			}
			break;
		default:
			assert("Invalid piece type" && false);
			break;
	}

	for(const BoardMove& move : moves)
	{
		const BoardPiece* pPiece = board.GetPiece(move.from);
		assert(pPiece != nullptr);

		if(pPiece->piece.id() == piece.piece.id())
		{
			if(move.capturedType != 0)
			{
				worth += 50.0f;
			}
		}
	}

	return worth;
}
