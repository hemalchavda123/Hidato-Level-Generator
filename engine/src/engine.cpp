#include "engine.h"
#include <bits/stdc++.h>

using namespace std;

// random seed
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

// storing puzzles in this map
map<int, map<Difficulty, vector<puzzleResult>>> puzzleBank;
const int MAX_CACHE_SIZE = 50;


// Helper functions

bool isValid(int r, int c, int height, int width, const vector<vector<int>>& grid)
{
    return r >= 0 && r < height && c >= 0 && c < width && grid[r][c] != -1;
}

bool isAdjacent(int r1, int c1, int r2, int c2) 
{
    return abs(r1 - r2) <= 1 && abs(c1 - c2) <= 1 && !(r1 == r2 && c1 == c2);
}

vector<Point> getNeighbours(int r, int c, int height, int width, const vector<vector<int>>& grid, bool onlyEmpty)
{
    vector<Point> neighbours;

    for(int dr=-1; dr<=1; dr++)
    {
        for (int dc = -1; dc <= 1; ++dc)
        {
            if (dr == 0 && dc == 0) continue;

            int nr = r + dr, nc = c + dc;

            if (isValid(nr, nc, height, width, grid))
                if (!onlyEmpty || grid[nr][nc] == 0)
                    neighbors.push_back({nr, nc});
        }
    }

    return neighbours;
}