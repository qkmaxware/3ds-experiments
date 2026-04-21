#include "../lib/ez3ds.hpp"

#ifndef NANO_EDITOR
#define NANO_EDITOR

class ConsoleDisplayBuffer {
private:
    const unsigned int OUTPUT_LINES = (240 / Typeface::LineHeight);
    const unsigned int OUTPUT_CHARS_PER_LINE = (400 / (Typeface::Width + Typeface::Kerning));
    const unsigned int OUTPUT_LENGTH = (OUTPUT_LINES) * (OUTPUT_CHARS_PER_LINE);

    std::vector<char> buf;
    std::vector<char>::size_type x;
    std::vector<char>::size_type y;

public:
    ConsoleDisplayBuffer(): buf(OUTPUT_LENGTH, 0),x(0),y(0) { }

    void Flush(Displays &displays) {
        displays.Upper.Clear();
        Imgui im(displays.Upper);
        for (unsigned int i = 0, line = 0; line < OUTPUT_LINES; line++) {
            im.BeginRow();
            for (unsigned int col = 0; col < OUTPUT_CHARS_PER_LINE; col++, i++) {
                uint8_t ch = buf[i];
                im.Glyph(Typeface::DefaultFont[ch], Imgui::DefaultLabelStyle);
            }
            im.EndRow();
        }
    }

    void Clear() {
        std::fill(this->buf.begin(), this->buf.end(), 0);
    }

    void Write(const std::string &str) {
        for (const char& c : str) {
            Write(c);
        }
    }

    void Writeln(const std::string &str) {
        Write(str);
        Write('\n');
    }

    void Write(const char c) {
        // Respect newlines (fill with \0)
        auto start_index = y * OUTPUT_CHARS_PER_LINE + x;
        if (c == '\n') {
            auto chars_left_in_line = OUTPUT_CHARS_PER_LINE - x;
            for (unsigned int i = 0; i < chars_left_in_line; i++)
                buf[start_index + i] = '\0';
            start_index += chars_left_in_line;
            x = 0;
            y += 1;
            if (y >= OUTPUT_LINES) {
                y = 0;
            }
            return;
        }

        // Wrapping buffer
        buf[start_index] = c;
        x += 1;
        if (x >= OUTPUT_CHARS_PER_LINE) {
            x = 0;
            y += 1;
            if (y >= OUTPUT_LINES) {
                y = 0;
            }
        }
    }
};

class Keypad {
private:
    const int ROW_HEIGHT                = 40;
    const int MAX_BUTTONS_PER_LINE      = 10;
    const int COLUMN_WIDTH              = 32;

    const Colour alpha                  = Colour::Transparent();
    const Colour fontColour             = Colour::White();
    const Colour highlightFontColour    = Colour::FromRgb(53, 118, 240);
    const Colour headerColour           = Colour::FromRgb(54,57,62);
    const Colour buttonColour           = Colour::FromRgb(66,69,73);
    const Colour drkButtonColour        = Colour::FromRgb(30,33,36);

public:
    Keypad()
    {

    }

    void Repaint(Displays &displays) {
        Screen &screen = displays.Lower;
        screen.Clear();

        RepaintHeader();
        RepaintKeys();
    }

protected:
    void RepaintHeader(Screen &screen) {
        screen.FillRect(0, 0, screen.Width, ROW_HEIGHT, headerColour, headerColour);
        
        // TODO tabs for different character sets

    }

    void RepaintKeys(Screen &screen) {
        // TODO different keyboard for different character sets
        RepaintKeysAlpha(screen);
    }

    void RepaintKeysAlpha(Screen &screen) {
        // Row 1
        DrawKey(screen, '1', 0, 0);
        DrawKey(screen, '2', 0, 1);
        DrawKey(screen, '3', 0, 2);
        DrawKey(screen, '4', 0, 3);
        DrawKey(screen, '5', 0, 4);
        DrawKey(screen, '6', 0, 5);
        DrawKey(screen, '7', 0, 6);
        DrawKey(screen, '8', 0, 7);
        DrawKey(screen, '9', 0, 8);
        DrawKey(screen, '0', 0, 9);

        // Row 2
        DrawKey(screen, 'q', 1, 0);
        DrawKey(screen, 'w', 1, 1);
        DrawKey(screen, 'e', 1, 2);
        DrawKey(screen, 'r', 1, 3);
        DrawKey(screen, 't', 1, 4);
        DrawKey(screen, 'y', 1, 5);
        DrawKey(screen, 'u', 1, 6);
        DrawKey(screen, 'i', 1, 7);
        DrawKey(screen, 'o', 1, 8);
        DrawKey(screen, 'p', 1, 9);

        // Row 3
        DrawKey(screen, 'a', 2, 0, 16);
        DrawKey(screen, 's', 2, 1, 16);
        DrawKey(screen, 'd', 2, 2, 16);
        DrawKey(screen, 'f', 2, 3, 16);
        DrawKey(screen, 'g', 2, 4, 16);
        DrawKey(screen, 'h', 2, 5, 16);
        DrawKey(screen, 'j', 2, 6, 16);
        DrawKey(screen, 'k', 2, 7, 16);
        DrawKey(screen, 'l', 2, 8, 16);

        // Row 4
        DrawKey(screen, 'z', 3, 0, 16 + 32);
        DrawKey(screen, 'x', 3, 1, 16 + 32);
        DrawKey(screen, 'c', 3, 2, 16 + 32);
        DrawKey(screen, 'v', 3, 3, 16 + 32);
        DrawKey(screen, 'b', 3, 4, 16 + 32);
        DrawKey(screen, 'n', 3, 5, 16 + 32);
        DrawKey(screen, 'm', 3, 6, 16 + 32);

        // Row 5
        DrawKey(screen, ',', 4, 0, 16 + 32);
        DrawKey(screen, ' ', 4, 1, 16 + 32, 5);
        DrawKey(screen, '.', 4, 6, 16 + 32);
    }

    inline void DrawKey(Screen &screen, char c, int row, int col, int offset = 0, int colspan=1) {
        const int VERT_OFFSET = ROW_HEIGHT;
        const int PADDING = 2;
        const int GLYPH_SCALE = 2;

        int x = col * CELL_SIZE + offset;
        int y = VERT_OFFSET + row * ROW_HEIGHT;
        screen.FillRect(x + PADDING, y + PADDING, COLUMN_WIDTH * colspan - 2 * PADDING, ROW_HEIGHT - 2 * PADDING, buttonColour, buttonColour);
        int gx = x + ((COLUMN_WIDTH * colspan) >> 1) - ((Typeface::Width * GLYPH_SCALE) >> 1);
        int gy = y + (ROW_HEIGHT >> 1) - (Typeface::Height >> 1);
        screen.StampGlyph(Typeface::DefaultFont[c], gx, gy, GLYPH_SCALE, fontColour, alpha);
    }
};

class Editor : public CitrusApp {
private:
    ConsoleDisplayBuffer console;
    Keypad keypad;

public:
    Editor(): console(), keypad() {}

    void oneTimeSetup() {

    }

    void setup() override {
        
    }

    void loop(Displays &displays, Input &input) override { 
        // Clear the upper screen
        displays.Upper.Clear();

        // Do stuff

        // Flush buffer(s)
        keypad.Repaint(displays);
        console.Flush(displays);
    }

    void cleanup() override {
        
    }
};

#endif