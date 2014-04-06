#include "AI.h"
#include "Timer.h"
#include "Heuristics.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <functional>

using std::cout;
using std::endl;
using namespace std::placeholders;

AI::AI(Connection* conn, unsigned int depth) : BaseAI(conn), m_totalTime(0), m_count(1),
	m_depth(depth), m_bInCheckmate(false) {}

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
	srand(time(0));
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

	m_board.Update(TurnsToStalemate(), moves, pieces);
	BoardMove bestMove;
	if(MiniMax(bestMove))
	{
		BoardPiece* pPiece = m_board.GetPiece(bestMove.from);
		assert(pPiece != nullptr);
		pPiece->piece.move(bestMove.to.x, bestMove.to.y, bestMove.promotion);

		// Spawn a new thread to think while the enemy is moving
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

bool AI::MiniMax(BoardMove& moveOut)
{
	bool bFoundMove = false;
	unsigned int d = 1;

	m_minimaxTimer.Reset();
	m_minimaxTimer.Start();

	m_bInCheckmate = false;
	m_bestIndex = 0;

	while((d <= m_depth) && ((m_minimaxTimer.GetTime()) < GetTimePerMove()) && (!m_bInCheckmate || (d != 2)))
	{
		bool bFoundAtDepth = MiniMax(d, playerID(), moveOut);
		if(bFoundAtDepth)
		{
#ifdef DEBUG_OUTPUT
			cout << "Depth " << d << " time: " << m_minimaxTimer.GetTime() << endl;
#endif
			bFoundMove = true;
		}

		++d;
	}

#ifdef DEBUG_OUTPUT
	if(bFoundMove)
	{
		cout << "Valid Moves: " << endl;
		for(auto iter : m_rootMoves)
		{
			if(iter.from == moveOut.from)
			{
				cout << iter << endl;
			}
		}
	}
#endif

	return bFoundMove;
}

bool AI::MiniMax(int depth, int playerID, BoardMove& moveOut)
{
	bool bFoundMove = false;

	m_rootMoves = m_board.GetMoves(playerID);
	if(!m_rootMoves.empty())
	{
		unsigned int index = 0;

		int a = std::numeric_limits<int>::min();
		int b = std::numeric_limits<int>::max();

		// Move the previous best move to the front of the list of moves
		assert(m_bestIndex < m_rootMoves.size());
		std::swap(m_rootMoves[0],m_rootMoves[m_bestIndex]);

		for(unsigned int i = 0; i < m_rootMoves.size(); ++i)
		{
			ApplyMove theMove(&m_rootMoves[i], &m_board);

			int val = MiniMax(depth - 1, playerID, a, b, -1);

			// If the new move is better than the last
			if(val > a)
			{
				index = i;
				a = val;
				bFoundMove = true;

#ifdef DEBUG_OUTPUT
				cout << val << endl;
#endif
			}

			// If we have ran out of time
			if(bFoundMove && ((m_minimaxTimer.GetTime()) >= GetTimePerMove()))
			{
				bFoundMove = false;
				break;
			}
		}

		if(bFoundMove)
		{
			moveOut = m_rootMoves[index];
			m_bestIndex = index;
		}
	}

	return bFoundMove;

}

int AI::MiniMax(int depth, int playerID, int a, int b, int color)
{
	if(m_board.IsInCheckmate(!playerID))
	{
		m_bInCheckmate = true;
		return 1000000;
	}

	if(m_board.IsInStalemate(!playerID))
		return -500;

	if(depth <= 0)
		return m_board.GetWorth(playerID, ChessHeuristic());

	std::vector<BoardMove> userMoves =  m_board.GetMoves(color == 1 ? playerID : !playerID);
	auto firstIter = std::partition(userMoves.begin(), userMoves.end(),[&](const BoardMove& a) -> bool
	{
		return (a.capturedType != 0);
	});

	std::random_shuffle(firstIter, userMoves.end());

	for(unsigned int i = 0; i < userMoves.size(); ++i)
	{
		ApplyMove theMove(&userMoves[i], &m_board);
		int score = MiniMax(depth - 1, playerID, a, b, -color);

		if(color == 1)
		{
			if(score >= b)
				return b;

			if(score > a)
				a = score;
		}
		else
		{
			if(score <= a)
				return a;

			if(score < b)
				b = score;
		}
	}

	if(color == 1)
		return a;

	return b;
}

std::uint64_t AI::GetTimePerMove()
{
	std::uint64_t time = (std::uint64_t)players[playerID()].time() / 50 * 1000000000;

	// If the game is almost going to time out,
	// do not search as deep
	if(time < 1)
	{
		m_depth = 4;
		time = std::numeric_limits<std::uint64_t>::max();
	}

	return time;
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
