#pragma once
/**
 * Terminal renderer for enen Demo
 *
 * Clean, decision-focused layout:
 * - Header with puzzle name, network info, progress
 * - Trial section with choices, pick, result
 * - History showing recent trials
 * - Visual box in bottom-right
 */

#include "puzzles.hpp"
#include "networks.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace enen {

// Terminal size
constexpr int TERM_WIDTH = 80;
constexpr int TERM_HEIGHT = 24;

// Trial history entry
struct TrialRecord {
    int trial_num;
    bool correct;
    std::string summary;  // Brief description
};

// Trial history - keeps last N trials
class TrialHistory {
public:
    static constexpr int MAX_ENTRIES = 4;

    void add(int trial_num, bool correct, const std::string& summary);
    const std::vector<TrialRecord>& entries() const { return entries_; }
    void clear() { entries_.clear(); }

private:
    std::vector<TrialRecord> entries_;
};

// Main renderer class
class Renderer {
public:
    Renderer();
    ~Renderer();

    // Setup/teardown
    void init();
    void cleanup();

    // Clear screen
    void clear();

    // Draw complete frame for each puzzle
    // Note: choseA/predictedSafe/correct are passed from main.cpp to show
    // what the network chose BEFORE learning, not after
    void drawPuzzle1(const MushroomTrial& trial, const GeneralizationNet& net,
                     bool choseA, bool correct,
                     const TrialHistory& history, int trial_num,
                     int successes, int required,
                     bool showContinue);

    void drawPuzzle2(const ShapeTrial& trial, const FeatureSelectionNet& net,
                     bool choseA, bool correct,
                     const TrialHistory& history, int trial_num,
                     int successes, int required,
                     bool showContinue);

    void drawPuzzle3(const XORTrial& trial, const XORNet& net,
                     bool predictedSafe, bool correct,
                     const TrialHistory& history, int trial_num,
                     int successes, int required,
                     bool showContinue);

    void drawPuzzle4(const SequencePuzzle& puzzle, const SequenceNet& net,
                     const TrialHistory& history, int trial_num,
                     int successes, int required,
                     bool showContinue);

    void drawPuzzle5(const CompositionTrial& trial, const CompositionNet& net,
                     bool choseA, bool correct,
                     const TrialHistory& history, const GauntletState& gauntlet,
                     bool showContinue);

    // Draw intro/victory screens
    void drawIntro(size_t totalModelBytes);
    void drawVictory(size_t totalModelBytes, int gauntletScore, int gauntletTotal);
    void drawPuzzleIntro(PuzzleType type);

    // Flush output
    void flush();

private:
    // Buffer for double-buffered rendering
    char buffer_[TERM_HEIGHT][TERM_WIDTH + 1];

    // Drawing primitives
    void clearBuffer();
    void putString(int x, int y, const char* str);
    void putChar(int x, int y, char c);
    void drawHLine(int x, int y, int len, char c = '-');

    // Layout components
    void drawHeader(const char* puzzleName, const char* rule,
                    const IntgrNNWrapper& net, const char* arch,
                    int successes, int required);
    void drawGauntletHeader(const char* puzzleName, const char* rule,
                            const IntgrNNWrapper& net, const char* arch,
                            const GauntletState& gauntlet);
    void drawProgressBar(int x, int y, int width, int value, int max);
    void drawHistory(const TrialHistory& history, int startY);
    void drawControls(int y, bool showContinue);
    void drawVisualBox(int x, int y, int w, int h);

    // Puzzle-specific visuals (in the visual box)
    void drawSizeBars(int boxX, int boxY, int16_t sizeA, int16_t sizeB);
    void drawShapes(int boxX, int boxY, bool aIsCircle);
    void drawLightAndPaths(int boxX, int boxY, bool lightOn);
    void drawButtons(int boxX, int boxY, int16_t scoreA, int16_t scoreB);
    void drawLightAndSizes(int boxX, int boxY, bool lightOn, int16_t sizeA, int16_t sizeB);

    // Brain diagram showing what enen sees, thinks, and decides
    void drawBrainBox(int x, int y, PuzzleType puzzleType, size_t modelBytes);
    void drawBrainBoxPreview(int x, int y, PuzzleType puzzleType);  // For intro screens

    // Completion message when puzzle is done
    void drawCompletionMessage(int y, PuzzleType type, int gauntletScore = 0, int gauntletTotal = 0);
};

} // namespace enen
