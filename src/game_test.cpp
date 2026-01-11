/**
 * Game Logic Test
 *
 * Tests the Game class without UI to verify puzzle completion.
 * Each puzzle should complete within a reasonable number of trials.
 */

#include "game.hpp"
#include <cstdio>
#include <ctime>

using namespace enen;

// Test configuration
constexpr int MAX_TRIALS = 500;
constexpr int NUM_RUNS = 10;  // Run multiple times with different seeds

struct TestResult {
    int trials_taken;
    bool success;
};

TestResult testPuzzle(Game& game, const char* name, int puzzleNum) {
    int trials = game.runPuzzleToCompletion(MAX_TRIALS);
    bool success = trials > 0;

    printf("  Puzzle %d (%s): ", puzzleNum, name);
    if (success) {
        printf("PASS in %d trials\n", trials);
    } else {
        printf("FAIL (did not complete in %d trials)\n", MAX_TRIALS);
    }

    return {trials, success};
}

bool runFullTest(uint32_t seed) {
    printf("\n=== Testing with seed %u ===\n", seed);

    Game game(seed);
    int totalTrials = 0;
    bool allPassed = true;

    // Puzzle 1: Generalization
    auto r1 = testPuzzle(game, "Generalization", 1);
    if (!r1.success) allPassed = false;
    totalTrials += r1.trials_taken > 0 ? r1.trials_taken : MAX_TRIALS;

    // Puzzle 2: Feature Selection
    game.nextPuzzle();
    auto r2 = testPuzzle(game, "Feature Selection", 2);
    if (!r2.success) allPassed = false;
    totalTrials += r2.trials_taken > 0 ? r2.trials_taken : MAX_TRIALS;

    // Puzzle 3: XOR
    game.nextPuzzle();
    auto r3 = testPuzzle(game, "XOR", 3);
    if (!r3.success) allPassed = false;
    totalTrials += r3.trials_taken > 0 ? r3.trials_taken : MAX_TRIALS;

    // Puzzle 4: Sequence
    game.nextPuzzle();
    auto r4 = testPuzzle(game, "Sequence", 4);
    if (!r4.success) allPassed = false;
    totalTrials += r4.trials_taken > 0 ? r4.trials_taken : MAX_TRIALS;

    // Puzzle 5: Composition
    game.nextPuzzle();
    auto r5 = testPuzzle(game, "Composition", 5);
    if (!r5.success) allPassed = false;
    totalTrials += r5.trials_taken > 0 ? r5.trials_taken : MAX_TRIALS;

    printf("  Total trials: %d\n", totalTrials);
    printf("  Result: %s\n", allPassed ? "ALL PASSED" : "SOME FAILED");

    return allPassed;
}

bool testFullDemo(uint32_t seed) {
    printf("\n=== Full Demo Test with seed %u ===\n", seed);

    Game game(seed);
    bool success = game.runFullDemo(MAX_TRIALS);

    printf("  Full demo: %s\n", success ? "COMPLETED" : "FAILED");
    printf("  Demo complete flag: %s\n",
           game.state().demo_complete ? "true" : "false");

    return success;
}

int main() {
    printf("Game Logic Test\n");
    printf("================\n");
    printf("Testing Game class without UI\n");
    printf("Max trials per puzzle: %d\n", MAX_TRIALS);

    int passedRuns = 0;
    int failedRuns = 0;

    // Test with multiple seeds
    for (int i = 0; i < NUM_RUNS; i++) {
        uint32_t seed = static_cast<uint32_t>(time(nullptr)) + i * 1000;
        if (runFullTest(seed)) {
            passedRuns++;
        } else {
            failedRuns++;
        }
    }

    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d\n", passedRuns, NUM_RUNS);
    printf("Failed: %d/%d\n", failedRuns, NUM_RUNS);

    // Also test runFullDemo
    printf("\n=== Full Demo Tests ===\n");
    int demoPassedRuns = 0;
    for (int i = 0; i < NUM_RUNS; i++) {
        uint32_t seed = static_cast<uint32_t>(time(nullptr)) + i * 1000 + 500;
        if (testFullDemo(seed)) {
            demoPassedRuns++;
        }
    }

    printf("\n=== Final Results ===\n");
    printf("Individual puzzle tests: %d/%d passed\n", passedRuns, NUM_RUNS);
    printf("Full demo tests: %d/%d passed\n", demoPassedRuns, NUM_RUNS);

    return (passedRuns == NUM_RUNS && demoPassedRuns == NUM_RUNS) ? 0 : 1;
}
