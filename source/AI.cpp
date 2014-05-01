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
	m_depth(depth), m_bInCheckmate(false), m_bStopMinimax(false), m_bFoundOpponentMove(false),
	m_randEngine(std::chrono::system_clock::now().time_since_epoch().count()) {}

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
	bool bRestartMinimax = true;

	Timer waitTimer;
	waitTimer.Start();
	
	// Check to see whether or not the last move that was made matched the predicted move from minimax
	// Todo: need to use mutexes so that only one thread can access this data
	if(!moves.empty() && m_ponderingFuture.valid())
	{
		const Move& lastMove = moves[0];
		
		m_bestMoveMutex.lock();
		bool bFoundValidPonderMove = (m_bFoundOpponentMove && (m_opponentBestMove.from == ivec2{lastMove.fromFile(), lastMove.fromRank()}) && 
									 (m_opponentBestMove.to == ivec2{lastMove.toFile(), lastMove.toRank()}));
		m_bestMoveMutex.unlock();
		
		if(bFoundValidPonderMove)
		{
			// The ponder thread can finish executing
			cout << "Ponder Hit" << endl;
			bRestartMinimax = false;
		}
		else
		{
			// Signal the thread to finish
			cout << "Ponder miss" << endl;
			m_bStopMinimax = true;
		}
		
		WaitForFuture(m_ponderingFuture, true);
		
		m_bFoundOpponentMove = false;
	}
	
	cout << "Waiting Time: " << waitTimer.GetTime() << endl;

#ifdef DEBUG_OUTPUT
	Timer timer;
	timer.Start();
#endif

#ifdef DEBUG_OUTPUT
	DrawBoard();
#endif

	m_board.Update(TurnsToStalemate(), moves, pieces);

	if(bRestartMinimax)
	{
		m_bStopMinimax = false;
		auto minimaxFuture = std::async(std::launch::async, [this]()
		{
			cout << "Normal" << endl;
			
			// Find the best move using Minimax
			MiniMax(playerID(), false, m_bestMove);
		});
		
		WaitForFuture(minimaxFuture);
	}

	// Get the piece to move and move the piece
	BoardPiece* pPiece = m_board.GetPiece(m_bestMove.from);
	assert(pPiece != nullptr);
	pPiece->piece.move(m_bestMove.to.x, m_bestMove.to.y, m_bestMove.promotion);

#ifdef DEBUG_OUTPUT
	m_totalTime += timer.GetTime();
	cout << "Average Time: " << m_totalTime / m_count << endl;
	cout << "Server time: " << 900 - players[playerID()].time() << endl;
	m_count++;
#endif

	m_bStopMinimax = false;
	m_ponderingFuture = std::async(std::launch::async, [this]()
	{
#ifdef DEBUG_OUTPUT
		cout << "Pondering" << endl;
#endif
		
		BoardMove predictedOpponentMove;
		ApplyMove theirMove(m_bestMove, &m_board);
		if(MiniMax(!playerID(), true, predictedOpponentMove))
		{
			if(!m_bStopMinimax)
			{
				m_bestMoveMutex.lock();
				m_opponentBestMove = predictedOpponentMove;
				m_bFoundOpponentMove = true;
				m_bestMoveMutex.unlock();
				
				// If their last move is the same as the move I have found
				// Then continue to search
				// Else signal the thread to exit
#ifdef DEBUG_OUTPUT
				cout << endl;
#endif
				
				BoardMove myBestMove;
				ApplyMove myMove(predictedOpponentMove, &m_board);
				if(MiniMax(playerID(), false, myBestMove))
				{
					cout << "Found ponder move" << endl;
					m_bestMove = myBestMove;
				}
			}
		}
		
	});

	return true;
}

//This function is run once, after your last turn.
void AI::end()
{
	m_bStopMinimax = true;
}

void AI::WaitForFuture(const std::future<void>& fut, bool bPondering)
{
	std::uint64_t timePerMove = GetTimePerMove();
	if(bPondering)
	{
		timePerMove /= 2;
	}

	// note: this is a hack as wait_for does not return a boolean in the standard.
	if(fut.wait_for(std::chrono::nanoseconds(timePerMove)) == false)
	{
		m_bStopMinimax = true;
		fut.wait();
		m_bStopMinimax = false;
	}
}

bool AI::MiniMax(int playerID, bool bCutDepth, BoardMove& moveOut)
{
	unsigned int d = 1;
	unsigned int depthLimit = (bCutDepth ? 3 : m_depth);
	bool bEnableCutoff = false;
	bool bFoundMove = false;

	m_bInCheckmate = false;
	
	Timer minimaxTimer;
	minimaxTimer.Start();
	
	ClearHistory();

	while((d <= depthLimit) && (!m_bInCheckmate || (d != 2)))
	{
		bool bFoundAtDepth = MiniMax(d, playerID, moveOut, bEnableCutoff);
		
		if(bFoundAtDepth)
		{
#ifdef DEBUG_OUTPUT
			cout << "Depth " << d << " time: " << minimaxTimer.GetTime() << endl;
#endif
			bEnableCutoff = true;
			bFoundMove = true;
		}
		else
		{
#ifdef DEBUG_OUTPUT
			cout << "No move was found at depth " << d << endl;
#endif
			break;
		}

		++d;
	}
	
	return bFoundMove;
}

bool AI::MiniMax(int depth, int playerID, BoardMove& moveOut, bool bEnableCutoff)
{
	bool bFoundMove = false;
	BoardMove bestMove;

	int alpha = std::numeric_limits<int>::min() + 1;
	int beta = std::numeric_limits<int>::max();

	// Build a priority queue of the frontier nodes
	FRONTIER_TYPE frontier = MoveOrdering(playerID);

	for(const BoardMove& currentMove : frontier)
	{
		if(bEnableCutoff && m_bStopMinimax)
		{
			bFoundMove = false;
			break;
		}
	
		ApplyMove theMove(currentMove, &m_board);
		int val = MiniMax(depth - 1, playerID, !playerID, alpha, beta, bEnableCutoff);

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
	}

	if(bFoundMove)
	{
		m_history[playerID][GetHistoryTableIndex(bestMove.from)][GetHistoryTableIndex(bestMove.to)] += (depth * depth) + 1;
		moveOut = bestMove;
	}

	return bFoundMove;
}

int AI::MiniMax(int depth, int playerID, int playerIDToMove, int alpha, int beta, bool bEnableCutoff)
{
	if(bEnableCutoff && m_bStopMinimax)
		return 0;

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

		// Apply the move in the queue with the highest priority
		ApplyMove theMove(currentMove, &m_board);
		int score = MiniMax(depth - 1, playerID, !playerIDToMove, alpha, beta, bEnableCutoff);

		if(playerID == playerIDToMove)
		{
			if(score >= beta)
			{
				m_history[playerIDToMove][GetHistoryTableIndex(currentMove.from)][GetHistoryTableIndex(currentMove.to)] += (depth * depth) + 1;
				return score;
			}
			
			if(score > alpha)
			{
				alpha = score;
				bestMove = currentMove;
				bFoundBestMove = true;
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
				beta = score;
				bestMove = currentMove;
				bFoundBestMove = true;
			}
		}
	}

	if(bFoundBestMove)
	{
		m_history[playerIDToMove][GetHistoryTableIndex(bestMove.from)][GetHistoryTableIndex(bestMove.to)] += (depth * depth) + 1;
	}
		
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
