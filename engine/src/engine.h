#ifndef HIDATO_GENERATOR_H
#define HIDATO_GENERATOR_H

#include <vector>

using namespace std;

enum Difficulty 
{ 
    EASY, 
    MEDIUM, 
    HARD 
};

struct Point 
{
    int r, c;
    
    bool operator==(const Point& other) const
    { 
        return r == other.r && c == other.c; 
    }
};

struct puzzleResult
{
    int finalWidth;
    int finalHeight;
    vector<vector<int>> puzzleGrid;
    vector<vector<int>> solutionGrid;
    Difficulty difficulty;
    int attempts;
};

puzzleResult generateTargetedHidato(int N, Difficulty target);

#endif