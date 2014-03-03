#include "Board.h"
#include <cassert>
#include <algorithm>

Board::Board() : m_iPlayerID(0)
{
	m_board.resize(8);

	for(auto& iter : m_board)
	{
		iter.resize(8,nullptr);
	}
}

void Board::Update(int playerID, std::vector<Piece>& pieces)
{
	Clear();
	for(Piece& p : pieces)
	{
		assert(p.file() <= 8 && p.file() >= 1);
		assert(p.rank() <= 8 && p.rank() >= 1);

		m_board[p.file() - 1][p.rank() - 1] = &p;
	}

	m_iPlayerID = playerID;
}

std::vector<BoardMove> Board::GetMoves() const
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
					GeneratePawnMoves(piece, moves);
					break;
				case 'N':
				case 'K':
					GenerateDiscreteMoves(piece, moves);
					break;
				case 'B':
				case 'R':
				case 'Q':
					GenerateDirectionMoves(piece, moves);
					break;
				default:
					assert("Invalid piece" && false);
				}
			}
		}
	}

	return moves;
}

void Board::GeneratePawnMoves(Piece* pPiece, std::vector<BoardMove>& moves) const
{
	assert(pPiece->type() == int('P'));

	int iNewRank = pPiece->rank() + ((m_iPlayerID == 0) ? 1 : -1);
	if(IsOnGrid(iNewRank))
	{
		// First check if we can move to the tile in front of us
		if(IsTileEmpty(pPiece->file(),iNewRank))
		{
			moves.push_back({pPiece, {pPiece->file(), iNewRank}});

			// Check if we can move 2 tiles if this is the first move
			if(!pPiece->hasMoved())
			{
				int iDoubleMoveRank = iNewRank + ((m_iPlayerID == 0) ? 1 : -1);

				// First check if we can move to the tile in front of us
				if(IsTileEmpty(pPiece->file(),iDoubleMoveRank))
				{
					moves.push_back({pPiece, {pPiece->file(), iDoubleMoveRank}});
				}
			}
		}

		// Check if we can capture a piece by moving to a forward diaganol tile
		for(int iNewFile : {pPiece->file() + 1, pPiece->file() - 1})
		{
			if(IsOnGrid(iNewFile))
			{
				if(!IsTileEmpty(iNewFile,iNewRank) && !IsTileOwner(iNewFile, iNewRank))
				{
					moves.push_back({pPiece, {iNewFile, iNewRank}});
				}
			}
		}
	}
}

void Board::GenerateDirectionMoves(Piece* pPiece, std::vector<BoardMove>& moves) const
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
		int x = pPiece->file();
		int y = pPiece->rank();

		while(IsOnGrid(x) && IsOnGrid(y))
		{
			x += dir[i].x;
			y += dir[i].y;

			if(IsOnGrid(x) && IsOnGrid(y))
			{
				if(!IsTileOwner(x,y))
				{
					moves.push_back({pPiece,{x,y}});
				}

				if(!IsTileEmpty(x,y))
				{
					break;
				}
			}
		}
	}
}

void Board::GenerateDiscreteMoves(Piece* pPiece, std::vector<BoardMove>& moves) const
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
		if(IsOnGrid(move.x) && IsOnGrid(move.y) && !IsTileOwner(move.x,move.y))
		{
			moves.push_back({pPiece, move});
		}
	}
}

bool Board::IsOnGrid(int coord) const
{
	return (coord <= 8 && coord >= 1);
}

bool Board::IsTileEmpty(int file, int rank) const
{
	assert(IsOnGrid(file) && IsOnGrid(rank));

	return m_board[file - 1][rank - 1] == nullptr;
}

bool Board::IsTileOwner(int file, int rank) const
{
	assert(IsOnGrid(file) && IsOnGrid(rank));

	if(IsTileEmpty(file,rank))
		return false;

	return (m_iPlayerID == m_board[file - 1][rank - 1]->owner());
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
