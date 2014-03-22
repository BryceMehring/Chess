#include "Board.h"
#include <cassert>
#include <algorithm>
#include <iostream>


using std::cout;
using std::endl;

ApplyMove::ApplyMove(const BoardMove* pMove, Board* pBoard) : m_pMove(pMove), m_pBoard(pBoard)
{
	// todo: clean up this code

	pBoard->m_moveHistory.push_front(*pMove);

	m_oldIndex = pBoard->m_board[pMove->from.x - 1][pMove->from.y - 1];
	m_newIndex = pBoard->m_board[pMove->to.x - 1][pMove->to.y - 1];

	pBoard->m_board[pMove->from.x - 1][pMove->from.y - 1] = 0;
	pBoard->m_board[pMove->to.x - 1][pMove->to.y - 1] = m_oldIndex;

	m_LastMove = pBoard->m_LastMove;
	pBoard->m_LastMove = *pMove;

	pMove->pFrom->file = pMove->to.x;
	pMove->pFrom->rank = pMove->to.y;

	m_hasMoved = pMove->pFrom->hasMoved;
	pMove->pFrom->hasMoved = 1;

	if(m_pMove->pFrom->type == 'K')
	{
		m_oldKingPos = pBoard->m_kingPos[pMove->pFrom->owner];
		pBoard->m_kingPos[pMove->pFrom->owner] = pMove->to;
	}
	else if(m_pMove->specialMove == SpecialMove::Promotion)
	{
		m_pMove->pFrom->type = m_pMove->promotion;
	}
}

ApplyMove::~ApplyMove()
{
	// todo: clean up this code

	m_pBoard->m_moveHistory.pop_front();

	if(m_pMove->pFrom->type == 'K')
	{
		m_pBoard->m_kingPos[m_pMove->pFrom->owner] = m_oldKingPos;
	}
	else if(m_pMove->specialMove == SpecialMove::Promotion)
	{
		m_pMove->pFrom->type = m_pMove->pFrom->piece.type();
	}

	m_pBoard->m_board[m_pMove->from.x - 1][m_pMove->from.y - 1] = m_oldIndex;
	m_pBoard->m_board[m_pMove->to.x - 1][m_pMove->to.y - 1] = m_newIndex;

	m_pBoard->m_LastMove = m_LastMove;

	m_pMove->pFrom->file = m_pMove->from.x;
	m_pMove->pFrom->rank = m_pMove->from.y;
	m_pMove->pFrom->hasMoved = m_hasMoved;
}

Board::Board()
{
	m_board.resize(8);

	for(auto& iter : m_board)
	{
		iter.resize(8);
	}
}

void Board::Update(const std::vector<Move>& moves, const std::vector<Piece>& pieces)
{
	// Clear the old board
	Clear();

	// Fill new board with pieces
	for(const Piece& p : pieces)
	{
		if(p.type() == 'K')
		{
			m_kingPos[p.owner()] = {p.file(), p.rank()};
		}

		m_board[p.file() - 1][p.rank() - 1] = p.id();
		m_pieces.insert({p.id(), {p, p.owner(), p.file(), p.rank(), p.hasMoved(), p.type()}});
	}

	if(!moves.empty())
	{
		m_LastMove = {{moves[0].fromFile(), moves[0].fromRank()}, {moves[0].toFile(), moves[0].toRank()}};

		// todo: this copy could be avoided
		m_moveHistory.clear();
		for(auto iter : moves)
		{
			m_moveHistory.push_back({{iter.fromFile(), iter.fromRank()}, {iter.toFile(), iter.toRank()}});
		}
	}
}

std::vector<BoardMove> Board::GetMoves(int playerID)
{
	return GetMoves(playerID, true);
}

float Board::GetWorth(int playerID, const std::function<float(const Board& board, const std::vector<BoardMove>&, const BoardPiece&)>& heuristic)
{
	if(IsInCheckmate(!playerID))
		return 10000.0f;

	if(IsInStalemate(playerID))
		return -10000.0f;

	/*if(m_LastMove.specialMove == SpecialMove::Promotion)
		return 50.0f;*/

	std::vector<BoardMove> moves = GetMoves(playerID, false);

	float fTotal[2] = {0,0};
	for(auto iter : m_board)
	{
		for(auto subIter : iter)
		{
			auto iter = m_pieces.find(subIter);
			if(iter != m_pieces.end())
			{
				const BoardPiece& piece = iter->second;

				fTotal[piece.owner] += heuristic(*this, moves, piece);
			}
		}
	}

	return fTotal[playerID] - fTotal[!playerID];
}

BoardPiece* Board::GetPiece(const ivec2 &pos)
{
	return const_cast<BoardPiece*>(static_cast<const Board*>(this)->GetPiece(pos));
}

const BoardPiece* Board::GetPiece(const ivec2& pos) const
{
	assert(IsOnBoard(pos));
	int id = m_board[pos.x - 1][pos.y - 1];

	auto iter = m_pieces.find(id);
	return (iter == m_pieces.end()) ? nullptr : &iter->second;
}

std::vector<BoardMove> Board::GetMoves(int playerID, bool bCheck)
{
	std::vector<BoardMove> moves;
	for(auto iter : m_board)
	{
		for(auto subIter : iter)
		{
			auto iter = m_pieces.find(subIter);
			if(iter != m_pieces.end())
			{
				const BoardPiece& piece = iter->second;
				if(playerID == piece.owner)
				{
					switch(piece.type)
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
	}

	return moves;
}

void Board::GeneratePawnMoves(const BoardPiece& piece, bool bCheck, std::vector<BoardMove>& moves)
{
	assert(piece.type == int('P'));

	int iNewRank = piece.rank + ((piece.owner == 0) ? 1 : -1);
	if(IsOnBoard(iNewRank))
	{
		// First check if we can move to the tile in front of us
		if(IsTileEmpty(piece.file,iNewRank))
		{
			// At this point, it is possible for us to promote
			GeneratePromotedPawnMoves({piece.file, piece.rank}, {piece.file, iNewRank}, piece.owner, bCheck, moves);

			// Check if we can move 2 tiles if this is the first move
			if(!piece.hasMoved)
			{
				int iDoubleMoveRank = iNewRank + ((piece.owner == 0) ? 1 : -1);

				// First check if we can move to the tile in front of us
				if(IsTileEmpty(piece.file,iDoubleMoveRank))
				{
					ivec2 from = {piece.file, piece.rank};
					ivec2 to = {piece.file, iDoubleMoveRank};
					AddMove({from, to, GetPiece(from), GetPiece(to)}, bCheck, moves);
				}
			}
		}

		// En passant check
		if((piece.rank == 5 && piece.owner == 0) || (piece.rank == 4 && piece.owner == 1))
		{
			if(abs(m_LastMove.to.y - m_LastMove.from.y) == 2)
			{
				BoardPiece* pLastPieceMoved = GetPiece(m_LastMove.to);

				if(pLastPieceMoved->type == int('P'))
				{
					if(pLastPieceMoved->rank == piece.rank)
					{
						int fileDiff = piece.file - pLastPieceMoved->file;

						if(abs(fileDiff) == 1)
						{
							ivec2 from = {piece.file, piece.rank};
							ivec2 to = {piece.file - fileDiff, iNewRank};
							AddMove({from, to, GetPiece(from), GetPiece(m_LastMove.to), 'Q', SpecialMove::EnPassant}, bCheck, moves);
						}
					}
				}
			}
		}

		// Check if we can capture a piece by moving to a forward diaganol tile
		for(int iNewFile : {piece.file + 1, piece.file - 1})
		{
			if(IsOnBoard(iNewFile))
			{
				if(!IsTileEmpty(iNewFile,iNewRank) && !IsTileOwner(iNewFile, iNewRank, piece.owner))
				{
					GeneratePromotedPawnMoves({piece.file, piece.rank}, {iNewFile, iNewRank}, piece.owner, bCheck, moves);
				}
			}
		}
	}
}

void Board::GeneratePromotedPawnMoves(const ivec2& from, const ivec2& to, int playerID, bool bCheck, std::vector<BoardMove>& moves)
{
	if((to.y == 1 && playerID == 1) || (to.y == 8 && playerID == 0))
	{
		for(int promotion : {'Q', 'B', 'N', 'R'})
		{
			AddMove({from, to, GetPiece(from), GetPiece(to), promotion, SpecialMove::Promotion}, bCheck, moves);
		}
	}
	else
	{
		AddMove({from, to, GetPiece(from), GetPiece(to)}, bCheck, moves);
	}
}

void Board::GenerateDirectionMoves(const BoardPiece& piece, bool bCheck, std::vector<BoardMove>& moves)
{
	assert(piece.type == int('B') || piece.type == int('R') || piece.type == int('Q'));

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

	if(piece.type == int('B'))
	{
		end = 4;
	}
	else if(piece.type == int('R'))
	{
		start = 4;
	}

	for(unsigned int i = start; i < end; ++i)
	{
		ivec2 pos = {piece.file, piece.rank};

		do
		{
			pos.x += dir[i].x;
			pos.y += dir[i].y;

			if(IsOnBoard(pos) && !IsTileOwner(pos.x,pos.y, piece.owner))
			{
				// If the tile is empty or is an enemy
				ivec2 from = {piece.file, piece.rank};
				AddMove({from, pos, GetPiece(from), GetPiece(pos)}, bCheck, moves);
			}

		} while(IsOnBoard(pos) && IsTileEmpty(pos.x,pos.y));
	}
}

void Board::GenerateDiscreteMoves(const BoardPiece& piece, bool bCheck, std::vector<BoardMove>& moves)
{
	assert(piece.type == int('N') || piece.type == int('K'));

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

	unsigned int index = piece.type == int('K') ? 1 : 0;

	for(const ivec2& move : dir[index])
	{
		ivec2 pos = {piece.file, piece.rank};
		pos.x += move.x;
		pos.y += move.y;

		if(IsOnBoard(pos) && !IsTileOwner(pos.x, pos.y, piece.owner))
		{
			ivec2 from = {piece.file, piece.rank};
			AddMove({from, pos, GetPiece(from), GetPiece(pos)}, bCheck, moves);
		}
	}
}

void Board::GenerateCastleMove(const BoardPiece& piece, bool bCheck, std::vector<BoardMove>& moves)
{
	if(!bCheck)
		return;

	if(!piece.hasMoved && !IsInCheck(piece.owner))
	{
		BoardPiece* rooks[2] = {GetPiece({1,piece.rank}), GetPiece({8,piece.rank})};
		int dir[2] = {-1, 1};

		// Generate moves for the other team
		std::vector<BoardMove> enemyMoves = GetMoves(!piece.owner, false);

		for(unsigned int i = 0; i < 2; ++i)
		{
			// See if the left or right rook has moved
			if(rooks[i] != nullptr && !rooks[i]->hasMoved)
			{
				ivec2 pos = {piece.file, piece.rank};

				// todo: Move this code into a function
				bool bValidState = true;

				do
				{
					pos.x += dir[i];
					bValidState &= IsTileEmpty(pos.x,pos.y);

					if(bValidState && (pos.x == 4 || pos.x == 6))
					{
						for(auto iter = enemyMoves.begin(); bValidState && (iter != enemyMoves.end()); ++iter)
						{
							bValidState &= (iter->to != pos);
						}
					}

				} while(bValidState && (pos.x > 2 && pos.x < 7));

				// If nothing is in the way of the rook and the king and the king cannot be attacked on either side
				if(bValidState)
				{
					ivec2 from = {piece.file, piece.rank};
					ivec2 to = {piece.file + dir[i] * 2, piece.rank};
					AddMove({from, to, GetPiece(from), GetPiece(to), 'Q', SpecialMove::Castle}, bCheck, moves);
				}
			}
		}

	}
}

void Board::AddMove(const BoardMove& move, bool bCheck, std::vector<BoardMove>& moves)
{
	if(bCheck)
	{
		ApplyMove triedMove(&move, this);

		if(!IsInCheck(move.pFrom->owner))
		{
			moves.push_back(move);
		}
	}
	else
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

	return m_board[file - 1][rank - 1] == 0;
}

bool Board::IsTileOwner(int file, int rank, int playerID) const
{
	assert(IsOnBoard(file) && IsOnBoard(rank));

	if(IsTileEmpty(file,rank))
		return false;

	const BoardPiece* pPiece = GetPiece({file, rank});
	return pPiece->owner == playerID;
}

bool Board::IsInCheck(int playerID)
{
	// Generate moves for the other team
	std::vector<BoardMove> moves = GetMoves(!playerID, false);

	// Check if any of their pieces are attacking our king
	auto iter = std::find_if(moves.begin(),moves.end(),[&](const BoardMove& m) -> bool
	{
		return (m.to.x == m_kingPos[playerID].x) && (m.to.y == m_kingPos[playerID].y);
	});

	return iter != moves.end();
}

bool Board::IsInCheckmate(int playerID)
{
	bool bCheckmate = false;
	if(IsInCheck(playerID))
	{
		std::vector<BoardMove> moves = GetMoves(playerID, false);
		bCheckmate = moves.empty();
	}
	return bCheckmate;
}

bool Board::IsInStalemate(int playerID)
{
	return IsNoLegalMovesStalemate(playerID) || IsNotEnoughPiecesStalemate() || IsThreeBoardStateStalemate();
}

bool Board::IsNoLegalMovesStalemate(int playerID)
{
	// Test 1: The game is automatically a draw if the player to move is not in check but has no legal move.
	if(!IsInCheck(playerID))
	{
		std::vector<BoardMove> moves = GetMoves(playerID, false);
		if(moves.empty())
			return true;
	}

	return false;
}

bool Board::IsNotEnoughPiecesStalemate() const
{
	// todo: cleanup this method

	// Test 2:
	// king against king;
	// king against king and bishop;
	// king against king and knight;
	// king and bishop against king and bishop, with both bishops on squares of the same color
	int counters[2][2] = {0}; // bishop, knight
	ivec2 bishopPos[2];

	for(auto iter : m_board)
	{
		for(auto subIter : iter)
		{
			auto iter = m_pieces.find(subIter);
			if(iter != m_pieces.end())
			{
				const BoardPiece& piece = iter->second;

				if(piece.type == 'B')
				{
					counters[piece.owner][0]++;
					bishopPos[piece.owner] = {piece.file, piece.rank};
				}
				else if(piece.type == 'N')
				{
					counters[piece.owner][1]++;
				}
				else if(piece.type != 'K')
				{
					return false;
				}
			}
		}
	}

	bool bOnlyKing[2] = {(counters[0][0] == 0 && counters[0][1] == 0), (counters[1][0] == 0 && counters[1][1] == 0)};

	// king against king
	if(bOnlyKing[0] && bOnlyKing[1])
		return true;

	// king against king and bishop
	if((bOnlyKing[0] && counters[1][1] == 0 && counters[1][0] == 1) ||
	   (bOnlyKing[1] && counters[0][1] == 0 && counters[0][0] == 1))
		return true;

	// king against king and knight
	if((bOnlyKing[0] && counters[1][1] == 1 && counters[1][0] == 0) ||
	   (bOnlyKing[1] && counters[0][1] == 1 && counters[0][0] == 0))
		return true;

	// king and bishop against king and bishop, with both bishops on squares of the same color
	if(counters[0][0] == 1 && counters[0][1] == 0 &&
	   counters[1][0] == 1 && counters[1][1] == 0)
	{
		if((bishopPos[0].x + bishopPos[0].y) % 2 == (bishopPos[1].x + bishopPos[1].y) % 2)
			return true;
	}

	return false;
}

bool Board::IsThreeBoardStateStalemate() const
{
	// todo: this could be optimized

	// Test 3: three board state repetition draw rule
	if(m_moveHistory.size() >= 8)
	{
		auto equalFunctor = [](const BoardMove& a, const BoardMove& b) -> bool
		{
			return (a.from == b.from && a.to == b.to);
		};

		return std::equal(m_moveHistory.begin(), m_moveHistory.begin() + 4, m_moveHistory.begin() + 4, equalFunctor);
	}

	return false;
}

void Board::Clear()
{
	for(auto& fileIter : m_board)
	{
		for(int& rankIter : fileIter)
		{
			rankIter = 0;
		}
	}

	m_pieces.clear();
}
