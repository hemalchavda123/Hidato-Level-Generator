#include "engine.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <queue>
#include <functional>

using namespace std;

HidatoGenerator::HidatoGenerator() : rng(chrono::steady_clock::now().time_since_epoch().count())
{
}

bool HidatoGenerator::isValid(int r, int c, int height, int width, const vector<vector<int>>& grid) const
{
    return r >= 0 && r < height && c >= 0 && c < width && grid[r][c] != CELL_OBSTACLE;
}

bool HidatoGenerator::isAdjacent(int r1, int c1, int r2, int c2) const
{
    return abs(r1 - r2) <= 1 && abs(c1 - c2) <= 1 && !(r1 == r2 && c1 == c2);
}

int HidatoGenerator::getNeighbours(int r, int c, int height, int width, const vector<vector<int>>& grid, bool onlyEmpty, Point* outNeighbours) const
{
    int count = 0;
    for(int dr = -1; dr <= 1; dr++)
    {
        for (int dc = -1; dc <= 1; ++dc)
        {
            if (dr == 0 && dc == 0) continue;

            int nr = r + dr, nc = c + dc;

            if (isValid(nr, nc, height, width, grid))
            {
                if (!onlyEmpty || grid[nr][nc] == CELL_EMPTY)
                {
                    outNeighbours[count++] = {nr, nc};
                }
            }
        }
    }
    return count;
}

int HidatoGenerator::countEmptyNeighbours(int r, int c, int height, int width, const vector<vector<int>>& grid) const
{
    int count = 0;
    for(int dr = -1; dr <= 1; dr++)
    {
        for (int dc = -1; dc <= 1; ++dc)
        {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < height && nc >= 0 && nc < width && grid[nr][nc] == CELL_EMPTY)
            {
                count++;
            }
        }
    }
    return count;
}

vector<vector<int>> HidatoGenerator::generateEmptyGrid(int N, Difficulty target)
{
    int canvasSize = N; 
    vector<vector<int>> canvas(canvasSize, vector<int>(canvasSize, CELL_OBSTACLE));
    
    int startR = canvasSize / 2;
    int startC = canvasSize / 2;
    canvas[startR][startC] = CELL_EMPTY;
    
    int cellsAdded = 1;
    Point currentTip = {startR, startC};
    vector<Point> allPlayableCells = {currentTip};

    while (cellsAdded < N)
    {
        vector<Point> potentialMoves;
        
        // EASY : Try to grow from the very last cell added to make a long snake
        if (target == Difficulty::EASY) {
            for (int dr = -1; dr <= 1; ++dr) 
            {
                for (int dc = -1; dc <= 1; ++dc) 
                {
                    int nr = currentTip.r + dr, nc = currentTip.c + dc;

                    if (nr >= 0 && nr < canvasSize && nc >= 0 && nc < canvasSize && canvas[nr][nc] == CELL_OBSTACLE)
                        potentialMoves.push_back({nr, nc});
                }
            }
        }
        
        // HARD/MEDIUM: Pick a completely random playable cell and expand outward
        if (potentialMoves.empty() || target != Difficulty::EASY) 
        {
            shuffle(allPlayableCells.begin(), allPlayableCells.end(), rng);

            for (const auto& cell : allPlayableCells) 
            {
                for (int dr = -1; dr <= 1; ++dr) 
                {
                    for (int dc = -1; dc <= 1; ++dc) 
                    {
                        int nr = cell.r + dr, nc = cell.c + dc;

                        if (nr >= 0 && nr < canvasSize && nc >= 0 && nc < canvasSize && canvas[nr][nc] == CELL_OBSTACLE) 
                            potentialMoves.push_back({nr, nc});
                    }
                }

                if (!potentialMoves.empty() && target != Difficulty::EASY) break; // Found an edge to expand
            }
        }

        // Pick a random valid expansion point
        if (!potentialMoves.empty()) 
        {
            Point nextCell = potentialMoves[uniform_int_distribution<int>(0, potentialMoves.size() - 1)(rng)];
            canvas[nextCell.r][nextCell.c] = CELL_EMPTY;
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
            if (canvas[r][c] == CELL_EMPTY) 
            {
                minR = min(minR, r); maxR = max(maxR, r);
                minC = min(minC, c); maxC = max(maxC, c);
            }
        }
    }

    int finalHeight = maxR - minR + 1;
    int finalWidth = maxC - minC + 1;
    vector<vector<int>> trimmedGrid(finalHeight, vector<int>(finalWidth, CELL_OBSTACLE));

    for (int r = minR; r <= maxR; ++r) 
        for (int c = minC; c <= maxC; ++c)
            trimmedGrid[r - minR][c - minC] = canvas[r][c];

    return trimmedGrid;
}

bool HidatoGenerator::generatePath(int r, int c, int currentVal, int N, vector<vector<int>>& grid, int& stepCount) 
{
    if (++stepCount > 5000) return false; 

    grid[r][c] = currentVal;
    if (currentVal == N) return true;

    int height = grid.size();
    int width = grid[0].size();
    
    Point neighborsArr[8];
    int numNeighbors = getNeighbours(r, c, height, width, grid, true, neighborsArr);
    
    pair<Point, int> neighborsWithCounts[8];
    for (int i = 0; i < numNeighbors; ++i) {
        neighborsWithCounts[i] = {neighborsArr[i], countEmptyNeighbours(neighborsArr[i].r, neighborsArr[i].c, height, width, grid)};
    }

    // Warnsdorff's Heuristic
    sort(neighborsWithCounts, neighborsWithCounts + numNeighbors, [](const pair<Point, int>& a, const pair<Point, int>& b) {
        return a.second < b.second;
    });

    for (int i = 0; i < numNeighbors; ++i)
        if (generatePath(neighborsWithCounts[i].first.r, neighborsWithCounts[i].first.c, currentVal + 1, N, grid, stepCount)) return true;

    grid[r][c] = CELL_EMPTY; // Backtrack
    return false;
}

int HidatoGenerator::solveBoard(int currentNum, int prevR, int prevC, vector<vector<int>>& grid, const vector<Point>& clues, const vector<int>& nextClueIndex, int limit, int N, int& stepCount)
{
    if (++stepCount > 20000) return limit + 1;

    if(currentNum > N) return 1;

    int height = grid.size();
    int width = grid[0].size();

    // Chebyshev distance pruning
    if (currentNum > 1 && prevR != -1) {
        int nextClue = nextClueIndex[currentNum - 1];
        if (nextClue != -1) {
            int dist = max(abs(prevR - clues[nextClue].r), abs(prevC - clues[nextClue].c));
            if (dist > nextClue - (currentNum - 1)) return 0;
        }
    }

    if(clues[currentNum].r != -1)
    {
        if(currentNum > 1 && !isAdjacent(prevR, prevC, clues[currentNum].r, clues[currentNum].c))
            return 0;
        return solveBoard(currentNum + 1, clues[currentNum].r, clues[currentNum].c, grid, clues, nextClueIndex, limit, N, stepCount);
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
                    if (grid[r][c] == CELL_EMPTY) 
                    {
                        grid[r][c] = 1;
                        ways += solveBoard(2, r, c, grid, clues, nextClueIndex, limit, N, stepCount);
                        grid[r][c] = CELL_EMPTY; 
                        if (ways > limit) return ways; 
                    }
                }
            }
        } 
        else 
        {
            Point neighborsArr[8];
            int numNeighbors = getNeighbours(prevR, prevC, height, width, grid, true, neighborsArr);
            for (int i = 0; i < numNeighbors; ++i) 
            {
                const auto& n = neighborsArr[i];
                grid[n.r][n.c] = currentNum;
                ways += solveBoard(currentNum + 1, n.r, n.c, grid, clues, nextClueIndex, limit, N, stepCount);
                grid[n.r][n.c] = CELL_EMPTY;
                if (ways > limit) return ways; 
            }
        }
        return ways;
    }
}

int HidatoGenerator::countSolutions(vector<vector<int>>& grid, int N, int limit)
{
    int height = grid.size();
    int width = grid[0].size();

    vector<Point> clues(N + 1, {-1, -1});

    for(int r=0; r<height; r++)
        for(int c=0; c<width; c++)
            if(grid[r][c] > 0)
                clues[grid[r][c]] = {r, c};

    vector<int> nextClueIndex(N + 1, -1);
    int lastClue = -1;
    for (int i = N; i >= 1; i--) {
        nextClueIndex[i] = lastClue;
        if (clues[i].r != -1) lastClue = i;
    }
                
    int stepCount = 0;
    return solveBoard(1, -1, -1, grid, clues, nextClueIndex, limit, N, stepCount);
}

vector<vector<int>> HidatoGenerator::pruneToUniqueMinimal(const vector<vector<int>>& solvedGrid, int N)
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
        int val = puzzle[cell.r][cell.c];
        if (val == 1 || val == N) continue; // Keep 1 and N as clues!
        
        puzzle[cell.r][cell.c] = CELL_EMPTY;

        if (countSolutions(puzzle, N, 2) > 1)
            puzzle[cell.r][cell.c] = val; // Restore if not unique
    }
    return puzzle;
}

// --- NEW GRADING ENGINE ALGORITHMS ---

bool HidatoGenerator::solveAndCountBacktracks(int currentNum, int prevR, int prevC, vector<vector<int>>& grid, const vector<Point>& clues, int N, int& backtracks) 
{
    if (currentNum > N) return true; 

    int height = grid.size();
    int width = grid[0].size();

    if (clues[currentNum].r != -1) 
    {
        if (currentNum > 1 && !isAdjacent(prevR, prevC, clues[currentNum].r, clues[currentNum].c)) 
        {
            backtracks++; 
            return false;
        }
        return solveAndCountBacktracks(currentNum + 1, clues[currentNum].r, clues[currentNum].c, grid, clues, N, backtracks);
    }

    Point neighborsArr[8];
    int numNeighbors = getNeighbours(prevR, prevC, height, width, grid, true, neighborsArr);

    vector<pair<Point, int>> rankedNeighbors;
    for (int i = 0; i < numNeighbors; ++i) 
    {
        int exits = countEmptyNeighbours(neighborsArr[i].r, neighborsArr[i].c, height, width, grid);
        rankedNeighbors.push_back({neighborsArr[i], exits});
    }
    
    // Warnsdorff's Heuristic
    sort(rankedNeighbors.begin(), rankedNeighbors.end(), [](const pair<Point, int>& a, const pair<Point, int>& b) {
        return a.second < b.second; 
    });

    for (const auto& n : rankedNeighbors) 
    {
        grid[n.first.r][n.first.c] = currentNum;
        if (solveAndCountBacktracks(currentNum + 1, n.first.r, n.first.c, grid, clues, N, backtracks)) 
            return true; 
            
        grid[n.first.r][n.first.c] = CELL_EMPTY; 
    }

    backtracks++; 
    return false;
}

int HidatoGenerator::calculateDeepestVoid(const vector<vector<int>>& puzzle) const 
{
    int height = puzzle.size();
    int width = puzzle[0].size();
    vector<vector<int>> dist(height, vector<int>(width, -1));
    queue<pair<Point, int>> q;

    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            if (puzzle[r][c] > 0) {
                q.push({{r, c}, 0});
                dist[r][c] = 0;
            } else if (puzzle[r][c] == CELL_OBSTACLE) {
                dist[r][c] = -2; 
            }
        }
    }

    int maxDepth = 0;
    while (!q.empty()) 
    {
        auto front = q.front();
        q.pop();
        Point pt = front.first;
        int d = front.second;
        maxDepth = max(maxDepth, d);

        Point neighborsArr[8];
        int numNeighbors = getNeighbours(pt.r, pt.c, height, width, puzzle, true, neighborsArr);
        
        for (int i = 0; i < numNeighbors; i++) {
            int nr = neighborsArr[i].r;
            int nc = neighborsArr[i].c;
            if (dist[nr][nc] == -1) { 
                dist[nr][nc] = d + 1;
                q.push(std::make_pair(Point{nr, nc}, d + 1));
            }
        }
    }
    return maxDepth;
}

int HidatoGenerator::countArticulationPoints(const vector<vector<int>>& puzzle) const 
{
    int height = puzzle.size();
    int width = puzzle[0].size();
    vector<vector<int>> disc(height, vector<int>(width, -1));
    vector<vector<int>> low(height, vector<int>(width, -1));
    vector<vector<bool>> ap(height, vector<bool>(width, false));
    int time = 0;

    std::function<void(int, int, int, int)> dfs = [&](int r, int c, int pr, int pc) {
        int children = 0;
        disc[r][c] = low[r][c] = ++time;

        Point neighborsArr[8];
        int numNeighbors = getNeighbours(r, c, height, width, puzzle, false, neighborsArr);

        for(int i = 0; i < numNeighbors; i++) {
            int nr = neighborsArr[i].r;
            int nc = neighborsArr[i].c;
            
            if (puzzle[nr][nc] == CELL_OBSTACLE) continue;

            if (disc[nr][nc] == -1) {
                children++;
                dfs(nr, nc, r, c);
                low[r][c] = min(low[r][c], low[nr][nc]);

                if (pr != -1 && low[nr][nc] >= disc[r][c]) {
                    ap[r][c] = true;
                }
            } else if (nr != pr || nc != pc) {
                low[r][c] = min(low[r][c], disc[nr][nc]);
            }
        }
        if (pr == -1 && children > 1) {
            ap[r][c] = true;
        }
    };

    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            if (puzzle[r][c] != CELL_OBSTACLE && disc[r][c] == -1) {
                dfs(r, c, -1, -1);
            }
        }
    }

    int count = 0;
    for (int r = 0; r < height; r++)
        for (int c = 0; c < width; c++)
            if (ap[r][c]) count++;

    return count;
}

Difficulty HidatoGenerator::evaluateDifficulty(const vector<vector<int>>& puzzle, int N) 
{
    int height = puzzle.size();
    int width = puzzle[0].size();

    vector<Point> clues(N + 1, {-1, -1});

    for(int r = 0; r < height; r++)
        for(int c = 0; c < width; c++)
            if(puzzle[r][c] > 0)
                clues[puzzle[r][c]] = {r, c};

    int maxGap = 0, currentGap = 0;
    for(int i = 1; i <= N; i++) 
    {
        if (clues[i].r == -1) 
        {
            currentGap++;
        }
        else 
        {
            maxGap = max(maxGap, currentGap);
            currentGap = 0;
        }
    }
    maxGap = max(maxGap, currentGap);

    int voidDepth = calculateDeepestVoid(puzzle);

    int chokePoints = countArticulationPoints(puzzle);

    vector<vector<int>> testGrid = puzzle;
    int backtracks = 0;
    solveAndCountBacktracks(2, clues[1].r, clues[1].c, testGrid, clues, N, backtracks);

    double spatialDifficulty = (maxGap * (voidDepth * 0.8));
    double chokeDampener = 1.0 + (chokePoints * 0.2);
    double score = (spatialDifficulty / chokeDampener);

    score += (backtracks * 2.5);

    double normalizedScore = score / static_cast<double>(N);

    constexpr double MEDIUM_THRESHOLD = 1.2;
    constexpr double HARD_THRESHOLD   = 2.5;

    if (normalizedScore < MEDIUM_THRESHOLD) return Difficulty::EASY;
    if (normalizedScore < HARD_THRESHOLD)   return Difficulty::MEDIUM;
    return Difficulty::HARD;
}

PuzzleResult HidatoGenerator::generateTargetedHidato(int N, Difficulty target) 
{
    if(!puzzleBank[N][target].empty()) 
    {
        PuzzleResult cachedPuzzle = puzzleBank[N][target].back();
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
        while (grid[startR][startC] != CELL_EMPTY);

        int pathStepCount = 0;
        if (!generatePath(startR, startC, 1, N, grid, pathStepCount)) continue;

        vector<vector<int>> puzzleGrid = pruneToUniqueMinimal(grid, N);

        // Uses the new unified mathematical/heuristic evaluator
        Difficulty generatedDifficulty = evaluateDifficulty(puzzleGrid, N);

        PuzzleResult result = 
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

void HidatoGenerator::printGrid(const vector<vector<int>>& grid) 
{
    for (const auto& row : grid) 
    {
        for (int val : row) 
        {
            if (val == CELL_OBSTACLE) cout << " ## "; 
            else if (val == CELL_EMPTY) cout << "  . "; 
            else cout << setw(3) << val << " "; 
        }
        cout << "\n";
    }
    cout << "\n";
}

string HidatoGenerator::difficultyToString(Difficulty d) 
{
    if (d == Difficulty::EASY) return "EASY";
    if (d == Difficulty::MEDIUM) return "MEDIUM";
    return "HARD";
}