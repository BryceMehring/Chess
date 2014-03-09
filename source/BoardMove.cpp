#include "BoardMove.h"
#include "Piece.h"

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
	
	stream << char(move.pFrom->type()) << NumberToLetter(move.from.x) << move.from.y << spacing << NumberToLetter(move.to.x) << move.to.y << " ";

	switch(move.specialMove)
	{
	case SpecialMove::EnPassant:
		stream << "e.p.";
		break;
	case SpecialMove::Castle:
		stream << "0-0";
		if((move.to.x - move.from.x) < 0)
		{
			stream << "-0";
		}
		break;
	case SpecialMove::Promotion:
		stream << char(move.promotion);
	default:
		break;

	}

	
	stream << endl;

	return stream;
}
