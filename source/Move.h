// -*-c++-*-

#ifndef MOVE_H
#define MOVE_H

#include <iostream>
#include "structures.h"


///A chess move
class Move {
  public:
  void* ptr;
  Move(_Move* ptr = NULL);

  // Accessors
  ///Unique Identifier
  int id() const;
  ///The initial file location
  int fromFile() const;
  ///The initial rank location
  int fromRank() const;
  ///The final file location
  int toFile() const;
  ///The final rank location
  int toRank() const;
  ///The type of the piece for pawn promotion. Q=Queen, B=Bishop, N=Knight, R=Rook
  int promoteType() const;

  // Actions

  // Properties


  friend std::ostream& operator<<(std::ostream& stream, Move ob);
};

#endif

