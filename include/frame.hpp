#pragma once
/**
 * Frame rendering infrastructure for enen Demo
 *
 * Provides:
 * - TextBuffer: Fixed 80x24 character buffer with drawing primitives
 * - FrameWriter: Outputs frames in asciinema v2 format with timing
 * - ansi:: namespace: Terminal escape codes for amber monochrome
 *
 * Design: Encapsulates all frame state (time, first_frame flag) in FrameWriter
 * rather than using global variables. TextBuffer is stateless and reusable.
 */

#include <string>
#include <cstring>
#include <cstdio>
#include <ctime>

namespace enen {

//=============================================================================
// Terminal Constants
//=============================================================================
namespace terminal {
    constexpr int WIDTH = 80;
    constexpr int HEIGHT = 24;
}

//=============================================================================
// ANSI Escape Codes - Amber Monochrome (24-bit color)
//=============================================================================
namespace ansi {
    // Amber foreground: #f2b233 (242, 178, 51)
    constexpr const char* FG = "\x1b[38;2;242;178;51m";

    // Near-black background: #0c0c0a (12, 12, 10)
    constexpr const char* BG = "\x1b[48;2;12;12;10m";

    // Combined: normal intensity + amber fg + dark bg
    constexpr const char* AMBER = "\x1b[22m\x1b[38;2;242;178;51m\x1b[48;2;12;12;10m";

    // Initial setup: clear + home + colors
    constexpr const char* INIT = "\x1b[2J\x1b[H\x1b[22m\x1b[38;2;242;178;51m\x1b[48;2;12;12;10m";

    // Subsequent frames: just clear + home (colors persist)
    constexpr const char* CLEAR = "\x1b[2J\x1b[H";
}

//=============================================================================
// TextBuffer - Fixed-size character buffer with drawing primitives
//
// All coordinates are (x, y) where:
//   x: column (0 = left, WIDTH-1 = right)
//   y: row (0 = top, HEIGHT-1 = bottom)
//=============================================================================
class TextBuffer {
public:
    void clear() {
        for (int y = 0; y < terminal::HEIGHT; y++) {
            std::memset(buffer_[y], ' ', terminal::WIDTH);
            buffer_[y][terminal::WIDTH] = '\0';
        }
    }

    void putString(int x, int y, const char* str) {
        if (y < 0 || y >= terminal::HEIGHT) return;
        int len = std::strlen(str);
        for (int i = 0; i < len && x + i < terminal::WIDTH; i++) {
            if (x + i >= 0) {
                buffer_[y][x + i] = str[i];
            }
        }
    }

    void putChar(int x, int y, char c) {
        if (x >= 0 && x < terminal::WIDTH && y >= 0 && y < terminal::HEIGHT) {
            buffer_[y][x] = c;
        }
    }

    void drawHLine(int x, int y, int length, char c = '-') {
        for (int i = 0; i < length && x + i < terminal::WIDTH; i++) {
            if (y >= 0 && y < terminal::HEIGHT && x + i >= 0) {
                buffer_[y][x + i] = c;
            }
        }
    }

    // Access raw buffer for frame output
    const char* line(int y) const {
        return (y >= 0 && y < terminal::HEIGHT) ? buffer_[y] : "";
    }

private:
    char buffer_[terminal::HEIGHT][terminal::WIDTH + 1];
};

//=============================================================================
// FrameWriter - Outputs asciinema v2 format frames with timing
//
// Encapsulates:
// - Current timestamp
// - First-frame color initialization flag
// - JSON escaping for asciinema format
//=============================================================================
class FrameWriter {
public:
    FrameWriter() : time_(0.0), firstFrame_(true) {}

    // Write asciinema header (call once at start)
    void writeHeader() {
        time_t now = std::time(nullptr);
        std::printf("{\"version\": 2, \"width\": %d, \"height\": %d, "
                    "\"timestamp\": %ld, \"env\": {\"TERM\": \"xterm-256color\"}}\n",
                    terminal::WIDTH, terminal::HEIGHT, now);
    }

    // Output a frame from TextBuffer
    void outputFrame(const TextBuffer& buffer, double pauseAfter = 0.0) {
        std::string content = buildFrameContent(buffer);
        outputRawFrame(content, pauseAfter);
    }

    // Output a raw string frame (for custom content)
    void outputRawFrame(const std::string& content, double pauseAfter = 0.0) {
        std::string escaped = escapeForJson(content);
        std::printf("[%.3f, \"o\", \"%s\"]\n", time_, escaped.c_str());
        time_ += pauseAfter;
    }

    double currentTime() const { return time_; }

private:
    double time_;
    bool firstFrame_;

    std::string buildFrameContent(const TextBuffer& buffer) {
        std::string result;

        // First frame sets colors, subsequent frames just clear
        if (firstFrame_) {
            result = ansi::INIT;
            firstFrame_ = false;
        } else {
            result = ansi::CLEAR;
        }

        // Append each line (already padded to WIDTH)
        for (int y = 0; y < terminal::HEIGHT; y++) {
            result += buffer.line(y);
            result += "\n";
        }

        return result;
    }

    static std::string escapeForJson(const std::string& content) {
        std::string escaped;
        escaped.reserve(content.size() * 2);

        for (char c : content) {
            switch (c) {
                case '"':  escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\n': escaped += "\\r\\n"; break;
                case '\r': break;  // Skip carriage returns
                case '\t': escaped += "\\t"; break;
                default:
                    if (c >= 32 && c < 127) {
                        escaped += c;
                    } else if (c < 32) {
                        char buf[8];
                        std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                        escaped += buf;
                    }
                    break;
            }
        }

        return escaped;
    }
};

} // namespace enen
