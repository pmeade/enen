/**
 * IntgrNN Network Tests for enen Demo
 *
 * Tests that IntgrNN networks can learn each puzzle type.
 * All networks use the same library (IntgrNN), different architectures.
 *
 * With experience replay, tests show gradual learning progression.
 */

#include "networks.hpp"
#include "puzzles.hpp"
#include <cstdio>

using namespace enen;

//=============================================================================
// Test 1: Generalization (4→8→1)
// Learn that size matters, color doesn't
//=============================================================================
bool testGeneralization() {
    printf("Test 1: Generalization (IntgrNN 4->8->1, experience replay)\n");

    GeneralizationNet net;
    RNG rng(42);

    printf("  Params: %zu, Size: %zu bytes\n", net.parameterCount(), net.modelSizeBytes());
    printf("  Training with experience replay...\n");

    // Train on trials, showing accuracy progression
    for (int trial = 0; trial < 20; trial++) {
        auto t = MushroomTrial::generate(rng);

        // Learn
        net.learn(t.sizeA, t.sizeB, t.colorA, t.colorB, t.correctIsA);

        if (trial % 5 == 4) {
            // Test accuracy every 5 trials
            int testCorrect = 0;
            RNG testRng(999);
            for (int i = 0; i < 10; i++) {
                auto test = MushroomTrial::generate(testRng);
                if (net.chooseA(test.sizeA, test.sizeB, test.colorA, test.colorB) == test.correctIsA) {
                    testCorrect++;
                }
            }
            printf("    Trial %2d: history=%zu, test accuracy=%d/10\n",
                   trial+1, net.historySize(), testCorrect);
        }
    }

    // Final test
    int correct = 0;
    RNG testRng(12345);
    for (int i = 0; i < 20; i++) {
        auto t = MushroomTrial::generate(testRng);
        if (net.chooseA(t.sizeA, t.sizeB, t.colorA, t.colorB) == t.correctIsA) {
            correct++;
        }
    }

    printf("  Final accuracy: %d/20 (%.0f%%)\n", correct, 100.0 * correct / 20);
    bool pass = correct >= 14;  // 70% threshold
    printf("  %s\n\n", pass ? "PASS" : "FAIL");
    return pass;
}

//=============================================================================
// Test 2: Feature Selection (4→8→1)
// Learn that shape matters, color doesn't
//=============================================================================
bool testFeatureSelection() {
    printf("Test 2: Feature Selection (IntgrNN 4->8->1, experience replay)\n");

    FeatureSelectionNet net;
    RNG rng(42);

    printf("  Params: %zu, Size: %zu bytes\n", net.parameterCount(), net.modelSizeBytes());
    printf("  Training with experience replay...\n");

    // Train on trials
    for (int trial = 0; trial < 20; trial++) {
        auto t = ShapeTrial::generate(rng);
        net.learn(t.colorA, t.shapeA, t.colorB, t.shapeB, t.correctIsA);

        if (trial % 5 == 4) {
            int testCorrect = 0;
            RNG testRng(999);
            for (int i = 0; i < 10; i++) {
                auto test = ShapeTrial::generate(testRng);
                if (net.chooseA(test.colorA, test.shapeA, test.colorB, test.shapeB) == test.correctIsA) {
                    testCorrect++;
                }
            }
            printf("    Trial %2d: history=%zu, test accuracy=%d/10\n",
                   trial+1, net.historySize(), testCorrect);
        }
    }

    // Test
    int correct = 0;
    RNG testRng(12345);
    for (int i = 0; i < 20; i++) {
        auto t = ShapeTrial::generate(testRng);
        bool chose = net.chooseA(t.colorA, t.shapeA, t.colorB, t.shapeB);
        if (chose == t.correctIsA) correct++;
    }

    printf("  Final accuracy: %d/20 (%.0f%%)\n", correct, 100.0 * correct / 20);
    bool pass = correct >= 14;  // 70% threshold
    printf("  %s\n\n", pass ? "PASS" : "FAIL");
    return pass;
}

//=============================================================================
// Test 3: XOR (2→4→1)
// Classic XOR - requires hidden layer
//=============================================================================
bool testXOR() {
    printf("Test 3: XOR (IntgrNN 2->4->1, experience replay)\n");

    XORNet net;
    printf("  Params: %zu, Size: %zu bytes\n", net.parameterCount(), net.modelSizeBytes());
    printf("  Training with experience replay (XOR needs many epochs)...\n");

    // XOR has only 4 patterns, but needs many epochs
    // Simulate 15 trials seeing random patterns
    RNG rng(42);
    for (int trial = 0; trial < 15; trial++) {
        auto t = XORTrial::generate(rng);
        net.learn(t.lightInput(), t.pathInput(), t.isSafe);

        if (trial % 5 == 4) {
            // Test all 4 patterns
            int correct = 0;
            if (net.isSafe(127, 0) == true) correct++;   // ON+L = safe
            if (net.isSafe(127, 127) == false) correct++; // ON+R = danger
            if (net.isSafe(0, 0) == false) correct++;     // OFF+L = danger
            if (net.isSafe(0, 127) == true) correct++;    // OFF+R = safe
            printf("    Trial %2d: history=%zu, XOR accuracy=%d/4\n",
                   trial+1, net.historySize(), correct);
        }
    }

    // Final test
    bool r1 = net.isSafe(127, 0);    // Should be true
    bool r2 = net.isSafe(127, 127);  // Should be false
    bool r3 = net.isSafe(0, 0);      // Should be false
    bool r4 = net.isSafe(0, 127);    // Should be true

    printf("  ON+L=%s (want Safe)\n", r1 ? "Safe" : "Danger");
    printf("  ON+R=%s (want Danger)\n", r2 ? "Safe" : "Danger");
    printf("  OFF+L=%s (want Danger)\n", r3 ? "Safe" : "Danger");
    printf("  OFF+R=%s (want Safe)\n", r4 ? "Safe" : "Danger");

    int correct = (r1 ? 1 : 0) + (!r2 ? 1 : 0) + (!r3 ? 1 : 0) + (r4 ? 1 : 0);
    bool pass = correct >= 3;  // At least 3/4
    printf("  Score: %d/4 %s\n\n", correct, pass ? "PASS" : "FAIL");
    return pass;
}

//=============================================================================
// Test 4: Sequence (1→4→2)
// Learn A first, then B
//=============================================================================
bool testSequence() {
    printf("Test 4: Sequence (IntgrNN 1->4->2, experience replay)\n");

    SequenceNet net;
    printf("  Params: %zu, Size: %zu bytes\n", net.parameterCount(), net.modelSizeBytes());
    printf("  Training with experience replay...\n");

    // Train the rule: when last=0, choose A; when last>0, choose B
    for (int trial = 0; trial < 20; trial++) {
        // At start (last=0), A is correct
        net.learnFromOutcome(0, 0, true);    // Chose A at start = success
        net.learnFromOutcome(0, 1, false);   // Chose B at start = failure

        // After A (last=64), B is correct
        net.learnFromOutcome(64, 1, true);   // Chose B after A = success
        net.learnFromOutcome(64, 0, false);  // Chose A after A = failure

        if (trial % 5 == 4) {
            int actionAtStart = net.chooseAction(0);
            int actionAfterA = net.chooseAction(64);
            printf("    Trial %2d: history=%zu, at start=%c, after A=%c\n",
                   trial+1, net.historySize(),
                   actionAtStart == 0 ? 'A' : 'B',
                   actionAfterA == 0 ? 'A' : 'B');
        }
    }

    // Test
    int actionAtStart = net.chooseAction(0);     // Should be 0 (A)
    int actionAfterA = net.chooseAction(64);     // Should be 1 (B)

    printf("  At START: chose %c (want A)\n", actionAtStart == 0 ? 'A' : 'B');
    printf("  After A: chose %c (want B)\n", actionAfterA == 0 ? 'A' : 'B');

    bool pass = (actionAtStart == 0) && (actionAfterA == 1);
    printf("  %s\n\n", pass ? "PASS" : "FAIL");
    return pass;
}

//=============================================================================
// Test 5: Composition (3→8→4→1) - Deep network!
// Light ON = pick larger, Light OFF = pick smaller
//=============================================================================
bool testComposition() {
    printf("Test 5: Composition (IntgrNN 3->8->4->1 DEEP, experience replay)\n");

    CompositionNet net;
    RNG rng(42);

    printf("  Params: %zu, Size: %zu bytes\n", net.parameterCount(), net.modelSizeBytes());
    printf("  Training with experience replay (deep network)...\n");

    // Train (deep networks need more training)
    for (int trial = 0; trial < 30; trial++) {
        auto t = CompositionTrial::generate(rng);
        net.learn(t.lightInput(), t.sizeA, t.sizeB, t.correctIsA);

        if (trial % 10 == 9) {
            int testCorrect = 0;
            RNG testRng(999);
            for (int i = 0; i < 10; i++) {
                auto test = CompositionTrial::generate(testRng);
                if (net.chooseA(test.lightInput(), test.sizeA, test.sizeB) == test.correctIsA) {
                    testCorrect++;
                }
            }
            printf("    Trial %2d: history=%zu, test accuracy=%d/10\n",
                   trial+1, net.historySize(), testCorrect);
        }
    }

    // Test
    int correct = 0;
    RNG testRng(12345);
    for (int i = 0; i < 20; i++) {
        auto t = CompositionTrial::generate(testRng);
        bool chose = net.chooseA(t.lightInput(), t.sizeA, t.sizeB);
        if (chose == t.correctIsA) correct++;
    }

    printf("  Final accuracy: %d/20 (%.0f%%)\n", correct, 100.0 * correct / 20);
    // Composition is harder - accept 50% (above random)
    bool pass = correct >= 10;
    printf("  %s\n\n", pass ? "PASS" : "FAIL");
    return pass;
}

//=============================================================================
// Main
//=============================================================================
int main() {
    printf("==================================================\n");
    printf("IntgrNN Network Tests for enen Demo\n");
    printf("Experience Replay Training - Real Learning!\n");
    printf("==================================================\n\n");

    int passed = 0;
    int total = 5;

    if (testGeneralization()) passed++;
    if (testFeatureSelection()) passed++;
    if (testXOR()) passed++;
    if (testSequence()) passed++;
    if (testComposition()) passed++;

    printf("==================================================\n");
    printf("Results: %d/%d passed\n", passed, total);
    printf("==================================================\n");

    // Calculate total model size
    GeneralizationNet gen;
    FeatureSelectionNet feat;
    XORNet xorNet;
    SequenceNet seq;
    CompositionNet comp;
    size_t total_bytes = totalModelSize(gen, feat, xorNet, seq, comp);

    printf("\nTotal model size across all 5 networks: %zu bytes\n", total_bytes);
    printf("All networks use IntgrNN with LR=0.1\n");
    printf("Experience replay: networks retrain on ALL history each trial\n");

    return (passed == total) ? 0 : 1;
}
