#ifndef AI_H
#define AI_H

#include "BaseAI.h"
#include <iostream>
#include <random>
#include <unordered_map>

using namespace std;

struct vec2
{
	int x; // rank
	int y; // file
};

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

  void DrawBoard() const;

  void BuildGrid();
  void ClearGrid();
  bool IsOnGrid(int coord) const;
  bool IsTileEmpty(int file, int rank) const;
  bool IsTileOwner(int file, int rank) const;

  // todo: replace this with a hash table of some sort
  std::unordered_map<int,Piece*> GetUserPieces(char type);
  std::vector<vec2> GetPieceMoves(const Piece* pPiece);

  std::vector<std::vector<Piece*>> m_grid;
  std::default_random_engine m_generator;
};

#endif
