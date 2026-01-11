/**
 * Terminal renderer implementation for enen Demo
 * Clean, decision-focused layout for video recording
 */

#include "renderer.hpp"
#include <cstdio>
#include <cstring>
#include <algorithm>

namespace enen {

//=============================================================================
// TrialHistory
//=============================================================================

void TrialHistory::add(int trial_num, bool correct, const std::string& summary) {
    entries_.push_back({trial_num, correct, summary});
    while (entries_.size() > MAX_ENTRIES) {
        entries_.erase(entries_.begin());
    }
}

//=============================================================================
// Renderer - Setup
//=============================================================================

Renderer::Renderer() {
    clearBuffer();
}

Renderer::~Renderer() {
    cleanup();
}

void Renderer::init() {
    printf("\033[?25l");  // Hide cursor
    printf("\033[2J\033[H");  // Clear screen
    fflush(stdout);
}

void Renderer::cleanup() {
    printf("\033[?25h");  // Show cursor
    fflush(stdout);
}

void Renderer::clear() {
    printf("\033[2J\033[H");
}

//=============================================================================
// Drawing Primitives
//=============================================================================

void Renderer::clearBuffer() {
    for (int y = 0; y < TERM_HEIGHT; y++) {
        memset(buffer_[y], ' ', TERM_WIDTH);
        buffer_[y][TERM_WIDTH] = '\0';
    }
}

void Renderer::putString(int x, int y, const char* str) {
    if (y < 0 || y >= TERM_HEIGHT) return;
    int len = strlen(str);
    for (int i = 0; i < len && x + i < TERM_WIDTH; i++) {
        if (x + i >= 0) {
            buffer_[y][x + i] = str[i];
        }
    }
}

void Renderer::putChar(int x, int y, char c) {
    if (y >= 0 && y < TERM_HEIGHT && x >= 0 && x < TERM_WIDTH) {
        buffer_[y][x] = c;
    }
}

void Renderer::drawHLine(int x, int y, int len, char c) {
    for (int i = 0; i < len; i++) {
        putChar(x + i, y, c);
    }
}

void Renderer::flush() {
    printf("\033[H");  // Home cursor
    for (int y = 0; y < TERM_HEIGHT; y++) {
        printf("%s\n", buffer_[y]);
    }
    fflush(stdout);
}

//=============================================================================
// Layout Components
//=============================================================================

void Renderer::drawProgressBar(int x, int y, int width, int value, int max) {
    int filled = (max > 0) ? (value * width / max) : 0;
    if (filled > width) filled = width;
    putChar(x, y, '[');
    for (int i = 0; i < width; i++) {
        putChar(x + 1 + i, y, i < filled ? '#' : '.');
    }
    putChar(x + width + 1, y, ']');
}

void Renderer::drawHeader(const char* puzzleName, const char* rule,
                          const IntgrNNWrapper& net, const char* arch,
                          int successes, int required) {
    char buf[TERM_WIDTH + 1];

    // Line 0: Title
    snprintf(buf, sizeof(buf), "ENEN DEMO: %s", puzzleName);
    putString(0, 0, buf);

    // Line 1: Separator (left side only, brain box goes on right)
    drawHLine(0, 1, 38, '=');

    // Line 2: Rule
    snprintf(buf, sizeof(buf), "Rule: %s", rule);
    putString(0, 2, buf);

    // Line 3: Progress
    putString(0, 3, "Progress: ");
    drawProgressBar(10, 3, 10, successes, required);
    snprintf(buf, sizeof(buf), " %d/%d", successes, required);
    putString(23, 3, buf);

    // Line 4: Separator (left side only)
    drawHLine(0, 4, 38, '-');

    (void)net;  // Brain box will show network info
    (void)arch;
}

void Renderer::drawGauntletHeader(const char* puzzleName, const char* rule,
                                   const IntgrNNWrapper& net, const char* arch,
                                   const GauntletState& gauntlet) {
    char buf[TERM_WIDTH + 1];

    // Line 0: Title
    snprintf(buf, sizeof(buf), "ENEN DEMO: %s", puzzleName);
    putString(0, 0, buf);

    // Line 1: Separator (left side only)
    drawHLine(0, 1, 38, '=');

    // Line 2: Rule
    snprintf(buf, sizeof(buf), "Rule: %s", rule);
    putString(0, 2, buf);

    // Line 3: Phase info
    if (gauntlet.inWarmup()) {
        snprintf(buf, sizeof(buf), "Phase: WARMUP %d/%d",
                 gauntlet.warmup_completed, GauntletState::WARMUP_TRIALS);
    } else {
        snprintf(buf, sizeof(buf), "Phase: SCORED %d/%d",
                 gauntlet.scored_completed, GauntletState::SCORED_TRIALS);
    }
    putString(0, 3, buf);

    // Line 4: Score (only if in scored phase)
    if (!gauntlet.inWarmup()) {
        snprintf(buf, sizeof(buf), "Score: %d/%d (%d%%)",
                 gauntlet.correct, gauntlet.scored_completed,
                 gauntlet.scorePercent());
        putString(0, 4, buf);
    }

    // Line 5: Separator (left side only)
    drawHLine(0, 5, 38, '-');

    (void)net;
    (void)arch;
}

void Renderer::drawHistory(const TrialHistory& history, int startY) {
    putString(0, startY, "HISTORY:");
    const auto& entries = history.entries();

    // Show most recent first (reverse order)
    for (size_t i = 0; i < entries.size() && (int)(startY + 1 + i) < TERM_HEIGHT - 2; i++) {
        size_t idx = entries.size() - 1 - i;
        const auto& e = entries[idx];
        char buf[TERM_WIDTH + 1];
        snprintf(buf, sizeof(buf), "  Trial %d: %s %s",
                 e.trial_num,
                 e.correct ? "[OK]" : "[X]",
                 e.summary.c_str());
        // Truncate if too long
        if (strlen(buf) > 50) buf[50] = '\0';
        putString(0, startY + 1 + i, buf);
    }
}

void Renderer::drawControls(int y, bool showContinue) {
    if (showContinue) {
        putString(0, y, "[Enter] Continue    [Q] Quit");
    } else {
        putString(0, y, "[Space] Next Trial    [Q] Quit");
    }
}

void Renderer::drawVisualBox(int x, int y, int w, int h) {
    // Top border
    putChar(x, y, '+');
    drawHLine(x + 1, y, w - 2, '-');
    putChar(x + w - 1, y, '+');

    // Sides
    for (int i = 1; i < h - 1; i++) {
        putChar(x, y + i, '|');
        putChar(x + w - 1, y + i, '|');
    }

    // Bottom border
    putChar(x, y + h - 1, '+');
    drawHLine(x + 1, y + h - 1, w - 2, '-');
    putChar(x + w - 1, y + h - 1, '+');
}

//=============================================================================
// Puzzle-Specific Visuals
//=============================================================================

void Renderer::drawSizeBars(int boxX, int boxY, int16_t sizeA, int16_t sizeB) {
    // Scale sizes to bar heights (1-4 chars)
    int hA = 1 + (sizeA - 32) / 32;
    int hB = 1 + (sizeB - 32) / 32;
    if (hA > 4) hA = 4;
    if (hB > 4) hB = 4;
    if (hA < 1) hA = 1;
    if (hB < 1) hB = 1;

    // Draw bars from bottom up
    int baseY = boxY + 4;
    for (int i = 0; i < hA; i++) {
        putChar(boxX + 3, baseY - i, '#');
    }
    for (int i = 0; i < hB; i++) {
        putChar(boxX + 9, baseY - i, '#');
    }

    // Labels
    putChar(boxX + 3, boxY + 5, 'A');
    putChar(boxX + 9, boxY + 5, 'B');
}

void Renderer::drawShapes(int boxX, int boxY, bool aIsCircle) {
    if (aIsCircle) {
        // A is circle, B is square
        putString(boxX + 2, boxY + 2, "O");
        putString(boxX + 8, boxY + 1, "+-+");
        putString(boxX + 8, boxY + 2, "| |");
        putString(boxX + 8, boxY + 3, "+-+");
    } else {
        // A is square, B is circle
        putString(boxX + 2, boxY + 1, "+-+");
        putString(boxX + 2, boxY + 2, "| |");
        putString(boxX + 2, boxY + 3, "+-+");
        putString(boxX + 9, boxY + 2, "O");
    }
    putChar(boxX + 3, boxY + 5, 'A');
    putChar(boxX + 9, boxY + 5, 'B');
}

void Renderer::drawLightAndPaths(int boxX, int boxY, bool lightOn) {
    // Light indicator
    if (lightOn) {
        putString(boxX + 4, boxY + 1, "(*)");
        putString(boxX + 4, boxY + 2, " ON");
    } else {
        putString(boxX + 4, boxY + 1, "( )");
        putString(boxX + 4, boxY + 2, "OFF");
    }

    // Path arrows
    putString(boxX + 1, boxY + 4, "<-A");
    putString(boxX + 7, boxY + 4, "B->");
}

void Renderer::drawButtons(int boxX, int boxY, int16_t scoreA, int16_t scoreB) {
    char buf[16];

    // Button A
    putString(boxX + 2, boxY + 1, "+-+");
    putString(boxX + 2, boxY + 2, "|A|");
    putString(boxX + 2, boxY + 3, "+-+");
    snprintf(buf, sizeof(buf), "%3d", scoreA);
    putString(boxX + 2, boxY + 4, buf);

    // Button B
    putString(boxX + 8, boxY + 1, "+-+");
    putString(boxX + 8, boxY + 2, "|B|");
    putString(boxX + 8, boxY + 3, "+-+");
    snprintf(buf, sizeof(buf), "%3d", scoreB);
    putString(boxX + 8, boxY + 4, buf);
}

void Renderer::drawLightAndSizes(int boxX, int boxY, bool lightOn, int16_t sizeA, int16_t sizeB) {
    // Light at top
    if (lightOn) {
        putString(boxX + 4, boxY + 1, "(*)");
        putString(boxX + 5, boxY + 2, "ON");
    } else {
        putString(boxX + 4, boxY + 1, "( )");
        putString(boxX + 4, boxY + 2, "OFF");
    }

    // Size bars below
    int hA = 1 + (sizeA - 32) / 32;
    int hB = 1 + (sizeB - 32) / 32;
    if (hA > 3) hA = 3;
    if (hB > 3) hB = 3;

    int baseY = boxY + 5;
    for (int i = 0; i < hA; i++) {
        putChar(boxX + 3, baseY - i, '#');
    }
    for (int i = 0; i < hB; i++) {
        putChar(boxX + 9, baseY - i, '#');
    }
    putChar(boxX + 3, boxY + 6, 'A');
    putChar(boxX + 9, boxY + 6, 'B');
}

void Renderer::drawBrainBox(int x, int y, PuzzleType puzzleType, size_t modelBytes) {
    char line[50];

    // Top border and title
    putString(x, y, "+---------------------------------------+");

    snprintf(line, sizeof(line), "| enen's brain (%zu bytes)", modelBytes);
    // Pad to fit the box width
    int len = strlen(line);
    while (len < 40) line[len++] = ' ';
    line[40] = '|';
    line[41] = '\0';
    putString(x, y + 1, line);

    putString(x, y + 2, "|                                       |");
    putString(x, y + 3, "| SEES         THINKS        DECIDES    |");
    putString(x, y + 4, "|                                       |");

    switch (puzzleType) {
        case PuzzleType::GENERALIZATION:
            putString(x, y + 5, "| size A  -+               +-> pick A   |");
            putString(x, y + 6, "| size B  -+-> 8 neurons --+            |");
            putString(x, y + 7, "| color A -+               +-> pick B   |");
            putString(x, y + 8, "| color B -+                            |");
            putString(x, y + 9, "+---------------------------------------+");
            break;

        case PuzzleType::FEATURE_SELECTION:
            putString(x, y + 5, "| color A -+               +-> pick A   |");
            putString(x, y + 6, "| shape A -+-> 8 neurons --+            |");
            putString(x, y + 7, "| color B -+               +-> pick B   |");
            putString(x, y + 8, "| shape B -+                            |");
            putString(x, y + 9, "+---------------------------------------+");
            break;

        case PuzzleType::XOR_CONTEXT:
            putString(x, y + 5, "| light   -+-> 4 neurons --> safe path  |");
            putString(x, y + 6, "| path    -+                            |");
            putString(x, y + 7, "|                                       |");
            putString(x, y + 8, "|                                       |");
            putString(x, y + 9, "+---------------------------------------+");
            break;

        case PuzzleType::SEQUENCE:
            putString(x, y + 5, "|                          +-> press A  |");
            putString(x, y + 6, "| last key --> 4 neurons --+            |");
            putString(x, y + 7, "|                          +-> press B  |");
            putString(x, y + 8, "|                                       |");
            putString(x, y + 9, "+---------------------------------------+");
            break;

        case PuzzleType::COMPOSITION:
            putString(x, y + 5, "| light  -+                +-> pick A   |");
            putString(x, y + 6, "| size A -+> 8 neurons > 4 neurons      |");
            putString(x, y + 7, "| size B -+                +-> pick B   |");
            putString(x, y + 8, "|                                       |");
            putString(x, y + 9, "| (two layers - this puzzle is harder)  |");
            putString(x, y + 10, "+---------------------------------------+");
            break;
    }
}

//=============================================================================
// Completion Message
//=============================================================================

void Renderer::drawCompletionMessage(int y, PuzzleType type, int gauntletScore, int gauntletTotal) {
    const char* message = "";

    switch (type) {
        case PuzzleType::GENERALIZATION:
            message = "enen learned: bigger is always safe.";
            break;
        case PuzzleType::FEATURE_SELECTION:
            message = "enen learned: circles safe, blue squares best.";
            break;
        case PuzzleType::XOR_CONTEXT:
            message = "enen learned: the light changes which path is safe.";
            break;
        case PuzzleType::SEQUENCE:
            message = "enen learned: A first, then B.";
            break;
        case PuzzleType::COMPOSITION:
            message = "enen learned: ON=bigger, OFF=smaller.";
            break;
    }

    putString(0, y, message);

    // For puzzle 5, also show final score
    if (type == PuzzleType::COMPOSITION && gauntletTotal > 0) {
        char scoreLine[40];
        snprintf(scoreLine, sizeof(scoreLine), "Final score: %d/%d (%d%%)",
                 gauntletScore, gauntletTotal, (gauntletScore * 100) / gauntletTotal);
        putString(0, y + 1, scoreLine);
    }
}

//=============================================================================
// Puzzle Draw Functions
//=============================================================================

void Renderer::drawPuzzle1(const MushroomTrial& trial, const GeneralizationNet& net,
                           bool choseA, bool correct,
                           const TrialHistory& history, int trial_num,
                           int successes, int required,
                           bool showContinue) {
    clearBuffer();

    drawHeader("SIZE", "Bigger is safe. Ignore color.",
               net, "4->8->1", successes, required);

    char buf[TERM_WIDTH + 1];

    // Brain box (right side)
    drawBrainBox(39, 0, PuzzleType::GENERALIZATION, net.modelSizeBytes());

    // Trial section (line 6-11)
    snprintf(buf, sizeof(buf), "TRIAL %d:", trial_num);
    putString(0, 6, buf);

    snprintf(buf, sizeof(buf), "  [A] %s, size %d",
             MushroomTrial::colorName(trial.colorA), trial.sizeA);
    putString(0, 7, buf);
    snprintf(buf, sizeof(buf), "  [B] %s, size %d",
             MushroomTrial::colorName(trial.colorB), trial.sizeB);
    putString(0, 8, buf);

    // Use passed choseA (what network chose BEFORE learning)
    bool aIsLarger = trial.sizeA > trial.sizeB;
    snprintf(buf, sizeof(buf), "  Pick: %c (%s)",
             choseA ? 'A' : 'B',
             (choseA == aIsLarger) ? "larger" : "smaller");
    putString(0, 9, buf);

    // Use passed correct (computed BEFORE learning)
    snprintf(buf, sizeof(buf), "  %s", correct ? "[OK] CORRECT" : "[X] WRONG");
    putString(0, 10, buf);

    // Separator
    drawHLine(0, 12, 38, '-');

    // History
    drawHistory(history, 13);

    // Completion message if done
    if (showContinue) {
        drawCompletionMessage(18, PuzzleType::GENERALIZATION);
    }

    // Controls
    drawHLine(0, TERM_HEIGHT - 3, TERM_WIDTH, '-');
    drawControls(TERM_HEIGHT - 2, showContinue);

    flush();
}

void Renderer::drawPuzzle2(const ShapeTrial& trial, const FeatureSelectionNet& net,
                           bool choseA, bool correct,
                           const TrialHistory& history, int trial_num,
                           int successes, int required,
                           bool showContinue) {
    clearBuffer();

    drawHeader("EXCEPTIONS", "Circle safe. Blue square best.",
               net, "4->8->1", successes, required);

    char buf[TERM_WIDTH + 1];

    // Brain box (right side)
    drawBrainBox(39, 0, PuzzleType::FEATURE_SELECTION, net.modelSizeBytes());

    // Trial section
    snprintf(buf, sizeof(buf), "TRIAL %d:", trial_num);
    putString(0, 6, buf);

    snprintf(buf, sizeof(buf), "  [A] %s %s",
             ShapeTrial::colorName(trial.colorA), ShapeTrial::shapeName(trial.shapeA));
    putString(0, 7, buf);
    snprintf(buf, sizeof(buf), "  [B] %s %s",
             ShapeTrial::colorName(trial.colorB), ShapeTrial::shapeName(trial.shapeB));
    putString(0, 8, buf);

    // Use passed choseA (what network chose BEFORE learning)
    int16_t pickedColor = choseA ? trial.colorA : trial.colorB;
    int16_t pickedShape = choseA ? trial.shapeA : trial.shapeB;
    snprintf(buf, sizeof(buf), "  Pick: %c (%s %s)",
             choseA ? 'A' : 'B',
             ShapeTrial::colorName(pickedColor),
             ShapeTrial::shapeName(pickedShape));
    putString(0, 9, buf);

    // Use passed correct (computed BEFORE learning)
    if (correct) {
        if (!ShapeTrial::isCircle(pickedShape) && ShapeTrial::isBlue(pickedColor)) {
            putString(0, 10, "  [OK] blue square is best!");
        } else if (ShapeTrial::isCircle(pickedShape)) {
            putString(0, 10, "  [OK] circle is safe");
        } else {
            putString(0, 10, "  [OK] CORRECT");
        }
    } else {
        putString(0, 10, "  [X] WRONG");
    }

    // Separator and history
    drawHLine(0, 12, 38, '-');
    drawHistory(history, 13);

    // Completion message if done
    if (showContinue) {
        drawCompletionMessage(18, PuzzleType::FEATURE_SELECTION);
    }

    // Controls
    drawHLine(0, TERM_HEIGHT - 3, TERM_WIDTH, '-');
    drawControls(TERM_HEIGHT - 2, showContinue);

    flush();
}

void Renderer::drawPuzzle3(const XORTrial& trial, const XORNet& net,
                           bool predictedSafe, bool correct,
                           const TrialHistory& history, int trial_num,
                           int successes, int required,
                           bool showContinue) {
    clearBuffer();

    drawHeader("CONTEXT", "ON=left, OFF=right.",
               net, "2->4->1", successes, required);

    char buf[TERM_WIDTH + 1];

    // Brain box (right side)
    drawBrainBox(39, 0, PuzzleType::XOR_CONTEXT, net.modelSizeBytes());

    // Trial section
    snprintf(buf, sizeof(buf), "TRIAL %d:", trial_num);
    putString(0, 6, buf);

    // Show the scenario being tested
    snprintf(buf, sizeof(buf), "  Scenario: Light %s, Path %s",
             trial.lightOn ? "ON" : "OFF",
             trial.choosingRight ? "RIGHT" : "LEFT");
    putString(0, 7, buf);

    // What enen predicted about safety
    snprintf(buf, sizeof(buf), "  enen predicts: %s", predictedSafe ? "SAFE" : "DANGER");
    putString(0, 8, buf);

    // Reality and whether prediction was correct
    snprintf(buf, sizeof(buf), "  Reality: %s — %s",
             trial.isSafe ? "SAFE" : "DANGER",
             correct ? "[OK] Correct!" : "[X] Wrong!");
    putString(0, 9, buf);

    // Separator and history
    drawHLine(0, 11, 38, '-');
    drawHistory(history, 12);

    // Completion message if done
    if (showContinue) {
        drawCompletionMessage(17, PuzzleType::XOR_CONTEXT);
    }

    // Controls
    drawHLine(0, TERM_HEIGHT - 3, TERM_WIDTH, '-');
    drawControls(TERM_HEIGHT - 2, showContinue);

    flush();
}

void Renderer::drawPuzzle4(const SequencePuzzle& puzzle, const SequenceNet& net,
                           const TrialHistory& history, int trial_num,
                           int successes, int required,
                           bool showContinue) {
    clearBuffer();

    drawHeader("ORDER", "A first, then B.",
               net, "1->4->2", successes, required);

    char buf[TERM_WIDTH + 1];

    // Brain box (right side)
    drawBrainBox(39, 0, PuzzleType::SEQUENCE, net.modelSizeBytes());

    // Trial section
    snprintf(buf, sizeof(buf), "TRIAL %d:", trial_num);
    putString(0, 6, buf);

    // State
    const char* stateStr = "Ready";
    switch (puzzle.state) {
        case SequenceState::START: stateStr = "Ready"; break;
        case SequenceState::PRESSED_A: stateStr = "A pressed..."; break;
        case SequenceState::SUCCESS: stateStr = "SUCCESS!"; break;
        case SequenceState::FAIL: stateStr = "FAIL!"; break;
    }
    snprintf(buf, sizeof(buf), "  State: %s", stateStr);
    putString(0, 7, buf);

    // Scores
    int16_t scoreA = net.scoreA(puzzle.lastActionInput());
    int16_t scoreB = net.scoreB(puzzle.lastActionInput());
    snprintf(buf, sizeof(buf), "  Scores: A=%d  B=%d", scoreA, scoreB);
    putString(0, 8, buf);

    // What enen will do
    int action = const_cast<SequenceNet&>(net).chooseAction(puzzle.lastActionInput());
    snprintf(buf, sizeof(buf), "  Pick: %c", action == 0 ? 'A' : 'B');
    putString(0, 9, buf);

    // Result based on state
    if (puzzle.isSuccess()) {
        putString(0, 10, "  [OK] Door opens!");
    } else if (puzzle.isFail()) {
        putString(0, 10, "  [X] Wrong order!");
    } else if (puzzle.inProgress()) {
        putString(0, 10, "  Good start...");
    }

    // Separator and history
    drawHLine(0, 12, 38, '-');
    drawHistory(history, 13);

    // Completion message if done
    if (showContinue) {
        drawCompletionMessage(18, PuzzleType::SEQUENCE);
    }

    // Controls
    drawHLine(0, TERM_HEIGHT - 3, TERM_WIDTH, '-');
    drawControls(TERM_HEIGHT - 2, showContinue);

    flush();
}

void Renderer::drawPuzzle5(const CompositionTrial& trial, const CompositionNet& net,
                           bool choseA, bool correct,
                           const TrialHistory& history, const GauntletState& gauntlet,
                           bool showContinue) {
    clearBuffer();

    drawGauntletHeader("EVERYTHING", "ON=bigger, OFF=smaller.",
                       net, "3->8->4->1", gauntlet);

    char buf[TERM_WIDTH + 1];

    // Brain box (right side)
    drawBrainBox(39, 0, PuzzleType::COMPOSITION, net.modelSizeBytes());

    // Trial section
    int trialNum = gauntlet.currentTrials();
    snprintf(buf, sizeof(buf), "TRIAL %d:", trialNum);
    putString(0, 7, buf);

    snprintf(buf, sizeof(buf), "  Light: %s -> pick %s",
             trial.lightOn ? "ON" : "OFF",
             trial.lightOn ? "LARGER" : "SMALLER");
    putString(0, 8, buf);

    snprintf(buf, sizeof(buf), "  [A] size %d", trial.sizeA);
    putString(0, 9, buf);
    snprintf(buf, sizeof(buf), "  [B] size %d", trial.sizeB);
    putString(0, 10, buf);

    // Use passed choseA (what network chose BEFORE learning)
    bool aIsLarger = trial.sizeA > trial.sizeB;
    snprintf(buf, sizeof(buf), "  Pick: %c (%s)",
             choseA ? 'A' : 'B',
             (choseA ? aIsLarger : !aIsLarger) ? "larger" : "smaller");
    putString(0, 11, buf);

    // Use passed correct (computed BEFORE learning)
    snprintf(buf, sizeof(buf), "  %s", correct ? "[OK] CORRECT" : "[X] WRONG");
    putString(0, 12, buf);

    // Separator and history
    drawHLine(0, 14, 38, '-');
    drawHistory(history, 15);

    // Completion message if done
    if (showContinue) {
        drawCompletionMessage(19, PuzzleType::COMPOSITION,
                              gauntlet.correct, GauntletState::SCORED_TRIALS);
    }

    // Controls
    drawHLine(0, TERM_HEIGHT - 3, TERM_WIDTH, '-');
    drawControls(TERM_HEIGHT - 2, showContinue);

    flush();
}

//=============================================================================
// Intro/Outro Screens
//=============================================================================

void Renderer::drawIntro(size_t totalModelBytes) {
    clearBuffer();

    putString(32, 2, "ENEN DEMO");
    drawHLine(32, 3, 9, '=');

    putString(10, 5, "A small creature named enen will solve five puzzles.");

    putString(10, 7, "enen starts with no knowledge - just random neural network");
    putString(10, 8, "weights. It will learn each puzzle from scratch by trying,");
    putString(10, 9, "failing, and updating its brain.");

    char buf[64];
    snprintf(buf, sizeof(buf), "The entire brain fits in %zu bytes.", totalModelBytes);
    putString(10, 11, buf);
    putString(10, 12, "No cloud. No pre-training. Just learning.");

    putString(10, 14, "Each puzzle teaches a different concept:");
    putString(12, 15, "1. Ignore distractions (color doesn't matter)");
    putString(12, 16, "2. Learn an exception (blue squares are special)");
    putString(12, 17, "3. Context changes the rule (check the light)");
    putString(12, 18, "4. Order matters (A then B)");
    putString(12, 19, "5. Combine rules together");

    putString(25, 22, "Press [Space] to begin...");

    flush();
}

void Renderer::drawVictory(size_t totalModelBytes, int gauntletScore, int gauntletTotal) {
    clearBuffer();

    putString(30, 3, "DEMO COMPLETE");
    drawHLine(30, 4, 13, '=');

    putString(12, 6, "enen solved all five puzzles.");

    putString(12, 8, "Starting from random weights, it learned:");
    putString(14, 9, "Puzzle 1: Ignore distractions (color doesn't matter)");
    putString(14, 10, "Puzzle 2: Rules have exceptions (blue squares win)");
    putString(14, 11, "Puzzle 3: Context changes the answer (check the light)");
    putString(14, 12, "Puzzle 4: Order matters (A then B)");
    putString(14, 13, "Puzzle 5: Combine skills (context + comparison)");

    char buf[64];
    snprintf(buf, sizeof(buf), "Final gauntlet score: %d/%d (%d%%)",
             gauntletScore, gauntletTotal,
             gauntletTotal > 0 ? (gauntletScore * 100) / gauntletTotal : 0);
    putString(20, 15, buf);

    snprintf(buf, sizeof(buf), "Total brain size: %zu bytes", totalModelBytes);
    putString(20, 17, buf);
    putString(20, 18, "All learning happened live. No pre-training.");

    putString(30, 21, "Press [Q] to exit");

    flush();
}

void Renderer::drawBrainBoxPreview(int x, int y, PuzzleType puzzleType) {
    // Brain box preview for intro screens (no byte count yet)
    putString(x, y, "+---------------------------------------+");
    putString(x, y + 1, "| enen's brain (before learning)       |");
    putString(x, y + 2, "|                                       |");
    putString(x, y + 3, "| SEES         THINKS        DECIDES    |");
    putString(x, y + 4, "|                                       |");

    switch (puzzleType) {
        case PuzzleType::GENERALIZATION:
            putString(x, y + 5, "| size A  -+               +-> pick A   |");
            putString(x, y + 6, "| size B  -+-> 8 neurons --+            |");
            putString(x, y + 7, "| color A -+               +-> pick B   |");
            putString(x, y + 8, "| color B -+                            |");
            putString(x, y + 9, "+---------------------------------------+");
            break;

        case PuzzleType::FEATURE_SELECTION:
            putString(x, y + 5, "| color A -+               +-> pick A   |");
            putString(x, y + 6, "| shape A -+-> 8 neurons --+            |");
            putString(x, y + 7, "| color B -+               +-> pick B   |");
            putString(x, y + 8, "| shape B -+                            |");
            putString(x, y + 9, "+---------------------------------------+");
            break;

        case PuzzleType::XOR_CONTEXT:
            putString(x, y + 5, "| light   -+-> 4 neurons --> safe path  |");
            putString(x, y + 6, "| path    -+                            |");
            putString(x, y + 7, "|                                       |");
            putString(x, y + 8, "|                                       |");
            putString(x, y + 9, "+---------------------------------------+");
            break;

        case PuzzleType::SEQUENCE:
            putString(x, y + 5, "|                          +-> press A  |");
            putString(x, y + 6, "| last key --> 4 neurons --+            |");
            putString(x, y + 7, "|                          +-> press B  |");
            putString(x, y + 8, "|                                       |");
            putString(x, y + 9, "+---------------------------------------+");
            break;

        case PuzzleType::COMPOSITION:
            putString(x, y + 5, "| light  -+                +-> pick A   |");
            putString(x, y + 6, "| size A -+> 8 neurons > 4 neurons      |");
            putString(x, y + 7, "| size B -+                +-> pick B   |");
            putString(x, y + 8, "|                                       |");
            putString(x, y + 9, "| (two layers - this puzzle is harder)  |");
            putString(x, y + 10, "+---------------------------------------+");
            break;
    }
}

void Renderer::drawPuzzleIntro(PuzzleType type) {
    clearBuffer();

    // Brain box preview on right side
    drawBrainBoxPreview(39, 2, type);

    switch (type) {
        case PuzzleType::GENERALIZATION: {
            putString(4, 2, "PUZZLE 1: SIZE");
            drawHLine(4, 3, 14, '-');
            putString(2, 5, "Two mushrooms appear.");
            putString(2, 6, "One is safe, one is poison.");
            putString(2, 8, "The bigger mushroom is");
            putString(2, 9, "always safe.");
            putString(2, 11, "But colors vary, and enen");
            putString(2, 12, "doesn't know color is noise.");
            break;
        }
        case PuzzleType::FEATURE_SELECTION: {
            putString(4, 2, "PUZZLE 2: EXCEPTIONS");
            drawHLine(4, 3, 20, '-');
            putString(2, 5, "Shapes appear.");
            putString(2, 6, "Some safe, some dangerous.");
            putString(2, 8, "Circles are usually safe.");
            putString(2, 9, "Squares are usually bad.");
            putString(2, 11, "But blue squares are");
            putString(2, 12, "the safest of all.");
            break;
        }
        case PuzzleType::XOR_CONTEXT: {
            putString(4, 2, "PUZZLE 3: CONTEXT");
            drawHLine(4, 3, 17, '-');
            putString(2, 5, "A light and two paths.");
            putString(2, 7, "Light ON  -> go left");
            putString(2, 8, "Light OFF -> go right");
            putString(2, 10, "The answer changes based");
            putString(2, 11, "on context. This is tricky.");
            break;
        }
        case PuzzleType::SEQUENCE: {
            putString(4, 2, "PUZZLE 4: ORDER");
            drawHLine(4, 3, 15, '-');
            putString(2, 5, "Two buttons: A and B.");
            putString(2, 7, "The right order is:");
            putString(2, 8, "A first, then B.");
            putString(2, 10, "enen must remember what");
            putString(2, 11, "it already pressed.");
            break;
        }
        case PuzzleType::COMPOSITION: {
            putString(4, 2, "PUZZLE 5: EVERYTHING");
            drawHLine(4, 3, 20, '-');
            putString(2, 5, "Light + two sizes.");
            putString(2, 7, "Light ON  -> pick bigger");
            putString(2, 8, "Light OFF -> pick smaller");
            putString(2, 10, "Context + comparison.");
            putString(2, 11, "Both skills together.");
            break;
        }
    }

    putString(5, 21, "Press [Space] to start...");

    flush();
}

} // namespace enen
