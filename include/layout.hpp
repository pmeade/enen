#pragma once
/**
 * Layout constants for enen Demo
 *
 * All magic numbers for screen positioning are defined here.
 * This makes the layout easy to adjust and documents the visual structure.
 *
 * Screen layout (80x24):
 * +--LEFT COLUMN (38 chars)-------------+--RIGHT COLUMN (41 chars)------------+
 * | Header, rule, progress              | Brain diagram box                    |
 * | Trial details                       |                                      |
 * | History                             |                                      |
 * +-------------------------------------+--------------------------------------+
 * | Controls footer                                                            |
 * +----------------------------------------------------------------------------+
 */

#include "frame.hpp"

namespace enen {

//=============================================================================
// Timing Constants (seconds)
//
// Pacing designed for video recording (~2:50 total):
// - Slower on intros and first trials (orient the viewer)
// - Faster on subsequent correct answers (maintain momentum)
// - Pause on errors (let failure register)
//=============================================================================
namespace timing {
    constexpr double INTRO_1 = 6.0;        // First intro screen
    constexpr double INTRO_2 = 6.0;        // Second intro screen
    constexpr double PUZZLE_INTRO = 5.0;   // Puzzle explanation screen
    constexpr double FIRST_TRIAL = 3.0;    // First trial of each puzzle
    constexpr double TRIAL_CORRECT = 1.5;  // Correct answer (keep momentum)
    constexpr double TRIAL_WRONG = 2.0;    // Wrong answer (let it register)
    constexpr double COMPLETION = 4.0;     // Puzzle completed message
    constexpr double VICTORY = 8.0;        // Final victory screen
    constexpr double SEQUENCE_STEP = 0.8;  // Intermediate step in sequence puzzle
}

//=============================================================================
// Layout Coordinates
//
// All positions are (x, y) where x=column, y=row.
// Named for their semantic purpose, not their pixel location.
//=============================================================================
namespace layout {
    // Screen regions
    constexpr int LEFT_COLUMN_WIDTH = 38;
    constexpr int RIGHT_COLUMN_START = 39;

    // Header section (rows 0-4)
    namespace header {
        constexpr int TITLE_Y = 0;
        constexpr int DIVIDER_Y = 1;
        constexpr int RULE_Y = 2;
        constexpr int PROGRESS_Y = 3;
        constexpr int PROGRESS_BAR_X = 10;
        constexpr int PROGRESS_COUNT_X = 23;
        constexpr int SECTION_END_Y = 4;
    }

    // Trial section (rows 6-12)
    namespace trial {
        constexpr int LABEL_Y = 6;
        constexpr int OPTION_A_Y = 7;
        constexpr int OPTION_B_Y = 8;
        constexpr int PICK_Y = 9;
        constexpr int RESULT_Y = 10;
    }

    // History section (rows 13-17)
    namespace history {
        constexpr int DIVIDER_Y = 12;
        constexpr int LABEL_Y = 13;
        constexpr int FIRST_ENTRY_Y = 14;
        constexpr int MAX_ENTRIES = 4;
    }

    // Completion message
    namespace completion {
        constexpr int MESSAGE_Y = 18;
        constexpr int SCORE_Y = 19;
    }

    // Controls footer (bottom 3 rows)
    namespace footer {
        constexpr int DIVIDER_Y = terminal::HEIGHT - 3;  // Row 21
        constexpr int CONTROLS_Y = terminal::HEIGHT - 2; // Row 22
    }

    // Brain diagram box (right column)
    namespace brain {
        constexpr int X = RIGHT_COLUMN_START;
        constexpr int Y = 0;
        constexpr int WIDTH = 41;
        constexpr int HEIGHT = 10;  // 11 for composition (extra row)
    }

    // Intro screens (centered content)
    namespace intro {
        constexpr int TITLE_X = 29;
        constexpr int TITLE_Y = 2;
        constexpr int CONTENT_X = 20;
        constexpr int CONTENT_START_Y = 6;
        constexpr int FOOTER_Y = 22;
    }

    // Victory screen
    namespace victory {
        constexpr int TITLE_X = 30;
        constexpr int TITLE_Y = 3;
        constexpr int CONTENT_X = 12;
        constexpr int PUZZLES_START_Y = 9;
        constexpr int SCORE_Y = 15;
        constexpr int SIZE_Y = 17;
    }

    // Puzzle intro (left side explanation)
    namespace puzzle_intro {
        constexpr int TITLE_X = 4;
        constexpr int TITLE_Y = 2;
        constexpr int CONTENT_X = 2;
        constexpr int CONTENT_START_Y = 5;
        constexpr int START_PROMPT_Y = 21;
    }
}

//=============================================================================
// Progress Bar Rendering
//=============================================================================
inline void drawProgressBar(TextBuffer& buffer, int x, int y, int value, int max) {
    char bar[20];
    int filled = (max > 0) ? (value * 10) / max : 0;
    if (filled > 10) filled = 10;

    bar[0] = '[';
    for (int i = 0; i < 10; i++) {
        bar[1 + i] = (i < filled) ? '#' : '.';
    }
    bar[11] = ']';
    bar[12] = '\0';

    buffer.putString(x, y, bar);
}

} // namespace enen
