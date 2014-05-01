#ifndef AI_H
#define AI_H

#include "BaseAI.h"
#include "Board.h"
#include "Timer.h"
#include <random>
#include <array>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <future>

enum class TranspositionTableFlag
{
	UPPERBOUND,
	LOWERBOUND,
	EXACT
};

struct TranspositionTableEntry
{
	int value;
	int depth;
	TranspositionTableFlag flag;
};

class HistoryFunctor;

///The class implementing gameplay logic.
class AI: public BaseAI
{
public:

  typedef std::array<std::array<std::array<int,64>,64>,2> HISTORY_ARRAY_TYPE;
  typedef std::vector<BoardMove> FRONTIER_TYPE;

  AI(Connection* c, unsigned int depth);
  virtual const char* username();
  virtual const char* password();
  virtual void init();
  virtual bool run();
  virtual void end();

private:

  void WaitForFuture(const std::future<void>& fut);

  // Finds the best move from minimax with alpha beta pruning, Quiescence Search, and History Table
  bool MiniMax(int playerID, bool bCutTime, BoardMove& moveOut);
  bool MiniMax(int depth, int playerID, BoardMove& moveOut, bool bEnableCutoff);
  int MiniMax(int depth, int playerID, int playerIDToMove, int a, int b, bool bEnableCutoff);

  // Returns the frontier nodes for the current player to move sorted from high to low based on the history table
  FRONTIER_TYPE MoveOrdering(int playerIDToMove);

  // Returns the amount of time that the AI has per turn
  std::uint64_t GetTimePerMove();
  
  void ClearHistory();

  // Draws the chess board to standard output
  void DrawBoard() const;

private:

  Board m_board;
  std::uint64_t m_totalTime;
  unsigned int m_count;
  unsigned int m_depth;
  bool m_bInCheckmate;

  std::mutex m_bestMoveMutex;
  std::atomic_bool m_bStopMinimax;
  BoardMove m_bestMove;
  BoardMove m_opponentBestMove;
  bool m_bFoundOpponentMove;
  std::future<void> m_ponderingFuture;

  HISTORY_ARRAY_TYPE m_history;

  Timer m_minimaxTimer;

  std::default_random_engine m_randEngine;
};

#endif
