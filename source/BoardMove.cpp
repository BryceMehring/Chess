#include "BoardMove.h"
#include "Board.h"

using std::endl;

static char NumberToLetter(int number)
{
	return char((number - 1) + 'a');
}

std::ostream& operator<<(std::ostream& stream, const BoardMove& move)
{
	char spacing = '-';
	if(move.pTo != nullptr)
	{
		spacing = 'x';
	}

	if(move.pFrom->type != 'P')
	{
		stream << char(move.pFrom->type);
	}

	if(move.specialMove != SpecialMove::Castle)
	{
		stream << NumberToLetter(move.from.x) << move.from.y << spacing << NumberToLetter(move.to.x) << move.to.y;

		if(move.specialMove == SpecialMove::EnPassant)
		{
			stream << " e.p.";
		}
		else if(move.specialMove == SpecialMove::Promotion)
		{
			stream << char(move.promotion);
		}
	}
	else
	{
		stream << "0-0";
		if((move.to.x - move.from.x) < 0)
		{
			stream << "-0";
		}
	}
	
	stream << endl;

	return stream;
}
