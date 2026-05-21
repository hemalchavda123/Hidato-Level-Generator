#include "engine.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    bool jsonOutput = false;
    int n = 0;
    int diff = -1;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--json") {
            jsonOutput = true;
        } else if (n == 0) {
            try { n = stoi(arg); } catch(...) {}
        } else if (diff == -1) {
            try { diff = stoi(arg); } catch(...) {}
        }
    }

    if (n == 0 || diff == -1) {
        if (!jsonOutput) {
            cout << "Enter the number n: ";
            cin >> n;
            cout << "Enter the target difficulty (0 for EASY, 1 for MEDIUM, 2 for HARD): ";
            cin >> diff;
        } else {
            cout << "{\"error\": \"Missing arguments\"}" << endl;
            return 1;
        }
    }

    Difficulty target;
    if (diff == 0) target = Difficulty::EASY;
    else if (diff == 1) target = Difficulty::MEDIUM;
    else if (diff == 2) target = Difficulty::HARD;
    else target = Difficulty::MEDIUM;

    if (!jsonOutput) {
        cout << "Forging a " << n << "-cell " << HidatoGenerator::difficultyToString(target) << " Hidato puzzle...\n";
        cout << "(This uses rejection sampling, wait a moment)\n\n";
    }

    auto start = std::chrono::high_resolution_clock::now();
    
    HidatoGenerator generator;
    PuzzleResult result = generator.generateTargetedHidato(n, target);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;

    if (jsonOutput) {
        cout << "{";
        cout << "\"finalWidth\": " << result.finalWidth << ",";
        cout << "\"finalHeight\": " << result.finalHeight << ",";
        cout << "\"difficulty\": \"" << HidatoGenerator::difficultyToString(target) << "\",";
        cout << "\"attempts\": " << result.attempts << ",";
        cout << "\"genTimeMs\": " << ms.count() << ",";
        
        cout << "\"puzzleGrid\": [";
        for (size_t r = 0; r < result.puzzleGrid.size(); ++r) {
            cout << "[";
            for (size_t c = 0; c < result.puzzleGrid[r].size(); ++c) {
                cout << result.puzzleGrid[r][c] << (c == result.puzzleGrid[r].size() - 1 ? "" : ",");
            }
            cout << "]" << (r == result.puzzleGrid.size() - 1 ? "" : ",");
        }
        cout << "],";
        
        cout << "\"solutionGrid\": [";
        for (size_t r = 0; r < result.solutionGrid.size(); ++r) {
            cout << "[";
            for (size_t c = 0; c < result.solutionGrid[r].size(); ++c) {
                cout << result.solutionGrid[r][c] << (c == result.solutionGrid[r].size() - 1 ? "" : ",");
            }
            cout << "]" << (r == result.solutionGrid.size() - 1 ? "" : ",");
        }
        cout << "]";
        
        cout << "}" << endl;
    } else {
        cout << "=== PUZZLE BOARD ===\n";
        HidatoGenerator::printGrid(result.puzzleGrid);

        cout << "=== ANSWER KEY ===\n";
        HidatoGenerator::printGrid(result.solutionGrid);

        cout << "--- Metadata ---\n";
        cout << "Target     : " << HidatoGenerator::difficultyToString(target) << "\n";
        cout << "Attempts   : " << result.attempts << " (Boards rejected before finding a match)\n";
        cout << "Dimensions : " << result.finalWidth << " width x " << result.finalHeight << " height\n";
        cout << "Gen Time   : " << ms.count() << " ms\n\n";
    }

    return 0;
}