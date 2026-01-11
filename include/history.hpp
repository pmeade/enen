#pragma once
/**
 * Trial history tracking for enen Demo
 *
 * Keeps a rolling window of the last N trials for display.
 * Shows the viewer what happened recently without cluttering the screen.
 */

#include "frame.hpp"
#include "layout.hpp"
#include <vector>
#include <string>
#include <cstdio>

namespace enen {

//=============================================================================
// HistoryEntry - A single trial result
//=============================================================================
struct HistoryEntry {
    int trialNum;
    bool correct;
    std::string summary;  // Brief description of what happened
};

//=============================================================================
// History - Rolling window of recent trials
//
// Keeps the last MAX_ENTRIES trials. Older entries are discarded.
// Designed for display in the history section of puzzle screens.
//=============================================================================
class History {
public:
    static constexpr size_t MAX_ENTRIES = 4;

    void add(int trialNum, bool correct, const std::string& summary) {
        entries_.push_back({trialNum, correct, summary});
        while (entries_.size() > MAX_ENTRIES) {
            entries_.erase(entries_.begin());
        }
    }

    void clear() {
        entries_.clear();
    }

    const std::vector<HistoryEntry>& entries() const {
        return entries_;
    }

    bool empty() const {
        return entries_.empty();
    }

private:
    std::vector<HistoryEntry> entries_;
};

//=============================================================================
// drawHistory - Render history entries to a TextBuffer
//
// Draws most recent entries first (reverse chronological order).
// Each entry shows: "Trial N: [OK] summary" or "Trial N: [X] summary"
//=============================================================================
inline void drawHistory(TextBuffer& buffer, const History& history, int startY) {
    buffer.putString(0, startY, "HISTORY:");

    const auto& entries = history.entries();
    for (size_t i = 0; i < entries.size() && static_cast<int>(startY + 1 + i) < terminal::HEIGHT - 2; i++) {
        // Show most recent first
        size_t idx = entries.size() - 1 - i;
        const auto& entry = entries[idx];

        char line[60];
        std::snprintf(line, sizeof(line), "  Trial %d: %s %s",
                      entry.trialNum,
                      entry.correct ? "[OK]" : "[X]",
                      entry.summary.c_str());

        // Truncate if too long
        if (std::strlen(line) > 50) {
            line[50] = '\0';
        }

        buffer.putString(0, startY + 1 + static_cast<int>(i), line);
    }
}

//=============================================================================
// calculateTrialTiming - Determine pause duration for a trial
//
// Implements the pacing rules:
// - Longer pause on first trial (viewer needs to orient)
// - Longer pause on completion (celebrate the achievement)
// - Shorter pause on correct answers (maintain momentum)
// - Medium pause on wrong answers (let failure register)
//=============================================================================
inline double calculateTrialTiming(bool isComplete, bool isFirstTrial, bool isCorrect) {
    if (isComplete) {
        return timing::COMPLETION;
    }
    if (isFirstTrial) {
        return timing::FIRST_TRIAL;
    }
    return isCorrect ? timing::TRIAL_CORRECT : timing::TRIAL_WRONG;
}

} // namespace enen
