#include "AI.h"
#include "util.h"

#include <algorithm>
#include <cassert>

using std::cout;
using std::endl;

AI::AI(Connection* conn) : BaseAI(conn) {}

const char* AI::username()
{
	return "Cole Xemi";
}

const char* AI::password()
{
	return "password";
}

//This function is run once, before your first turn.
void AI::init()
{
}

//This function is called each time it is your turn.
//Return true to end your turn, return false to ask the server for updated information.
bool AI::run()
{
	DrawBoard();

	std::vector<BoardMove> userMoves = m_board.Update(playerID(),pieces);

	if(!userMoves.empty())
	{
		std::uniform_int_distribution<unsigned int> distribution(0,userMoves.size() - 1);
		unsigned int iRandomPiece = distribution(m_generator);

		cout << *userMoves[iRandomPiece].pPiece << endl;
		cout << "(" << userMoves[iRandomPiece].move.x <<","<<userMoves[iRandomPiece].move.y<<")"<<endl;
		cout << userMoves.size() << endl;

		userMoves[iRandomPiece].pPiece->move(userMoves[iRandomPiece].move.x, userMoves[iRandomPiece].move.y, 'Q');
	}

	return true;
}

//This function is run once, after your last turn.
void AI::end()
{

}

void AI::DrawBoard() const
{
	// Print out the current board state
	cout<<"+---+---+---+---+---+---+---+---+"<<endl;
	for(int rank=8; rank>0; rank--)
	{
		cout<<"|";
		for(int file=1; file<=8; file++)
		{
			bool found = false;
			// Loops through all of the pieces
			for(unsigned int p=0; !found && p<pieces.size(); p++)
			{
				// determines if that piece is at the current rank and file
				if(pieces[p].rank() == rank && pieces[p].file() == file)
				{
					found = true;
					// Checks if the piece is black
					if(pieces[p].owner() == 1)
					{
						cout<<"*";
					}
					else
					{
						cout<<" ";
					}
					// prints the piece's type
					cout<<(char)pieces[p].type()<<" ";
				}
			}

			if(!found)
			{
				cout<<"   ";
			}
			cout<<"|";
		}
		cout<<endl<<"+---+---+---+---+---+---+---+---+"<<endl;
	}
}
