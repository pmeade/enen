/**
 * Debug test to trace puzzle failures
 */

#include "game.hpp"
#include <cstdio>

using namespace enen;

void debugPuzzle2() {
    printf("\n=== Debug Puzzle 2 (Feature Selection) ===\n");

    Game game(54321);  // Different seed
    game.nextPuzzle();  // Skip to puzzle 2

    auto& s = game.state();

    for (int i = 0; i < 50; i++) {
        // Get current weights and trial data before decision
        int w_color = s.feat_net.colorAttention();
        int w_shape = s.feat_net.shapeAttention();

        // Store weights for score calculation
        int16_t wc = s.feat_net.w_color;
        int16_t ws = s.feat_net.w_shape;

        bool completed = game.runTrial();

        // After trial, show trial data
        auto& t = s.current_shape;
        int16_t scoreA = (wc * t.colorA + ws * t.shapeA) / 128;
        int16_t scoreB = (wc * t.colorB + ws * t.shapeB) / 128;

        printf("Trial %2d: colA=%3d colB=%3d shpA=%3d shpB=%3d | scA=%4d scB=%4d | correct=%c fail=%d succ=%d %s\n",
               i+1, t.colorA, t.colorB, t.shapeA, t.shapeB,
               scoreA, scoreB,
               t.correctIsA ? 'A' : 'B',
               s.validator.failures, s.validator.successes,
               completed ? "DONE!" : "");

        if (completed) break;
    }
}

void debugPuzzle4() {
    printf("\n=== Debug Puzzle 4 (Sequence) ===\n");

    Game game(12345);
    game.nextPuzzle();  // Skip to puzzle 2
    game.nextPuzzle();  // Skip to puzzle 3
    game.nextPuzzle();  // Skip to puzzle 4

    auto& s = game.state();

    for (int i = 0; i < 100; i++) {
        int16_t last = s.seq_puzzle.lastActionInput();
        int16_t scoreA = s.seq_net.scoreA(last);
        int16_t scoreB = s.seq_net.scoreB(last);

        bool completed = game.runTrial();

        printf("Trial %2d: last=%3d scoreA=%4d scoreB=%4d | wA=%4d wB=%4d bA=%4d bB=%4d | fail=%d succ=%d %s\n",
               i+1, last, scoreA, scoreB,
               s.seq_net.wA_last, s.seq_net.wB_last,
               s.seq_net.bA, s.seq_net.bB,
               s.validator.failures, s.validator.successes,
               completed ? "DONE!" : "");

        if (completed) break;
    }
}

void debugPuzzle5() {
    printf("\n=== Debug Puzzle 5 (Composition) ===\n");

    Game game(1768016321);  // A seed that failed
    for (int i = 0; i < 4; i++) game.nextPuzzle();  // Skip to puzzle 5

    auto& s = game.state();

    for (int i = 0; i < 100; i++) {
        bool completed = game.runTrial();

        auto& t = s.current_composition;
        printf("Trial %2d: light=%s sA=%3d sB=%3d correct=%c | fail=%d succ=%d (need %d) %s\n",
               i+1, t.lightOn ? "ON " : "OFF",
               t.sizeA, t.sizeB,
               t.correctIsA ? 'A' : 'B',
               s.validator.failures, s.validator.successes,
               s.validator.requiredSuccesses(),
               completed ? "DONE!" : "");

        if (completed) break;
    }
}

int main() {
    debugPuzzle2();
    debugPuzzle4();
    debugPuzzle5();
    return 0;
}
