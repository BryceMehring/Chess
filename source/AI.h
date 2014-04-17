#ifndef AI_H
#define AI_H

#include "BaseAI.h"
#include "Board.h"
#include "Timer.h"
#include <random>
#include <array>
#include <queue>

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

//template < class T, unsigned int A, unsigned int B, unsigned int C >
//using array3d = std::array<std::array<std::array<int,C>,B>,A>;

class HistoryFunctor;

///The class implementing gameplay logic.
class AI: public BaseAI
{
public:

  typedef std::array<std::array<std::array<int,64>,64>,2> HISTORY_ARRAY_TYPE;
  //using HISTORY_ARRAY_TYPE = array3d<int,2,64,64>;
  //typedef std::priority_queue<BoardMove, std::vector<BoardMove>, HistoryFunctor> FRONTIER_TYPE;
  typedef std::vector<BoardMove> FRONTIER_TYPE;
  //using FRONTIER_TYPE = std::priority_queue<BoardMove, std::vector<BoardMove>, HistoryFunctor>;

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
  int MiniMax(int depth, int playerID, int playerIDToMove, int a, int b);

  // Returns the frontier nodes for the current player to move as a priority queue
  FRONTIER_TYPE MoveOrdering(int playerIDToMove);

  // Returns the amount of time that the AI has per turn
  std::uint64_t GetTimePerMove();

  // Draws the chess board to standard output
  void DrawBoard() const;

  Board m_board;
  std::uint64_t m_totalTime;
  unsigned int m_count;
  unsigned int m_depth;
  bool m_bInCheckmate;

  HISTORY_ARRAY_TYPE m_history;

  Timer m_minimaxTimer;

  std::default_random_engine m_randEngine;
};

#endif
