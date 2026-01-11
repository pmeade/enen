#pragma once
/**
 * IntgrNN wrapper for enen Demo
 *
 * Thin wrapper around intgr_nn::IntegerGD to provide
 * puzzle-specific interfaces.
 *
 * All five puzzles use IntgrNN — same library, different architectures:
 * - Puzzle 1: 4→8→1 (generalization)
 * - Puzzle 2: 4→8→1 (feature selection)
 * - Puzzle 3: 2→4→1 (XOR)
 * - Puzzle 4: 1→4→2 (sequence)
 * - Puzzle 5: 3→8→4→1 (composition, deep)
 *
 * Experience Replay: Each network stores all training samples and
 * retrains on the complete history after each new sample. This is
 * real learning — the viewer watches genuine learning from scratch.
 */

#include <intgr_nn/intgr_nn.h>
#include <memory>
#include <cstdint>
#include <algorithm>
#include <random>
#include <vector>

namespace enen {

// Scale int16_t (0-127) to uint8_t (0-255) for IntgrNN input
inline uint8_t scaleToU8(int16_t val) {
    return static_cast<uint8_t>(std::clamp(static_cast<int>(val) * 2, 0, 255));
}

// Interpret output: >128 means true/A/safe
inline bool interpretBool(uint8_t out) {
    return out > 128;
}

//=============================================================================
// Base wrapper with common functionality
//=============================================================================
class IntgrNNWrapper {
protected:
    // mutable: forward() is logically const (inference doesn't change the model)
    mutable std::unique_ptr<intgr_nn::IntegerGD> net_;

    static intgr_nn::Config defaultConfig() {
        intgr_nn::Config config;
        config.learning_rate = 0.1;  // Small dataset (from IntgrNN docs)
        return config;
    }

public:
    virtual ~IntgrNNWrapper() = default;

    void reset(uint32_t seed = 0) {
        if (seed == 0) seed = std::random_device{}();
        net_->reinitialize(seed);
        clearHistory();  // Also clear experience
    }

    // Subclasses implement these
    virtual void clearHistory() = 0;
    virtual size_t historySize() const = 0;

    size_t parameterCount() const { return net_->parameterCount(); }
    size_t modelSizeBytes() const { return net_->modelSizeBytes(); }
    double learningRate() const { return net_->learningRate(); }
};

//=============================================================================
// Puzzle 1: Generalization
// Inputs: sizeA, sizeB, colorA, colorB
// Output: chooseA (>128 = yes)
// Goal: Learn that size matters, color doesn't
//=============================================================================
class GeneralizationNet : public IntgrNNWrapper {
    // Experience history for replay
    struct Sample {
        int16_t sizeA, sizeB, colorA, colorB;
        bool chooseA;
    };
    std::vector<Sample> history_;

    static constexpr int EPOCHS_PER_TRIAL = 50;

public:
    GeneralizationNet() {
        // NO ETG - start with random weights
        net_ = intgr_nn::IntegerGD::create(4, 8, 1, defaultConfig());
    }

    bool chooseA(int16_t sizeA, int16_t sizeB, int16_t colorA, int16_t colorB) const {
        intgr_nn::Tensor input(1, 4);
        input.at_u8(0, 0) = scaleToU8(sizeA);
        input.at_u8(0, 1) = scaleToU8(sizeB);
        input.at_u8(0, 2) = scaleToU8(colorA);
        input.at_u8(0, 3) = scaleToU8(colorB);

        auto output = net_->forward(input);
        return interpretBool(output.at_u8(0, 0));
    }

    void learn(int16_t sizeA, int16_t sizeB, int16_t colorA, int16_t colorB, bool shouldChooseA) {
        // Add to history
        history_.push_back({sizeA, sizeB, colorA, colorB, shouldChooseA});

        // Retrain on ALL history
        for (int epoch = 0; epoch < EPOCHS_PER_TRIAL; epoch++) {
            for (const auto& s : history_) {
                intgr_nn::Tensor input(1, 4);
                input.at_u8(0, 0) = scaleToU8(s.sizeA);
                input.at_u8(0, 1) = scaleToU8(s.sizeB);
                input.at_u8(0, 2) = scaleToU8(s.colorA);
                input.at_u8(0, 3) = scaleToU8(s.colorB);

                auto output = net_->forward(input);

                intgr_nn::Tensor target(1, 1);
                target.at_u8(0, 0) = s.chooseA ? 255 : 0;

                net_->backward(output, target);
            }
        }
    }

    void clearHistory() override { history_.clear(); }
    size_t historySize() const override { return history_.size(); }
};

//=============================================================================
// Puzzle 2: Feature Selection
// Inputs: colorA, shapeA, colorB, shapeB
// Output: chooseA (>128 = yes)
// Goal: Learn that shape matters, color doesn't
//=============================================================================
class FeatureSelectionNet : public IntgrNNWrapper {
    struct Sample {
        int16_t colorA, shapeA, colorB, shapeB;
        bool chooseA;
    };
    std::vector<Sample> history_;

    static constexpr int EPOCHS_PER_TRIAL = 50;

public:
    FeatureSelectionNet() {
        net_ = intgr_nn::IntegerGD::create(4, 8, 1, defaultConfig());
    }

    bool chooseA(int16_t colorA, int16_t shapeA, int16_t colorB, int16_t shapeB) const {
        intgr_nn::Tensor input(1, 4);
        input.at_u8(0, 0) = scaleToU8(colorA);
        input.at_u8(0, 1) = scaleToU8(shapeA);
        input.at_u8(0, 2) = scaleToU8(colorB);
        input.at_u8(0, 3) = scaleToU8(shapeB);

        auto output = net_->forward(input);
        return interpretBool(output.at_u8(0, 0));
    }

    void learn(int16_t colorA, int16_t shapeA, int16_t colorB, int16_t shapeB, bool shouldChooseA) {
        history_.push_back({colorA, shapeA, colorB, shapeB, shouldChooseA});

        for (int epoch = 0; epoch < EPOCHS_PER_TRIAL; epoch++) {
            for (const auto& s : history_) {
                intgr_nn::Tensor input(1, 4);
                input.at_u8(0, 0) = scaleToU8(s.colorA);
                input.at_u8(0, 1) = scaleToU8(s.shapeA);
                input.at_u8(0, 2) = scaleToU8(s.colorB);
                input.at_u8(0, 3) = scaleToU8(s.shapeB);

                auto output = net_->forward(input);

                intgr_nn::Tensor target(1, 1);
                target.at_u8(0, 0) = s.chooseA ? 255 : 0;

                net_->backward(output, target);
            }
        }
    }

    void clearHistory() override { history_.clear(); }
    size_t historySize() const override { return history_.size(); }
};

//=============================================================================
// Puzzle 3: XOR (Context-Dependent Choice)
// Inputs: light, path
// Output: safe (>128 = yes)
// Goal: Learn XOR - requires hidden layer
// NOTE: XOR is hard, needs ~1000 total epochs to converge
//=============================================================================
class XORNet : public IntgrNNWrapper {
    struct Sample {
        int16_t light, path;
        bool safe;
    };
    std::vector<Sample> history_;

    // XOR needs more epochs - it's a harder problem
    static constexpr int EPOCHS_PER_TRIAL = 200;

public:
    XORNet() {
        net_ = intgr_nn::IntegerGD::create(2, 4, 1, defaultConfig());
    }

    bool isSafe(int16_t light, int16_t path) const {
        intgr_nn::Tensor input(1, 2);
        input.at_u8(0, 0) = scaleToU8(light);
        input.at_u8(0, 1) = scaleToU8(path);

        auto output = net_->forward(input);
        return interpretBool(output.at_u8(0, 0));
    }

    void learn(int16_t light, int16_t path, bool shouldBeSafe) {
        history_.push_back({light, path, shouldBeSafe});

        for (int epoch = 0; epoch < EPOCHS_PER_TRIAL; epoch++) {
            for (const auto& s : history_) {
                intgr_nn::Tensor input(1, 2);
                input.at_u8(0, 0) = scaleToU8(s.light);
                input.at_u8(0, 1) = scaleToU8(s.path);

                auto output = net_->forward(input);

                intgr_nn::Tensor target(1, 1);
                target.at_u8(0, 0) = s.safe ? 255 : 0;

                net_->backward(output, target);
            }
        }
    }

    void clearHistory() override { history_.clear(); }
    size_t historySize() const override { return history_.size(); }
};

//=============================================================================
// Puzzle 4: Sequence Learning
// Input: last_action (0=none, 64=A)
// Outputs: scoreA, scoreB (higher wins)
// Goal: Learn A first, then B
//=============================================================================
class SequenceNet : public IntgrNNWrapper {
    struct Sample {
        int16_t lastAction;
        int action;  // 0=A, 1=B
        bool success;
    };
    std::vector<Sample> history_;

    static constexpr int EPOCHS_PER_TRIAL = 50;

public:
    SequenceNet() {
        net_ = intgr_nn::IntegerGD::create(1, 4, 2, defaultConfig());
    }

    int chooseAction(int16_t lastAction) {
        intgr_nn::Tensor input(1, 1);
        input.at_u8(0, 0) = scaleToU8(lastAction);

        auto output = net_->forward(input);
        uint8_t scoreA = output.at_u8(0, 0);
        uint8_t scoreB = output.at_u8(0, 1);

        return (scoreA >= scoreB) ? 0 : 1;
    }

    // Overload for API compatibility (ignores availA/availB)
    int chooseAction(int16_t lastAction, int16_t /*availA*/, int16_t /*availB*/) {
        return chooseAction(lastAction);
    }

    // Get raw scores for display
    void getScores(int16_t lastAction, uint8_t& scoreA, uint8_t& scoreB) {
        intgr_nn::Tensor input(1, 1);
        input.at_u8(0, 0) = scaleToU8(lastAction);

        auto output = net_->forward(input);
        scoreA = output.at_u8(0, 0);
        scoreB = output.at_u8(0, 1);
    }

    // For display compatibility
    int16_t scoreA(int16_t lastAction) const {
        intgr_nn::Tensor input(1, 1);
        input.at_u8(0, 0) = scaleToU8(lastAction);
        auto output = net_->forward(input);
        return static_cast<int16_t>(output.at_u8(0, 0));
    }

    int16_t scoreB(int16_t lastAction) const {
        intgr_nn::Tensor input(1, 1);
        input.at_u8(0, 0) = scaleToU8(lastAction);
        auto output = net_->forward(input);
        return static_cast<int16_t>(output.at_u8(0, 1));
    }

    void learnFromOutcome(int16_t lastAction, int action, bool success) {
        history_.push_back({lastAction, action, success});

        for (int epoch = 0; epoch < EPOCHS_PER_TRIAL; epoch++) {
            for (const auto& s : history_) {
                intgr_nn::Tensor input(1, 1);
                input.at_u8(0, 0) = scaleToU8(s.lastAction);

                auto output = net_->forward(input);

                // Target: if success, reinforce chosen action
                // if failure, reinforce opposite action
                intgr_nn::Tensor target(1, 2);
                if (s.success) {
                    target.at_u8(0, 0) = (s.action == 0) ? 255 : 0;
                    target.at_u8(0, 1) = (s.action == 1) ? 255 : 0;
                } else {
                    target.at_u8(0, 0) = (s.action == 0) ? 0 : 255;
                    target.at_u8(0, 1) = (s.action == 1) ? 0 : 255;
                }

                net_->backward(output, target);
            }
        }
    }

    // Overload for API compatibility (ignores availA/availB)
    void learnFromOutcome(int16_t lastAction, int16_t /*availA*/, int16_t /*availB*/,
                          int action, bool success) {
        learnFromOutcome(lastAction, action, success);
    }

    void clearHistory() override { history_.clear(); }
    size_t historySize() const override { return history_.size(); }
};

//=============================================================================
// Puzzle 5: Composition
// Inputs: light, sizeA, sizeB
// Output: chooseA (>128 = yes)
// Goal: Light ON = pick larger, Light OFF = pick smaller
// Deep network (3→8→4→1) needs more training
//=============================================================================
class CompositionNet : public IntgrNNWrapper {
    struct Sample {
        int16_t light, sizeA, sizeB;
        bool chooseA;
    };
    std::vector<Sample> history_;

    // Deep network needs more epochs
    static constexpr int EPOCHS_PER_TRIAL = 100;

public:
    CompositionNet() {
        net_ = intgr_nn::IntegerGD::createDeep(3, {8, 4}, 1, defaultConfig());
    }

    bool chooseA(int16_t light, int16_t sizeA, int16_t sizeB) const {
        intgr_nn::Tensor input(1, 3);
        input.at_u8(0, 0) = scaleToU8(light);
        input.at_u8(0, 1) = scaleToU8(sizeA);
        input.at_u8(0, 2) = scaleToU8(sizeB);

        auto output = net_->forward(input);
        return interpretBool(output.at_u8(0, 0));
    }

    void learn(int16_t light, int16_t sizeA, int16_t sizeB, bool shouldChooseA) {
        history_.push_back({light, sizeA, sizeB, shouldChooseA});

        for (int epoch = 0; epoch < EPOCHS_PER_TRIAL; epoch++) {
            for (const auto& s : history_) {
                intgr_nn::Tensor input(1, 3);
                input.at_u8(0, 0) = scaleToU8(s.light);
                input.at_u8(0, 1) = scaleToU8(s.sizeA);
                input.at_u8(0, 2) = scaleToU8(s.sizeB);

                auto output = net_->forward(input);

                intgr_nn::Tensor target(1, 1);
                target.at_u8(0, 0) = s.chooseA ? 255 : 0;

                net_->backward(output, target);
            }
        }
    }

    void clearHistory() override { history_.clear(); }
    size_t historySize() const override { return history_.size(); }
};

//=============================================================================
// Utility: Get total model size across all networks
//=============================================================================
inline size_t totalModelSize(const GeneralizationNet& gen,
                              const FeatureSelectionNet& feat,
                              const XORNet& xorNet,
                              const SequenceNet& seq,
                              const CompositionNet& comp) {
    return gen.modelSizeBytes() + feat.modelSizeBytes() +
           xorNet.modelSizeBytes() + seq.modelSizeBytes() +
           comp.modelSizeBytes();
}

} // namespace enen
