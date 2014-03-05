#ifndef _BOARD_
#define _BOARD_

#include "Piece.h"
#include "Move.h"

#include <vector>

struct vec2
{
	vec2() : x(0), y(0) {}
	vec2(int x, int y) : x(x), y(y) {}

	int x;
	int y;
};

struct BoardMove
{
	vec2 from;
	vec2 to;
};

/*struct SimpleBoardMove
{
	SimpleBoardMove() : type(0) {}
	SimpleBoardMove(const vec2& from, const vec2& to, int type) : from(from), to(to), type(type) {}

	vec2 from;
	vec2 to;
	int type;
};*/

class ApplyMove
{
public:

	ApplyMove(const BoardMove* pMove, class Board* pBoard);
	~ApplyMove();

private:

	const BoardMove* m_pMove;
	Board* m_pBoard;

	vec2 m_oldKingPos;
	Piece* m_pOldDest;
	Piece* m_pMovingPiece;
};

class Board
{
public:

	friend class ApplyMove;

	// Constructs an empty board
	Board();

	// Updates the grid and returns all valid moves
	std::vector<BoardMove> Update(int playerID, const Move* pLastMove, std::vector<Piece>& pieces);

	Piece* GetPiece(const vec2& pos);

private:

	std::vector<BoardMove> GetMoves(bool bCheck = true);

	// Generate valid moves for pawns
	void GeneratePawnMoves(Piece* pPiece, bool bCheck, std::vector<BoardMove>& moves);

	// Generates valid moves for bishops rooks and queens
	void GenerateDirectionMoves(Piece* pPiece, bool bCheck, std::vector<BoardMove>& moves);

	// Generates valid moves for knights and kings
	void GenerateDiscreteMoves(Piece* pPiece, bool bCheck, std::vector<BoardMove>& moves);

	void AddMove(const BoardMove& move, bool bCheck, std::vector<BoardMove>& moves);

	// Returns true if pos is on the board
	bool IsOnBoard(int pos) const;

	// Returns true if pos is on the board
	bool IsOnBoard(const vec2& pos) const;

	// Returns true if the current tile is empty
	bool IsTileEmpty(int file, int rank) const;

	// Returns true if we currently own the tile
	bool IsTileOwner(int file, int rank) const;

	// Returns true if the current state of the board is in check
	bool IsInCheck(const BoardMove& move);

	// Clears the board
	void Clear();

	std::vector<std::vector<Piece*>> m_board;
	vec2 m_kingPos[2];
	BoardMove m_LastMove;
	//SimpleBoardMove m_LastMove;
	int m_iPlayerID;
};

#endif // _BOARD_