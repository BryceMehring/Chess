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
	m_depth(depth), m_bInCheckmate(false), m_bStopMinimax(false), m_bExit(false), m_randEngine(std::chrono::system_clock::now().time_since_epoch().count()) {}

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
	auto ponderMethod = [this]()
	{
		while(true)
		{
			std::unique_lock<std::mutex> lck(m_mutex);
			m_cv.wait(lck);
			
			if(m_bExit)
				break;

			BoardMove move;

			ApplyMove theMove(m_bestMove, &m_board);
			MiniMax(!playerID(), true, move);
		}
	};

	m_ponderThread = std::thread(ponderMethod);
}

//This function is called each time it is your turn.
//Return true to end your turn, return false to ask the server for updated information.
bool AI::run()
{
	Timer waitTimer;
	waitTimer.Start();

	m_bStopMinimax = true;
	std::unique_lock<std::mutex> lck(m_mutex);
	m_bStopMinimax = false;

	cout << "Waiting Time: " << waitTimer.GetTime() << endl;

#ifdef DEBUG_OUTPUT
	Timer timer;
	timer.Start();
#endif

#ifdef DEBUG_OUTPUT
	DrawBoard();
#endif

	m_board.Update(TurnsToStalemate(), moves, pieces);

	// Find the best move using Minimax
	MiniMax(playerID(), false, m_bestMove);

	// Get the piece to move and move the piece
	BoardPiece* pPiece = m_board.GetPiece(m_bestMove.from);
	assert(pPiece != nullptr);
	pPiece->piece.move(m_bestMove.to.x, m_bestMove.to.y, m_bestMove.promotion);

	ClearHistory();
	
	m_cv.notify_one();

#ifdef DEBUG_OUTPUT
	m_totalTime += timer.GetTime();
	cout << "Average Time: " << m_totalTime / m_count << endl;
	cout << "Server time: " << 900 - players[playerID()].time() << endl;
	m_count++;
#endif

	return true;
}

//This function is run once, after your last turn.
void AI::end()
{
	if(m_ponderThread.joinable())
	{
		m_bExit = true;
		std::unique_lock<std::mutex> lck(m_mutex);
		
		m_cv.notify_one();
	}
	
	if(m_ponderThread.joinable())
		m_ponderThread.join();
}

void AI::MiniMax(int playerID, bool bPonder, BoardMove& moveOut)
{
	unsigned int d = 1;
	bool bEnableTimer = false;

	m_minimaxTimer.Reset();
	m_minimaxTimer.Start();

	m_bInCheckmate = false;

	while((d <= m_depth) && (!m_bInCheckmate || (d != 2)))
	{
		if(!bPonder && bEnableTimer && (m_minimaxTimer.GetTime() >= GetTimePerMove()))
		{
			break;
		}
		
		if(bPonder && (m_bStopMinimax || m_bExit))
			break;

		bool bFoundAtDepth = MiniMax(d, playerID, bPonder, bEnableTimer, moveOut);
		
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

bool AI::MiniMax(int depth, int playerID, bool bPonder, bool bEnableTimer, BoardMove& moveOut)
{
	bool bFoundMove = false;
	BoardMove bestMove;

	int alpha = std::numeric_limits<int>::min() + 1;
	int beta = std::numeric_limits<int>::max();

	// Build a priority queue of the frontier nodes
	FRONTIER_TYPE frontier = MoveOrdering(playerID);

	for(const BoardMove& currentMove : frontier)
	{
		if(bPonder && (m_bStopMinimax || m_bExit))
			break;
		
		ApplyMove theMove(currentMove, &m_board);

		int val = MiniMax(depth - 1, playerID, !playerID, alpha, beta, bPonder, bEnableTimer);

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

		if(bEnableTimer && !bPonder)
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

int AI::MiniMax(int depth, int playerID, int playerIDToMove, int alpha, int beta, bool bPonder, bool bEnableTimer)
{
	if(bPonder && (m_bStopMinimax || m_bExit))
		return 0;
	
#ifdef STRICT_DEADLINE
	if(bEnableTimer && !bPonder)
	{
		// If we have ran out of time
		if((m_minimaxTimer.GetTime()) >= GetTimePerMove())
			return 0;
	}
#endif

	/*int alphaOrig = alpha;

	auto iter = this->m_transpositionTable.find(m_board.GetState());
	if(iter != m_transpositionTable.end())
	{
		const TranspositionTableEntry& ttEntry = iter->second;
		if(ttEntry.depth >= depth)
		{
			if(ttEntry.flag == TranspositionTableFlag::EXACT)
				return ttEntry.value;
			if(ttEntry.flag == TranspositionTableFlag::LOWERBOUND)
				alpha = std::max(alpha, ttEntry.value);
			else if(ttEntry.flag == TranspositionTableFlag::UPPERBOUND)
				beta = std::min(beta, ttEntry.value);

			if(alpha >= beta)
				return ttEntry.value;
		}
	}*/

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
		
		if(depth <= -2)
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
			if((currentMove.capturedType == 0) && (currentMove.specialMove != SpecialMove::Promotion))
			{
				continue;
			}
		}

		// Apply the move in the queue with the higest priority
		ApplyMove theMove(currentMove, &m_board);
		int score = MiniMax(depth - 1, playerID, !playerIDToMove, alpha, beta, bPonder, bEnableTimer);

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

	/*TranspositionTableEntry ttEntry;
	ttEntry.value = (playerID == playerIDToMove) ? alpha : beta;

	if(ttEntry.value <= alphaOrig)
	{
		ttEntry.flag = TranspositionTableFlag::UPPERBOUND;
	}
	else if(ttEntry.value >= beta)
	{
		ttEntry.flag = TranspositionTableFlag::LOWERBOUND;
	}
	else
	{
		ttEntry.flag = TranspositionTableFlag::EXACT;
	}
	ttEntry.depth = depth;

	if(iter != m_transpositionTable.end())
	{
		if(ttEntry.depth > iter->second.depth)
		{
			iter->second = ttEntry;
		}
	}
	else
	{
		m_transpositionTable[m_board.GetState()] = ttEntry;
	}*/
	
	return (playerID == playerIDToMove) ? alpha : beta;
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

void AI::ClearHistory()
{
	std::memset(m_history.data(),0,sizeof(m_history));
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
