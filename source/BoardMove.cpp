#include "BoardMove.h"

using std::endl;

static char NumberToLetter(int number)
{
	return char((number - 1) + 'a');
}

std::ostream& operator<<(std::ostream& stream, const BoardMove& move)
{
	stream << NumberToLetter(move.from.x) << move.from.y << " " << NumberToLetter(move.to.x) << move.to.y << endl;

	if(move.specialMove != SpecialMove::None)
	{
		const char* SpecialMoveStrings[] =
		{
			"EnPassant", "Castle", "Promotion", "Capture"
		};

		stream << "Special Move: " << SpecialMoveStrings[(int)move.specialMove] << endl;

		if(move.specialMove == SpecialMove::Promotion)
		{
			stream << "Promotion Type: " << char(move.promotion) << endl;
		}
	}

	return stream;
}
