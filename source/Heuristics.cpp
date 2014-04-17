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

const int ChessHeuristic::m_pawnEndGameMoveTable[8][8] =
{
	{0,  0,  0,  0,  0,  0,  0,  0},
	{50, 50, 50, 50, 50, 50, 50, 50},
	{40, 40, 40, 40, 40, 40, 40, 40},
	{20, 20, 20, 20, 20, 20, 20, 20},
	{-20,-20,-20,-20,-20,-20,-20,-20},
	{-40,-40,-40,-40,-40,-40,-40,-40},
	{-50,-50,-50,-50,-50,-50,-50,-50},
	{  0,  0,  0,  0,  0,  0,  0,  0}
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
	int value = GetMaterialValue(board, {piece.file, piece.rank}, piece.type, piece.owner);

	for(const BoardMove& move : moves)
	{
		if(move.capturedType != 0)
		{
			value += GetMaterialValue(board, move.to, move.capturedType, !piece.owner) / 32;
		}
	}

	return value;
}

int ChessHeuristic::GetMaterialValue(const Board& board, const ivec2& pos, int type, int owner) const
{
	int value = 0;
	int rank = ((owner == 1) ? pos.y - 1 : 8 - pos.y);

	switch(type)
	{
		case 'P':
			value += 100;
			if(board.GetNumPieces() > 16)
			{
				value += m_pawnMoveTable[rank][pos.x - 1];
			}
			else
			{
				value += m_pawnEndGameMoveTable[rank][pos.x - 1];
			}
			break;
		case 'N':
			value += 320;
			value += m_knightMoveTable[rank][pos.x - 1];
			break;
		case 'B':
			value += 330;
			value += m_bishopMoveTable[rank][pos.x - 1];
			break;
		case 'R':
			value += 550;
			value += m_rookMoveTable[rank][pos.x - 1];
			break;
		case 'Q':
			value += 900;
			value += m_queenMoveTable[rank][pos.x - 1];
			break;
		case 'K':
			if(board.GetNumPieces() > 16)
			{
				value += m_kingMiddleGameTable[rank][pos.x - 1];
			}
			else
			{
				value += m_kingEndGameMoveTable[rank][pos.x - 1];
			}
			break;
		default:
			assert("Invalid piece type" && false);
			break;
	}
	return value;
}
