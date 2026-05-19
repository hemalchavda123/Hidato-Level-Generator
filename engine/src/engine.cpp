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
                    neighbours.push_back({nr, nc});
        }
    }

    return neighbours;
}

vector<vector<int>> generateEmptyGrid(int N, Difficulty target)
{
    int canvasSize = N; 
    vector<vector<int>> canvas(canvasSize, vector<int>(canvasSize, -1));
    
    int startR = canvasSize / 2;
    int startC = canvasSize / 2;
    canvas[startR][startC] = 0;
    
    int cellsAdded = 1;
    Point currentTip = {startR, startC};
    vector<Point> allPlayableCells = {currentTip};

    while (cellsAdded < N)
    {
        vector<Point> potentialMoves;
        
        // EASY : Try to grow from the very last cell added to make a long snake
        if (target == EASY) {
            for (int dr = -1; dr <= 1; ++dr) 
            {
                for (int dc = -1; dc <= 1; ++dc) 
                {
                    int nr = currentTip.r + dr, nc = currentTip.c + dc;

                    if (nr >= 0 && nr < canvasSize && nc >= 0 && nc < canvasSize && canvas[nr][nc] == -1)
                        potentialMoves.push_back({nr, nc});
                }
            }
        }
        
        // HARD/MEDIUM: Pick a completely random playable cell and expand outward
        if (potentialMoves.empty() || target != EASY) 
        {
            shuffle(allPlayableCells.begin(), allPlayableCells.end(), rng);

            for (const auto& cell : allPlayableCells) 
            {
                for (int dr = -1; dr <= 1; ++dr) 
                {
                    for (int dc = -1; dc <= 1; ++dc) 
                    {
                        int nr = cell.r + dr, nc = cell.c + dc;

                        if (nr >= 0 && nr < canvasSize && nc >= 0 && nc < canvasSize && canvas[nr][nc] == -1) 
                            potentialMoves.push_back({nr, nc});
                    }
                }

                if (!potentialMoves.empty() && target != EASY) break; // Found an edge to expand
            }
        }

        // Pick a random valid expansion point
        if (!potentialMoves.empty()) 
        {
            Point nextCell = potentialMoves[uniform_int_distribution<int>(0, potentialMoves.size() - 1)(rng)];
            canvas[nextCell.r][nextCell.c] = 0;
            currentTip = nextCell;
            allPlayableCells.push_back(nextCell);
            cellsAdded++;
        }
    }

    // Shrink-wrap the canvas to find the exact bounding box
    int minR = canvasSize, maxR = 0, minC = canvasSize, maxC = 0;

    for (int r = 0; r < canvasSize; ++r) 
    {
        for (int c = 0; c < canvasSize; ++c) 
        {
            if (canvas[r][c] == 0) 
            {
                minR = min(minR, r); maxR = max(maxR, r);
                minC = min(minC, c); maxC = max(maxC, c);
            }
        }
    }

    int finalHeight = maxR - minR + 1;
    int finalWidth = maxC - minC + 1;
    vector<vector<int>> trimmedGrid(finalHeight, vector<int>(finalWidth, -1));

    for (int r = minR; r <= maxR; ++r) 
        for (int c = minC; c <= maxC; ++c)
            trimmedGrid[r - minR][c - minC] = canvas[r][c];

    return trimmedGrid;
}

int countEmptyNeighbours(int r, int c, int height, int width, const vector<vector<int>>& grid) 
{
    return getNeighbours(r, c, height, width, grid, true).size();
}

bool generatePath(int r, int c, int currentVal, int N, vector<vector<int>>& grid) 
{
    grid[r][c] = currentVal;
    if (currentVal == N) return true; // Path complete!

    int height = grid.size();
    int width = grid[0].size();
    auto neighbors = getNeighbours(r, c, height, width, grid, true);
    
    // Warnsdorff's Heuristic: Move to the cell with the fewest exits
    sort(neighbors.begin(), neighbors.end(), [&](const Point& a, const Point& b) {
        return countEmptyNeighbours(a.r, a.c, height, width, grid) < countEmptyNeighbours(b.r, b.c, height, width, grid);
    });

    for (const auto& n : neighbors)
        if (generatePath(n.r, n.c, currentVal + 1, N, grid)) return true;

    grid[r][c] = 0; // Backtrack
    return false;
}

