#include "AI.h"

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

#ifdef DEBUG_OUTPUT
	DrawBoard();
#endif

	Move* pPreviousMove = nullptr;
	if(!moves.empty())
	{
		pPreviousMove = &moves[0];
	}

	std::vector<BoardMove> userMoves = m_board.Update(playerID(), pPreviousMove, pieces);

	if(!userMoves.empty())
	{
		std::uniform_int_distribution<unsigned int> distribution(0,userMoves.size() - 1);
		unsigned int iRandomPiece = distribution(m_generator);

		Piece* pPiece = m_board.GetPiece(userMoves[iRandomPiece].from);

#ifdef DEBUG_OUTPUT

		// Display all moves for this piece:

		cout << "Valid Piece Moves: " << endl;
		for(const BoardMove& m : userMoves)
		{
			if(m.from == userMoves[iRandomPiece].from)
			{
				cout << m << endl;
			}
		}

#endif // DEBUG_OUTPUT

		pPiece->move(userMoves[iRandomPiece].to.x, userMoves[iRandomPiece].to.y, userMoves[iRandomPiece].promotion);
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
