#pragma once
/**
 * Brain diagram rendering for enen Demo
 *
 * Draws the ASCII art neural network diagram showing:
 * - SEES: Input features (size, color, light, etc.)
 * - THINKS: Hidden layer neurons
 * - DECIDES: Output (pick A/B, safe/danger)
 *
 * Each puzzle type has a unique diagram reflecting its architecture.
 * The diagram can show either byte count (during gameplay) or
 * "before learning" text (during intro screens).
 */

#include "frame.hpp"
#include "layout.hpp"
#include "puzzles.hpp"
#include <cstdio>
#include <cstring>

namespace enen {

//=============================================================================
// Brain Diagram - Unified rendering for all puzzle types
//
// Parameters:
//   buffer: TextBuffer to draw into
//   x, y: Top-left corner of the box
//   type: Which puzzle's architecture to show
//   bytes: Model size in bytes (0 = show "before learning" instead)
//=============================================================================
inline void drawBrainDiagram(TextBuffer& buffer, int x, int y,
                              PuzzleType type, size_t bytes = 0) {
    // Top border
    buffer.putString(x, y, "+---------------------------------------+");

    // Header line: either bytes or "before learning"
    char header[50];
    if (bytes > 0) {
        std::snprintf(header, sizeof(header), "| enen's brain (%zu bytes)", bytes);
    } else {
        std::snprintf(header, sizeof(header), "| enen's brain (before learning)");
    }
    // Pad to width
    size_t len = std::strlen(header);
    while (len < 40) header[len++] = ' ';
    header[40] = '|';
    header[41] = '\0';
    buffer.putString(x, y + 1, header);

    // Common structure lines
    buffer.putString(x, y + 2, "|                                       |");
    buffer.putString(x, y + 3, "| SEES         THINKS        DECIDES    |");
    buffer.putString(x, y + 4, "|                                       |");

    // Puzzle-specific architecture diagram
    switch (type) {
        case PuzzleType::GENERALIZATION:
            // 4 inputs -> 8 neurons -> 2 outputs
            buffer.putString(x, y + 5, "| size A  -+               +-> pick A   |");
            buffer.putString(x, y + 6, "| size B  -+-> 8 neurons --+            |");
            buffer.putString(x, y + 7, "| color A -+               +-> pick B   |");
            buffer.putString(x, y + 8, "| color B -+                            |");
            buffer.putString(x, y + 9, "+---------------------------------------+");
            break;

        case PuzzleType::FEATURE_SELECTION:
            // 4 inputs -> 8 neurons -> 2 outputs
            buffer.putString(x, y + 5, "| color A -+               +-> pick A   |");
            buffer.putString(x, y + 6, "| shape A -+-> 8 neurons --+            |");
            buffer.putString(x, y + 7, "| color B -+               +-> pick B   |");
            buffer.putString(x, y + 8, "| shape B -+                            |");
            buffer.putString(x, y + 9, "+---------------------------------------+");
            break;

        case PuzzleType::XOR_CONTEXT:
            // 2 inputs -> 4 neurons -> 1 output
            buffer.putString(x, y + 5, "| light   -+-> 4 neurons --> safe path  |");
            buffer.putString(x, y + 6, "| path    -+                            |");
            buffer.putString(x, y + 7, "|                                       |");
            buffer.putString(x, y + 8, "|                                       |");
            buffer.putString(x, y + 9, "+---------------------------------------+");
            break;

        case PuzzleType::SEQUENCE:
            // 1 input -> 4 neurons -> 2 outputs
            buffer.putString(x, y + 5, "|                          +-> press A  |");
            buffer.putString(x, y + 6, "| last key --> 4 neurons --+            |");
            buffer.putString(x, y + 7, "|                          +-> press B  |");
            buffer.putString(x, y + 8, "|                                       |");
            buffer.putString(x, y + 9, "+---------------------------------------+");
            break;

        case PuzzleType::COMPOSITION:
            // 3 inputs -> 8 neurons -> 4 neurons -> 2 outputs (deep)
            buffer.putString(x, y + 5,  "| light  -+                +-> pick A   |");
            buffer.putString(x, y + 6,  "| size A -+> 8 neurons > 4 neurons      |");
            buffer.putString(x, y + 7,  "| size B -+                +-> pick B   |");
            buffer.putString(x, y + 8,  "|                                       |");
            buffer.putString(x, y + 9,  "| (two layers - this puzzle is harder)  |");
            buffer.putString(x, y + 10, "+---------------------------------------+");
            break;
    }
}

} // namespace enen
