#include "Board.h"
#include <cassert>
#include <algorithm>
#include <iostream>


using namespace std;

Board::Board() : m_iPlayerID(0)
{
	m_board.resize(8);

	for(auto& iter : m_board)
	{
		iter.resize(8,nullptr);
	}
}

std::vector<BoardMove> Board::Update(int playerID, std::vector<Piece>& pieces)
{
	m_iPlayerID = playerID;

	// Clear the old board
	Clear();

	// Fill new board with pieces
	for(Piece& p : pieces)
	{
		if(p.type() == 'K')
		{
			m_kingPos[p.owner()] = {p.file(), p.rank()};
		}

		m_board[p.file() - 1][p.rank() - 1] = &p;
	}

	return GetMoves();
}

std::vector<BoardMove> Board::GetMoves(bool bCheck)
{
	std::vector<BoardMove> moves;
	for(auto& fileIter : m_board)
	{
		for(Piece* piece : fileIter)
		{
			if(piece != nullptr && m_iPlayerID == piece->owner())
			{
				switch(piece->type())
				{
				case 'P':
					GeneratePawnMoves(piece, bCheck, moves);
					break;
				case 'N':
				case 'K':
					GenerateDiscreteMoves(piece, bCheck, moves);
					break;
				case 'B':
				case 'R':
				case 'Q':
					GenerateDirectionMoves(piece, bCheck, moves);
					break;
				default:
					assert("Invalid piece" && false);
				}
			}
		}
	}

	return moves;
}

void Board::GeneratePawnMoves(Piece* pPiece, bool bCheck, std::vector<BoardMove>& moves)
{
	assert(pPiece->type() == int('P'));

	int iNewRank = pPiece->rank() + ((m_iPlayerID == 0) ? 1 : -1);
	if(IsOnBoard(iNewRank))
	{
		// First check if we can move to the tile in front of us
		if(IsTileEmpty(pPiece->file(),iNewRank))
		{
			AddMove({pPiece, {pPiece->file(), iNewRank}}, bCheck, moves);

			// Check if we can move 2 tiles if this is the first move
			if(!pPiece->hasMoved())
			{
				int iDoubleMoveRank = iNewRank + ((m_iPlayerID == 0) ? 1 : -1);

				// First check if we can move to the tile in front of us
				if(IsTileEmpty(pPiece->file(),iDoubleMoveRank))
				{
					AddMove({pPiece, {pPiece->file(), iDoubleMoveRank}}, bCheck, moves);
				}
			}
		}

		// Check if we can capture a piece by moving to a forward diaganol tile
		for(int iNewFile : {pPiece->file() + 1, pPiece->file() - 1})
		{
			if(IsOnBoard(iNewFile))
			{
				if(!IsTileEmpty(iNewFile,iNewRank) && !IsTileOwner(iNewFile, iNewRank))
				{
					AddMove({pPiece, {iNewFile, iNewRank}}, bCheck, moves);
				}
			}
		}
	}
}

void Board::GenerateDirectionMoves(Piece* pPiece, bool bCheck, std::vector<BoardMove>& moves)
{
	assert(pPiece->type() == int('B') || pPiece->type() == int('R') || pPiece->type() == int('Q'));

	const vec2 dir[] =
	{
		// Bishop
		{1,1},
		{-1,1},
		{1,-1},
		{-1,-1},

		// Rook
		{1,0},
		{-1,0},
		{0,1},
		{0,-1}
	};

	unsigned int start = 0;
	unsigned int end = 8;

	if(pPiece->type() == int('B'))
	{
		end = 4;
	}
	else if(pPiece->type() == int('R'))
	{
		start = 4;
	}

	for(unsigned int i = start; i < end; ++i)
	{
		vec2 pos = {pPiece->file(), pPiece->rank()};

		while(IsOnBoard(pos))
		{
			pos.x += dir[i].x;
			pos.y += dir[i].y;

			if(IsOnBoard(pos))
			{
				if(!IsTileOwner(pos.x,pos.y))
				{
					AddMove({pPiece, pos}, bCheck, moves);
				}

				if(!IsTileEmpty(pos.x,pos.y))
				{
					break;
				}
			}
		}
	}
}

void Board::GenerateDiscreteMoves(Piece* pPiece, bool bCheck, std::vector<BoardMove>& moves)
{
	assert(pPiece->type() == int('N') || pPiece->type() == int('K'));

	int x = pPiece->file();
	int y = pPiece->rank();

	const vec2 currentMoves[][8] =
	{
		// Knight
		{
			{x + 1, y + 2}, {x + 2, y + 1}, {x - 2, y + 1}, {x - 1, y + 2},
			{x + 1, y - 2}, {x + 2, y - 1}, {x - 2, y - 1}, {x - 1, y - 2}
		},
		// King
		{
			{x - 1, y - 1}, {x,y - 1}, {x + 1,y - 1}, {x - 1, y},
			{x + 1, y}, {x - 1, y + 1}, {x, y + 1}, {x + 1,y + 1}
		}
	};

	unsigned int index = pPiece->type() == int('K') ? 1 : 0;

	for(const vec2& move : currentMoves[index])
	{
		if(IsOnBoard(move) && !IsTileOwner(move.x,move.y))
		{
			AddMove({pPiece, move}, bCheck, moves);
		}
	}
}

void Board::AddMove(const BoardMove& move, bool bCheck, std::vector<BoardMove>& moves)
{
	// todo: make this cleaner

	int x = move.pPiece->file() - 1;
	int y = move.pPiece->rank() - 1;

	vec2 oldKingPos = m_kingPos[move.pPiece->owner()];

	Piece* pOldDest = m_board[move.move.x - 1][move.move.y - 1];

	m_board[x][y] = nullptr;
	m_board[move.move.x - 1][move.move.y - 1] = move.pPiece;

	if(move.pPiece->type() == 'K')
	{
		m_kingPos[move.pPiece->owner()] = move.move;
	}

	if(!bCheck || !IsInCheck())
	{
		moves.push_back(move);
	}
	else
	{
		//cout << "Did not move" << endl;
	}

	if(move.pPiece->type() == 'K')
	{
		m_kingPos[move.pPiece->owner()] = oldKingPos;
	}

	m_board[x][y] = move.pPiece;
	m_board[move.move.x - 1][move.move.y - 1] = pOldDest;
}

bool Board::IsOnBoard(int coord) const
{
	return (coord <= 8 && coord >= 1);
}

bool Board::IsOnBoard(const vec2& coord) const
{
	return IsOnBoard(coord.x) && IsOnBoard(coord.y);
}

bool Board::IsTileEmpty(int file, int rank) const
{
	assert(IsOnBoard(file) && IsOnBoard(rank));

	return m_board[file - 1][rank - 1] == nullptr;
}

bool Board::IsTileOwner(int file, int rank) const
{
	assert(IsOnBoard(file) && IsOnBoard(rank));

	if(IsTileEmpty(file,rank))
		return false;

	return (m_iPlayerID == m_board[file - 1][rank - 1]->owner());
}

bool Board::IsInCheck()
{
	// todo: clean this up

	int oldPlayerID = m_iPlayerID;

	m_iPlayerID = !m_iPlayerID;
	std::vector<BoardMove> moves = GetMoves(false);

	bool bCheck = false;
	for(const auto& m : moves)
	{
		if(m.move.x == m_kingPos[oldPlayerID].x && m.move.y == m_kingPos[oldPlayerID].y)
		{
			bCheck = true;
			break;
		}
	}

	m_iPlayerID = oldPlayerID;

	return bCheck;
}

void Board::Clear()
{
	for(auto& fileIter : m_board)
	{
		for(Piece*& rankIter : fileIter)
		{
			rankIter = nullptr;
		}
	}
}
