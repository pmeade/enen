/**
 * enen Demo: Auto-Run for Video Recording
 *
 * Outputs asciinema v2 format to stdout.
 * Usage:
 *   ./enen_autorun > demo.cast
 *   agg demo.cast demo.mp4
 *
 * This file orchestrates the demo. Screen rendering is delegated to:
 * - screens.hpp: Intro and victory screens
 * - brain_diagram.hpp: Neural network visualization
 * - history.hpp: Trial history display
 * - layout.hpp: Screen coordinates and timing
 * - frame.hpp: TextBuffer and frame output
 */

#include "networks.hpp"
#include "puzzles.hpp"
#include "frame.hpp"
#include "layout.hpp"
#include "brain_diagram.hpp"
#include "history.hpp"
#include "screens.hpp"
#include <cstdio>

using namespace enen;

//=============================================================================
// Puzzle Trial Renderers
//
// Each puzzle has a specific trial display showing:
// - Header with title, rule, progress
// - Current trial details
// - Result (correct/wrong)
// - History of recent trials
// - Brain diagram
//=============================================================================

namespace {

void renderPuzzle1Trial(TextBuffer& buffer, const MushroomTrial& trial,
                        bool choseA, bool correct, const History& history,
                        int trialNum, int successes, size_t bytes, bool complete) {
    buffer.clear();

    // Header
    buffer.putString(0, layout::header::TITLE_Y, "ENEN DEMO: SIZE");
    buffer.drawHLine(0, layout::header::DIVIDER_Y, layout::LEFT_COLUMN_WIDTH, '=');
    buffer.putString(0, layout::header::RULE_Y, "Rule: Bigger is safe. Ignore color.");
    buffer.putString(0, layout::header::PROGRESS_Y, "Progress: ");
    drawProgressBar(buffer, layout::header::PROGRESS_BAR_X, layout::header::PROGRESS_Y, successes, 4);
    char countBuf[16];
    std::snprintf(countBuf, sizeof(countBuf), " %d/4", successes);
    buffer.putString(layout::header::PROGRESS_COUNT_X, layout::header::PROGRESS_Y, countBuf);
    buffer.drawHLine(0, layout::header::SECTION_END_Y, layout::LEFT_COLUMN_WIDTH, '-');

    // Brain diagram
    drawBrainDiagram(buffer, layout::brain::X, layout::brain::Y,
                     PuzzleType::GENERALIZATION, bytes);

    // Trial details
    char lineBuf[64];
    std::snprintf(lineBuf, sizeof(lineBuf), "TRIAL %d:", trialNum);
    buffer.putString(0, layout::trial::LABEL_Y, lineBuf);

    std::snprintf(lineBuf, sizeof(lineBuf), "  [A] %s, size %d",
                  MushroomTrial::colorName(trial.colorA), trial.sizeA);
    buffer.putString(0, layout::trial::OPTION_A_Y, lineBuf);

    std::snprintf(lineBuf, sizeof(lineBuf), "  [B] %s, size %d",
                  MushroomTrial::colorName(trial.colorB), trial.sizeB);
    buffer.putString(0, layout::trial::OPTION_B_Y, lineBuf);

    bool aIsLarger = trial.sizeA > trial.sizeB;
    std::snprintf(lineBuf, sizeof(lineBuf), "  Pick: %c (%s)",
                  choseA ? 'A' : 'B', (choseA == aIsLarger) ? "larger" : "smaller");
    buffer.putString(0, layout::trial::PICK_Y, lineBuf);
    buffer.putString(0, layout::trial::RESULT_Y, correct ? "  [OK] CORRECT" : "  [X] WRONG");

    // History
    buffer.drawHLine(0, layout::history::DIVIDER_Y, layout::LEFT_COLUMN_WIDTH, '-');
    drawHistory(buffer, history, layout::history::LABEL_Y);

    // Completion message
    if (complete) {
        buffer.putString(0, layout::completion::MESSAGE_Y, "enen learned: bigger is always safe.");
    }

    // Footer
    buffer.drawHLine(0, layout::footer::DIVIDER_Y, terminal::WIDTH, '-');
    buffer.putString(0, layout::footer::CONTROLS_Y,
                     complete ? "[Enter] Continue    [Q] Quit" : "[Space] Next Trial    [Q] Quit");
}

void renderPuzzle2Trial(TextBuffer& buffer, const ShapeTrial& trial,
                        bool choseA, bool correct, const History& history,
                        int trialNum, int successes, size_t bytes, bool complete) {
    buffer.clear();

    buffer.putString(0, layout::header::TITLE_Y, "ENEN DEMO: EXCEPTIONS");
    buffer.drawHLine(0, layout::header::DIVIDER_Y, layout::LEFT_COLUMN_WIDTH, '=');
    buffer.putString(0, layout::header::RULE_Y, "Rule: Circle safe. Blue square best.");
    buffer.putString(0, layout::header::PROGRESS_Y, "Progress: ");
    drawProgressBar(buffer, layout::header::PROGRESS_BAR_X, layout::header::PROGRESS_Y, successes, 4);
    char countBuf[16];
    std::snprintf(countBuf, sizeof(countBuf), " %d/4", successes);
    buffer.putString(layout::header::PROGRESS_COUNT_X, layout::header::PROGRESS_Y, countBuf);
    buffer.drawHLine(0, layout::header::SECTION_END_Y, layout::LEFT_COLUMN_WIDTH, '-');

    drawBrainDiagram(buffer, layout::brain::X, layout::brain::Y,
                     PuzzleType::FEATURE_SELECTION, bytes);

    char lineBuf[64];
    std::snprintf(lineBuf, sizeof(lineBuf), "TRIAL %d:", trialNum);
    buffer.putString(0, layout::trial::LABEL_Y, lineBuf);

    std::snprintf(lineBuf, sizeof(lineBuf), "  [A] %s %s",
                  ShapeTrial::colorName(trial.colorA), ShapeTrial::shapeName(trial.shapeA));
    buffer.putString(0, layout::trial::OPTION_A_Y, lineBuf);

    std::snprintf(lineBuf, sizeof(lineBuf), "  [B] %s %s",
                  ShapeTrial::colorName(trial.colorB), ShapeTrial::shapeName(trial.shapeB));
    buffer.putString(0, layout::trial::OPTION_B_Y, lineBuf);

    int16_t pickedColor = choseA ? trial.colorA : trial.colorB;
    int16_t pickedShape = choseA ? trial.shapeA : trial.shapeB;
    std::snprintf(lineBuf, sizeof(lineBuf), "  Pick: %c (%s %s)",
                  choseA ? 'A' : 'B',
                  ShapeTrial::colorName(pickedColor),
                  ShapeTrial::shapeName(pickedShape));
    buffer.putString(0, layout::trial::PICK_Y, lineBuf);
    buffer.putString(0, layout::trial::RESULT_Y, correct ? "  [OK] CORRECT" : "  [X] WRONG");

    buffer.drawHLine(0, layout::history::DIVIDER_Y, layout::LEFT_COLUMN_WIDTH, '-');
    drawHistory(buffer, history, layout::history::LABEL_Y);

    if (complete) {
        buffer.putString(0, layout::completion::MESSAGE_Y,
                         "enen learned: circles safe, blue squares best.");
    }

    buffer.drawHLine(0, layout::footer::DIVIDER_Y, terminal::WIDTH, '-');
    buffer.putString(0, layout::footer::CONTROLS_Y,
                     complete ? "[Enter] Continue    [Q] Quit" : "[Space] Next Trial    [Q] Quit");
}

void renderPuzzle3Trial(TextBuffer& buffer, const XORTrial& trial,
                        bool predictedSafe, bool correct, const History& history,
                        int trialNum, int successes, size_t bytes, bool complete) {
    buffer.clear();

    buffer.putString(0, layout::header::TITLE_Y, "ENEN DEMO: CONTEXT");
    buffer.drawHLine(0, layout::header::DIVIDER_Y, layout::LEFT_COLUMN_WIDTH, '=');
    buffer.putString(0, layout::header::RULE_Y, "Rule: ON=left, OFF=right.");
    buffer.putString(0, layout::header::PROGRESS_Y, "Progress: ");
    drawProgressBar(buffer, layout::header::PROGRESS_BAR_X, layout::header::PROGRESS_Y, successes, 4);
    char countBuf[16];
    std::snprintf(countBuf, sizeof(countBuf), " %d/4", successes);
    buffer.putString(layout::header::PROGRESS_COUNT_X, layout::header::PROGRESS_Y, countBuf);
    buffer.drawHLine(0, layout::header::SECTION_END_Y, layout::LEFT_COLUMN_WIDTH, '-');

    drawBrainDiagram(buffer, layout::brain::X, layout::brain::Y,
                     PuzzleType::XOR_CONTEXT, bytes);

    char lineBuf[64];
    std::snprintf(lineBuf, sizeof(lineBuf), "TRIAL %d:", trialNum);
    buffer.putString(0, layout::trial::LABEL_Y, lineBuf);

    std::snprintf(lineBuf, sizeof(lineBuf), "  Scenario: Light %s, Path %s",
                  trial.lightOn ? "ON" : "OFF", trial.choosingRight ? "RIGHT" : "LEFT");
    buffer.putString(0, layout::trial::OPTION_A_Y, lineBuf);

    std::snprintf(lineBuf, sizeof(lineBuf), "  enen predicts: %s",
                  predictedSafe ? "SAFE" : "DANGER");
    buffer.putString(0, layout::trial::OPTION_B_Y, lineBuf);

    std::snprintf(lineBuf, sizeof(lineBuf), "  Reality: %s - %s",
                  trial.isSafe ? "SAFE" : "DANGER",
                  correct ? "[OK] Correct!" : "[X] Wrong!");
    buffer.putString(0, layout::trial::PICK_Y, lineBuf);

    buffer.drawHLine(0, 11, layout::LEFT_COLUMN_WIDTH, '-');
    drawHistory(buffer, history, 12);

    if (complete) {
        buffer.putString(0, 17, "enen learned: the light changes which path is safe.");
    }

    buffer.drawHLine(0, layout::footer::DIVIDER_Y, terminal::WIDTH, '-');
    buffer.putString(0, layout::footer::CONTROLS_Y,
                     complete ? "[Enter] Continue    [Q] Quit" : "[Space] Next Trial    [Q] Quit");
}

void renderPuzzle4Trial(TextBuffer& buffer, int action, bool success, bool inProgress,
                        const History& history, int trialNum, int successes,
                        size_t bytes, bool complete) {
    buffer.clear();

    buffer.putString(0, layout::header::TITLE_Y, "ENEN DEMO: ORDER");
    buffer.drawHLine(0, layout::header::DIVIDER_Y, layout::LEFT_COLUMN_WIDTH, '=');
    buffer.putString(0, layout::header::RULE_Y, "Rule: A first, then B.");
    buffer.putString(0, layout::header::PROGRESS_Y, "Progress: ");
    drawProgressBar(buffer, layout::header::PROGRESS_BAR_X, layout::header::PROGRESS_Y, successes, 4);
    char countBuf[16];
    std::snprintf(countBuf, sizeof(countBuf), " %d/4", successes);
    buffer.putString(layout::header::PROGRESS_COUNT_X, layout::header::PROGRESS_Y, countBuf);
    buffer.drawHLine(0, layout::header::SECTION_END_Y, layout::LEFT_COLUMN_WIDTH, '-');

    drawBrainDiagram(buffer, layout::brain::X, layout::brain::Y,
                     PuzzleType::SEQUENCE, bytes);

    char lineBuf[64];
    std::snprintf(lineBuf, sizeof(lineBuf), "TRIAL %d:", trialNum);
    buffer.putString(0, layout::trial::LABEL_Y, lineBuf);

    std::snprintf(lineBuf, sizeof(lineBuf), "  enen presses: %c", action == 0 ? 'A' : 'B');
    buffer.putString(0, layout::trial::OPTION_A_Y, lineBuf);

    if (inProgress) {
        buffer.putString(0, layout::trial::OPTION_B_Y, "  Good start...");
    } else if (success) {
        buffer.putString(0, layout::trial::OPTION_B_Y, "  [OK] Door opens!");
    } else {
        buffer.putString(0, layout::trial::OPTION_B_Y, "  [X] Wrong order!");
    }

    buffer.drawHLine(0, layout::history::DIVIDER_Y, layout::LEFT_COLUMN_WIDTH, '-');
    drawHistory(buffer, history, layout::history::LABEL_Y);

    if (complete) {
        buffer.putString(0, layout::completion::MESSAGE_Y, "enen learned: A first, then B.");
    }

    buffer.drawHLine(0, layout::footer::DIVIDER_Y, terminal::WIDTH, '-');
    buffer.putString(0, layout::footer::CONTROLS_Y,
                     complete ? "[Enter] Continue    [Q] Quit" : "[Space] Next Trial    [Q] Quit");
}

void renderPuzzle5Trial(TextBuffer& buffer, const CompositionTrial& trial,
                        bool choseA, bool correct, const History& history,
                        const GauntletState& gauntlet, size_t bytes, bool complete) {
    buffer.clear();

    buffer.putString(0, layout::header::TITLE_Y, "ENEN DEMO: EVERYTHING");
    buffer.drawHLine(0, layout::header::DIVIDER_Y, layout::LEFT_COLUMN_WIDTH, '=');
    buffer.putString(0, layout::header::RULE_Y, "Rule: ON=bigger, OFF=smaller.");

    char phaseBuf[48];
    if (gauntlet.inWarmup()) {
        std::snprintf(phaseBuf, sizeof(phaseBuf), "Phase: WARMUP %d/%d",
                      gauntlet.warmup_completed, GauntletState::WARMUP_TRIALS);
    } else {
        std::snprintf(phaseBuf, sizeof(phaseBuf), "Phase: SCORED %d/%d",
                      gauntlet.scored_completed, GauntletState::SCORED_TRIALS);
    }
    buffer.putString(0, layout::header::PROGRESS_Y, phaseBuf);

    if (!gauntlet.inWarmup()) {
        char scoreBuf[48];
        std::snprintf(scoreBuf, sizeof(scoreBuf), "Score: %d/%d (%d%%)",
                      gauntlet.correct, gauntlet.scored_completed, gauntlet.scorePercent());
        buffer.putString(0, layout::header::SECTION_END_Y, scoreBuf);
    }
    buffer.drawHLine(0, 5, layout::LEFT_COLUMN_WIDTH, '-');

    drawBrainDiagram(buffer, layout::brain::X, layout::brain::Y,
                     PuzzleType::COMPOSITION, bytes);

    int trialNum = gauntlet.currentTrials();
    char lineBuf[64];
    std::snprintf(lineBuf, sizeof(lineBuf), "TRIAL %d:", trialNum);
    buffer.putString(0, 7, lineBuf);

    std::snprintf(lineBuf, sizeof(lineBuf), "  Light: %s -> pick %s",
                  trial.lightOn ? "ON" : "OFF", trial.lightOn ? "LARGER" : "SMALLER");
    buffer.putString(0, 8, lineBuf);

    std::snprintf(lineBuf, sizeof(lineBuf), "  [A] size %d", trial.sizeA);
    buffer.putString(0, 9, lineBuf);

    std::snprintf(lineBuf, sizeof(lineBuf), "  [B] size %d", trial.sizeB);
    buffer.putString(0, 10, lineBuf);

    bool aIsLarger = trial.sizeA > trial.sizeB;
    std::snprintf(lineBuf, sizeof(lineBuf), "  Pick: %c (%s)",
                  choseA ? 'A' : 'B',
                  (choseA ? aIsLarger : !aIsLarger) ? "larger" : "smaller");
    buffer.putString(0, 11, lineBuf);
    buffer.putString(0, 12, correct ? "  [OK] CORRECT" : "  [X] WRONG");

    buffer.drawHLine(0, 14, layout::LEFT_COLUMN_WIDTH, '-');
    drawHistory(buffer, history, 15);

    if (complete) {
        buffer.putString(0, 19, "enen learned: ON=bigger, OFF=smaller.");
        char finalBuf[48];
        std::snprintf(finalBuf, sizeof(finalBuf), "Final score: %d/%d (%d%%)",
                      gauntlet.correct, GauntletState::SCORED_TRIALS, gauntlet.scorePercent());
        buffer.putString(0, 20, finalBuf);
    }

    buffer.drawHLine(0, layout::footer::DIVIDER_Y, terminal::WIDTH, '-');
    buffer.putString(0, layout::footer::CONTROLS_Y,
                     complete ? "[Enter] to see final results..." : "[Space] Next Trial    [Q] Quit");
}

} // anonymous namespace

//=============================================================================
// Puzzle Runners
//
// Each puzzle follows the same pattern:
// 1. Show puzzle intro
// 2. Loop until learned: generate trial, evaluate, learn, render, output
// 3. Use adaptive timing based on correctness
//=============================================================================

void runPuzzle1(FrameWriter& writer, TextBuffer& buffer, RNG& rng,
                GeneralizationNet& net, History& history, LearningValidator& validator) {
    renderPuzzleIntro(buffer, PuzzleType::GENERALIZATION);
    writer.outputFrame(buffer, timing::PUZZLE_INTRO);

    validator.reset();
    history.clear();

    while (!validator.hasLearned()) {
        bool adversarial = validator.isFirstTrial();
        auto trial = MushroomTrial::generate(rng, adversarial);

        bool choseA = net.chooseA(trial.sizeA, trial.sizeB, trial.colorA, trial.colorB);
        bool correct = (choseA == trial.correctIsA);

        net.learn(trial.sizeA, trial.sizeB, trial.colorA, trial.colorB, trial.correctIsA);
        validator.recordOutcome(correct);

        char summary[48];
        std::snprintf(summary, sizeof(summary), "%s(%d) vs %s(%d)",
                      MushroomTrial::colorName(trial.colorA), trial.sizeA,
                      MushroomTrial::colorName(trial.colorB), trial.sizeB);
        history.add(validator.total_trials, correct, summary);

        bool complete = validator.hasLearned();
        bool isFirst = (validator.total_trials == 1);
        double pause = calculateTrialTiming(complete, isFirst, correct);

        renderPuzzle1Trial(buffer, trial, choseA, correct, history,
                           validator.total_trials, validator.successes,
                           net.modelSizeBytes(), complete);
        writer.outputFrame(buffer, pause);
    }
}

void runPuzzle2(FrameWriter& writer, TextBuffer& buffer, RNG& rng,
                FeatureSelectionNet& net, History& history, LearningValidator& validator) {
    renderPuzzleIntro(buffer, PuzzleType::FEATURE_SELECTION);
    writer.outputFrame(buffer, timing::PUZZLE_INTRO);

    validator.reset();
    history.clear();

    while (!validator.hasLearned()) {
        bool adversarial = validator.isFirstTrial();
        auto trial = ShapeTrial::generate(rng, adversarial);

        bool choseA = net.chooseA(trial.colorA, trial.shapeA, trial.colorB, trial.shapeB);
        bool correct = (choseA == trial.correctIsA);

        net.learn(trial.colorA, trial.shapeA, trial.colorB, trial.shapeB, trial.correctIsA);
        validator.recordOutcome(correct);

        char summary[48];
        std::snprintf(summary, sizeof(summary), "%s %s vs %s %s",
                      ShapeTrial::colorName(trial.colorA), ShapeTrial::shapeName(trial.shapeA),
                      ShapeTrial::colorName(trial.colorB), ShapeTrial::shapeName(trial.shapeB));
        history.add(validator.total_trials, correct, summary);

        bool complete = validator.hasLearned();
        bool isFirst = (validator.total_trials == 1);
        double pause = calculateTrialTiming(complete, isFirst, correct);

        renderPuzzle2Trial(buffer, trial, choseA, correct, history,
                           validator.total_trials, validator.successes,
                           net.modelSizeBytes(), complete);
        writer.outputFrame(buffer, pause);
    }
}

void runPuzzle3(FrameWriter& writer, TextBuffer& buffer, RNG& rng,
                XORNet& net, History& history, LearningValidator& validator) {
    renderPuzzleIntro(buffer, PuzzleType::XOR_CONTEXT);
    writer.outputFrame(buffer, timing::PUZZLE_INTRO);

    validator.reset();
    history.clear();

    while (!validator.hasLearned()) {
        auto trial = XORTrial::generate(rng);

        bool predictedSafe = net.isSafe(trial.lightInput(), trial.pathInput());
        bool correct = (predictedSafe == trial.isSafe);

        net.learn(trial.lightInput(), trial.pathInput(), trial.isSafe);
        validator.recordOutcome(correct);

        char summary[48];
        std::snprintf(summary, sizeof(summary), "pred %s, was %s",
                      predictedSafe ? "safe" : "danger",
                      trial.isSafe ? "safe" : "danger");
        history.add(validator.total_trials, correct, summary);

        bool complete = validator.hasLearned();
        bool isFirst = (validator.total_trials == 1);
        double pause = calculateTrialTiming(complete, isFirst, correct);

        renderPuzzle3Trial(buffer, trial, predictedSafe, correct, history,
                           validator.total_trials, validator.successes,
                           net.modelSizeBytes(), complete);
        writer.outputFrame(buffer, pause);
    }
}

void runPuzzle4(FrameWriter& writer, TextBuffer& buffer,
                SequenceNet& net, History& history, LearningValidator& validator) {
    renderPuzzleIntro(buffer, PuzzleType::SEQUENCE);
    writer.outputFrame(buffer, timing::PUZZLE_INTRO);

    validator.reset();
    history.clear();
    SequencePuzzle puzzle;

    while (!validator.hasLearned()) {
        int16_t last = puzzle.lastActionInput();
        int action = net.chooseAction(last);
        puzzle.pressButton(action);

        bool success = false;
        bool inProgress = false;

        if (puzzle.isSuccess()) {
            success = true;
            net.learnFromOutcome(last, action, true);
            validator.recordOutcome(true);
            history.add(validator.total_trials, true, "A->B SUCCESS");
            puzzle.reset();
        } else if (puzzle.isFail()) {
            net.learnFromOutcome(last, action, false);
            validator.recordOutcome(false);
            const char* msg = (action == 1) ? "B first FAIL" : "A->A FAIL";
            history.add(validator.total_trials, false, msg);
            puzzle.reset();
        } else {
            inProgress = true;
            net.learnFromOutcome(last, action, true);
        }

        bool complete = validator.hasLearned();
        bool isFirst = (validator.total_trials == 1 && !inProgress);
        double pause = inProgress ? timing::SEQUENCE_STEP
                     : calculateTrialTiming(complete, isFirst, success);

        renderPuzzle4Trial(buffer, action, success, inProgress, history,
                           validator.total_trials, validator.successes,
                           net.modelSizeBytes(), complete);
        writer.outputFrame(buffer, pause);
    }
}

void runPuzzle5(FrameWriter& writer, TextBuffer& buffer, RNG& rng,
                CompositionNet& net, History& history, GauntletState& gauntlet) {
    renderPuzzleIntro(buffer, PuzzleType::COMPOSITION);
    writer.outputFrame(buffer, timing::PUZZLE_INTRO);

    gauntlet.reset();
    history.clear();

    while (!gauntlet.isComplete()) {
        auto trial = CompositionTrial::generate(rng);

        bool choseA = net.chooseA(trial.lightInput(), trial.sizeA, trial.sizeB);
        bool correct = (choseA == trial.correctIsA);

        net.learn(trial.lightInput(), trial.sizeA, trial.sizeB, trial.correctIsA);
        gauntlet.recordOutcome(correct);

        char summary[48];
        bool aLarger = trial.sizeA > trial.sizeB;
        std::snprintf(summary, sizeof(summary), "%s - %c(%d) %s %c(%d)",
                      trial.lightOn ? "ON" : "OFF",
                      choseA ? 'A' : 'B', choseA ? trial.sizeA : trial.sizeB,
                      (choseA ? aLarger : !aLarger) ? ">" : "<",
                      choseA ? 'B' : 'A', choseA ? trial.sizeB : trial.sizeA);
        history.add(gauntlet.currentTrials(), correct, summary);

        bool complete = gauntlet.isComplete();
        bool isFirst = (gauntlet.currentTrials() == 1);
        double pause = calculateTrialTiming(complete, isFirst, correct);

        renderPuzzle5Trial(buffer, trial, choseA, correct, history,
                           gauntlet, net.modelSizeBytes(), complete);
        writer.outputFrame(buffer, pause);
    }
}

//=============================================================================
// Main - Orchestrates the complete demo
//=============================================================================
int main() {
    // Fixed seed for reproducible demo
    RNG rng(42);

    // Neural networks for each puzzle
    GeneralizationNet genNet;
    FeatureSelectionNet featNet;
    XORNet xorNet;
    SequenceNet seqNet;
    CompositionNet compNet;

    // Shared state
    LearningValidator validator;
    GauntletState gauntlet;
    History history;
    TextBuffer buffer;
    FrameWriter writer;

    size_t totalBytes = totalModelSize(genNet, featNet, xorNet, seqNet, compNet);

    // Output asciinema header
    writer.writeHeader();

    // Two-part intro
    renderIntro1(buffer, totalBytes);
    writer.outputFrame(buffer, timing::INTRO_1);

    renderIntro2(buffer);
    writer.outputFrame(buffer, timing::INTRO_2);

    // Run all five puzzles
    runPuzzle1(writer, buffer, rng, genNet, history, validator);
    runPuzzle2(writer, buffer, rng, featNet, history, validator);
    runPuzzle3(writer, buffer, rng, xorNet, history, validator);
    runPuzzle4(writer, buffer, seqNet, history, validator);
    runPuzzle5(writer, buffer, rng, compNet, history, gauntlet);

    // Victory screen
    renderVictory(buffer, totalBytes, gauntlet.correct, GauntletState::SCORED_TRIALS);
    writer.outputFrame(buffer, timing::VICTORY);

    return 0;
}
