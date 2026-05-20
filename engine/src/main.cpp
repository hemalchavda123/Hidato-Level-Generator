#include "engine.h"
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace std;

int main() {
    int n;
    cout << "Enter the number n: ";
    cin >> n;

    Difficulty target;
    int diff;
    cout << "Enter the target difficulty (0 for EASY, 1 for MEDIUM, 2 for HARD): ";
    cin >> diff;

    if (diff == 0) target = Difficulty::EASY;
    else if (diff == 1) target = Difficulty::MEDIUM;
    else if (diff == 2) target = Difficulty::HARD;

    cout << "Forging a " << n << "-cell " << HidatoGenerator::difficultyToString(target) << " Hidato puzzle...\n";
    cout << "(This uses rejection sampling, wait a moment)\n\n";

    auto start = std::chrono::high_resolution_clock::now();
    
    HidatoGenerator generator;
    PuzzleResult result = generator.generateTargetedHidato(n, target);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;

    cout << "=== PUZZLE BOARD ===\n";
    HidatoGenerator::printGrid(result.puzzleGrid);

    cout << "=== ANSWER KEY ===\n";
    HidatoGenerator::printGrid(result.solutionGrid);

    cout << "--- Metadata ---\n";
    cout << "Target     : " << HidatoGenerator::difficultyToString(target) << "\n";
    cout << "Attempts   : " << result.attempts << " (Boards rejected before finding a match)\n";
    cout << "Dimensions : " << result.finalWidth << " width x " << result.finalHeight << " height\n";
    cout << "Gen Time   : " << ms.count() << " ms\n\n";

    return 0;
}