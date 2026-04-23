#include "../lib/ez3ds.hpp"
#include "document.hpp"
#include <functional>

#ifndef NANO_EDITOR
#define NANO_EDITOR

struct KeyEvent {
    char Key;
    struct {
        bool Shift;
        bool Ctrl;
        bool Meta;
    } Modifiers;

    KeyEvent(char key, bool shift = false, bool ctrl = false, bool meta = false): Key(key), Modifiers{ shift, ctrl, meta } {

    }
};

class Keypad {
public:
    using Callback = std::function<void(KeyEvent)>;

private:
    const int ROW_HEIGHT                = 40;
    const int MAX_BUTTONS_PER_LINE      = 10;
    const int COLUMN_WIDTH              = 32;

    const Colour alpha                  = Colour::Transparent();
    const Colour headerColour           = Colour::FromRgb(54,57,62);
    const Imgui::ButtonStyle styleButtonNormal = Imgui::ButtonStyle(
        // Normal
        Colour::White(), Colour::FromRgb(66,69,73), Colour::FromRgb(66,69,73),
        // Pressed
        Colour::White(), Colour::FromRgb(40,43,48), Colour::FromRgb(40,43,48)
    );
    const Imgui::ButtonStyle styleButtonDark = Imgui::ButtonStyle(
        // Normal
        Colour::White(), Colour::FromRgb(66,69,73), Colour::FromRgb(66,69,73),
        // Pressed
        Colour::White(), Colour::FromRgb(53, 118, 240), Colour::FromRgb(53, 118, 240)
    ); 
    const Imgui::ButtonStyle styleButtonDarkToggled = Imgui::ButtonStyle(
        // Normal
        Colour::White(), Colour::FromRgb(53, 118, 240), Colour::FromRgb(53, 118, 240),
        // Pressed
        Colour::White(), Colour::FromRgb(66,69,73), Colour::FromRgb(66,69,73)
    ); 

    bool caps;

    Callback callback;

public:
    Keypad(Callback callback = nullptr): caps(false), callback(callback)
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
    }

    void RepaintKeys(Screen &screen, Input &input) {
        // TODO different keyboard for different character sets
        RepaintKeysAlpha(screen, input);
    }

    void RepaintKeysAlpha(Screen &screen, Input &input) {
        // Row 1
        DispatchIfPressed(DrawKey(screen, input, '1', 0, 0), KeyEvent('1', caps));
        DispatchIfPressed(DrawKey(screen, input, '2', 0, 1), KeyEvent('2', caps));
        DispatchIfPressed(DrawKey(screen, input, '3', 0, 2), KeyEvent('3', caps));
        DispatchIfPressed(DrawKey(screen, input, '4', 0, 3), KeyEvent('4', caps));
        DispatchIfPressed(DrawKey(screen, input, '5', 0, 4), KeyEvent('5', caps));
        DispatchIfPressed(DrawKey(screen, input, '6', 0, 5), KeyEvent('6', caps));
        DispatchIfPressed(DrawKey(screen, input, '7', 0, 6), KeyEvent('7', caps));
        DispatchIfPressed(DrawKey(screen, input, '8', 0, 7), KeyEvent('8', caps));
        DispatchIfPressed(DrawKey(screen, input, '9', 0, 8), KeyEvent('9', caps));
        DispatchIfPressed(DrawKey(screen, input, '0', 0, 9), KeyEvent('0', caps));

        // Row 2
        DispatchIfPressed(DrawKey(screen, input, caps ? 'Q' : 'q', 1, 0), KeyEvent(caps ? 'Q' : 'q', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'W' : 'w', 1, 1), KeyEvent(caps ? 'W' : 'w', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'E' : 'e', 1, 2), KeyEvent(caps ? 'E' : 'e', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'R' : 'r', 1, 3), KeyEvent(caps ? 'R' : 'r', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'T' : 't', 1, 4), KeyEvent(caps ? 'T' : 't', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'Y' : 'y', 1, 5), KeyEvent(caps ? 'Y' : 'y', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'U' : 'u', 1, 6), KeyEvent(caps ? 'U' : 'u', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'I' : 'i', 1, 7), KeyEvent(caps ? 'I' : 'i', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'O' : 'o', 1, 8), KeyEvent(caps ? 'O' : 'o', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'P' : 'p', 1, 9), KeyEvent(caps ? 'P' : 'p', caps));

        // Row 3
        DispatchIfPressed(DrawKey(screen, input, caps ? 'A' : 'a', 2, 0, 16), KeyEvent(caps ? 'A' : 'a', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'S' : 's', 2, 1, 16), KeyEvent(caps ? 'S' : 's', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'D' : 'd', 2, 2, 16), KeyEvent(caps ? 'D' : 'd', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'F' : 'f', 2, 3, 16), KeyEvent(caps ? 'F' : 'f', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'G' : 'g', 2, 4, 16), KeyEvent(caps ? 'G' : 'g', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'H' : 'h', 2, 5, 16), KeyEvent(caps ? 'H' : 'h', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'J' : 'j', 2, 6, 16), KeyEvent(caps ? 'J' : 'j', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'K' : 'k', 2, 7, 16), KeyEvent(caps ? 'K' : 'k', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'L' : 'l', 2, 8, 16), KeyEvent(caps ? 'L' : 'l', caps));

        // Row 4
        if (DrawKey(screen, input, Typeface::SpecialGlyphs::ChevronUp, 3, 0, 16, 1, caps ? styleButtonDarkToggled : styleButtonDark)) {caps = !caps;}
        DispatchIfPressed(DrawKey(screen, input, caps ? 'Z' :'z', 3, 0, 16 + 32), KeyEvent(caps ? 'Z' :'z', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'X' :'x', 3, 1, 16 + 32), KeyEvent(caps ? 'X' :'x', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'C' :'c', 3, 2, 16 + 32), KeyEvent(caps ? 'C' :'c', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'V' :'v', 3, 3, 16 + 32), KeyEvent(caps ? 'V' :'v', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'B' :'b', 3, 4, 16 + 32), KeyEvent(caps ? 'B' :'b', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'N' :'n', 3, 5, 16 + 32), KeyEvent(caps ? 'N' :'n', caps));
        DispatchIfPressed(DrawKey(screen, input, caps ? 'M' :'m', 3, 6, 16 + 32), KeyEvent(caps ? 'M' :'m', caps));
        DispatchIfPressed(DrawKey(screen, input, Typeface::SpecialGlyphs::Backspace, 3, 7, 16 + 32, 1, styleButtonDark), KeyEvent('\b', caps));

        // Row 5
        DispatchIfPressed(DrawKey(screen, input, ',', 4, 0, 16 + 32), KeyEvent(',', caps));
        DispatchIfPressed(DrawKey(screen, input, ' ', 4, 1, 16 + 32, 5), KeyEvent(' ', caps));
        DispatchIfPressed(DrawKey(screen, input, '.', 4, 6, 16 + 32), KeyEvent('.', caps));
        DispatchIfPressed(DrawKey(screen, input, Typeface::SpecialGlyphs::Return, 4, 7, 16 + 32, 1, styleButtonDark), KeyEvent('\n', caps));
    }

    inline void DispatchIfPressed(bool keyPressed, KeyEvent evt) {
        if (!keyPressed)
            return;

        if (callback)
            callback(evt);
    }

    inline bool DrawKey(Screen &screen, Input &input, const Typeface::Glyph glyph, int row, int col, int offset, int colspan, const Imgui::ButtonStyle &style) {
        const int VERT_OFFSET = ROW_HEIGHT;
        const int PADDING = 2;
        const int GLYPH_SCALE = 2;

        int x = col * COLUMN_WIDTH + offset;
        int y = VERT_OFFSET + row * ROW_HEIGHT;

        const Touchpad &pad = input.GetTouchpad();
        bool isPressed = pad.IsTouchInRect(x, y, COLUMN_WIDTH * colspan, ROW_HEIGHT);

        screen.FillRect(x + PADDING, y + PADDING, COLUMN_WIDTH * colspan - 2 * PADDING, ROW_HEIGHT - 2 * PADDING, isPressed ? style.HoverBorderColour : style.NormalBorderColour, isPressed ? style.HoverBackgroundColour : style.NormalBackgroundColour);
        int gx = x + ((COLUMN_WIDTH * colspan) >> 1) - ((Typeface::Width * GLYPH_SCALE) >> 1);
        int gy = y + (ROW_HEIGHT >> 1) - (Typeface::Height >> 1);
        screen.StampGlyph(glyph, gx, gy, GLYPH_SCALE, isPressed ? style.HoverFontColour : style.NormalFontColour, alpha);
        
        return pad.IsJustPressed() && isPressed; // Did the user press this button?
    }

    inline bool DrawKey(Screen &screen, Input &input, char c, int row, int col, int offset = 0, int colspan=1) {
        return DrawKey(screen, input, Typeface::DefaultFont[c], row, col, offset, colspan, styleButtonNormal);
    }
};

enum class EditorState {
    Editing,
    ExitRequested,
};

/// @brief Text viewport for rendering and editing documents
class TextViewport {
public:
    struct Position {
        size_t line;
        size_t column;

        Position(size_t l = 0, size_t c = 0) : line(l), column(c) {}

        bool operator==(const Position& other) const {
            return line == other.line && column == other.column;
        }

        bool operator!=(const Position& other) const {
            return !(*this == other);
        }

        bool operator<(const Position& other) const {
            return line < other.line || (line == other.line && column < other.column);
        }

        bool operator>(const Position& other) const {
            return line > other.line || (line == other.line && column > other.column);
        }

        bool operator<=(const Position& other) const {
            return *this < other || *this == other;
        }

        bool operator>=(const Position& other) const {
            return *this > other || *this == other;
        }
    };

private:
    Document& document;
    const unsigned int VIEWPORT_LINES = (240 / Typeface::LineHeight);
    const unsigned int VIEWPORT_COLS = (400 / (Typeface::Width + Typeface::Kerning));

    Position cursor;                        ///< Current cursor position
    Position selection_start;               ///< Selection start (may be after end)
    Position selection_end;                 ///< Selection end (may be before start)
    size_t scroll_offset_line = 0;          ///< First visible line
    
    std::vector<size_t> line_starts;        ///< Cached line start positions in document
    bool line_cache_valid = false;          ///< Whether line_starts cache is valid

    /// @brief Rebuild the line position cache
    void RebuildLineCache() {
        line_starts.clear();
        line_starts.push_back(0);

        size_t doc_index = 0;
        char buf[1];

        while (document.ReadChars(doc_index, buf, 1) > 0) {
            if (buf[0] == '\n') {
                line_starts.push_back(doc_index + 1);
            }
            doc_index++;
        }

        line_cache_valid = true;
    }

    /// @brief Get the number of lines in the document
    size_t GetLineCount() {
        if (!line_cache_valid) {
            RebuildLineCache();
        }
        return line_starts.size();
    }

    /// @brief Get the starting document index for a given line
    size_t GetLineStartIndex(size_t line) {
        if (!line_cache_valid) {
            RebuildLineCache();
        }
        if (line >= line_starts.size()) {
            return document.GetLength();
        }
        return line_starts[line];
    }

    /// @brief Get the length of a line (excluding newline)
    size_t GetLineLength(size_t line) {
        if (line >= GetLineCount()) {
            return 0;
        }

        size_t start = GetLineStartIndex(line);
        size_t end_index = start;
        char buf[1];

        while (document.ReadChars(end_index, buf, 1) > 0 && buf[0] != '\n') {
            end_index++;
        }

        return end_index - start;
    }

    /// @brief Clamp position to valid document bounds
    Position ClampPosition(const Position& pos) {
        size_t line_count = GetLineCount();
        if (line_count == 0) {
            return Position(0, 0);
        }

        size_t clamped_line = (pos.line >= line_count) ? line_count - 1 : pos.line;
        size_t line_len = GetLineLength(clamped_line);
        size_t clamped_col = (pos.column > line_len) ? line_len : pos.column;

        return Position(clamped_line, clamped_col);
    }

    /// @brief Update scroll to keep cursor in view
    void EnsureCursorVisible() {
        if (cursor.line < scroll_offset_line) {
            scroll_offset_line = cursor.line;
        } else if (cursor.line >= scroll_offset_line + VIEWPORT_LINES) {
            scroll_offset_line = cursor.line - VIEWPORT_LINES + 1;
        }
    }

public:
    TextViewport(Document& doc) : document(doc), cursor(0, 0), selection_start(0, 0), selection_end(0, 0) {}

    /// @brief Move cursor left
    void MoveCursorLeft() {
        if (cursor.column > 0) {
            cursor.column--;
        } else if (cursor.line > 0) {
            cursor.line--;
            cursor.column = GetLineLength(cursor.line);
        }
        selection_start = selection_end = cursor;
        EnsureCursorVisible();
    }

    /// @brief Move cursor right
    void MoveCursorRight() {
        size_t line_len = GetLineLength(cursor.line);
        if (cursor.column < line_len) {
            cursor.column++;
        } else if (cursor.line < GetLineCount() - 1) {
            cursor.line++;
            cursor.column = 0;
        }
        selection_start = selection_end = cursor;
        EnsureCursorVisible();
    }

    /// @brief Move cursor up
    void MoveCursorUp() {
        if (cursor.line > 0) {
            cursor.line--;
            size_t line_len = GetLineLength(cursor.line);
            cursor.column = (cursor.column > line_len) ? line_len : cursor.column;
        }
        selection_start = selection_end = cursor;
        EnsureCursorVisible();
    }

    /// @brief Move cursor down
    void MoveCursorDown() {
        if (cursor.line < GetLineCount() - 1) {
            cursor.line++;
            size_t line_len = GetLineLength(cursor.line);
            cursor.column = (cursor.column > line_len) ? line_len : cursor.column;
        }
        selection_start = selection_end = cursor;
        EnsureCursorVisible();
    }

    /// @brief Extend selection left
    void ExtendSelectionLeft() {
        if (selection_end.column > 0) {
            selection_end.column--;
        } else if (selection_end.line > 0) {
            selection_end.line--;
            selection_end.column = GetLineLength(selection_end.line);
        }
        cursor = selection_end;
        EnsureCursorVisible();
    }

    /// @brief Extend selection right
    void ExtendSelectionRight() {
        size_t line_len = GetLineLength(selection_end.line);
        if (selection_end.column < line_len) {
            selection_end.column++;
        } else if (selection_end.line < GetLineCount() - 1) {
            selection_end.line++;
            selection_end.column = 0;
        }
        cursor = selection_end;
        EnsureCursorVisible();
    }

    /// @brief Extend selection up
    void ExtendSelectionUp() {
        if (selection_end.line > 0) {
            selection_end.line--;
            size_t line_len = GetLineLength(selection_end.line);
            selection_end.column = (selection_end.column > line_len) ? line_len : selection_end.column;
        }
        cursor = selection_end;
        EnsureCursorVisible();
    }

    /// @brief Extend selection down
    void ExtendSelectionDown() {
        if (selection_end.line < GetLineCount() - 1) {
            selection_end.line++;
            size_t line_len = GetLineLength(selection_end.line);
            selection_end.column = (selection_end.column > line_len) ? line_len : selection_end.column;
        }
        cursor = selection_end;
        EnsureCursorVisible();
    }

    /// @brief Get selected text (if any)
    std::string GetSelectedText() {
        Position start = (selection_start < selection_end) ? selection_start : selection_end;
        Position end = (selection_start < selection_end) ? selection_end : selection_start;

        if (start == end) {
            return "";
        }

        size_t start_idx = GetLineStartIndex(start.line) + start.column;
        size_t end_idx = GetLineStartIndex(end.line) + end.column;

        return document.GetSubstring(start_idx, end_idx - start_idx);
    }

    /// @brief Delete selected text
    void DeleteSelection() {
        std::string selected = GetSelectedText();
        if (selected.empty()) {
            return;
        }

        Position start = (selection_start < selection_end) ? selection_start : selection_end;
        size_t start_idx = GetLineStartIndex(start.line) + start.column;
        size_t length = selected.length();

        document.DeleteRange(start_idx, length);
        line_cache_valid = false;

        cursor = start;
        selection_start = selection_end = cursor;
    }

    /// @brief Insert text at cursor position
    void InsertText(const std::string& text) {
        DeleteSelection();
        
        size_t insert_idx = GetLineStartIndex(cursor.line) + cursor.column;
        document.InsertAt(insert_idx, text);
        line_cache_valid = false;

        // Update cursor position
        for (char c : text) {
            if (c == '\n') {
                cursor.line++;
                cursor.column = 0;
            } else {
                cursor.column++;
            }
        }

        selection_start = selection_end = cursor;
        EnsureCursorVisible();
    }

    /// @brief Handle backspace
    void Backspace() {
        if (!GetSelectedText().empty()) {
            DeleteSelection();
        } else if (cursor.column > 0) {
            size_t del_idx = GetLineStartIndex(cursor.line) + cursor.column - 1;
            document.DeleteRange(del_idx, 1);
            line_cache_valid = false;
            cursor.column--;
            selection_start = selection_end = cursor;
        } else if (cursor.line > 0) {
            // Join with previous line
            size_t prev_line_end = GetLineStartIndex(cursor.line) - 1; // Position of \n
            size_t prev_line_len = GetLineLength(cursor.line - 1);
            document.DeleteRange(prev_line_end, 1);
            line_cache_valid = false;
            cursor.line--;
            cursor.column = prev_line_len;
            selection_start = selection_end = cursor;
        }
        EnsureCursorVisible();
    }

    /// @brief Render the viewport to the screen
    void Render(Screen& screen) {
        screen.Clear();
        size_t y = 0;

        for (unsigned int display_line = 0; display_line < VIEWPORT_LINES; display_line++) {
            size_t doc_line = scroll_offset_line + display_line;

            // Check if this line is within the document
            if (doc_line < GetLineCount()) {
                size_t line_start_idx = GetLineStartIndex(doc_line);
                size_t line_len = GetLineLength(doc_line);

                // Read the line
                std::string line;
                if (line_len > 0) {
                    char buf[1024];
                    size_t to_read = (line_len > sizeof(buf) - 1) ? sizeof(buf) - 1 : line_len;
                    size_t read_count = document.ReadChars(line_start_idx, buf, to_read);
                    line = std::string(buf, read_count);
                }

                // Render line with selection highlighting
                for (unsigned int col = 0; col < VIEWPORT_COLS; col++) {
                    Position char_pos(doc_line, col);
                    bool is_selected = IsPositionInSelection(char_pos);

                    uint8_t ch = (col < line.length()) ? static_cast<uint8_t>(line[col]) : 0;
                    Colour foreground = Colour::White();
                    Colour background = Colour::Transparent();
                    
                    if (is_selected) {
                        // Highlight selected region
                        background = Colour::FromRgb(53, 118, 240);
                        foreground = Colour::White();
                    }

                    if (char_pos == cursor && !is_selected) {
                        // Draw cursor
                        background = Colour::White();
                        foreground = Colour::Black();
                    }

                    screen.StampGlyph(Typeface::DefaultFont[ch], col * (Typeface::Width + Typeface::Kerning), y, 1, foreground, background);
                }
            } else {
                // Empty lines
                for (unsigned int col = 0; col < VIEWPORT_COLS; col++) {
                    screen.StampGlyph(Typeface::DefaultFont[0], col * (Typeface::Width + Typeface::Kerning), y, 1, Colour::White(), Colour::Transparent());
                }
            }

            y += Typeface::LineHeight;
        }
    }

    /// @brief Check if a position is within the current selection
    bool IsPositionInSelection(const Position& pos) {
        Position start = (selection_start < selection_end) ? selection_start : selection_end;
        Position end = (selection_start < selection_end) ? selection_end : selection_start;
        return pos >= start && pos < end;
    }

    /// @brief Invalidate line cache (should be called when document structure changes externally)
    void InvalidateLineCache() {
        line_cache_valid = false;
    }

    /// @brief Get cursor position
    const Position& GetCursorPosition() const {
        return cursor;
    }
};

class Editor : public CitrusApp {
private:
    Keypad keypad;
    Document document;
    TextViewport viewport;
    std::string current_path;

public:
    EditorState State;
    Editor(): 
        keypad([this](KeyEvent evt) { this->key_press(evt); }), 
        document(), 
        viewport(document),
        current_path(), 
        State(EditorState::Editing) {}

    void oneTimeSetup() {

    }

    void setup() override {
        State = EditorState::Editing;
    }

    bool load_file(const std::string &path) {
        current_path = path;
        if (document.TryOpen(path)) {
            viewport.InvalidateLineCache();
            return true;
        }
        return false;
    }

    bool save_file() {
        if (current_path.size() == 0)
            return false;

        bool did_save = document.TryOverwrite(current_path);
        return did_save;
    }

    void key_press(KeyEvent evt) {
        if (evt.Key == '\b') {
            viewport.Backspace();
        } else if (evt.Key == '\n') {
            viewport.InsertText("\n");
        } else if (evt.Key >= 32 && evt.Key < 127) {
            viewport.InsertText(std::string(1, evt.Key));
        }
    }

    void loop(Displays &displays, Input &input) override { 
        // Basic UI
        bool extend_mode = input.Pressed(KeyCodes::L) || input.Pressed(KeyCodes::R);
        bool up = input.JustPressed(KeyCodes::DPadUp);
        bool down = input.JustPressed(KeyCodes::DPadDown);
        bool left = input.JustPressed(KeyCodes::DPadLeft);
        bool right = input.JustPressed(KeyCodes::DPadRight);

        if (up) {
            extend_mode ? viewport.ExtendSelectionUp() : viewport.MoveCursorUp();
        } else if (down) {
            extend_mode ? viewport.ExtendSelectionDown() :viewport.MoveCursorDown();
        } else if (left) {
            extend_mode ? viewport.ExtendSelectionLeft() :viewport.MoveCursorLeft();
        } else if(right) {
            extend_mode ? viewport.ExtendSelectionRight() :viewport.MoveCursorRight();
        }

        // Render document to upper screen
        viewport.Render(displays.Upper);

        // Render keyboard to lower screen
        keypad.Repaint(displays, input);
    }

    void cleanup() override {
        
    }
};

#endif