#include "AI.h"
#include "util.h"

#include <algorithm>
#include <cassert>
#include <unordered_map>

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
	m_grid.resize(8);

	for(auto& iter : m_grid)
	{
		iter.resize(8);
	}

	m_generator.seed(time(0));
}

//This function is called each time it is your turn.
//Return true to end your turn, return false to ask the server for updated information.
bool AI::run()
{
	BuildGrid();
	DrawBoard();

	// if there has been a move, print the most recent move
	if(moves.size() > 0)
	{
		cout<<"Last Move Was: "<<endl<<moves[0]<<endl;
	}

	std::vector<int> userPieces = GetUserPieces('P');

	if(!userPieces.empty())
	{
		std::uniform_int_distribution<int> distribution(0,userPieces.size() - 1);
		unsigned int iRandomPiece = distribution(m_generator);

		std::vector<vec2> moves = GetPieceMoves(userPieces[iRandomPiece]);

		if(!moves.empty())
		{
			distribution = std::uniform_int_distribution<int>(0,moves.size() - 1);
			unsigned int iRandomMove = distribution(m_generator);

			// file, rank
			pieces[userPieces[iRandomPiece]].move(moves[iRandomMove].x, moves[iRandomMove].y, int('Q'));
		}
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

void AI::BuildGrid()
{
	ClearGrid();

	for(const Piece& p : pieces)
	{
		assert(p.file() <= 8 && p.file() >= 1);
		assert(p.rank() <= 8 && p.rank() >= 1);

		m_grid[p.file() - 1][p.rank() - 1] = p.id() - 2;
	}
}

void AI::ClearGrid()
{
	for(auto& fileIter : m_grid)
	{
		for(int& rankIter : fileIter)
		{
			rankIter = -1;
		}
	}
}

bool AI::IsTileEmpty(int file, int rank) const
{
	assert(file <= 8 && file >= 1);
	assert(rank <= 8 && rank >= 1);

	return m_grid[file - 1][rank - 1] == -1;
}

bool AI::IsTileOwner(int file, int rank) const
{
	assert(file <= 8 && file >= 1);
	assert(rank <= 8 && rank >= 1);

	if(IsTileEmpty(file,rank))
		return false;

	return (playerID() == pieces[m_grid[file - 1][rank - 1]].owner());
}

std::vector<int> AI::GetUserPieces(char type) const
{
	std::vector<int> userPieces;

	int iPlayerID = playerID();

	for(const Piece& p : pieces)
	{
		if((iPlayerID == p.owner()) && (p.type() == int(type)))
		{
			userPieces.push_back(p.id() - 2);
		}
	}

	return userPieces;
}

std::vector<vec2> AI::GetPieceMoves(int index) const
{
	std::vector<vec2> pieceMoves;

	// Pawn movement
	const Piece& piece = pieces[index];
	cout << piece.owner() << endl;
	assert(piece.owner() == playerID());

	int iNewRank = piece.rank() + ((playerID() == 0) ? 1 : -1);
	if(iNewRank <= 8 && iNewRank >= 1)
	{
		// First check if we can move to the tile in front of us
		if(IsTileEmpty(piece.file(),iNewRank))
		{
			pieceMoves.push_back({piece.file(), iNewRank});
		}

		int iNewFile = piece.file() + 1;
		if(iNewFile <= 8 && iNewFile >= 1)
		{
			if(!IsTileEmpty(iNewFile,iNewRank) && !IsTileOwner(iNewFile, iNewRank))
			{
				pieceMoves.push_back({iNewFile, iNewRank});
			}
		}

		iNewFile = piece.file() - 1;
		if(iNewFile <= 8 && iNewFile >= 1)
		{
			if(!IsTileEmpty(iNewFile,iNewRank) && !IsTileOwner(iNewFile, iNewRank))
			{
				pieceMoves.push_back({iNewFile, iNewRank});
			}
		}
	}

	// Check if we can capture a piece by moving to a forward diaganol tile

	return pieceMoves;
}











