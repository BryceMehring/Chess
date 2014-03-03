#ifndef _BOARD_
#define _BOARD_

#include "Piece.h"
#include "Move.h"

#include <vector>

struct vec2
{
	int x;
	int y;
};

struct BoardMove
{
	Piece* pPiece;
	vec2 move;
};

class Board
{
public:

	Board();

	void Update(int playerID, std::vector<Piece>& pieces);

	std::vector<BoardMove> GetMoves() const;

private:

	void GeneratePawnMoves(Piece* pPiece, std::vector<BoardMove>& moves) const;
	void GenerateDirectionMoves(Piece* pPiece, std::vector<BoardMove>& moves) const;
	void GenerateDiscreteMoves(Piece* pPiece, std::vector<BoardMove>& moves) const;

	bool IsOnGrid(int coord) const;

	bool IsTileEmpty(int file, int rank) const;

	bool IsTileOwner(int file, int rank) const;

	void Clear();

	std::vector<std::vector<Piece*>> m_board;
	int m_iPlayerID;

};

#endif // _BOARD_
