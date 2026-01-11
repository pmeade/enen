#pragma once
/**
 * Game logic for enen Demo
 *
 * Separated from UI for testability.
 * Can run full demo without renderer.
 */

#include "networks.hpp"
#include "puzzles.hpp"
#include <string>
#include <vector>
#include <functional>

namespace enen {

// Event types for logging/UI
enum class EventType {
    TRIAL_START,
    CHOICE_MADE,
    OUTCOME,
    LEARNING,
    PUZZLE_COMPLETE
};

struct GameEvent {
    EventType type;
    std::string message;
    bool success;  // For OUTCOME events
};

// Callback for game events (UI can subscribe)
using EventCallback = std::function<void(const GameEvent&)>;

// Game state - contains all puzzle state
struct GameState {
    PuzzleType current_puzzle = PuzzleType::GENERALIZATION;
    bool puzzle_complete = false;
    bool demo_complete = false;

    // Learning validator (Puzzles 1-4)
    LearningValidator validator;

    // Gauntlet state (Puzzle 5)
    GauntletState gauntlet;

    // IntgrNN Networks
    GeneralizationNet gen_net;
    FeatureSelectionNet feat_net;
    XORNet xor_net;
    SequenceNet seq_net;
    CompositionNet comp_net;

    // Puzzle state
    SequencePuzzle seq_puzzle;

    // RNG
    RNG rng;

    // Current trial data (for UI display)
    MushroomTrial current_mushroom;
    ShapeTrial current_shape;
    XORTrial current_xor;
    CompositionTrial current_composition;

    GameState(uint32_t seed = 12345) : rng(seed) {}

    void reset() {
        validator.reset();
        gauntlet.reset();
        puzzle_complete = false;
    }

    void resetNetwork() {
        switch (current_puzzle) {
            case PuzzleType::GENERALIZATION:
                gen_net.reset();
                break;
            case PuzzleType::FEATURE_SELECTION:
                feat_net.reset();
                break;
            case PuzzleType::XOR_CONTEXT:
                xor_net.reset();
                break;
            case PuzzleType::SEQUENCE:
                seq_net.reset();
                seq_puzzle.reset();
                break;
            case PuzzleType::COMPOSITION:
                comp_net.reset();
                gauntlet.reset();
                break;
        }
    }

    bool nextPuzzle() {
        int next = static_cast<int>(current_puzzle) + 1;
        if (next >= NUM_PUZZLES) {
            demo_complete = true;
            return false;
        }
        current_puzzle = static_cast<PuzzleType>(next);
        reset();
        return true;
    }

    size_t totalModelBytes() const {
        return totalModelSize(gen_net, feat_net, xor_net, seq_net, comp_net);
    }
};

// Game logic - runs puzzles, emits events
class Game {
public:
    Game(uint32_t seed = 12345);

    // Set event callback for UI
    void setEventCallback(EventCallback cb) { callback_ = cb; }

    // Access state (for UI display)
    const GameState& state() const { return state_; }
    GameState& state() { return state_; }

    // Run one trial of current puzzle
    // Returns true if puzzle completed
    bool runTrial();

    // Reset current puzzle
    void resetPuzzle();

    // Advance to next puzzle
    bool nextPuzzle();

    // Run entire puzzle until complete (for testing)
    // Returns number of trials taken
    int runPuzzleToCompletion(int maxTrials = 1000);

    // Run entire demo (all 5 puzzles)
    // Returns true if all completed successfully
    bool runFullDemo(int maxTrialsPerPuzzle = 1000);

private:
    GameState state_;
    EventCallback callback_;

    void emit(EventType type, const std::string& msg, bool success = false);

    // Individual puzzle trial runners
    bool runPuzzle1Trial();
    bool runPuzzle2Trial();
    bool runPuzzle3Trial();
    bool runPuzzle4Trial();
    bool runPuzzle5Trial();
};

} // namespace enen
