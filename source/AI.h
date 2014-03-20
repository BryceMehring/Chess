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
  AI(Connection* c);
  virtual const char* username();
  virtual const char* password();
  virtual void init();
  virtual bool run();
  virtual void end();

private:

  unsigned int MiniMax();
  float MiniMax(int depth, float worth, int playerID, bool bMax, unsigned int& index);
  void DrawBoard() const;

  Board m_board;
  std::random_device m_generator;
  std::uint64_t m_totalTime;
  unsigned int m_count;
};

#endif
