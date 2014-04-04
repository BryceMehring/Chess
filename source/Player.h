// -*-c++-*-

#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>
#include "structures.h"


class Player {
  public:
  void* ptr;
  Player(_Player* ptr = NULL);

  // Accessors
  ///Unique Identifier
  int id() const;
  ///Player's Name
  char* playerName() const;
  ///Time remaining, updated at start of turn
  float time() const;

  // Actions

  // Properties


  friend std::ostream& operator<<(std::ostream& stream, Player ob);
};

#endif

