// -*-c++-*-

#include "Move.h"
#include "game.h"


Move::Move(_Move* pointer)
{
    ptr = (void*) pointer;
}

int Move::id() const
{
  return ((_Move*)ptr)->id;
}

int Move::fromFile() const
{
  return ((_Move*)ptr)->fromFile;
}

int Move::fromRank() const
{
  return ((_Move*)ptr)->fromRank;
}

int Move::toFile() const
{
  return ((_Move*)ptr)->toFile;
}

int Move::toRank() const
{
  return ((_Move*)ptr)->toRank;
}

int Move::promoteType() const
{
  return ((_Move*)ptr)->promoteType;
}




std::ostream& operator<<(std::ostream& stream,Move ob)
{
  stream << "id: " << ((_Move*)ob.ptr)->id  <<'\n';
  stream << "fromFile: " << ((_Move*)ob.ptr)->fromFile  <<'\n';
  stream << "fromRank: " << ((_Move*)ob.ptr)->fromRank  <<'\n';
  stream << "toFile: " << ((_Move*)ob.ptr)->toFile  <<'\n';
  stream << "toRank: " << ((_Move*)ob.ptr)->toRank  <<'\n';
  stream << "promoteType: " << ((_Move*)ob.ptr)->promoteType  <<'\n';
  return stream;
}
