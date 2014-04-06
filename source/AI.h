#ifndef AI_H
#define AI_H

#include "BaseAI.h"
#include "Board.h"
#include "Timer.h"
#include <iostream>
#include <random>

enum class TranspositionTableFlag
{
	UPPERBOUND,
	LOWERBOUND,
	EXACT
};

struct TranspositionTableEntry
{
	float value;
	unsigned int depth;
	TranspositionTableFlag flag;
};

///The class implementing gameplay logic.
class AI: public BaseAI
{
public:
  AI(Connection* c, unsigned int depth);
  virtual const char* username();
  virtual const char* password();
  virtual void init();
  virtual bool run();
  virtual void end();

private:

  // Minimax algorithm with alpha beta pruning
  // Returns the best possible move at the current depth limit
  void MiniMax(BoardMove& moveOut);
  bool MiniMax(int depth, int playerID, bool bEnableTime, BoardMove& moveOut);
  int MiniMax(int depth, int playerID, int a, int b, int color);

  // Returns the amount of time that the AI has per turn
  std::uint64_t GetTimePerMove();

  // Draws the chess board to standard output
  void DrawBoard() const;

  Board m_board;
  std::uint64_t m_totalTime;
  unsigned int m_count;
  unsigned int m_depth;
  unsigned int m_bestIndex;
  bool m_bInCheckmate;

  std::vector<BoardMove> m_rootMoves;

 //std::unordered_map<std::vector<std::vector<int>>, TranspositionTableEntry, BoardMoveHash> m_transpositionTable;

  //std::vector<unsigned int> m_bestMoves;
  //std::vector<unsigned int> m_bestUsableMoves;

  Timer m_minimaxTimer;
};

#endif
