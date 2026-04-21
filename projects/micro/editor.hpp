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

    bool caps;

public:
    Keypad(): caps(false)
    {

    }

    void Repaint(Displays &displays, Input &input) {
        Screen &screen = displays.Lower;
        screen.Clear();

        RepaintHeader(screen, input);
        RepaintKeys(screen, input);
    }

protected:
    void RepaintHeader(Screen &screen, Input &input) {
        screen.FillRect(0, 0, screen.Width, ROW_HEIGHT, headerColour, headerColour);
        
        // TODO tabs for different character sets

    }

    void RepaintKeys(Screen &screen, Input &input) {
        // TODO different keyboard for different character sets
        RepaintKeysAlpha(screen, input);
    }

    void RepaintKeysAlpha(Screen &screen, Input &input) {
        // Row 1
        DrawKey(screen, input, '1', 0, 0);
        DrawKey(screen, input, '2', 0, 1);
        DrawKey(screen, input, '3', 0, 2);
        DrawKey(screen, input, '4', 0, 3);
        DrawKey(screen, input, '5', 0, 4);
        DrawKey(screen, input, '6', 0, 5);
        DrawKey(screen, input, '7', 0, 6);
        DrawKey(screen, input, '8', 0, 7);
        DrawKey(screen, input, '9', 0, 8);
        DrawKey(screen, input, '0', 0, 9);

        // Row 2
        DrawKey(screen, input, caps ? 'Q' : 'q', 1, 0);
        DrawKey(screen, input, caps ? 'W' : 'w', 1, 1);
        DrawKey(screen, input, caps ? 'E' : 'e', 1, 2);
        DrawKey(screen, input, caps ? 'R' : 'r', 1, 3);
        DrawKey(screen, input, caps ? 'T' : 't', 1, 4);
        DrawKey(screen, input, caps ? 'Y' : 'y', 1, 5);
        DrawKey(screen, input, caps ? 'U' : 'u', 1, 6);
        DrawKey(screen, input, caps ? 'I' : 'i', 1, 7);
        DrawKey(screen, input, caps ? 'O' : 'o', 1, 8);
        DrawKey(screen, input, caps ? 'P' : 'p', 1, 9);

        // Row 3
        DrawKey(screen, input, caps ? 'A' : 'a', 2, 0, 16);
        DrawKey(screen, input, caps ? 'S' : 's', 2, 1, 16);
        DrawKey(screen, input, caps ? 'D' : 'd', 2, 2, 16);
        DrawKey(screen, input, caps ? 'F' : 'f', 2, 3, 16);
        DrawKey(screen, input, caps ? 'G' : 'g', 2, 4, 16);
        DrawKey(screen, input, caps ? 'H' : 'h', 2, 5, 16);
        DrawKey(screen, input, caps ? 'J' : 'j', 2, 6, 16);
        DrawKey(screen, input, caps ? 'K' : 'k', 2, 7, 16);
        DrawKey(screen, input, caps ? 'L' : 'l', 2, 8, 16);

        // Row 4
        if (DrawKey(screen, input, Typeface::SpecialGlyphs::ChevronUp, 3, 0, 16, 1, caps ? highlightFontColour : fontColour, drkButtonColour)) {caps = !caps;}
        DrawKey(screen, input, caps ? 'Z' :'z', 3, 0, 16 + 32);
        DrawKey(screen, input, caps ? 'X' :'x', 3, 1, 16 + 32);
        DrawKey(screen, input, caps ? 'C' :'c', 3, 2, 16 + 32);
        DrawKey(screen, input, caps ? 'V' :'v', 3, 3, 16 + 32);
        DrawKey(screen, input, caps ? 'B' :'b', 3, 4, 16 + 32);
        DrawKey(screen, input, caps ? 'N' :'n', 3, 5, 16 + 32);
        DrawKey(screen, input, caps ? 'M' :'m', 3, 6, 16 + 32);
        DrawKey(screen, input, Typeface::SpecialGlyphs::Backspace, 3, 7, 16 + 32, 1, fontColour, drkButtonColour);

        // Row 5
        DrawKey(screen, input, ',', 4, 0, 16 + 32);
        DrawKey(screen, input, ' ', 4, 1, 16 + 32, 5);
        DrawKey(screen, input, '.', 4, 6, 16 + 32);
        DrawKey(screen, input, Typeface::SpecialGlyphs::Return, 4, 7, 16 + 32, 1, fontColour, drkButtonColour);
    }

    inline bool DrawKey(Screen &screen, Input &input, const Typeface::Glyph glyph, int row, int col, int offset, int colspan, const Colour& font, const Colour& button) {
        const int VERT_OFFSET = ROW_HEIGHT;
        const int PADDING = 2;
        const int GLYPH_SCALE = 2;

        int x = col * COLUMN_WIDTH + offset;
        int y = VERT_OFFSET + row * ROW_HEIGHT;
        screen.FillRect(x + PADDING, y + PADDING, COLUMN_WIDTH * colspan - 2 * PADDING, ROW_HEIGHT - 2 * PADDING, button, button);
        int gx = x + ((COLUMN_WIDTH * colspan) >> 1) - ((Typeface::Width * GLYPH_SCALE) >> 1);
        int gy = y + (ROW_HEIGHT >> 1) - (Typeface::Height >> 1);
        screen.StampGlyph(glyph, gx, gy, GLYPH_SCALE, font, alpha);
        
        const Touchpad &pad = input.GetTouchpad();
        return pad.IsJustPressed() && pad.IsTouchInRect(x, y, COLUMN_WIDTH, ROW_HEIGHT); // Did the user press this button?
    }

    inline bool DrawKey(Screen &screen, Input &input, char c, int row, int col, int offset = 0, int colspan=1) {
        return DrawKey(screen, input, Typeface::DefaultFont[c], row, col, offset, colspan, fontColour, buttonColour);
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
        keypad.Repaint(displays, input);
        console.Flush(displays);
    }

    void cleanup() override {
        
    }
};

#endif