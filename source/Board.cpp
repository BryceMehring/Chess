#include "Board.h"
#include <algorithm>
#include <iostream>
#include <cassert>

using std::cout;
using std::endl;

bool operator ==(const BoardPiece& a, const BoardPiece& b)
{
	return (a.type == b.type) && (a.hasMoved == b.hasMoved) && (a.owner == b.owner);
}

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
	
	if(m_move.capturedType != 0)
	{
		m_pBoard->m_piecesCount[!pFrom->owner]--;
		
		// Update the number of knights and bishops
		if(m_move.capturedType == 'N')
		{
			m_pBoard->m_knightCounter[!pFrom->owner]--;
		}
		else if(m_move.capturedType == 'B')
		{
			m_pBoard->m_bishopCounter[!pFrom->owner]--;
		}
	}
	
	if(pFrom->type == 'K') // Keep track of the kings
	{
		m_oldKingPos = m_pBoard->m_kingPos[pFrom->owner];
		m_pBoard->m_kingPos[pFrom->owner] = m_move.to;
	}
	
	// Promotion logic
	else if(m_move.specialMove == SpecialMove::Promotion)
	{
		m_type = pFrom->type;
		pFrom->type = m_move.promotion;
		
		// Update the number of knights and bishops upon promotion
		if(m_move.promotion == 'N')
		{
			m_pBoard->m_knightCounter[!pFrom->owner]++;
		}
		else if(m_move.promotion == 'B')
		{
			m_pBoard->m_bishopCounter[!pFrom->owner]++;
			m_pBoard->m_bishopPos[!pFrom->owner] = m_move.to;
		}
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
		pFrom->type = m_type;
		
		// Update the number of knights and bishops upon promotion
		if(m_move.promotion == 'N')
		{
			m_pBoard->m_knightCounter[!pFrom->owner]--;
		}
		else if(m_move.promotion == 'B')
		{
			m_pBoard->m_bishopCounter[!pFrom->owner]--;
		}
	}
	else if(m_move.specialMove == SpecialMove::Castle)
	{
		ApplyCastleMove(false);
	}

	// Turns left for stalemate logic
	if((m_move.capturedType == 0) && (pFrom->type != 'P'))
	{
		m_pBoard->m_turnsToStalemate++;
	}
	else
	{
		m_pBoard->m_turnsToStalemate = m_oldTurnsToStalemate;
	}
	
	if(m_move.capturedType != 0)
	{
		m_pBoard->m_piecesCount[!pFrom->owner]++;
		
		// Update the number of knights and bishops
		if(m_move.capturedType == 'N')
		{
			m_pBoard->m_knightCounter[!pFrom->owner]++;
		}
		else if(m_move.capturedType == 'B')
		{
			m_pBoard->m_bishopCounter[!pFrom->owner]++;
		}
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

void BoardHash::SetBoard(const Board* pBoard)
{
	s_pBoard = pBoard;
}

std::size_t BoardHash::operator()(const std::vector<std::vector<int>>& key) const
{
	std::size_t h = 5381;
	for(const auto& iter : key)
	{
		for(int i : iter)
		{
			int c = '0';
			const BoardPiece* pPiece = s_pBoard->GetPiece(i);
			if(pPiece != nullptr)
			{
				c = pPiece->type;
			}
			
			h ^= c + 0x9e3779b9 + (h<<6) + (h>>2);
		}
	}

	return h;
}

const Board* BoardHash::s_pBoard = nullptr;

void BoardEqual::SetBoard(const Board* pBoard)
{
	s_pBoard = pBoard;
}

bool BoardEqual::operator()(const std::vector<std::vector<int>>& a, const std::vector<std::vector<int>>& b) const
{
	return std::equal(a.begin(), a.end(), b.begin(), [this](const std::vector<int>& subA, const std::vector<int>& subB) -> bool
	{
		return std::equal(subA.begin(), subA.end(), subB.begin(), [this](int idA, int idB) -> bool
		{
			if((idA == 0) && (idB == 0))
				return true;
			
			const BoardPiece* pPieceA = s_pBoard->GetPiece(idA);
			const BoardPiece* pPieceB = s_pBoard->GetPiece(idB);
			
			if(pPieceA == nullptr || pPieceB == nullptr)
				return false;
			
			return (*pPieceA) == (*pPieceB);
		});
	});
}

const Board* BoardEqual::s_pBoard = nullptr;

Board::Board() : m_turnsToStalemate(0), m_cacheHit(0), m_cacheTotal(0)
{
	BoardHash::SetBoard(this);
	BoardEqual::SetBoard(this);
	
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
	for(const Piece& p : pieces)
	{
		m_piecesCount[p.owner()]++;
		
		// Cache locations and count of pieces on the board
		switch(p.type())
		{
			case 'N':
				m_knightCounter[p.owner()]++;
				break;
			case 'B':
				m_bishopCounter[p.owner()]++;
				m_bishopPos[p.owner()] = {p.file(), p.rank()};
				break;
			case 'K':
				m_kingPos[p.owner()] = {p.file(), p.rank()};
				break;
			default:
				break;
		}

		unsigned int index = p.id();

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

std::vector<BoardMove> Board::GetMoves(int playerID)
{
	return GetMoves(playerID, true);
}

int Board::GetWorth(int playerID, const std::function<int(const Board& board, const BoardPiece&)>& heuristic)
{
	int iTotal[2] = {0,0};
	for(auto& iter : m_board)
	{
		for(auto& subIter : iter)
		{
			auto iter = m_pieces.find(subIter);
			if(iter != m_pieces.end())
			{
				const BoardPiece& piece = iter->second;
				iTotal[piece.owner] += heuristic(*this, piece);
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
	if(!IsOnBoard(pos))
		return nullptr;
		
	int id = m_board[pos.x - 1][pos.y - 1];
	return GetPiece(id);
}

BoardPiece* Board::GetPiece(int id)
{
	return const_cast<BoardPiece*>(static_cast<const Board*>(this)->GetPiece(id));
}

const BoardPiece* Board::GetPiece(int id) const
{
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
	if(!IsOnBoard(pos))
		return false;

	return m_board[pos.x - 1][pos.y - 1] == 0;
}

bool Board::IsTileOwner(const ivec2& pos, int playerID) const
{
	if(!IsOnBoard(pos))
		return false;

	if(IsTileEmpty(pos))
		return false;

	const BoardPiece* pPiece = GetPiece(pos);
	assert(pPiece != nullptr);
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
	return (m_piecesCount[0] + m_piecesCount[1]);
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
		return (m.to == m_kingPos[playerID]);
	});

	return iter != validMoves.end();
}

bool Board::IsNoLegalMovesStalemate(int playerID)
{
	return GetMoves(playerID).empty() && !IsInCheck(playerID);
}

bool Board::IsNotEnoughPiecesStalemate() const
{	
	if((m_piecesCount[0] > 2) || (m_piecesCount[1] > 2))
	{
		return false;
	}

	// king against king;
	// king against king and bishop;
	// king against king and knight;
	// king and bishop against king and bishop, with both bishops on squares of the same color
	
	// king against king
	if((m_piecesCount[0] == 1) && (m_piecesCount[1] == 1))
	{
		//cout << "Stalemate: king against king" << endl;
		return true;
	}

	// king against king and bishop;
	// king against king and knight;
	if(((m_knightCounter[0] == 1) ^ (m_bishopCounter[0] == 1)) ||
	   ((m_knightCounter[1] == 1) ^ (m_bishopCounter[1] == 1)))
	{
		//cout << "Stalemate: king against king and (bishop or knight)" << endl;
		return true;
	}

	// king and bishop against king and bishop, with both bishops on squares of the same color
	if((m_bishopCounter[0] == 1) && (m_bishopCounter[1] == 1))
	{
		if(((m_bishopPos[0].x + m_bishopPos[0].y) % 2) == ((m_bishopPos[1].x + m_bishopPos[1].y) % 2))
		{
			//cout << "Stalemate: king and bishop against king and bishop, with both bishops on squares of the same color" << endl;
			return true;
		}
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

		return std::equal(m_moveHistory.begin(), m_moveHistory.begin() + 4, m_moveHistory.begin() + 4, equalFunctor);
	}

	return false;
}

void Board::Clear()
{
	m_cacheHit = m_cacheTotal = 0;
	
	m_piecesCount[0] = m_piecesCount[1] = 0;
	m_knightCounter[0] = m_knightCounter[1] = 0;
	m_bishopCounter[0] = m_bishopCounter[1] = 0;
	m_bishopPos[0] = m_bishopPos[1] = ivec2();

	// Clear the board of pieces
	for(auto& fileIter : m_board)
	{
		for(int& rankIter : fileIter)
		{
			rankIter = 0;
		}
	}

	// Clear the list of pieces
	m_pieces.clear();
}
