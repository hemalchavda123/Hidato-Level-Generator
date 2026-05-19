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

int solveBoard(int currentNum, int prevR, int prevC, vector<vector<int>>& grid, vector<Point>& clues, int limit, int N)
{
    if(currentNum > N) return 1;

    int height = grid.size();
    int width = grid[0].size();

    if(clues[currentNum].r != -1)
    {
        if(currentNum > 1 && !isAdjacent(prevR, prevC, clues[currentNum].r, clues[currentNum].c))
            return 0; // Clue is not adjacent to previous number
        return solveBoard(currentNum + 1, clues[currentNum].r, clues[currentNum].c, grid, clues, limit, N);
    }
    else
    {
        int ways = 0;

        if(currentNum == 1) 
        {
            for (int r = 0; r < height; r++) 
            {
                for (int c = 0; c < width; c++) 
                {
                    if (grid[r][c] == 0) 
                    {
                        grid[r][c] = 1;
                        ways += solveBoard(2, r, c, grid, clues, limit, N);
                        grid[r][c] = 0; 
                        if (ways > limit) return ways; 
                    }
                }
            }
        } 
        else 
        {
            for (const auto& n : getNeighbours(prevR, prevC, height, width, grid, true)) 
            {
                grid[n.r][n.c] = currentNum;
                ways += solveBoard(currentNum + 1, n.r, n.c, grid, clues, limit, N);
                grid[n.r][n.c] = 0;
                if (ways > limit) return ways; 
            }
        }

        return ways;
    }
}

int countSolutions(vector<vector<int>>& grid, int N, int limit)
{
    int height = grid.size();
    int width = grid[0].size();

    vector<Point> clues(N + 1, {-1, -1});

    for(int r=0; r<height; r++)
        for(int c=0; c<width; c++)
            if(grid[r][c] > 0)
                clues[grid[r][c]] = {r, c};
                
    return solveBoard(1, -1, -1, grid, clues, limit, N);
}

vector<vector<int>> pruneToUniqueMinimal(const vector<vector<int>>& solvedGrid, int N)
{
    vector<vector<int>> puzzle = solvedGrid;
    int height = puzzle.size();
    int width = puzzle[0].size();

    vector<Point> cells;

    for (int r = 0; r < height; r++)
        for (int c = 0; c < width; c++)
            if (puzzle[r][c] > 0)
                cells.push_back({r, c});

    shuffle(cells.begin(), cells.end(), rng);

    for (const auto& cell : cells) 
    {
        int backup = puzzle[cell.r][cell.c];
        puzzle[cell.r][cell.c] = 0;

        if (countSolutions(puzzle, 2, N) > 1)
            puzzle[cell.r][cell.c] = backup; // Restore if not unique
    }
    return puzzle;
}

bool matchesStatisticalProfile(const vector<vector<int>>& puzzle, Difficulty target, int N) 
{
    int maxGap = 0, currentGap = 0, totalClues = 0;
    int height = puzzle.size();
    int width = puzzle[0].size();

    for(int i=1; i<=N; i++) 
    {
        bool found = false;

        for (int r = 0; r < height && !found; r++) 
            for (int c = 0; c < width && !found; c++) 
                if (puzzle[r][c] == i) 
                    found = true;  
                                
        if (!found) currentGap++;
        else 
        {
            maxGap = max(maxGap, currentGap);
            currentGap = 0;
            totalClues++;
        }
    }

    maxGap = max(maxGap, currentGap);
    double clueDensity = (double)totalClues / N;

    if (target == EASY) return clueDensity > 0.35 && maxGap <= 5;
    if (target == MEDIUM) return maxGap > 4 && maxGap <= 15;
    if (target == HARD) return maxGap > 15 || clueDensity < 0.15;

    return false;
}

puzzleResult generateTargetedHidato(int N, Difficulty target) 
{
   
    if(!puzzleBank[N][target].empty()) 
    {
        puzzleResult cachedPuzzle = puzzleBank[N][target].back();
        puzzleBank[N][target].pop_back();

        return cachedPuzzle;
    }

    int attempts = 0;

    while(true)
    {
        attempts++;

        vector<vector<int>> grid = generateEmptyGrid(N, target);
        int finalHeight = grid.size();
        int finalWidth = grid[0].size();

        int startR, startC;
        do
        {
            startR = uniform_int_distribution<int>(0, finalHeight - 1)(rng);
            startC = uniform_int_distribution<int>(0, finalWidth - 1)(rng);
        } 
        while (grid[startR][startC] != 0);

        if (!generatePath(startR, startC, 1, N, grid)) continue;

        vector<vector<int>> puzzleGrid = pruneToUniqueMinimal(grid, N);

        Difficulty generatedDifficulty;
        if (matchesStatisticalProfile(puzzleGrid, EASY, N)) generatedDifficulty = EASY;
        else if (matchesStatisticalProfile(puzzleGrid, MEDIUM, N)) generatedDifficulty = MEDIUM;
        else if (matchesStatisticalProfile(puzzleGrid, HARD, N)) generatedDifficulty = HARD;
        else continue;

        puzzleResult result = 
        {
            finalWidth,
            finalHeight,
            puzzleGrid,
            grid,
            generatedDifficulty,
            attempts
        };

        if (generatedDifficulty == target)
            return result;
        else if (puzzleBank[N][generatedDifficulty].size() < MAX_CACHE_SIZE)
                puzzleBank[N][generatedDifficulty].push_back(result);
    }
}