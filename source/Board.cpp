#include "Board.h"
#include <cassert>
#include <algorithm>
#include <iostream>


using std::cout;
using std::endl;

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

Piece* Board::GetPiece(const ivec2 &pos)
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
				case 'K':
					GenerateCastleMove(piece, bCheck, moves);
				case 'N':
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
			// At this point, it is possible for us to promote
			GeneratePromotedPawnMoves({pPiece->file(), pPiece->rank()}, {pPiece->file(), iNewRank}, bCheck, moves);

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
		if((pPiece->rank() == 5 && m_iPlayerID == 0) || (pPiece->rank() == 4 && m_iPlayerID == 1))
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
							AddMove({{pPiece->file(), pPiece->rank()}, {pPiece->file() - fileDiff, iNewRank}, 'Q', SpecialMove::EnPassant}, bCheck, moves);
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
					GeneratePromotedPawnMoves({pPiece->file(), pPiece->rank()}, {iNewFile, iNewRank}, bCheck, moves);
				}
			}
		}
	}
}

void Board::GeneratePromotedPawnMoves(const ivec2& from, const ivec2& to, bool bCheck, std::vector<BoardMove>& moves)
{
	if((to.y == 1 && m_iPlayerID == 1) || (to.y == 8 && m_iPlayerID == 0))
	{
		for(int promotion : {'Q', 'B', 'N', 'R'})
		{
			AddMove({from, to, promotion, SpecialMove::Promotion}, bCheck, moves);
		}
	}
	else
	{
		AddMove({from, to}, bCheck, moves);
	}
}

void Board::GenerateDirectionMoves(Piece* pPiece, bool bCheck, std::vector<BoardMove>& moves)
{
	assert(pPiece->type() == int('B') || pPiece->type() == int('R') || pPiece->type() == int('Q'));

	const ivec2 dir[] =
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
		ivec2 pos = {pPiece->file(), pPiece->rank()};

		do
		{
			pos.x += dir[i].x;
			pos.y += dir[i].y;

			if(IsOnBoard(pos) && !IsTileOwner(pos.x,pos.y))
			{
				// If the tile is empty or is an enemy
				AddMove({{pPiece->file(), pPiece->rank()}, pos}, bCheck, moves);
			}

		} while(IsOnBoard(pos) && IsTileEmpty(pos.x,pos.y));
	}
}

void Board::GenerateDiscreteMoves(Piece* pPiece, bool bCheck, std::vector<BoardMove>& moves)
{
	assert(pPiece->type() == int('N') || pPiece->type() == int('K'));

	const ivec2 dir[][8] =
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

	for(const ivec2& move : dir[index])
	{
		ivec2 pos = {pPiece->file(), pPiece->rank()};
		pos.x += move.x;
		pos.y += move.y;

		if(IsOnBoard(pos) && !IsTileOwner(pos.x,pos.y))
		{
			AddMove({{pPiece->file(), pPiece->rank()},pos}, bCheck, moves);
		}
	}
}

void Board::GenerateCastleMove(Piece* pPiece, bool bCheck, std::vector<BoardMove>& moves)
{
	if(!bCheck)
		return;

	// Castle move logic
	// Generate moves for the other team
	// Check to see if the king has moved
	if(!pPiece->hasMoved() && !IsInCheck())
	{
		int y = pPiece->rank();

		Piece* rooks[2] = {m_board.front()[y - 1], m_board.back()[y - 1]};
		int dir[2] = {-1, 1};

		int oldPlayerID = m_iPlayerID;
		m_iPlayerID = !m_iPlayerID;

		// Generate moves for the other team
		std::vector<BoardMove> enemyMoves = GetMoves(false);
		m_iPlayerID = oldPlayerID;

		for(unsigned int i = 0; i < 2; ++i)
		{
			// See if the left or right rook has moved
			if(rooks[i] != nullptr && !rooks[i]->hasMoved())
			{
				ivec2 pos = {pPiece->file(), pPiece->rank()};

				// todo: Move this code into a function
				bool bValidState = true;

				do
				{
					pos.x += dir[i];
					bValidState &= IsTileEmpty(pos.x,pos.y);

					if(bValidState && (pos.x == 4 || pos.x == 6))
					{
						for(const BoardMove& m : enemyMoves)
						{
							bValidState &= (m.to != pos);
						}
					}

				} while(bValidState && (pos.x > 2 && pos.x < 7));

				// If nothing is in the way of the rook and the king
				if(bValidState)
				{
					AddMove({{pPiece->file(), pPiece->rank()}, {pPiece->file() + dir[i] * 2, pPiece->rank()}, 'Q', SpecialMove::Castle}, bCheck, moves);
				}
			}
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

bool Board::IsOnBoard(const ivec2& coord) const
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

	return IsInCheck();
}

bool Board::IsInCheck()
{
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
