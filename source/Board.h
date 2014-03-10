#ifndef _BOARD_
#define _BOARD_

#include "Piece.h"
#include "Move.h"
#include "vec2.h"
#include "BoardMove.h"

#include <vector>

// Moves, then unmoves a piece move upon destruction
class ApplyMove
{
public:

	ApplyMove(const BoardMove* pMove, class Board* pBoard);
	~ApplyMove();

private:

	const BoardMove* m_pMove;
	Board* m_pBoard;

	ivec2 m_oldKingPos;
	Piece* m_pOldDest;
	Piece* m_pMovingPiece;

	BoardMove m_LastMove;
};

// Defines a chess board which manages generating valid action states
class Board
{
public:

	friend class ApplyMove;

	// Constructs an empty board
	Board();

	// Updates the grid and returns all valid moves
	std::vector<BoardMove> Update(int playerID, const Move* pLastMove, std::vector<Piece>& pieces);

	float GetWorth() const;

	// Returns the piece at pos
	// If there is not a piece at pos, nullptr is returned
	Piece* GetPiece(const ivec2& pos);

private:

	std::vector<BoardMove> GetMoves(bool bCheck = true);

	// Generate valid moves for pawns
	void GeneratePawnMoves(Piece* pPiece, bool bCheck, std::vector<BoardMove>& moves);

	// Generates valid moves for pawns that have the possilbity of being promoted
	void GeneratePromotedPawnMoves(const ivec2& from, const ivec2& to, bool bCheck, std::vector<BoardMove>& moves);

	// Generates valid moves for bishops rooks and queens
	void GenerateDirectionMoves(Piece* pPiece, bool bCheck, std::vector<BoardMove>& moves);

	// Generates valid moves for knights and kings
	void GenerateDiscreteMoves(Piece* pPiece, bool bCheck, std::vector<BoardMove>& moves);

	// Generates valid castle moves
	void GenerateCastleMove(Piece* pPiece, bool bCheck, std::vector<BoardMove>& moves);

	// Adds a move to the move list only if after applying the move, it does not put us in check, or if bCheck is false
	void AddMove(const BoardMove& move, bool bCheck, std::vector<BoardMove>& moves);

	// Returns true if pos is on the board
	bool IsOnBoard(int pos) const;

	// Returns true if pos is on the board
	bool IsOnBoard(const ivec2& pos) const;

	// Returns true if the current tile is empty
	bool IsTileEmpty(int file, int rank) const;

	// Returns true if we currently own the tile
	bool IsTileOwner(int file, int rank) const;

	// Returns true if the current state of the board is in check
	bool IsInCheck(const BoardMove& move);

	// Returns true if the current state of the board is in check
	bool IsInCheck();

	// Clears the board
	void Clear();

private:

	std::vector<std::vector<Piece*>> m_board;
	ivec2 m_kingPos[2];
	BoardMove m_LastMove;
	int m_iPlayerID;
};

#endif // _BOARD_
