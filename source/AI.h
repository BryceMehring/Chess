#ifndef AI_H
#define AI_H

#include "BaseAI.h"
#include "Board.h"
#include <iostream>
#include <random>

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

  // Minimax algorithm:
  // Returns the best possible move at the current depth limit
  unsigned int MiniMax();
  float MiniMax(int depth, int playerID, bool bMax, unsigned int& index);

  // Draws the chess board to standard output
  void DrawBoard() const;

  Board m_board;
  std::uint64_t m_totalTime;
  unsigned int m_count;
  unsigned int m_depth;
};

#endif
