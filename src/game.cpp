/**
 * Game logic implementation for enen Demo
 */

#include "game.hpp"
#include <cstdio>

namespace enen {

Game::Game(uint32_t seed) : state_(seed) {}

void Game::emit(EventType type, const std::string& msg, bool success) {
    if (callback_) {
        callback_({type, msg, success});
    }
}

bool Game::runTrial() {
    switch (state_.current_puzzle) {
        case PuzzleType::GENERALIZATION:
            return runPuzzle1Trial();
        case PuzzleType::FEATURE_SELECTION:
            return runPuzzle2Trial();
        case PuzzleType::XOR_CONTEXT:
            return runPuzzle3Trial();
        case PuzzleType::SEQUENCE:
            return runPuzzle4Trial();
        case PuzzleType::COMPOSITION:
            return runPuzzle5Trial();
    }
    return false;
}

void Game::resetPuzzle() {
    state_.reset();
    state_.resetNetwork();
    emit(EventType::TRIAL_START, "--- RESET ---");
}

bool Game::nextPuzzle() {
    return state_.nextPuzzle();
}

int Game::runPuzzleToCompletion(int maxTrials) {
    state_.reset();
    for (int i = 0; i < maxTrials; i++) {
        if (runTrial()) {
            return i + 1;
        }
    }
    return -1;  // Failed to complete
}

bool Game::runFullDemo(int maxTrialsPerPuzzle) {
    for (int p = 0; p < NUM_PUZZLES; p++) {
        int trials = runPuzzleToCompletion(maxTrialsPerPuzzle);
        if (trials < 0) {
            return false;  // Failed
        }
        if (p < NUM_PUZZLES - 1) {
            nextPuzzle();
        }
    }
    state_.demo_complete = true;
    return true;
}

//=============================================================================
// Puzzle 1: Generalization
//=============================================================================
bool Game::runPuzzle1Trial() {
    auto& s = state_;

    // First trial is adversarial (likely to fail, but evaluated honestly)
    bool adversarial = s.validator.isFirstTrial();
    s.current_mushroom = MushroomTrial::generate(s.rng, adversarial);
    const auto& trial = s.current_mushroom;

    // enen makes a choice — honest evaluation
    bool choseA = s.gen_net.chooseA(trial.sizeA, trial.sizeB, trial.colorA, trial.colorB);
    bool correct = (choseA == trial.correctIsA);

    char msg[128];
    snprintf(msg, sizeof(msg), "enen sees %s(%d) vs %s(%d)",
             MushroomTrial::colorName(trial.colorA), trial.sizeA,
             MushroomTrial::colorName(trial.colorB), trial.sizeB);
    emit(EventType::TRIAL_START, msg);

    snprintf(msg, sizeof(msg), "enen picks %c (%s)",
             choseA ? 'A' : 'B', choseA ? "left" : "right");
    emit(EventType::CHOICE_MADE, msg);

    if (correct) {
        emit(EventType::OUTCOME, "CORRECT! The bigger one was safe.", true);
    } else {
        emit(EventType::OUTCOME, "WRONG! Picked the smaller one.", false);
    }

    // Always learn - IntgrNN handles gradient computation
    s.gen_net.learn(trial.sizeA, trial.sizeB, trial.colorA, trial.colorB, trial.correctIsA);

    // Record outcome and check for learning
    s.validator.recordOutcome(correct);
    if (s.validator.hasLearned()) {
        s.puzzle_complete = true;
        emit(EventType::PUZZLE_COMPLETE, "* enen learned: pick the bigger one. Color doesn't matter.");
        return true;
    }
    return false;
}

//=============================================================================
// Puzzle 2: Feature Interaction (circles safe, blue squares safest)
//=============================================================================
bool Game::runPuzzle2Trial() {
    auto& s = state_;

    // First trial is adversarial (blue square vs circle — tests the exception)
    bool adversarial = s.validator.isFirstTrial();
    s.current_shape = ShapeTrial::generate(s.rng, adversarial);
    const auto& trial = s.current_shape;

    // Honest evaluation
    bool choseA = s.feat_net.chooseA(trial.colorA, trial.shapeA, trial.colorB, trial.shapeB);
    bool correct = (choseA == trial.correctIsA);

    char msg[128];
    snprintf(msg, sizeof(msg), "enen sees %s %s vs %s %s",
             ShapeTrial::colorName(trial.colorA), ShapeTrial::shapeName(trial.shapeA),
             ShapeTrial::colorName(trial.colorB), ShapeTrial::shapeName(trial.shapeB));
    emit(EventType::TRIAL_START, msg);

    snprintf(msg, sizeof(msg), "enen picks %c (%s %s)",
             choseA ? 'A' : 'B',
             ShapeTrial::colorName(choseA ? trial.colorA : trial.colorB),
             ShapeTrial::shapeName(choseA ? trial.shapeA : trial.shapeB));
    emit(EventType::CHOICE_MADE, msg);

    // Explain why correct or wrong based on the new rule
    if (correct) {
        int16_t pickedColor = choseA ? trial.colorA : trial.colorB;
        int16_t pickedShape = choseA ? trial.shapeA : trial.shapeB;
        if (!ShapeTrial::isCircle(pickedShape) && ShapeTrial::isBlue(pickedColor)) {
            emit(EventType::OUTCOME, "SAFE! Blue square is the best choice.", true);
        } else if (ShapeTrial::isCircle(pickedShape)) {
            emit(EventType::OUTCOME, "SAFE! Circle is a good choice.", true);
        } else {
            emit(EventType::OUTCOME, "SAFE! Correct choice.", true);
        }
    } else {
        emit(EventType::OUTCOME, "DANGER! Wrong choice.", false);
    }

    // Always learn
    s.feat_net.learn(trial.colorA, trial.shapeA, trial.colorB, trial.shapeB, trial.correctIsA);

    s.validator.recordOutcome(correct);
    if (s.validator.hasLearned()) {
        s.puzzle_complete = true;
        emit(EventType::PUZZLE_COMPLETE, "* enen learned: circles are safe, but blue squares are even better.");
        return true;
    }
    return false;
}

//=============================================================================
// Puzzle 3: XOR (Context-dependent choice)
//=============================================================================
bool Game::runPuzzle3Trial() {
    auto& s = state_;
    s.current_xor = XORTrial::generate(s.rng);
    const auto& trial = s.current_xor;

    // enen predicts safety — honest evaluation
    bool predictedSafe = s.xor_net.isSafe(trial.lightInput(), trial.pathInput());
    bool correct = (predictedSafe == trial.isSafe);

    char msg[128];
    snprintf(msg, sizeof(msg), "Light is %s, path is %s",
             trial.lightOn ? "ON" : "OFF",
             trial.choosingRight ? "RIGHT" : "LEFT");
    emit(EventType::TRIAL_START, msg);

    snprintf(msg, sizeof(msg), "enen predicts: %s", predictedSafe ? "SAFE" : "DANGER");
    emit(EventType::CHOICE_MADE, msg);

    if (correct) {
        emit(EventType::OUTCOME, "Correct prediction!", true);
    } else {
        emit(EventType::OUTCOME, "Wrong prediction!", false);
    }

    // Always learn
    s.xor_net.learn(trial.lightInput(), trial.pathInput(), trial.isSafe);

    s.validator.recordOutcome(correct);
    if (s.validator.hasLearned()) {
        s.puzzle_complete = true;
        emit(EventType::PUZZLE_COMPLETE, "* enen learned: the light changes which path is safe.");
        return true;
    }
    return false;
}

//=============================================================================
// Puzzle 4: Sequence (A then B)
//=============================================================================
bool Game::runPuzzle4Trial() {
    auto& s = state_;

    int16_t last = s.seq_puzzle.lastActionInput();
    // IntgrNN network decides based on last_action alone
    int action = s.seq_net.chooseAction(last);

    char msg[128];
    snprintf(msg, sizeof(msg), "enen presses %c", action == 0 ? 'A' : 'B');
    emit(EventType::CHOICE_MADE, msg);

    s.seq_puzzle.pressButton(action);

    if (s.seq_puzzle.isSuccess()) {
        // Honest evaluation — if enen succeeded, it succeeded
        emit(EventType::OUTCOME, "SUCCESS! Door opens!", true);
        s.seq_net.learnFromOutcome(last, action, true);
        s.validator.recordOutcome(true);
        s.seq_puzzle.reset();

        if (s.validator.hasLearned()) {
            s.puzzle_complete = true;
            emit(EventType::PUZZLE_COMPLETE, "* enen learned: press A first, then press B.");
            return true;
        }
    } else if (s.seq_puzzle.isFail()) {
        emit(EventType::OUTCOME, "FAIL! Wrong order!", false);
        s.seq_net.learnFromOutcome(last, action, false);
        s.validator.recordOutcome(false);
        s.seq_puzzle.reset();
    } else {
        // In progress (pressed A, now need B)
        emit(EventType::OUTCOME, "Good start — now press B", true);
        s.seq_net.learnFromOutcome(last, action, true);
    }
    return false;
}

//=============================================================================
// Puzzle 5: Composition Gauntlet
// 10 warmup trials (learning), then 20 scored trials
//=============================================================================
bool Game::runPuzzle5Trial() {
    auto& s = state_;

    // Check if gauntlet already complete
    if (s.gauntlet.isComplete()) {
        s.puzzle_complete = true;
        return true;
    }

    s.current_composition = CompositionTrial::generate(s.rng);
    const auto& trial = s.current_composition;

    bool choseA = s.comp_net.chooseA(trial.lightInput(), trial.sizeA, trial.sizeB);
    bool correct = (choseA == trial.correctIsA);

    char msg[128];

    if (s.gauntlet.inWarmup()) {
        snprintf(msg, sizeof(msg), "Warmup %d/%d: Light %s, sizes %d vs %d",
                 s.gauntlet.warmup_completed + 1, GauntletState::WARMUP_TRIALS,
                 trial.lightOn ? "ON" : "OFF", trial.sizeA, trial.sizeB);
    } else {
        snprintf(msg, sizeof(msg), "Scored %d/%d: Light %s, sizes %d vs %d",
                 s.gauntlet.scored_completed + 1, GauntletState::SCORED_TRIALS,
                 trial.lightOn ? "ON" : "OFF", trial.sizeA, trial.sizeB);
    }
    emit(EventType::TRIAL_START, msg);

    snprintf(msg, sizeof(msg), "enen picks %c — %s",
             choseA ? 'A' : 'B', correct ? "CORRECT!" : "WRONG!");
    emit(EventType::OUTCOME, msg, correct);

    // Always learn (this is the key - training happens here)
    s.comp_net.learn(trial.lightInput(), trial.sizeA, trial.sizeB, trial.correctIsA);

    // Record in gauntlet
    s.gauntlet.recordOutcome(correct);

    // Check for completion
    if (s.gauntlet.isComplete()) {
        s.puzzle_complete = true;
        snprintf(msg, sizeof(msg), "GAUNTLET COMPLETE! Score: %d/%d (%d%%)",
                 s.gauntlet.correct, GauntletState::SCORED_TRIALS,
                 s.gauntlet.scorePercent());
        emit(EventType::PUZZLE_COMPLETE, msg);
        return true;
    }
    return false;
}

} // namespace enen
