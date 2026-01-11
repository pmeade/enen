# enen Demo: Neural Network Puzzles

## Overview

Five puzzles, each requiring a specific neural network capability. Each puzzle demonstrates that IntgrNN learns generalizable rules, not memorized responses.

**Core loop:**
1. enen enters puzzle
2. enen makes a choice
3. Wrong = reset, learn from failure
4. Right = proceed (or test with novel instance)
5. Repeat until enen solves consistently

**Success criteria:** enen solves NOVEL instances it hasn't seen before. This proves generalization.

---

## Puzzle 1: Generalization

### The Challenge

Two mushrooms appear. Colors vary randomly each trial. The LARGER one is always safe.

```
Trial 1: Small red (size 3), Large blue (size 7) → Blue correct
Trial 2: Large red (size 8), Small blue (size 4) → Red correct
Trial 3: Small green (size 2), Large yellow (size 6) → Yellow correct
```

**Rule:** size_A > size_B → choose A

### Network Architecture

Single perceptron with bias.

```
Inputs (4):
  - size_A (normalized 0-1)
  - size_B (normalized 0-1)
  - color_A (one-hot or hue, NOISE)
  - color_B (one-hot or hue, NOISE)

Output (1):
  - choice: >0.5 = A, <0.5 = B

Weights (4 + bias):
  w_sizeA, w_sizeB, w_colorA, w_colorB, bias

Target after learning:
  w_sizeA ≈ +1.0
  w_sizeB ≈ -1.0
  w_colorA ≈ 0.0
  w_colorB ≈ 0.0
  bias ≈ 0.0
```

### Learning

```cpp
struct GeneralizationNet {
    int16_t w_sizeA = 0;
    int16_t w_sizeB = 0;
    int16_t w_colorA = 0;
    int16_t w_colorB = 0;
    int16_t bias = 0;
    
    static constexpr int16_t SCALE = 128;  // Fixed point
    
    // Forward pass
    int16_t evaluate(int16_t sizeA, int16_t sizeB, int16_t colorA, int16_t colorB) {
        int32_t sum = bias;
        sum += (w_sizeA * sizeA) / SCALE;
        sum += (w_sizeB * sizeB) / SCALE;
        sum += (w_colorA * colorA) / SCALE;
        sum += (w_colorB * colorB) / SCALE;
        return static_cast<int16_t>(std::clamp(sum, -32768, 32767));
    }
    
    bool chooseA(int16_t sizeA, int16_t sizeB, int16_t colorA, int16_t colorB) {
        return evaluate(sizeA, sizeB, colorA, colorB) > 0;
    }
    
    // Hebbian-style learning
    void learn(int16_t sizeA, int16_t sizeB, int16_t colorA, int16_t colorB, bool shouldChooseA) {
        int16_t target = shouldChooseA ? SCALE : -SCALE;
        int16_t output = evaluate(sizeA, sizeB, colorA, colorB);
        int16_t error = target - output;
        
        constexpr int16_t LR = 8;  // Learning rate (out of 128)
        
        // Update weights toward reducing error
        w_sizeA += (error * sizeA * LR) / (SCALE * SCALE);
        w_sizeB += (error * sizeB * LR) / (SCALE * SCALE);
        w_colorA += (error * colorA * LR) / (SCALE * SCALE);
        w_colorB += (error * colorB * LR) / (SCALE * SCALE);
        bias += (error * LR) / SCALE;
    }
};
```

### Trial Generation

```cpp
struct MushroomTrial {
    int16_t sizeA;
    int16_t sizeB;
    int16_t colorA;
    int16_t colorB;
    bool correctIsA;
    
    static MushroomTrial generate(RNG& rng) {
        MushroomTrial t;
        t.sizeA = 32 + rng.next() % 96;   // 32-127
        t.sizeB = 32 + rng.next() % 96;
        while (t.sizeB == t.sizeA) {
            t.sizeB = 32 + rng.next() % 96;  // Ensure different sizes
        }
        t.colorA = rng.next() % 128;  // Random color (noise)
        t.colorB = rng.next() % 128;
        t.correctIsA = t.sizeA > t.sizeB;  // Larger is correct
        return t;
    }
};
```

### What It Proves

- enen learns a RULE (larger = correct), not a lookup table
- Color weights decay to zero — learned to ignore irrelevant features
- Works on mushrooms enen has never seen

### UI Display

```
PUZZLE 1: GENERALIZATION
Pick the larger mushroom. Color is irrelevant.

    🍄          🍄
   (big)      (small)
    red        blue
    
    [A]         [B]
```

After success on novel colors:
```
NARRATIVE
─────────────────────────────────
enen sees two mushrooms.
enen picks the small red one.
POISON — enen resets.
enen sees two mushrooms.
enen picks the large blue one.
SUCCESS — enen learned: bigger is safer!
NEW TRIAL: orange vs purple
enen picks the large purple one.
SUCCESS — enen generalized!
```

---

## Puzzle 2: Feature Selection

### The Challenge

Objects have COLOR and SHAPE. Only SHAPE matters.

```
Rule: CIRCLE = safe. SQUARE = dangerous.

Trial 1: Red circle, Blue square → Red circle correct
Trial 2: Blue circle, Blue square → Blue circle correct
Trial 3: Red square, Green circle → Green circle correct
Trial 4: Yellow circle, Yellow square → Yellow circle correct
```

**Rule:** shape_A == CIRCLE → choose A (regardless of color)

### Network Architecture

Two features per object, attention weights determine which matters.

```
Inputs (4):
  - color_A (0-127)
  - shape_A (0 = square, 127 = circle)
  - color_B (0-127)
  - shape_B (0 = square, 127 = circle)

Hidden computation:
  score_A = w_color * color_A + w_shape * shape_A
  score_B = w_color * color_B + w_shape * shape_B

Output:
  choose A if score_A > score_B

Target weights after learning:
  w_color ≈ 0 (ignore color)
  w_shape ≈ +1 (attend to shape)
```

### Learning

```cpp
struct FeatureSelectionNet {
    int16_t w_color = 64;   // Start uncertain
    int16_t w_shape = 64;
    
    static constexpr int16_t SCALE = 128;
    
    int16_t score(int16_t color, int16_t shape) {
        int32_t s = (w_color * color + w_shape * shape) / SCALE;
        return static_cast<int16_t>(s);
    }
    
    bool chooseA(int16_t colA, int16_t shpA, int16_t colB, int16_t shpB) {
        return score(colA, shpA) > score(colB, shpB);
    }
    
    void learn(int16_t colA, int16_t shpA, int16_t colB, int16_t shpB, bool shouldChooseA) {
        bool chose = chooseA(colA, shpA, colB, shpB);
        if (chose == shouldChooseA) return;  // Correct, no update
        
        constexpr int16_t LR = 12;
        
        if (shouldChooseA) {
            // Should have chosen A but chose B
            // Increase weight of features where A > B
            int16_t colorDiff = colA - colB;
            int16_t shapeDiff = shpA - shpB;
            w_color += (colorDiff * LR) / SCALE;
            w_shape += (shapeDiff * LR) / SCALE;
        } else {
            // Should have chosen B but chose A
            int16_t colorDiff = colB - colA;
            int16_t shapeDiff = shpB - shpA;
            w_color += (colorDiff * LR) / SCALE;
            w_shape += (shapeDiff * LR) / SCALE;
        }
        
        // Clamp weights to valid range
        w_color = std::clamp(w_color, (int16_t)0, (int16_t)127);
        w_shape = std::clamp(w_shape, (int16_t)0, (int16_t)127);
    }
};
```

### Trial Generation

```cpp
struct ShapeTrial {
    int16_t colorA, shapeA;
    int16_t colorB, shapeB;
    bool correctIsA;
    
    static ShapeTrial generate(RNG& rng) {
        ShapeTrial t;
        t.colorA = rng.next() % 128;
        t.colorB = rng.next() % 128;
        
        // One circle, one square
        if (rng.next() % 2 == 0) {
            t.shapeA = 127;  // Circle
            t.shapeB = 0;    // Square
            t.correctIsA = true;
        } else {
            t.shapeA = 0;    // Square
            t.shapeB = 127;  // Circle
            t.correctIsA = false;
        }
        return t;
    }
};
```

### What It Proves

- enen learns WHICH FEATURE MATTERS
- Color weight → 0, shape weight → high
- This is attention / feature selection
- Core capability for any real-world ML

### UI Display

```
PUZZLE 2: FEATURE SELECTION
One shape is safe. Figure out which.

    ■           ●
   red         blue
   
   [A]          [B]

WHAT ENEN KNOWS
───────────────────────
Attention:
  Color: ██░░░░░░░░ 20%
  Shape: ████████░░ 80%
```

---

## Puzzle 3: Context-Dependent Choice (XOR)

### The Challenge

A light on the wall (externally controlled) determines which path is safe.

```
Light ON  + Left  = Safe
Light ON  + Right = Danger
Light OFF + Left  = Danger
Light OFF + Right = Safe
```

This is XOR — not linearly separable.

### Network Architecture

Two-layer network required for XOR.

```
Inputs (2):
  - light (0 = off, 127 = on)
  - path (0 = left, 127 = right)

Hidden layer (2 neurons):
  h1 = relu(w1_light * light + w1_path * path + b1)
  h2 = relu(w2_light * light + w2_path * path + b2)

Output (1):
  safe = w_out1 * h1 + w_out2 * h2 + b_out
  
XOR solution (one of many):
  h1 activates when (light AND path)  — both high
  h2 activates when (NOT light AND NOT path) — both low
  output = -(h1 + h2)  — danger when either extreme
```

### Learning

```cpp
struct XORNet {
    // Layer 1: 2 inputs → 2 hidden
    int16_t w1_light = 0, w1_path = 0, b1 = 0;
    int16_t w2_light = 0, w2_path = 0, b2 = 0;
    
    // Layer 2: 2 hidden → 1 output
    int16_t w_out1 = 0, w_out2 = 0, b_out = 0;
    
    static constexpr int16_t SCALE = 128;
    
    int16_t relu(int16_t x) { return x > 0 ? x : 0; }
    
    int16_t evaluate(int16_t light, int16_t path) {
        // Hidden layer
        int16_t h1 = relu((w1_light * light + w1_path * path) / SCALE + b1);
        int16_t h2 = relu((w2_light * light + w2_path * path) / SCALE + b2);
        
        // Output
        int32_t out = (w_out1 * h1 + w_out2 * h2) / SCALE + b_out;
        return static_cast<int16_t>(std::clamp(out, (int32_t)-128, (int32_t)127));
    }
    
    bool isSafe(int16_t light, int16_t path) {
        return evaluate(light, path) > 0;
    }
    
    // Simplified learning: adjust weights based on error
    void learn(int16_t light, int16_t path, bool shouldBeSafe) {
        int16_t target = shouldBeSafe ? 64 : -64;
        int16_t output = evaluate(light, path);
        int16_t error = target - output;
        
        if (std::abs(error) < 16) return;  // Close enough
        
        constexpr int16_t LR = 4;
        
        // Compute hidden activations for gradient
        int16_t h1_pre = (w1_light * light + w1_path * path) / SCALE + b1;
        int16_t h2_pre = (w2_light * light + w2_path * path) / SCALE + b2;
        int16_t h1 = relu(h1_pre);
        int16_t h2 = relu(h2_pre);
        
        // Output layer gradients
        w_out1 += (error * h1 * LR) / (SCALE * SCALE);
        w_out2 += (error * h2 * LR) / (SCALE * SCALE);
        b_out += (error * LR) / SCALE;
        
        // Hidden layer gradients (backprop through relu)
        if (h1_pre > 0) {
            int16_t grad1 = (error * w_out1) / SCALE;
            w1_light += (grad1 * light * LR) / (SCALE * SCALE);
            w1_path += (grad1 * path * LR) / (SCALE * SCALE);
            b1 += (grad1 * LR) / SCALE;
        }
        if (h2_pre > 0) {
            int16_t grad2 = (error * w_out2) / SCALE;
            w2_light += (grad2 * light * LR) / (SCALE * SCALE);
            w2_path += (grad2 * path * LR) / (SCALE * SCALE);
            b2 += (grad2 * LR) / SCALE;
        }
    }
};
```

### Trial Generation

```cpp
struct XORTrial {
    bool lightOn;
    bool choosingRight;
    bool isSafe;
    
    static XORTrial generate(RNG& rng) {
        XORTrial t;
        t.lightOn = rng.next() % 2;
        t.choosingRight = rng.next() % 2;
        t.isSafe = t.lightOn ^ t.choosingRight;  // XOR!
        return t;
    }
    
    int16_t lightInput() { return lightOn ? 127 : 0; }
    int16_t pathInput() { return choosingRight ? 127 : 0; }
};
```

### What It Proves

- XOR is the classic "perceptrons can't do this" problem
- Requires hidden layer, non-linear activation
- enen learns a NON-LINEAR decision boundary
- This is the "hero moment" of the demo

### UI Display

```
PUZZLE 3: CONTEXT MATTERS
The light determines which path is safe.

        💡 ON
        
    ←─────┼─────→
    LEFT       RIGHT
    
   [A]          [B]

NARRATIVE
─────────────────────────────────
Light is ON.
enen chooses Right.
DANGER — enen resets.
Light is ON.
enen chooses Left.
SAFE — enen proceeds!
Light switches OFF.
enen chooses Left.
DANGER — enen resets.
enen realizes: light changes the rule!
```

---

## Puzzle 4: Sequence Learning

### The Challenge

Two buttons. Must press in correct ORDER.

```
Correct: A then B → door opens
Wrong: B then A → reset
Wrong: A then A → nothing
Wrong: B then B → nothing
Wrong: just A → nothing (need second step)
Wrong: just B → reset
```

**Rule:** First action must be A. Second action must be B.

### Network Architecture

State-aware network — input includes last action.

```
Inputs (3):
  - last_action (0 = none, 64 = A, 127 = B)
  - available_A (127 if A available)
  - available_B (127 if B available)

Output (2):
  - score_A: desirability of pressing A
  - score_B: desirability of pressing B

Target behavior:
  If last_action == none → press A (score_A high)
  If last_action == A → press B (score_B high)
  If last_action == B → already failed
```

### Learning

```cpp
struct SequenceNet {
    // Weights for choosing A
    int16_t wA_last = 0, wA_availA = 64, wA_availB = 0, bA = 0;
    
    // Weights for choosing B
    int16_t wB_last = 0, wB_availA = 0, wB_availB = 64, bB = 0;
    
    static constexpr int16_t SCALE = 128;
    
    int16_t scoreA(int16_t last, int16_t availA, int16_t availB) {
        return (wA_last * last + wA_availA * availA + wA_availB * availB) / SCALE + bA;
    }
    
    int16_t scoreB(int16_t last, int16_t availA, int16_t availB) {
        return (wB_last * last + wB_availA * availA + wB_availB * availB) / SCALE + bB;
    }
    
    int chooseAction(int16_t last, int16_t availA, int16_t availB) {
        int16_t sA = scoreA(last, availA, availB);
        int16_t sB = scoreB(last, availA, availB);
        return (sA > sB) ? 0 : 1;  // 0 = A, 1 = B
    }
    
    void learnFromOutcome(int16_t last, int16_t availA, int16_t availB, 
                          int action, bool success) {
        constexpr int16_t LR = 8;
        
        if (success) {
            // Reinforce chosen action
            if (action == 0) {
                wA_last += (last * LR) / SCALE;
                wA_availA += (availA * LR) / SCALE;
                bA += LR;
            } else {
                wB_last += (last * LR) / SCALE;
                wB_availB += (availB * LR) / SCALE;
                bB += LR;
            }
        } else {
            // Punish chosen action
            if (action == 0) {
                wA_last -= (last * LR) / SCALE;
                bA -= LR;
            } else {
                wB_last -= (last * LR) / SCALE;
                bB -= LR;
            }
        }
    }
};
```

### State Machine

```cpp
enum class SequenceState { START, PRESSED_A, SUCCESS, FAIL };

struct SequencePuzzle {
    SequenceState state = SequenceState::START;
    
    bool pressButton(int button) {  // 0 = A, 1 = B
        switch (state) {
            case SequenceState::START:
                if (button == 0) {
                    state = SequenceState::PRESSED_A;
                    return true;  // Good so far
                } else {
                    state = SequenceState::FAIL;
                    return false;  // Wrong! Reset
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
    
    void reset() { state = SequenceState::START; }
};
```

### What It Proves

- enen understands ORDER, not just "both pressed"
- Network uses last_action as input — temporal context
- This is sequential decision-making
- Core capability for any multi-step task

### UI Display

```
PUZZLE 4: SEQUENCE
Two buttons. Order matters.

      [A]    [B]
      
    Last action: A
    
NARRATIVE
─────────────────────────────────
enen presses B.
FAIL — wrong order! Reset.
enen presses A.
Good start...
enen presses A again.
FAIL — needed B next! Reset.
enen presses A.
Good start...
enen presses B.
SUCCESS — door opens!
enen learned: A then B!
```

---

## Puzzle 5: Composition

### The Challenge

Combines Puzzle 1 (size comparison) + Puzzle 3 (context switch).

```
Light ON  → pick LARGER
Light OFF → pick SMALLER
```

### Network Architecture

Context-gated size comparison.

```
Inputs (3):
  - light (0 = off, 127 = on)
  - size_A (0-127)
  - size_B (0-127)

Architecture:
  // Compute size difference
  diff = size_A - size_B  // Positive means A is larger
  
  // Context modulates interpretation
  // Light ON: positive diff → choose A (larger)
  // Light OFF: negative diff → choose A (smaller)
  
  hidden = w_light * light + w_diff * diff + b
  output = activation(hidden)

Target behavior:
  Light ON:  output = sign(size_A - size_B)
  Light OFF: output = sign(size_B - size_A)
  
This is: output = XOR(light, size_A > size_B)? A : B
```

### Learning

```cpp
struct CompositionNet {
    // Two-layer network like XOR
    int16_t w1_light = 0, w1_diff = 0, b1 = 0;
    int16_t w2_light = 0, w2_diff = 0, b2 = 0;
    int16_t w_out1 = 0, w_out2 = 0, b_out = 0;
    
    static constexpr int16_t SCALE = 128;
    
    int16_t relu(int16_t x) { return x > 0 ? x : 0; }
    
    bool chooseA(int16_t light, int16_t sizeA, int16_t sizeB) {
        int16_t diff = sizeA - sizeB;
        
        int16_t h1 = relu((w1_light * light + w1_diff * diff) / SCALE + b1);
        int16_t h2 = relu((w2_light * light + w2_diff * diff) / SCALE + b2);
        
        int16_t out = (w_out1 * h1 + w_out2 * h2) / SCALE + b_out;
        return out > 0;
    }
    
    void learn(int16_t light, int16_t sizeA, int16_t sizeB, bool shouldChooseA) {
        // Similar backprop as XOR puzzle
        // ... (same pattern as XORNet::learn)
    }
};
```

### Trial Generation

```cpp
struct CompositionTrial {
    bool lightOn;
    int16_t sizeA, sizeB;
    bool correctIsA;
    
    static CompositionTrial generate(RNG& rng) {
        CompositionTrial t;
        t.lightOn = rng.next() % 2;
        t.sizeA = 32 + rng.next() % 96;
        t.sizeB = 32 + rng.next() % 96;
        while (t.sizeB == t.sizeA) {
            t.sizeB = 32 + rng.next() % 96;
        }
        
        bool aIsLarger = t.sizeA > t.sizeB;
        
        // Light ON → pick larger, Light OFF → pick smaller
        if (t.lightOn) {
            t.correctIsA = aIsLarger;
        } else {
            t.correctIsA = !aIsLarger;
        }
        return t;
    }
};
```

### What It Proves

- enen combines learned rules
- Context (light) modulates a comparison operation
- Not re-learning from scratch — composing capabilities
- This is transfer learning / compositional generalization

### UI Display

```
PUZZLE 5: COMPOSITION
The light changes the rule.

        💡 OFF
        
    🍄          🍄
   large       small
   
   [A]          [B]

NARRATIVE
─────────────────────────────────
Light is OFF.
enen picks the large mushroom.
WRONG — light off means pick smaller!
Light is OFF.
enen picks the small mushroom.
CORRECT!
Light switches ON.
enen picks the large mushroom.
CORRECT — enen adapted to context!
enen learned: light flips the rule!
```

---

## Demo Flow

### Per-Puzzle Structure

```
1. INTRO: Show puzzle, explain challenge (narration)
2. ATTEMPT 1: enen likely fails (doesn't know rule)
3. RESET: enen resets, weights update
4. ATTEMPTS 2-N: enen tries again, learning
5. FIRST SUCCESS: enen solves it
6. NOVELTY TEST: New instance (different inputs)
7. GENERALIZATION: enen succeeds on novel instance!
8. PROCEED: Next puzzle
```

### Timing (5 min total)

```
0:00 - Hook (15 sec)
       "What if game AI could learn like an animal?"
       
0:15 - Problem statement (30 sec)
       "Current game AI: scripted or cloud-dependent"
       
0:45 - Puzzle 1: Generalization (45 sec)
       enen learns "larger = correct"
       Novel colors → still works
       
1:30 - Puzzle 2: Feature Selection (45 sec)
       enen learns to ignore color, attend to shape
       
2:15 - Puzzle 3: XOR (60 sec) — HERO MOMENT
       enen learns non-linear context rule
       This is what simple nets CAN'T do
       
3:15 - Puzzle 4: Sequence (30 sec)
       enen learns "A then B" order
       
3:45 - Puzzle 5: Composition (30 sec)
       enen combines rules from 1 + 3
       
4:15 - Technical reveal (30 sec)
       "All integer math. 2KB total. No cloud."
       
4:45 - Call to action (15 sec)
       "IntgrNN. Available now."
```

---

## Summary Table

| Puzzle | Network | Inputs | Hidden | Proves |
|--------|---------|--------|--------|--------|
| 1. Generalization | Perceptron | 4 | 0 | Learns rules, not instances |
| 2. Feature Selection | Weighted attention | 4 | 0 | Learns what to ignore |
| 3. XOR (Context) | 2-layer | 2 | 2 | Non-linear boundaries |
| 4. Sequence | State-aware | 3 | 0 | Temporal reasoning |
| 5. Composition | 2-layer | 3 | 2 | Combines learned rules |

**Total network memory:** ~200 bytes across all puzzles

---

## Implementation Checklist

- [ ] GeneralizationNet class
- [ ] FeatureSelectionNet class  
- [ ] XORNet class
- [ ] SequenceNet class
- [ ] CompositionNet class
- [ ] Trial generators for each puzzle
- [ ] Reset/retry logic
- [ ] Novelty instance generation
- [ ] Narrative message triggers
- [ ] UI for each puzzle type
- [ ] Learning visualization (weight changes)
- [ ] Demo script/automation mode

---

## Key Demo Messages

These are the "money shots" for the video:

1. **Generalization:** "enen has never seen these colors before, but still picks correctly."

2. **Feature Selection:** "Watch the attention weights — color drops to zero."

3. **XOR:** "This is the problem that proved simple perceptrons weren't enough. enen's network solves it."

4. **Sequence:** "enen learned ORDER matters, not just 'both pressed.'"

5. **Composition:** "enen didn't re-learn from scratch. It combined what it already knew."

**Closing:** "Five puzzles. Five neural architectures. 200 bytes. No cloud. No pre-training. Just learning."
