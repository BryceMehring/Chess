#include "AI.h"
#include "Timer.h"
#include "Heuristics.h"

#include <algorithm>
#include <limits>
#include <functional>
#include <cstring>
#include <cassert>

using std::cout;
using std::endl;
using namespace std::placeholders;

static unsigned int GetHistoryTableIndex(const ivec2& pos)
{
	return (8*(pos.x - 1) + (pos.y - 1));
}

AI::AI(Connection* conn, unsigned int depth) : BaseAI(conn), m_totalTime(0), m_count(1),
	m_depth(depth), m_bInCheckmate(false), m_randEngine(std::chrono::system_clock::now().time_since_epoch().count()) {}

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

	m_board.Update(TurnsToStalemate(), moves, pieces);

	// Find the best move using Minimax
	BoardMove bestMove;
	MiniMax(bestMove);

	// Get the piece to move and move the piece
	BoardPiece* pPiece = m_board.GetPiece(bestMove.from);
	assert(pPiece != nullptr);
	pPiece->piece.move(bestMove.to.x, bestMove.to.y, bestMove.promotion);

#ifdef DEBUG_OUTPUT
	m_totalTime += timer.GetTime();
	cout << "Average Time: " << m_totalTime / m_count << endl;
	cout << "Server time: " << 900 - players[playerID()].time() << endl;
	cout << "Move Cache Collision ratio: " << m_board.GetLoadFactor() << endl;
	cout << "Move Cache Hit ratio: " << m_board.GetMoveCacheHitRatio() << endl;
	cout << "Hash table size: " << m_board.GetHashTableSize() << endl;

	m_count++;
#endif

	return true;
}

//This function is run once, after your last turn.
void AI::end()
{
}

void AI::MiniMax(BoardMove& moveOut)
{
	unsigned int d = 1;
	bool bEnableTimer = false;

	m_minimaxTimer.Reset();
	m_minimaxTimer.Start();

	m_bInCheckmate = false;

	std::memset(m_history.data(),0,sizeof(m_history));

	while((!bEnableTimer || (m_minimaxTimer.GetTime() < GetTimePerMove())) && (d <= m_depth) && (!m_bInCheckmate || (d != 2)))
	{
		bool bFoundAtDepth = MiniMax(d, playerID(), bEnableTimer, moveOut);
		if(bFoundAtDepth)
		{
#ifdef DEBUG_OUTPUT
			cout << "Depth " << d << " time: " << m_minimaxTimer.GetTime() << endl;
#endif
			bEnableTimer = true;
		}
		else
		{
#ifdef DEBUG_OUTPUT
			cout << "No move was found at depth " << d << endl;
			break;
#endif
		}

		++d;
	}
}

bool AI::MiniMax(int depth, int playerID, bool bEnableTimer, BoardMove& moveOut)
{
	bool bFoundMove = false;
	BoardMove bestMove;

	int alpha = std::numeric_limits<int>::min() + 1;
	int beta = std::numeric_limits<int>::max();

	// Build a priority queue of the frontier nodes
	FRONTIER_TYPE frontier = MoveOrdering(playerID);

	for(const BoardMove& currentMove : frontier)
	{
		ApplyMove theMove(currentMove, &m_board);

		int val = MiniMax(depth - 1, playerID, !playerID, alpha, beta, bEnableTimer);

		// If the new move is better than the last
		if(val > alpha)
		{
			alpha = val;
			bestMove = currentMove;
			bFoundMove = true;

#ifdef DEBUG_OUTPUT
			cout << val << endl;
#endif
		}

		if(bEnableTimer)
		{
			// If we have ran out of time
			if((m_minimaxTimer.GetTime()) >= GetTimePerMove())
			{
				bFoundMove = false;
				break;
			}
		}
	}

	if(bFoundMove)
	{
		m_history[playerID][GetHistoryTableIndex(bestMove.from)][GetHistoryTableIndex(bestMove.to)] += (depth * depth) + 1;
		moveOut = bestMove;
	}

	return bFoundMove;
}

int AI::MiniMax(int depth, int playerID, int playerIDToMove, int alpha, int beta, bool bEnableTimer)
{
	if(bEnableTimer)
	{
		// If we have ran out of time
		if((m_minimaxTimer.GetTime()) >= GetTimePerMove())
			return 0;
	}

	// If a checkmate has been found, return a large number
	if(m_board.IsInCheckmate(!playerID))
	{
		m_bInCheckmate = true;
		return 1000000;
	}

	// If a stalemate is found, return 0 which is neutral for both sides
	if(m_board.IsInStalemate(!playerID))
		return 0;

	// If this is a leaf node
	if(depth <= 0)
	{
		// Initiate quiescent search

		// Get the heuristic value of the node
		int stand_pat = m_board.GetWorth(playerID, ChessHeuristic());

		if(depth <= -1)
			return stand_pat;

		if(playerID == playerIDToMove)
		{
			if(stand_pat >= beta)
			{
				return stand_pat;
			}

			// See if we can do better than alpha
			if(stand_pat > alpha)
			{
				alpha = stand_pat;
			}
		}
		else
		{
			if(stand_pat <= alpha)
			{
				return stand_pat;
			}
			// See if we can do better than beta
			if(stand_pat < beta)
			{
				beta = stand_pat;
			}
		}
	}

	// Build a priority queue of the frontier nodes
	FRONTIER_TYPE frontier = MoveOrdering(playerIDToMove);

	BoardMove bestMove;
	bool bFoundBestMove = false;

	for(const BoardMove& currentMove : frontier)
	{
		// If we are applying Quiescence Search, only look at attacking moves
		if(depth <= 0)
		{
			if((currentMove.capturedType == 0) && (currentMove.specialMove != SpecialMove::None))
			{
				continue;
			}
		}

		// Apply the move in the queue with the higest priority
		ApplyMove theMove(currentMove, &m_board);
		int score = MiniMax(depth - 1, playerID, !playerIDToMove, alpha, beta, bEnableTimer);

		if(playerID == playerIDToMove)
		{
			if(score >= beta)
			{
				m_history[playerIDToMove][GetHistoryTableIndex(currentMove.from)][GetHistoryTableIndex(currentMove.to)] += (depth * depth) + 1;
				return score;
			}

			if(score > alpha)
			{
				bFoundBestMove = true;
				alpha = score;
				bestMove = currentMove;
			}
		}
		else
		{
			if(score <= alpha)
			{
				m_history[playerIDToMove][GetHistoryTableIndex(currentMove.from)][GetHistoryTableIndex(currentMove.to)] += (depth * depth) + 1;
				return score;
			}

			if(score < beta)
			{
				bFoundBestMove = true;
				beta = score;
				bestMove = currentMove;
			}
		}
	}

	if(bFoundBestMove)
	{
		m_history[playerIDToMove][GetHistoryTableIndex(bestMove.from)][GetHistoryTableIndex(bestMove.to)] += (depth * depth) + 1;
	}

	if(playerID == playerIDToMove)
		return alpha;

	return beta;
}

AI::FRONTIER_TYPE AI::MoveOrdering(int playerIDToMove)
{
	std::vector<BoardMove> moves = m_board.GetMoves(playerIDToMove);
	std::shuffle(moves.begin(), moves.end(), m_randEngine);

	std::sort(moves.begin(), moves.end(), [&](const BoardMove& a, const BoardMove& b) -> bool
	{
		return (m_history[playerIDToMove][GetHistoryTableIndex(a.from)][GetHistoryTableIndex(a.to)]) >
			   (m_history[playerIDToMove][GetHistoryTableIndex(b.from)][GetHistoryTableIndex(b.to)]);
	});

	return std::move(moves);
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
