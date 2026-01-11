#pragma once
/**
 * Puzzle infrastructure for enen Demo
 *
 * Trial generators for each of the 5 puzzles:
 * 1. MushroomTrial - Generalization (size comparison)
 * 2. ShapeTrial - Feature Selection (shape vs color)
 * 3. XORTrial - Context-dependent choice
 * 4. SequencePuzzle - Sequence learning state machine
 * 5. CompositionTrial - Combined context + size
 */

#include <cstdint>
#include <cstdlib>

namespace enen {

// Simple RNG for trial generation (xorshift32)
struct RNG {
    uint32_t state;

    RNG(uint32_t seed = 12345) : state(seed) {}

    uint32_t next() {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }
};

//=============================================================================
// Puzzle 1: Generalization - Mushroom size comparison
//=============================================================================
struct MushroomTrial {
    int16_t sizeA;
    int16_t sizeB;
    int16_t colorA;
    int16_t colorB;
    bool correctIsA;

    static MushroomTrial generate(RNG& rng, bool adversarial = false) {
        if (adversarial) {
            return generateAdversarial(rng);
        }
        MushroomTrial t;
        t.sizeA = 32 + rng.next() % 96;   // 32-127
        t.sizeB = 32 + rng.next() % 96;
        // Ensure meaningful size difference (at least 20)
        while (std::abs(t.sizeA - t.sizeB) < 20) {
            t.sizeB = 32 + rng.next() % 96;
        }
        t.colorA = rng.next() % 128;  // Random color (noise)
        t.colorB = rng.next() % 128;
        t.correctIsA = t.sizeA > t.sizeB;  // Larger is correct
        return t;
    }

    // Color names for display
    static const char* colorName(int16_t color) {
        if (color < 16) return "red";
        if (color < 32) return "orange";
        if (color < 48) return "yellow";
        if (color < 64) return "green";
        if (color < 80) return "cyan";
        if (color < 96) return "blue";
        if (color < 112) return "purple";
        return "pink";
    }

private:
    // Adversarial trial: bright color on smaller, dull on larger
    // Random weights mixing color and size will pick wrong
    static MushroomTrial generateAdversarial(RNG& rng) {
        MushroomTrial t;
        t.sizeA = 90 + rng.next() % 38;   // 90-127 (large)
        t.sizeB = 32 + rng.next() % 38;   // 32-69 (small)
        t.colorA = 10 + rng.next() % 30;  // 10-39 (dull)
        t.colorB = 90 + rng.next() % 38;  // 90-127 (bright)
        t.correctIsA = true;  // A is larger, so A is correct
        return t;
    }
};

//=============================================================================
// Puzzle 2: Feature Interaction - Circles safe, but blue squares safest
// Ranking: 1. Blue square (best), 2. Any circle (middle), 3. Non-blue square (worst)
//=============================================================================
struct ShapeTrial {
    int16_t colorA, shapeA;  // shape: 0=square, 127=circle
    int16_t colorB, shapeB;  // color: 0-25=blue, 26-127=other colors
    bool correctIsA;

    // Ranking function: blue square=2, circle=1, non-blue square=0
    static int rank(int16_t color, int16_t shape) {
        bool isCircle = shape > 64;
        bool isBlue = color < 26;

        if (!isCircle && isBlue) return 2;  // Blue square = best
        if (isCircle) return 1;              // Circle = middle
        return 0;                            // Non-blue square = worst
    }

    static ShapeTrial generate(RNG& rng, bool adversarial = false) {
        if (adversarial) {
            return generateAdversarial(rng);
        }

        ShapeTrial t;
        t.shapeA = (rng.next() % 2) ? 127 : 0;
        t.shapeB = (rng.next() % 2) ? 127 : 0;
        t.colorA = rng.next() % 128;
        t.colorB = rng.next() % 128;

        int rankA = rank(t.colorA, t.shapeA);
        int rankB = rank(t.colorB, t.shapeB);

        // Ensure different ranks (no ties)
        while (rankA == rankB) {
            t.colorB = rng.next() % 128;
            t.shapeB = (rng.next() % 2) ? 127 : 0;
            rankB = rank(t.colorB, t.shapeB);
        }

        t.correctIsA = rankA > rankB;
        return t;
    }

    static const char* shapeName(int16_t shape) {
        return shape > 64 ? "circle" : "square";
    }

    static const char* colorName(int16_t color) {
        if (color < 26) return "blue";
        if (color < 52) return "green";
        if (color < 78) return "yellow";
        if (color < 104) return "red";
        return "purple";
    }

    static bool isBlue(int16_t color) {
        return color < 26;
    }

    static bool isCircle(int16_t shape) {
        return shape > 64;
    }

private:
    // Adversarial trial: blue square vs bright circle
    // Random networks will likely pick the circle (circles usually win)
    static ShapeTrial generateAdversarial(RNG& rng) {
        ShapeTrial t;
        t.shapeA = 0;                         // square
        t.colorA = 5 + rng.next() % 20;       // blue (5-24)
        t.shapeB = 127;                       // circle
        t.colorB = 80 + rng.next() % 48;      // bright non-blue
        t.correctIsA = true;                  // blue square beats circle
        return t;
    }
};

//=============================================================================
// Puzzle 3: XOR - Context-dependent choice
//=============================================================================
struct XORTrial {
    bool lightOn;
    bool choosingRight;
    bool isSafe;

    static XORTrial generate(RNG& rng) {
        XORTrial t;
        t.lightOn = rng.next() % 2;
        t.choosingRight = rng.next() % 2;
        t.isSafe = t.lightOn != t.choosingRight;  // XOR: safe when different
        return t;
    }

    int16_t lightInput() const { return lightOn ? 127 : 0; }
    int16_t pathInput() const { return choosingRight ? 127 : 0; }
};

//=============================================================================
// Puzzle 4: Sequence - State machine for A then B
//=============================================================================
enum class SequenceState { START, PRESSED_A, SUCCESS, FAIL };

struct SequencePuzzle {
    SequenceState state = SequenceState::START;

    // Returns true if action was valid (not necessarily final success)
    bool pressButton(int button) {  // 0 = A, 1 = B
        switch (state) {
            case SequenceState::START:
                if (button == 0) {
                    state = SequenceState::PRESSED_A;
                    return true;  // Good so far
                } else {
                    state = SequenceState::FAIL;
                    return false;  // Wrong! B first is failure
                }

            case SequenceState::PRESSED_A:
                if (button == 1) {
                    state = SequenceState::SUCCESS;
                    return true;  // Success!
                } else {
                    state = SequenceState::FAIL;
                    return false;  // A then A is wrong
                }

            default:
                return false;
        }
    }

    bool isSuccess() const { return state == SequenceState::SUCCESS; }
    bool isFail() const { return state == SequenceState::FAIL; }
    bool inProgress() const { return state == SequenceState::PRESSED_A; }

    void reset() { state = SequenceState::START; }

    // Get encoded last action for network input
    int16_t lastActionInput() const {
        switch (state) {
            case SequenceState::PRESSED_A: return 64;   // A was pressed
            case SequenceState::START: return 0;        // Nothing pressed
            default: return 0;
        }
    }
};

//=============================================================================
// Puzzle 5: Composition - Context-gated size comparison
//=============================================================================
struct CompositionTrial {
    bool lightOn;
    int16_t sizeA, sizeB;
    bool correctIsA;

    static CompositionTrial generate(RNG& rng) {
        CompositionTrial t;
        t.lightOn = rng.next() % 2;
        t.sizeA = 32 + rng.next() % 96;
        t.sizeB = 32 + rng.next() % 96;
        // Ensure meaningful size difference (at least 20)
        while (std::abs(t.sizeA - t.sizeB) < 20) {
            t.sizeB = 32 + rng.next() % 96;
        }

        bool aIsLarger = t.sizeA > t.sizeB;

        // Light ON -> pick larger, Light OFF -> pick smaller
        if (t.lightOn) {
            t.correctIsA = aIsLarger;
        } else {
            t.correctIsA = !aIsLarger;
        }
        return t;
    }

    int16_t lightInput() const { return lightOn ? 127 : 0; }
};

//=============================================================================
// Learning Validator
// Uses adversarial first trials (likely to fail) with honest evaluation.
// Requirements:
// 1. At least 5 trials (viewer sees progression)
// 2. At least 4 consecutive successes (proves mastery)
// Note: No longer requires failures — if enen aces it, that's valid
//=============================================================================
struct LearningValidator {
    int failures = 0;
    int successes = 0;
    int total_trials = 0;

    void recordOutcome(bool success) {
        total_trials++;
        if (success) {
            successes++;
        } else {
            failures++;
            // Reset success streak - must earn them after failures
            successes = 0;
        }
    }

    bool hasLearned() const {
        // Requirements for "learned":
        // 1. At least 5 trials (viewer sees progression)
        // 2. At least 4 consecutive successes (proves mastery)
        return total_trials >= 5 && successes >= requiredSuccesses();
    }

    int requiredSuccesses() const {
        return 4;  // Fixed requirement
    }

    void reset() {
        failures = 0;
        successes = 0;
        total_trials = 0;
    }

    bool isFirstTrial() const {
        return total_trials == 0;
    }
};

//=============================================================================
// Gauntlet State (for Puzzle 5)
// With real training, add warmup phase where learning happens.
// Warmup trials train the network but don't count toward score.
// Scored trials count toward final score.
//=============================================================================
struct GauntletState {
    int warmup_completed = 0;
    int scored_completed = 0;
    int correct = 0;

    static constexpr int WARMUP_TRIALS = 10;   // Learning phase, not scored
    static constexpr int SCORED_TRIALS = 20;   // These count
    static constexpr int TOTAL_TRIALS = WARMUP_TRIALS + SCORED_TRIALS;

    bool inWarmup() const { return warmup_completed < WARMUP_TRIALS; }

    void recordOutcome(bool success) {
        if (inWarmup()) {
            warmup_completed++;
            // Still learning, success doesn't count yet
        } else {
            scored_completed++;
            if (success) correct++;
        }
    }

    bool isComplete() const {
        return scored_completed >= SCORED_TRIALS;
    }

    int scorePercent() const {
        if (scored_completed == 0) return 0;
        return (correct * 100) / scored_completed;
    }

    int currentScore() const { return correct; }
    int currentTrials() const { return warmup_completed + scored_completed; }

    void reset() {
        warmup_completed = 0;
        scored_completed = 0;
        correct = 0;
    }
};

//=============================================================================
// Overall demo state
//=============================================================================
enum class PuzzleType {
    GENERALIZATION = 0,
    FEATURE_SELECTION = 1,
    XOR_CONTEXT = 2,
    SEQUENCE = 3,
    COMPOSITION = 4
};

constexpr int NUM_PUZZLES = 5;

} // namespace enen
