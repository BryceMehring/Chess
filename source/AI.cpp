#include "AI.h"
#include "Timer.h"
#include "Heuristics.h"

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <functional>

using std::cout;
using std::endl;
using namespace std::placeholders;

AI::AI(Connection* conn, unsigned int depth) : BaseAI(conn), m_totalTime(0), m_count(1), m_depth(depth) {}

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
	Timer timer;
	timer.Start();
#endif

#ifdef DEBUG_OUTPUT
	DrawBoard();
#endif

	Move* pPreviousMove = nullptr;
	if(!moves.empty())
	{
		pPreviousMove = &moves[0];
	}

	m_board.Update(pPreviousMove, pieces);
	std::vector<BoardMove> userMoves = m_board.GetMoves(playerID());

	if(!userMoves.empty())
	{
		unsigned int index = MiniMax();

		Piece* pPiece = &m_board.GetPiece(userMoves[index].from)->piece;

#ifdef DEBUG_OUTPUT

		// Display all moves for this piece:

		cout << "Valid Piece Moves: " << endl;
		for(const BoardMove& m : userMoves)
		{
			if(m.from == userMoves[index].from)
			{
				cout << m << endl;
			}
		}

#endif // DEBUG_OUTPUT
		pPiece->move(userMoves[index].to.x, userMoves[index].to.y, userMoves[index].promotion);
	}

#ifdef DEBUG_OUTPUT
	m_totalTime += timer.GetTime();
	cout << "Average Time Spent: " << m_totalTime / m_count << endl;
	cout << "Total Time Spent: " << m_totalTime << endl;
	cout << "Servers time: " << 900 - players[playerID()].time() << endl;

	m_count++;
#endif

	return true;
}

//This function is run once, after your last turn.
void AI::end()
{

}

unsigned int AI::MiniMax()
{
	unsigned int index = -1;
	cout << "Minimax woth " << endl;
	for(unsigned int i = 1; i <= m_depth; ++i)
	{
		float worth = MiniMax(i,0, playerID(),true,index);

		cout << "Depth " << m_depth << " ";
		cout << worth << endl;
	}

	return index;
}

float AI::MiniMax(int depth, float worth, int playerID, bool bMax, unsigned int& index)
{
	if(depth <= 0)
		return worth;

	float value = bMax ? -FLT_MAX : FLT_MAX;
	std::vector<BoardMove> userMoves =  m_board.GetMoves(bMax ? playerID : !playerID);
	for(unsigned int i = 0; i < userMoves.size(); ++i)
	{
		ApplyMove theMove(&userMoves[i], &m_board);

		unsigned int unusedIndex;
		float fMiniMaxValue = MiniMax(depth - 1, m_board.GetWorth(playerID, ChessHeuristic()), playerID, !bMax, unusedIndex);

		if(bMax)
		{
			if(fMiniMaxValue > value)
			{
				index = i;
				value = fMiniMaxValue;
			}
		}
		else
		{
			if(fMiniMaxValue < value)
			{
				index = i;
				value = fMiniMaxValue;
			}
		}
	}

	return value;
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
