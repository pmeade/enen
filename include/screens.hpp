#pragma once
/**
 * Common screen rendering for enen Demo
 *
 * Shared screens that appear between puzzles:
 * - Intro screens (what the demo is, how learning works)
 * - Puzzle intro screens (explanation of each puzzle)
 * - Victory screen (final results)
 *
 * Each function takes a TextBuffer and fills it with the screen content.
 * The caller is responsible for outputting the frame.
 */

#include "frame.hpp"
#include "layout.hpp"
#include "brain_diagram.hpp"
#include "puzzles.hpp"
#include <cstdio>

namespace enen {

//=============================================================================
// Intro Screen 1: What the Demo Is
//
// Hook the viewer: small creature, five puzzles, tiny brain
//=============================================================================
inline void renderIntro1(TextBuffer& buffer, size_t totalBytes) {
    buffer.clear();

    buffer.putString(layout::intro::TITLE_X, layout::intro::TITLE_Y, "ENEN DEMO");
    buffer.drawHLine(layout::intro::TITLE_X, layout::intro::TITLE_Y + 1, 9, '=');

    int y = layout::intro::CONTENT_START_Y;
    buffer.putString(layout::intro::CONTENT_X, y,     "You're about to watch a small creature");
    buffer.putString(layout::intro::CONTENT_X, y + 1, "named enen solve five puzzles.");

    buffer.putString(layout::intro::CONTENT_X, y + 3, "enen starts knowing nothing.");

    buffer.putString(layout::intro::CONTENT_X, y + 5, "It will guess, get feedback, and learn.");

    buffer.putString(layout::intro::CONTENT_X, y + 7, "After a few tries, it figures out the pattern.");

    char byteLine[64];
    std::snprintf(byteLine, sizeof(byteLine),
                  "The twist: enen's entire brain is %zu bytes.", totalBytes);
    buffer.putString(layout::intro::CONTENT_X, y + 10, byteLine);

    buffer.putString(layout::intro::CONTENT_X, y + 12, "That's smaller than this sentence.");

    buffer.putString(layout::intro::TITLE_X, layout::intro::FOOTER_Y, "[Screen 1 of 2]");
}

//=============================================================================
// Intro Screen 2: How Learning Works
//
// Explain the learning loop in simple terms
//=============================================================================
inline void renderIntro2(TextBuffer& buffer) {
    buffer.clear();

    buffer.putString(26, layout::intro::TITLE_Y, "HOW ENEN LEARNS");
    buffer.drawHLine(26, layout::intro::TITLE_Y + 1, 15, '=');

    int y = layout::intro::CONTENT_START_Y;
    buffer.putString(layout::intro::CONTENT_X, y, "Each puzzle works the same way:");

    buffer.putString(layout::intro::CONTENT_X, y + 2, "1. enen sees a choice (A or B)");
    buffer.putString(layout::intro::CONTENT_X, y + 4, "2. enen guesses (using its neural network)");
    buffer.putString(layout::intro::CONTENT_X, y + 6, "3. enen finds out if it was right or wrong");
    buffer.putString(layout::intro::CONTENT_X, y + 8, "4. enen updates its brain (backpropagation)");
    buffer.putString(layout::intro::CONTENT_X, y + 10, "5. Repeat until enen gets it");

    buffer.putString(layout::intro::CONTENT_X, y + 13, "This is real machine learning.");
    buffer.putString(layout::intro::CONTENT_X, y + 14, "No tricks. No pre-loaded answers.");
    buffer.putString(layout::intro::CONTENT_X, y + 15, "Just a tiny network learning from scratch.");

    buffer.putString(25, layout::intro::FOOTER_Y + 1, "[Press Space to begin]");
}

//=============================================================================
// Puzzle Intro Screen
//
// Explain the current puzzle's rule with brain diagram preview
//=============================================================================
inline void renderPuzzleIntro(TextBuffer& buffer, PuzzleType type) {
    buffer.clear();

    // Brain diagram on right (shows "before learning")
    drawBrainDiagram(buffer, layout::brain::X, layout::brain::Y, type, 0);

    int x = layout::puzzle_intro::CONTENT_X;
    int titleX = layout::puzzle_intro::TITLE_X;
    int titleY = layout::puzzle_intro::TITLE_Y;
    int y = layout::puzzle_intro::CONTENT_START_Y;

    switch (type) {
        case PuzzleType::GENERALIZATION:
            buffer.putString(titleX, titleY, "PUZZLE 1: SIZE");
            buffer.drawHLine(titleX, titleY + 1, 14, '-');
            buffer.putString(x, y,     "Two mushrooms appear.");
            buffer.putString(x, y + 1, "One is safe, one is poison.");
            buffer.putString(x, y + 3, "The bigger mushroom is");
            buffer.putString(x, y + 4, "always safe.");
            buffer.putString(x, y + 6, "But colors vary, and enen");
            buffer.putString(x, y + 7, "doesn't know color is noise.");
            break;

        case PuzzleType::FEATURE_SELECTION:
            buffer.putString(titleX, titleY, "PUZZLE 2: EXCEPTIONS");
            buffer.drawHLine(titleX, titleY + 1, 20, '-');
            buffer.putString(x, y,     "Shapes appear.");
            buffer.putString(x, y + 1, "Some safe, some dangerous.");
            buffer.putString(x, y + 3, "Circles are usually safe.");
            buffer.putString(x, y + 4, "Squares are usually bad.");
            buffer.putString(x, y + 6, "But blue squares are");
            buffer.putString(x, y + 7, "the safest of all.");
            break;

        case PuzzleType::XOR_CONTEXT:
            buffer.putString(titleX, titleY, "PUZZLE 3: CONTEXT");
            buffer.drawHLine(titleX, titleY + 1, 17, '-');
            buffer.putString(x, y,     "A light and two paths.");
            buffer.putString(x, y + 2, "Light ON  -> go left");
            buffer.putString(x, y + 3, "Light OFF -> go right");
            buffer.putString(x, y + 5, "The answer changes based");
            buffer.putString(x, y + 6, "on context. This is tricky.");
            break;

        case PuzzleType::SEQUENCE:
            buffer.putString(titleX, titleY, "PUZZLE 4: ORDER");
            buffer.drawHLine(titleX, titleY + 1, 15, '-');
            buffer.putString(x, y,     "Two buttons: A and B.");
            buffer.putString(x, y + 2, "The right order is:");
            buffer.putString(x, y + 3, "A first, then B.");
            buffer.putString(x, y + 5, "enen must remember what");
            buffer.putString(x, y + 6, "it already pressed.");
            break;

        case PuzzleType::COMPOSITION:
            buffer.putString(titleX, titleY, "PUZZLE 5: EVERYTHING");
            buffer.drawHLine(titleX, titleY + 1, 20, '-');
            buffer.putString(x, y,     "Light + two sizes.");
            buffer.putString(x, y + 2, "Light ON  -> pick bigger");
            buffer.putString(x, y + 3, "Light OFF -> pick smaller");
            buffer.putString(x, y + 5, "Context + comparison.");
            buffer.putString(x, y + 6, "Both skills together.");
            break;
    }

    buffer.putString(5, layout::puzzle_intro::START_PROMPT_Y, "Press [Space] to start...");
}

//=============================================================================
// Victory Screen
//
// Celebrate completion and show final stats
//=============================================================================
inline void renderVictory(TextBuffer& buffer, size_t totalBytes,
                          int gauntletScore, int gauntletTotal) {
    buffer.clear();

    buffer.putString(layout::victory::TITLE_X, layout::victory::TITLE_Y, "DEMO COMPLETE");
    buffer.drawHLine(layout::victory::TITLE_X, layout::victory::TITLE_Y + 1, 13, '=');

    int x = layout::victory::CONTENT_X;
    buffer.putString(x, 6, "enen solved all five puzzles.");

    buffer.putString(x, 8, "Starting from random weights, it learned:");
    buffer.putString(x + 2, layout::victory::PUZZLES_START_Y,
                     "Puzzle 1: Ignore distractions (color doesn't matter)");
    buffer.putString(x + 2, layout::victory::PUZZLES_START_Y + 1,
                     "Puzzle 2: Rules have exceptions (blue squares win)");
    buffer.putString(x + 2, layout::victory::PUZZLES_START_Y + 2,
                     "Puzzle 3: Context changes the answer (check the light)");
    buffer.putString(x + 2, layout::victory::PUZZLES_START_Y + 3,
                     "Puzzle 4: Order matters (A then B)");
    buffer.putString(x + 2, layout::victory::PUZZLES_START_Y + 4,
                     "Puzzle 5: Combine skills (context + comparison)");

    char scoreLine[64];
    int percent = gauntletTotal > 0 ? (gauntletScore * 100) / gauntletTotal : 0;
    std::snprintf(scoreLine, sizeof(scoreLine),
                  "Final gauntlet score: %d/%d (%d%%)",
                  gauntletScore, gauntletTotal, percent);
    buffer.putString(20, layout::victory::SCORE_Y, scoreLine);

    char sizeLine[64];
    std::snprintf(sizeLine, sizeof(sizeLine), "Total brain size: %zu bytes", totalBytes);
    buffer.putString(20, layout::victory::SIZE_Y, sizeLine);
    buffer.putString(20, layout::victory::SIZE_Y + 1,
                     "All learning happened live. No pre-training.");

    buffer.putString(30, 21, "Press [Q] to exit");
}

} // namespace enen
