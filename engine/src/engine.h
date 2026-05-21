#ifndef HIDATO_GENERATOR_H
#define HIDATO_GENERATOR_H

#include <vector>
#include <string>
#include <map>
#include <random>

enum class Difficulty 
{ 
    EASY, 
    MEDIUM, 
    HARD 
};

const int CELL_EMPTY = 0;
const int CELL_OBSTACLE = -1;

struct Point 
{
    int r, c;
    
    bool operator==(const Point& other) const
    { 
        return r == other.r && c == other.c; 
    }
};

struct PuzzleResult
{
    int finalWidth;
    int finalHeight;
    std::vector<std::vector<int>> puzzleGrid;
    std::vector<std::vector<int>> solutionGrid;
    Difficulty difficulty;
    int attempts;
};

class HidatoGenerator 
{
public:
    HidatoGenerator();
    
    PuzzleResult generateTargetedHidato(int N, Difficulty target);
    
    static void printGrid(const std::vector<std::vector<int>>& grid);
    static std::string difficultyToString(Difficulty d);

private:
    std::mt19937 rng;
    std::map<int, std::map<Difficulty, std::vector<PuzzleResult>>> puzzleBank;
    const int MAX_CACHE_SIZE = 50;

    bool isValid(int r, int c, int height, int width, const std::vector<std::vector<int>>& grid) const;
    bool isAdjacent(int r1, int c1, int r2, int c2) const;
    
    int getNeighbours(int r, int c, int height, int width, const std::vector<std::vector<int>>& grid, bool onlyEmpty, Point* outNeighbours) const;
    
    int countEmptyNeighbours(int r, int c, int height, int width, const std::vector<std::vector<int>>& grid) const;

    std::vector<std::vector<int>> generateEmptyGrid(int N, Difficulty target);

    bool generatePath(int r, int c, int currentVal, int N, std::vector<std::vector<int>>& grid, int& stepCount);
    
    int solveBoard(int currentNum, int prevR, int prevC, std::vector<std::vector<int>>& grid, const std::vector<Point>& clues, const std::vector<int>& nextClueIndex, int limit, int N, int& stepCount);
    int countSolutions(std::vector<std::vector<int>>& grid, int N, int limit);
    
    std::vector<std::vector<int>> pruneToUniqueMinimal(const std::vector<std::vector<int>>& solvedGrid, int N); 
    
    double calculateBranchingFactor(const std::vector<std::vector<int>>& puzzle) const;
    
    bool solveAndCountBacktracks(int currentNum, int prevR, int prevC, std::vector<std::vector<int>>& grid, const std::vector<Point>& clues, int N, int& backtracks);

    int calculateDeepestVoid(const std::vector<std::vector<int>>& puzzle) const;

    int countArticulationPoints(const std::vector<std::vector<int>>& puzzle) const;
    
    Difficulty evaluateDifficulty(const std::vector<std::vector<int>>& puzzle, int N);
};

#endif