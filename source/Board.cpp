#include "Board.h"
#include <cassert>
#include <algorithm>
#include <iostream>


using std::cout;
using std::endl;

ApplyMove::ApplyMove(const BoardMove& move, Board* pBoard) : m_move(move), m_pBoard(pBoard)
{
	// todo: clean up this code
	assert(pBoard != nullptr);

	BoardPiece* pFrom = m_pBoard->GetPiece(m_move.from);
	assert(pFrom != nullptr);

	m_pBoard->m_moveHistory.push_front(m_move);

	m_oldIndex = m_pBoard->m_board[m_move.from.x - 1][m_move.from.y - 1];
	m_newIndex = m_pBoard->m_board[m_move.to.x - 1][m_move.to.y - 1];

	m_pBoard->m_board[m_move.from.x - 1][m_move.from.y - 1] = 0;
	m_pBoard->m_board[m_move.to.x - 1][m_move.to.y - 1] = m_oldIndex;

	m_LastMove = m_pBoard->m_LastMove;
	m_pBoard->m_LastMove = m_move;

	pFrom->file = m_move.to.x;
	pFrom->rank = m_move.to.y;

	m_hasMoved = pFrom->hasMoved;
	pFrom->hasMoved = 1;

	// Turns left for stalemate logic
	if(m_move.capturedType == 0 && pFrom->type != 'P')
	{
		m_pBoard->m_turnsToStalemate--;
	}
	else
	{
		m_oldTurnsToStalemate = m_pBoard->m_turnsToStalemate;
		m_pBoard->m_turnsToStalemate = 100;
	}

	// Keep track of the kings
	if(pFrom->type == 'K')
	{
		m_oldKingPos = m_pBoard->m_kingPos[pFrom->owner];
		m_pBoard->m_kingPos[pFrom->owner] = m_move.to;
	}
	// Promotion logic
	else if(m_move.specialMove == SpecialMove::Promotion)
	{
		pFrom->type = m_move.promotion;
	}
	else if(m_move.specialMove == SpecialMove::Castle)
	{
		ApplyCastleMove(true);
	}
}

ApplyMove::~ApplyMove()
{
	// todo: clean up this code
	BoardPiece* pFrom = m_pBoard->GetPiece(m_move.to);
	assert(pFrom != nullptr);

	m_pBoard->m_moveHistory.pop_front();

	// Keep track of the kings
	if(pFrom->type == 'K')
	{
		m_pBoard->m_kingPos[pFrom->owner] = m_oldKingPos;
	}
	// Promotion logic
	else if(m_move.specialMove == SpecialMove::Promotion)
	{
		pFrom->type = pFrom->piece.type();
	}
	else if(m_move.specialMove == SpecialMove::Castle)
	{
		ApplyCastleMove(false);
	}

	// Turns left for stalemate logic
	if(m_move.capturedType == 0 && pFrom->type != 'P')
	{
		m_pBoard->m_turnsToStalemate++;
	}
	else
	{
		m_pBoard->m_turnsToStalemate = m_oldTurnsToStalemate;
	}

	m_pBoard->m_board[m_move.from.x - 1][m_move.from.y - 1] = m_oldIndex;
	m_pBoard->m_board[m_move.to.x - 1][m_move.to.y - 1] = m_newIndex;

	m_pBoard->m_LastMove = m_LastMove;

	pFrom->file = m_move.from.x;
	pFrom->rank = m_move.from.y;
	pFrom->hasMoved = m_hasMoved;
}

void ApplyMove::ApplyCastleMove(bool bApply)
{
	int rookFile = 1;
	int rookToFile = 4;

	if((m_move.to.x - m_move.from.x) > 0)
	{
		// Rook will move to the left
		rookFile = 8;
		rookToFile = 6;
	}

	if(!bApply)
	{
		std::swap(rookFile, rookToFile);
	}

	BoardPiece* pRook = m_pBoard->GetPiece({rookFile,m_move.from.y});
	assert(pRook != nullptr);

	pRook->file = rookToFile;
	pRook->hasMoved = bApply;

	std::swap(m_pBoard->m_board[rookFile - 1][m_move.from.y - 1], m_pBoard->m_board[rookToFile - 1][m_move.from.y - 1]);
}

Board::Board() : m_turnsToStalemate(0)
{
	m_board.resize(8);

	for(auto& iter : m_board)
	{
		iter.resize(8);
	}
}

void Board::Update(int turnsToStalemate, const std::vector<Move>& moves, const std::vector<Piece>& pieces)
{
	// Clear the old board
	Clear();

	// Fill new board with pieces
	for(unsigned int i = 0; i < pieces.size(); ++i)
	{
		const Piece& p = pieces[i];

		if(p.type() == 'K')
		{
			m_kingPos[p.owner()] = {p.file(), p.rank()};
		}

		unsigned int index = p.id() * p.type();

		m_board[p.file() - 1][p.rank() - 1] = index;
		m_pieces.insert({index, {p, p.owner(), p.file(), p.rank(), p.hasMoved(), p.type()}});
	}

	if(!moves.empty())
	{
		m_LastMove = {{moves[0].fromFile(), moves[0].fromRank()}, {moves[0].toFile(), moves[0].toRank()}};

		// todo: this copy could be avoided
		if(moves.size() >= 8)
		{
			m_moveHistory.clear();

			for(unsigned int i = 0; i < 8; ++i)
			{
				m_moveHistory.push_back({{moves[i].fromFile(), moves[i].fromRank()}, {moves[i].toFile(), moves[i].toRank()}});
			}
		}
	}

	m_turnsToStalemate = turnsToStalemate;
}

const std::vector<BoardMove>& Board::GetMoves(int playerID)
{
	auto iter = m_validMoveCache[playerID].find(m_board);
	if(iter != m_validMoveCache[playerID].end())
	{
		return iter->second;
	}

	auto iterPair = m_validMoveCache[playerID].insert(std::make_pair(m_board, GetMoves(playerID, true)));
	return iterPair.first->second;
}

int Board::GetWorth(int playerID, const std::function<int(const Board& board, const std::vector<BoardMove>&, const BoardPiece&)>& heuristic)
{
	std::vector<BoardMove> moves = GetMoves(playerID, false);

	int iTotal[2] = {0,0};
	for(auto& iter : m_board)
	{
		for(auto& subIter : iter)
		{
			auto iter = m_pieces.find(subIter);
			if(iter != m_pieces.end())
			{
				const BoardPiece& piece = iter->second;
				iTotal[piece.owner] += heuristic(*this, moves, piece);
			}
		}
	}

	return (iTotal[playerID] - iTotal[!playerID]);
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

bool Board::IsOnBoard(int pos) const
{
	return (pos <= 8 && pos >= 1);
}

bool Board::IsOnBoard(const ivec2& pos) const
{
	return IsOnBoard(pos.x) && IsOnBoard(pos.y);
}

bool Board::IsTileEmpty(const ivec2& pos) const
{
	assert(IsOnBoard(pos));

	return m_board[pos.x - 1][pos.y - 1] == 0;
}

bool Board::IsTileOwner(const ivec2& pos, int playerID) const
{
	assert(IsOnBoard(pos));

	if(IsTileEmpty(pos))
		return false;

	const BoardPiece* pPiece = GetPiece(pos);
	return pPiece->owner == playerID;
}

bool Board::IsInCheckmate(int playerID)
{
	return IsInCheck(playerID) && GetMoves(playerID).empty();
}

bool Board::IsInStalemate(int playerID)
{
	return (m_turnsToStalemate == 0) || IsThreeBoardStateStalemate() || IsNotEnoughPiecesStalemate() || IsNoLegalMovesStalemate(playerID);
}

unsigned int Board::GetNumPieces() const
{
	return m_pieces.size();
}

std::vector<BoardMove> Board::GetMoves(int playerID, bool bCheck)
{
	std::vector<BoardMove> moves;
	moves.reserve(35);
	for(auto& iter : m_board)
	{
		for(auto& subIter : iter)
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

	return std::move(moves);
}

void Board::GeneratePawnMoves(const BoardPiece& piece, bool bCheck, std::vector<BoardMove>& moves)
{
	assert(piece.type == int('P'));

	int iNewRank = piece.rank + ((piece.owner == 0) ? 1 : -1);
	if(IsOnBoard(iNewRank))
	{
		// First check if we can move to the tile in front of us
		if(IsTileEmpty({piece.file,iNewRank}))
		{
			// At this point, it is possible for us to promote
			GeneratePromotedPawnMoves({piece.file, piece.rank}, {piece.file, iNewRank}, piece.owner, bCheck, moves);

			// Check if we can move 2 tiles if this is the first move
			if(!piece.hasMoved)
			{
				int iDoubleMoveRank = iNewRank + ((piece.owner == 0) ? 1 : -1);

				// First check if we can move to the tile in front of us
				if(IsTileEmpty({piece.file,iDoubleMoveRank}))
				{
					ivec2 from = {piece.file, piece.rank};
					ivec2 to = {piece.file, iDoubleMoveRank};
					AddMove({from, to}, bCheck, moves);
				}
			}
		}

		// En passant check
		if((piece.rank == 5 && piece.owner == 0) || (piece.rank == 4 && piece.owner == 1))
		{
			if(abs(m_LastMove.to.y - m_LastMove.from.y) == 2)
			{
				BoardPiece* pLastPieceMoved = GetPiece(m_LastMove.to);
				assert(pLastPieceMoved != nullptr);

				if(pLastPieceMoved->type == int('P'))
				{
					if(pLastPieceMoved->rank == piece.rank)
					{
						int fileDiff = piece.file - pLastPieceMoved->file;

						if(abs(fileDiff) == 1)
						{
							ivec2 from = {piece.file, piece.rank};
							ivec2 to = {piece.file - fileDiff, iNewRank};
							AddMove({from, to, 'P', 'Q', SpecialMove::EnPassant}, bCheck, moves);
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
				if(!IsTileEmpty({iNewFile,iNewRank}) && !IsTileOwner({iNewFile, iNewRank}, piece.owner))
				{
					GeneratePromotedPawnMoves({piece.file, piece.rank}, {iNewFile, iNewRank}, piece.owner, bCheck, moves);
				}
			}
		}
	}
}

void Board::GeneratePromotedPawnMoves(const ivec2& from, const ivec2& to, int playerID, bool bCheck, std::vector<BoardMove>& moves)
{
	int capturedType = GetPieceType(to);
	if((to.y == 1 && playerID == 1) || (to.y == 8 && playerID == 0))
	{
		for(int promotion : {'Q', 'B', 'N', 'R'})
		{
			AddMove({from, to, capturedType, promotion, SpecialMove::Promotion}, bCheck, moves);
		}
	}
	else
	{
		AddMove({from, to, capturedType, 'Q'}, bCheck, moves);
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

			if(IsOnBoard(pos) && !IsTileOwner(pos, piece.owner))
			{
				// If the tile is empty or is an enemy
				ivec2 from = {piece.file, piece.rank};
				AddMove({from, pos, GetPieceType(pos)}, bCheck, moves);
			}

		} while(IsOnBoard(pos) && IsTileEmpty(pos));
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

		if(IsOnBoard(pos) && !IsTileOwner(pos, piece.owner))
		{
			ivec2 from = {piece.file, piece.rank};
			AddMove({from, pos, GetPieceType(pos)}, bCheck, moves);
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
					bValidState &= IsTileEmpty(pos);

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
					const ivec2 invalidPawnPositions[2][2] =
					{
						{
							{7,1},
							{2,1}
						},
						{
							{7,7},
							{2,7}
						}
					};

					// Check to make sure that there is not a pawn attacking that did not get picked up in the moves previously generated
					for(auto iter : invalidPawnPositions[piece.owner])
					{
						bValidState &= !(IsTileOwner(iter, !piece.owner) && GetPieceType(iter) == 'P');
					}

					if(bValidState)
					{
						ivec2 from = {piece.file, piece.rank};
						ivec2 to = {piece.file + dir[i] * 2, piece.rank};
						AddMove({from, to, 0, 'Q', SpecialMove::Castle}, bCheck, moves);
					}
				}
			}
		}
	}
}

int Board::GetPieceType(const ivec2& pos) const
{
	const BoardPiece* pTo = GetPiece(pos);
	return ((pTo != nullptr) ? pTo->type : 0);
}

void Board::AddMove(const BoardMove& move, bool bCheck, std::vector<BoardMove>& moves)
{
	if(bCheck)
	{
		BoardPiece* pFrom = GetPiece(move.from);
		assert(pFrom != nullptr);

		ApplyMove triedMove(move, this);

		if(!IsInCheck(pFrom->owner))
		{
			moves.push_back(move);
		}
	}
	else
	{
		moves.push_back(move);
	}
}

bool Board::IsInCheck(int playerID)
{
	// Generate moves for the other team
	std::vector<BoardMove> validMoves = GetMoves(!playerID, false);

	// Check if any of their pieces are attacking our king
	auto iter = std::find_if(validMoves.begin(),validMoves.end(),[&](const BoardMove& m) -> bool
	{
		return (m.to.x == m_kingPos[playerID].x) && (m.to.y == m_kingPos[playerID].y);
	});

	return iter != validMoves.end();
}

bool Board::IsNoLegalMovesStalemate(int playerID)
{
	return GetMoves(playerID).empty() && !IsInCheck(playerID);
}

bool Board::IsNotEnoughPiecesStalemate() const
{
	// todo: cleanup this method

	// Test 2:
	// king against king;
	// king against king and bishop;
	// king against king and knight;
	// king and bishop against king and bishop, with both bishops on squares of the same color
	int counters[2][2] = {{0}}; // bishop, knight
	ivec2 bishopPos[2];

	for(auto& iter : m_board)
	{
		for(auto& subIter : iter)
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
	{
		cout << "King vs. king stalemate detected!" << endl;
		return true;
	}

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
	if(m_turnsToStalemate <= 92 && m_moveHistory.size() >= 8)
	{
		auto equalFunctor = [](const BoardMove& a, const BoardMove& b) -> bool
		{
			return (a.from == b.from && a.to == b.to);
		};

		bool bEqual = std::equal(m_moveHistory.begin(), m_moveHistory.begin() + 4, m_moveHistory.begin() + 4, equalFunctor);

#ifdef DEBUG_OUTPUT
		if(bEqual)
		{
			cout << "Three board state stalemate detected!" << endl;
		}
#endif

		return bEqual;
	}

	return false;
}

void Board::Clear()
{
	// Clear the board of pieces
	for(auto& fileIter : m_board)
	{
		for(int& rankIter : fileIter)
		{
			rankIter = 0;
		}
	}

	// Clear the cache
	for(auto& iter : m_validMoveCache)
	{
		iter.clear();
	}

	// Clear the list of pieces
	m_pieces.clear();
}
