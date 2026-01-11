# enen

A tiny creature learns to solve puzzles using a neural network small enough to fit in a tweet.

## What This Is

enen is a demo showing real machine learning in action. You watch a small creature:

1. Start knowing nothing
2. Guess answers to puzzles
3. Get feedback (right or wrong)
4. Update its brain (backpropagation)
5. Eventually figure out the pattern

No pre-training. No tricks. Just a tiny network learning from scratch.

## The Five Puzzles

| Puzzle | What enen Learns |
|--------|------------------|
| Size | Ignore distractions (color doesn't matter) |
| Exceptions | Rules have exceptions (blue squares are safe) |
| Context | The answer depends on context (check the light) |
| Order | Sequence matters (A then B) |
| Everything | Combine multiple skills |

## Brain Size

enen's entire neural network is under 200 bytes. That's smaller than this paragraph.

The demo uses [IntgrNN](https://github.com/pmeade/intgr_nn), an 8-bit integer neural network library designed for embedded systems.

## Building

### Prerequisites

- C++17 compiler (GCC 7+ or Clang 5+)
- CMake 3.14+
- GitHub CLI (`gh`) for automatic setup

### Setup

```bash
git clone https://github.com/pmeade/enen.git
cd enen
./setup
```

The setup script downloads [IntgrNN](https://github.com/pmeade/intgr_nn), which is required to build enen.

### Build

```bash
mkdir build && cd build
cmake ..
make
```

### Run

```bash
./enen           # Interactive demo
./enen-autorun   # Auto-run for video recording (asciinema format)
```

## Video Recording

The `enen-autorun` executable outputs [asciinema v2](https://github.com/asciinema/asciinema/blob/develop/doc/asciicast-v2.md) format:

```bash
./enen-autorun > demo.cast
agg demo.cast demo.gif    # Convert to GIF
agg demo.cast demo.mp4    # Convert to MP4
```

## License

enen is released under the [MIT License](LICENSE).

**Note:** enen depends on [IntgrNN](https://github.com/pmeade/intgr_nn), which has its own license. See the IntgrNN repository for licensing details.
