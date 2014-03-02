#include "AI.h"
#include "util.h"

#include <algorithm>

AI::AI(Connection* conn) : BaseAI(conn) {}

const char* AI::username()
{
	return "Shell AI";
}

const char* AI::password()
{
	return "password";
}

//This function is run once, before your first turn.
void AI::init()
{
	srand(time(NULL));
}

//This function is called each time it is your turn.
//Return true to end your turn, return false to ask the server for updated information.
bool AI::run()
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

	// if there has been a move, print the most recent move
	if(moves.size() > 0)
	{
		cout<<"Last Move Was: "<<endl<<moves[0]<<endl;
	}

	std::vector<int> userPieces = GetUserPieces();

	if(!userPieces.empty())
	{
		unsigned int iRandomPiece = rand() % userPieces.size();

		cout << pieces[userPieces[iRandomPiece]].id() << endl;

		cout << playerID() << endl;

		// X = Files
		// Y = Rank
		pieces[userPieces[iRandomPiece]].move(pieces[userPieces[iRandomPiece]].file(), pieces[userPieces[iRandomPiece]].rank() + ((playerID() == 0) ? 1 : -1), int('Q'));
	}

	return true;
}

//This function is run once, after your last turn.
void AI::end()
{

}

std::vector<int> AI::GetUserPieces() const
{
	std::vector<int> userPieces;

	int iPlayerID = playerID();

	for(const Piece& p : pieces)
	{
		if(iPlayerID == p.owner() && p.type() == int('P'))
		{
			userPieces.push_back(p.id() - 2);
		}
	}

	return userPieces;
}











