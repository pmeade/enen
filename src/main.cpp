/**
 * enen Demo: Neural Network Puzzles
 *
 * Five puzzles demonstrating IntgrNN capabilities:
 * 1. Generalization - Learn rules, not instances
 * 2. Feature Selection - Learn what to ignore
 * 3. XOR - Non-linear decision boundaries
 * 4. Sequence - Temporal reasoning
 * 5. Composition - Combine learned rules (deep network)
 *
 * All using IntgrNN - 8-bit integer neural networks.
 */

#include "networks.hpp"
#include "puzzles.hpp"
#include "renderer.hpp"
#include <cstdio>
#include <ctime>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace enen;

// Terminal input handling
#ifdef _WIN32

void enableRawMode() {
    // Windows console is already in a suitable mode for _getch()
}

void disableRawMode() {
    // Nothing to restore on Windows
}

char readKey() {
    if (_kbhit()) {
        return static_cast<char>(_getch());
    }
    Sleep(100);  // 100ms polling interval
    return 0;
}

#else

static termios orig_termios;

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

char readKey() {
    char c = 0;
    read(STDIN_FILENO, &c, 1);
    return c;
}

#endif

// Demo state
struct DemoState {
    PuzzleType current_puzzle = PuzzleType::GENERALIZATION;
    bool puzzle_complete = false;
    bool demo_complete = false;

    // Learning validator - tracks failures and required successes (Puzzles 1-4)
    LearningValidator validator;

    // Gauntlet state (Puzzle 5)
    GauntletState gauntlet;

    // IntgrNN Networks
    GeneralizationNet gen_net;
    FeatureSelectionNet feat_net;
    XORNet xor_net;
    SequenceNet seq_net;
    CompositionNet comp_net;

    // Puzzles
    SequencePuzzle seq_puzzle;

    // RNG
    RNG rng{static_cast<uint32_t>(time(nullptr))};

    // Trial history for each puzzle
    TrialHistory history;

    // Current trials for rendering
    MushroomTrial current_mushroom;
    ShapeTrial current_shape;
    XORTrial current_xor;
    CompositionTrial current_composition;

    void reset() {
        validator.reset();
        gauntlet.reset();
        puzzle_complete = false;
        history.clear();
    }

    void nextPuzzle() {
        int next = static_cast<int>(current_puzzle) + 1;
        if (next >= NUM_PUZZLES) {
            demo_complete = true;
        } else {
            current_puzzle = static_cast<PuzzleType>(next);
            reset();
        }
    }

    size_t totalModelBytes() const {
        return totalModelSize(gen_net, feat_net, xor_net, seq_net, comp_net);
    }
};

//=============================================================================
// Puzzle 1: Generalization
//=============================================================================
void runPuzzle1Trial(DemoState& state, Renderer& renderer) {
    // First trial is adversarial (likely to fail, evaluated honestly)
    bool adversarial = state.validator.isFirstTrial();
    state.current_mushroom = MushroomTrial::generate(state.rng, adversarial);
    const auto& trial = state.current_mushroom;

    // enen makes a choice — honest evaluation
    bool choseA = state.gen_net.chooseA(trial.sizeA, trial.sizeB, trial.colorA, trial.colorB);
    bool correct = (choseA == trial.correctIsA);

    // Always learn
    state.gen_net.learn(trial.sizeA, trial.sizeB, trial.colorA, trial.colorB, trial.correctIsA);

    // Record outcome
    state.validator.recordOutcome(correct);

    // Build history summary
    char summary[64];
    snprintf(summary, sizeof(summary), "%s(%d) vs %s(%d)",
             MushroomTrial::colorName(trial.colorA), trial.sizeA,
             MushroomTrial::colorName(trial.colorB), trial.sizeB);
    state.history.add(state.validator.total_trials, correct, summary);

    // Check for completion
    if (state.validator.hasLearned()) {
        state.puzzle_complete = true;
    }

    renderer.drawPuzzle1(trial, state.gen_net, choseA, correct,
                         state.history, state.validator.total_trials,
                         state.validator.successes,
                         state.validator.requiredSuccesses(),
                         state.puzzle_complete);
}

//=============================================================================
// Puzzle 2: Feature Interaction (circles safe, blue squares safest)
//=============================================================================
void runPuzzle2Trial(DemoState& state, Renderer& renderer) {
    // First trial is adversarial (blue square vs circle — tests the exception)
    bool adversarial = state.validator.isFirstTrial();
    state.current_shape = ShapeTrial::generate(state.rng, adversarial);
    const auto& trial = state.current_shape;

    // Honest evaluation
    bool choseA = state.feat_net.chooseA(trial.colorA, trial.shapeA, trial.colorB, trial.shapeB);
    bool correct = (choseA == trial.correctIsA);

    // Always learn
    state.feat_net.learn(trial.colorA, trial.shapeA, trial.colorB, trial.shapeB, trial.correctIsA);

    // Record outcome
    state.validator.recordOutcome(correct);

    // Build history summary
    char summary[64];
    snprintf(summary, sizeof(summary), "%s %s vs %s %s",
             ShapeTrial::colorName(trial.colorA), ShapeTrial::shapeName(trial.shapeA),
             ShapeTrial::colorName(trial.colorB), ShapeTrial::shapeName(trial.shapeB));
    state.history.add(state.validator.total_trials, correct, summary);

    // Check for completion
    if (state.validator.hasLearned()) {
        state.puzzle_complete = true;
    }

    renderer.drawPuzzle2(trial, state.feat_net, choseA, correct,
                         state.history, state.validator.total_trials,
                         state.validator.successes,
                         state.validator.requiredSuccesses(),
                         state.puzzle_complete);
}

//=============================================================================
// Puzzle 3: XOR (Context-dependent choice)
//=============================================================================
void runPuzzle3Trial(DemoState& state, Renderer& renderer) {
    state.current_xor = XORTrial::generate(state.rng);
    const auto& trial = state.current_xor;

    // enen predicts safety — honest evaluation
    bool predictedSafe = state.xor_net.isSafe(trial.lightInput(), trial.pathInput());
    bool correct = (predictedSafe == trial.isSafe);

    // Always learn
    state.xor_net.learn(trial.lightInput(), trial.pathInput(), trial.isSafe);

    // Record outcome
    state.validator.recordOutcome(correct);

    // Build history summary (prediction vs reality)
    char summary[64];
    snprintf(summary, sizeof(summary), "pred %s, was %s",
             predictedSafe ? "safe" : "danger",
             trial.isSafe ? "safe" : "danger");
    state.history.add(state.validator.total_trials, correct, summary);

    // Check for completion
    if (state.validator.hasLearned()) {
        state.puzzle_complete = true;
    }

    renderer.drawPuzzle3(trial, state.xor_net, predictedSafe, correct,
                         state.history, state.validator.total_trials,
                         state.validator.successes,
                         state.validator.requiredSuccesses(),
                         state.puzzle_complete);
}

//=============================================================================
// Puzzle 4: Sequence (A then B)
//=============================================================================
void runPuzzle4Step(DemoState& state, Renderer& renderer) {
    int16_t last = state.seq_puzzle.lastActionInput();

    // IntgrNN network decides based on last_action alone
    int action = state.seq_net.chooseAction(last);

    state.seq_puzzle.pressButton(action);

    bool trialComplete = false;
    bool correct = false;
    const char* resultStr = "";

    if (state.seq_puzzle.isSuccess()) {
        trialComplete = true;
        // Honest evaluation — if enen succeeded, it succeeded
        correct = true;
        resultStr = "A->B SUCCESS";
        state.seq_net.learnFromOutcome(last, action, true);
        state.seq_puzzle.reset();
    } else if (state.seq_puzzle.isFail()) {
        trialComplete = true;
        correct = false;
        resultStr = action == 1 ? "B first FAIL" : "A->A FAIL";
        state.seq_net.learnFromOutcome(last, action, false);
        state.seq_puzzle.reset();
    } else {
        // In progress (pressed A, waiting for B)
        state.seq_net.learnFromOutcome(last, action, true);
    }

    if (trialComplete) {
        state.validator.recordOutcome(correct);
        state.history.add(state.validator.total_trials, correct, resultStr);

        if (state.validator.hasLearned()) {
            state.puzzle_complete = true;
        }
    }

    renderer.drawPuzzle4(state.seq_puzzle, state.seq_net, state.history,
                         state.validator.total_trials,
                         state.validator.successes,
                         state.validator.requiredSuccesses(),
                         state.puzzle_complete);
}

//=============================================================================
// Puzzle 5: Composition Gauntlet
//=============================================================================
void runPuzzle5Trial(DemoState& state, Renderer& renderer) {
    // Check if gauntlet already complete
    if (state.gauntlet.isComplete()) {
        state.puzzle_complete = true;
        return;
    }

    state.current_composition = CompositionTrial::generate(state.rng);
    const auto& trial = state.current_composition;

    bool choseA = state.comp_net.chooseA(trial.lightInput(), trial.sizeA, trial.sizeB);
    bool correct = (choseA == trial.correctIsA);

    // Always learn
    state.comp_net.learn(trial.lightInput(), trial.sizeA, trial.sizeB, trial.correctIsA);

    // Record in gauntlet
    state.gauntlet.recordOutcome(correct);

    // Build history summary
    char summary[64];
    bool aLarger = trial.sizeA > trial.sizeB;
    snprintf(summary, sizeof(summary), "%s — %c(%d) %s %c(%d)",
             trial.lightOn ? "ON" : "OFF",
             choseA ? 'A' : 'B',
             choseA ? trial.sizeA : trial.sizeB,
             (choseA ? aLarger : !aLarger) ? ">" : "<",
             choseA ? 'B' : 'A',
             choseA ? trial.sizeB : trial.sizeA);
    state.history.add(state.gauntlet.currentTrials(), correct, summary);

    // Check for completion
    if (state.gauntlet.isComplete()) {
        state.puzzle_complete = true;
    }

    renderer.drawPuzzle5(trial, state.comp_net, choseA, correct,
                         state.history, state.gauntlet, state.puzzle_complete);
}

//=============================================================================
// Main
//=============================================================================
int main() {
    enableRawMode();

    Renderer renderer;
    renderer.init();

    DemoState state;
    bool showIntro = true;
    bool showPuzzleIntro = false;

    // Show demo intro
    renderer.drawIntro(state.totalModelBytes());

    while (!state.demo_complete) {
        char key = readKey();

        if (key == 'q' || key == 'Q') {
            break;
        }

        if (showIntro) {
            if (key == ' ') {
                showIntro = false;
                showPuzzleIntro = true;
                renderer.drawPuzzleIntro(state.current_puzzle);
            }
            continue;
        }

        if (showPuzzleIntro) {
            if (key == ' ') {
                showPuzzleIntro = false;
                // Fall through to run first trial immediately
            } else {
                continue;
            }
        }

        // Handle puzzle completion - require Enter to advance (not Space)
        if (state.puzzle_complete) {
            if (key == '\n' || key == '\r') {
                state.nextPuzzle();
                if (!state.demo_complete) {
                    showPuzzleIntro = true;
                    renderer.drawPuzzleIntro(state.current_puzzle);
                }
            }
            continue;
        }

        // Next trial (Space only)
        if (key == ' ') {
            switch (state.current_puzzle) {
                case PuzzleType::GENERALIZATION:
                    runPuzzle1Trial(state, renderer);
                    break;
                case PuzzleType::FEATURE_SELECTION:
                    runPuzzle2Trial(state, renderer);
                    break;
                case PuzzleType::XOR_CONTEXT:
                    runPuzzle3Trial(state, renderer);
                    break;
                case PuzzleType::SEQUENCE:
                    runPuzzle4Step(state, renderer);
                    break;
                case PuzzleType::COMPOSITION:
                    runPuzzle5Trial(state, renderer);
                    break;
            }
        }
    }

    // Show victory screen
    if (state.demo_complete) {
        renderer.drawVictory(state.totalModelBytes(),
                             state.gauntlet.correct,
                             GauntletState::SCORED_TRIALS);
        while (true) {
            char key = readKey();
            if (key == 'q' || key == 'Q') break;
        }
    }

    renderer.cleanup();
    disableRawMode();
    printf("\n");

    return 0;
}
