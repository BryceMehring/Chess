#include "Board.h"
#include <cassert>
#include <algorithm>
#include <iostream>


using namespace std;

ApplyMove::ApplyMove(const BoardMove* pMove, Board* pBoard) : m_pMove(pMove), m_pBoard(pBoard)
{
	// todo: clean up this code

	m_pMovingPiece = pBoard->GetPiece(pMove->from);

	m_oldKingPos = pBoard->m_kingPos[m_pMovingPiece->owner()];
	m_pOldDest = pBoard->GetPiece(pMove->to);

	pBoard->m_board[pMove->from.x - 1][pMove->from.y - 1] = nullptr;
	pBoard->m_board[pMove->to.x - 1][pMove->to.y - 1] = m_pMovingPiece;

	m_LastMove = pBoard->m_LastMove;
	pBoard->m_LastMove = *pMove;

	if(m_pMovingPiece->type() == 'K')
	{
		pBoard->m_kingPos[m_pMovingPiece->owner()] = pMove->to;
	}
}

ApplyMove::~ApplyMove()
{
	// todo: clean up this code

	if(m_pMovingPiece->type() == 'K')
	{
		m_pBoard->m_kingPos[m_pMovingPiece->owner()] = m_oldKingPos;
	}

	m_pBoard->m_board[m_pMove->from.x - 1][m_pMove->from.y - 1] = m_pMovingPiece;
	m_pBoard->m_board[m_pMove->to.x - 1][m_pMove->to.y - 1] = m_pOldDest;

	m_pBoard->m_LastMove = m_LastMove;
}

Board::Board() : m_iPlayerID(0)
{
	m_board.resize(8);

	for(auto& iter : m_board)
	{
		iter.resize(8,nullptr);
	}
}

std::vector<BoardMove> Board::Update(int playerID, const Move* pLastMove, std::vector<Piece>& pieces)
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

	if(pLastMove != nullptr)
	{
		m_LastMove = {{pLastMove->fromFile(), pLastMove->fromRank()}, {pLastMove->toFile(), pLastMove->toRank()}};
	}

	return GetMoves();
}

Piece* Board::GetPiece(const vec2 &pos)
{
	assert(IsOnBoard(pos));
	return m_board[pos.x - 1][pos.y - 1];
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
					assert("Invalid piece type" && false);
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
			AddMove({{pPiece->file(), pPiece->rank()}, {pPiece->file(), iNewRank}}, bCheck, moves);

			// Check if we can move 2 tiles if this is the first move
			if(!pPiece->hasMoved())
			{
				int iDoubleMoveRank = iNewRank + ((m_iPlayerID == 0) ? 1 : -1);

				// First check if we can move to the tile in front of us
				if(IsTileEmpty(pPiece->file(),iDoubleMoveRank))
				{
					AddMove({{pPiece->file(), pPiece->rank()}, {pPiece->file(), iDoubleMoveRank}}, bCheck, moves);
				}
			}
		}

		// En passant check
		if(m_LastMove.from.x != 0)
		{
			if(abs(m_LastMove.to.y - m_LastMove.from.y) == 2)
			{
				Piece* pLastPieceMoved = GetPiece(m_LastMove.to);

				if(pLastPieceMoved->type() == int('P'))
				{
					if(pLastPieceMoved->rank() == pPiece->rank())
					{
						int fileDiff = pPiece->file() - pLastPieceMoved->file();

						if(abs(fileDiff) == 1)
						{
							AddMove({{pPiece->file(), pPiece->rank()}, {pPiece->file() - fileDiff, iNewRank}}, bCheck, moves);
						}
					}
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
					AddMove({{pPiece->file(), pPiece->rank()}, {iNewFile, iNewRank}}, bCheck, moves);
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

		do
		{
			pos.x += dir[i].x;
			pos.y += dir[i].y;

			if(IsOnBoard(pos))
			{
				if(!IsTileOwner(pos.x,pos.y))
				{
					// If the tile is empty or is an enemy
					AddMove({{pPiece->file(), pPiece->rank()}, pos}, bCheck, moves);
				}
			}
		} while(IsOnBoard(pos) && IsTileEmpty(pos.x,pos.y));
	}
}

void Board::GenerateDiscreteMoves(Piece* pPiece, bool bCheck, std::vector<BoardMove>& moves)
{
	assert(pPiece->type() == int('N') || pPiece->type() == int('K'));

	const vec2 dir[][8] =
	{
		// Knight
		{
			{1, 2}, {2, 1}, {-2, 1}, {-1, 2},
			{1, -2}, {2, -1}, {-2, -1}, {-1, -2}
		},
		// King
		{
			{-1, -1}, {0, -1}, {1, -1}, { -1, 0},
			{ 1, 0}, {-1, 1}, {0, 1}, {1, 1}
		}
	};

	unsigned int index = pPiece->type() == int('K') ? 1 : 0;

	for(const vec2& move : dir[index])
	{
		vec2 pos = {pPiece->file(), pPiece->rank()};
		pos.x += move.x;
		pos.y += move.y;

		if(IsOnBoard(pos) && !IsTileOwner(pos.x,pos.y))
		{
			AddMove({{pPiece->file(), pPiece->rank()},pos}, bCheck, moves);
		}
	}
}

void Board::AddMove(const BoardMove& move, bool bCheck, std::vector<BoardMove>& moves)
{
	if(!bCheck || !IsInCheck(move))
	{
		moves.push_back(move);
	}
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

bool Board::IsInCheck(const BoardMove& move)
{
	// todo: clean this up
	ApplyMove triedMove(&move, this);

	int oldPlayerID = m_iPlayerID;
	m_iPlayerID = !m_iPlayerID;

	// Generate moves for the other team
	std::vector<BoardMove> moves = GetMoves(false);

	// Check if any of their pieces are attacking our king
	auto iter = std::find_if(moves.begin(),moves.end(),[&](const BoardMove& m) -> bool
	{
		return (m.to.x == m_kingPos[oldPlayerID].x) && (m.to.y == m_kingPos[oldPlayerID].y);
	});

	m_iPlayerID = oldPlayerID;

	return iter != moves.end();
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
