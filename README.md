# enen

A small creature learns to solve five puzzles using neural networks that fit in 206 bytes.

## What This Is

enen is a demo of runtime machine learning. The creature starts with random weights — no pre-training, no loaded answers. Each trial, it guesses, gets feedback, and retrains on everything it remembers. After a few tries, the mistakes stop repeating.

The entire brain that does this fits in **206 bytes**. That's smaller than the paragraph you just read.

![demo](https://github.com/user-attachments/assets/614045c2-e886-4fb3-85ad-4a392e8819b6)

> **Note**
>
> This repository is an educational demonstration and showcase.
> It does not contain the IntgrNN engine or its proprietary learning algorithms.
> IntgrNN is a separate product with its own licensing terms.

## The Five Puzzles

| Puzzle | The Problem | Network | Bytes |
|--------|-------------|---------|-------|
| Size | Bigger mushroom is safe; color is noise | 4→8→1 | 49 |
| Exceptions | Circles safe, squares dangerous — except blue squares | 4→8→1 | 49 |
| Context | Light ON = go left, Light OFF = go right (XOR problem) | 2→4→1 | 17 |
| Order | Press A then B; only input is last key pressed | 1→4→2 | 18 |
| Everything | Light ON = pick bigger, Light OFF = pick smaller | 3→8→4→1 | 73 |

Each puzzle teaches a different skill: ignoring irrelevant features, learning exceptions, using context, responding to history, and combining abilities.

## Why Integer Neural Networks?

The demo uses [IntgrNN](https://github.com/double-star-games/intgr_nn), an 8-bit integer neural network library.

**Size**: Each parameter is one byte. All five networks total 206 bytes — small enough to live in L1 cache.

**Speed**: Integer math is fast. A forward pass takes single-digit microseconds. Learning fits comfortably in a 16ms frame budget.

**Determinism**: Integer operations produce identical results across platforms. Same seed, same weights. Same inputs, same outputs. Same learning, same behavior.

For game developers, this opens a new design space: agents that learn during play, behavior that adapts to the player, systems that are fully deterministic and debuggable but still surprising.

## Building

### Linux / macOS

```bash
git clone https://github.com/pmeade/enen.git
cd enen
./setup                      # Downloads IntgrNN
mkdir build && cd build
cmake .. && make
```

### Windows

```powershell
git clone https://github.com/pmeade/enen.git
cd enen
.\setup.bat                  # Downloads IntgrNN
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

Requires: C++17 compiler, CMake 3.15+, GitHub CLI (`gh`)

## Running

```bash
# Linux / macOS
./enen           # Interactive demo
./enen-autorun   # Auto-run for video recording (asciinema v2 format)

# Windows (from build directory)
.\Release\enen.exe
.\Release\enen-autorun.exe
```

Video recording:
```bash
./enen-autorun > demo.cast
agg demo.cast demo.mp4
```

## License

enen is released under the [MIT License](LICENSE).

enen depends on [IntgrNN](https://github.com/double-star-games/intgr_nn), which has its own license. See the IntgrNN repository for details.
