#include "lisp.hpp"
#include "../lib/ez3ds.hpp"
#include <cstdio>

bool ends_with(const std::string &str, const std::string &suffix) {
    if (suffix.size() > str.size()) 
        return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

class StringStream: public ICharStream {
private:
    std::string src;
    unsigned int ptr;

public:
    StringStream(const std::string &str): src(str), ptr(0) { }

    bool Eof() const override {
        return ptr >= src.size();
    }
    char Peek() const override {
        if (Eof())
            return '\0';
        return src[ptr];
    }
    char PeekFuture() const override {
        if ((ptr + 1) < src.size())
            return src[ptr + 1];
        return '\0';
    }
    char Advance() override {
        char c = Peek();
        if (!Eof())
            ptr += 1;
        return c;
    }
};

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
        x = 0;
        y = 0;
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

enum class LispRuntimeState {
    Idle,
    ProgramLoaded,
    ProgramRunning,
    ProgramWaiting,
    ProgramError,
    ProgramDone,
};

class LispRuntime3ds: public LispRuntime {
private:
    LispRuntimeState state;
    ConsoleDisplayBuffer display;

public: 
    LispRuntime3ds(): LispRuntime(1024), state(LispRuntimeState::Idle), display() { }

    void Render(Displays &displays) {
        display.Flush(displays);
    }

    void ResetState() {
        this->Reset();
        display.Clear();
        this->state = LispRuntimeState::Idle;
    }

    LispRuntimeState GetState() {
        return this->state;
    }

    void ParseAndRun(ICharStream &stream) {
        this->state = LispRuntimeState::ProgramLoaded;
        std::vector<LispRef> exprs = this->Parse(stream);
        if (!DidParseSuccessfully()) {
            this->state = LispRuntimeState::ProgramError;
            return;
        }

        this->state = LispRuntimeState::ProgramRunning;
        for (LispRef expr : exprs) {
            std::string line = this->Stringify(expr);
            display.Write('>'); display.Write(' '); display.Writeln(line);

            LispRef result = this->Eval(expr);
            std::string output = this->Stringify(result);
            display.Writeln(output);
        }
        this->state = LispRuntimeState::ProgramDone;
    }

    bool TryExecBuiltin(const LispRef &currentEnv, const LispValue &func, const LispRef &argsRef, LispRef &result) override {
        // Test platform agnostic builtin functions here
        if (LispRuntime::TryExecBuiltin(currentEnv, func, argsRef, result)) {
            return true;
        }

        // 3ds specific builtin functions

        // Not a builtin function
        return false;
    }
};

enum AppState {
    None,
    Menu,
    Browse,
    File,
    Repl,
};

class Interpreter: public CitrusApp {
private:
    AppState lastAppState;
    AppState state;
    int menu_item ;
    std::string script_path;
    Imgui::FileBrowser fb;
    LispRuntime3ds runtime;

public:
    Interpreter(): lastAppState(AppState::None), state(AppState::Menu), menu_item(0), script_path(), fb(), runtime() {}

    void setup() {
        fb.SetDir("/");
    }

    void loop(Displays &displays, Input &input) override { 
        bool state_changed = lastAppState == state;

        switch (state) {
            case AppState::None:
                state = AppState::Menu;
                lastAppState = AppState::Menu;
                break;
            case AppState::Menu:
                if (state_changed) {
                Console::Clear();
                Console::Println("Select an option");
                Console::Println("");
                Console::Println("Press START to quit");
                }
                menu(displays, input); 
                break;

            case AppState::Browse:
                if (state_changed) {
                Console::Clear();
                Console::Println("Browse for a LISP program");
                Console::Println("");
                Console::Println("Press B to return to the menu");
                Console::Println("Press START to quit");
                }
                browse(displays, input); 
                break;

            case AppState::File:
                if (state_changed) {
                Console::Clear();
                Console::Println("Program Running");
                Console::Println("file: %s", script_path.c_str());
                Console::Println("");
                Console::Println("Press B to return to the menu");
                Console::Println("Press START to quit");
                }
                run_file(displays, input); 
                break;

            case AppState::Repl:
                if (state_changed) {
                Console::Clear();
                Console::Println("REPL");
                Console::Println("");
                Console::Println("Press A to enter an expression");
                Console::Println("Press B to return to the menu");
                Console::Println("Press START to quit");
                }
                repl(displays, input); 
                break;
        }

        lastAppState = state;
    }

    void menu(Displays &displays, Input &input) {
        // Clear the upper screen
        displays.Upper.Clear();

        Imgui im(displays.Upper);
        im.Label("Main Menu", Imgui::DefaultLabelStyle);
        im.NextLine();
        im.BeginRow();
        if (menu_item == 1) { im.Glyph(Typeface::SpecialGlyphs::LeftHalfBlock, Imgui::DefaultLabelStyle); } else { im.Label(" ", Imgui::DefaultLabelStyle); }
        im.Label("Browse for LISP file", Imgui::DefaultLabelStyle);
        im.EndRow();

        im.BeginRow();
        if (menu_item == 2) { im.Glyph(Typeface::SpecialGlyphs::LeftHalfBlock, Imgui::DefaultLabelStyle); } else { im.Label(" ", Imgui::DefaultLabelStyle); }
        im.Label("Read-Evaluate-Print Loop (REPL)", Imgui::DefaultLabelStyle);
        im.EndRow();

        if (input.JustPressed(KeyCodes::A)) {
            switch (menu_item) {
                case 1:
                    state = AppState::Browse; 
                    break;
                case 2:
                    state = AppState::Repl; 
                    break;
            }
        }

        if (input.JustPressed(KeyCodes::DPadUp)) {
            menu_item -= 1;
            if (menu_item < 0)
                menu_item = 0;
        } else if (input.JustPressed(KeyCodes::DPadDown)) {
            menu_item += 1;
            if (menu_item > 2)
                menu_item = 2;
        }
    }

    void browse(Displays &displays, Input &input) {
        // Clear the upper screen
        displays.Upper.Clear();

        if (input.JustPressed(KeyCodes::B)) {
            this->state = AppState::Menu;
            return;
        }

        // Browse
        if (fb.SelectFile(displays.Upper, input)) {
            // Test if selected file is a valid BF script
            auto selected_file = fb.Highlighted();
            bool is_bf = ends_with(selected_file, ".lisp");

            if (is_bf) {
                this->script_path = selected_file;
                
                // Load the script and init the interpreter
                runtime.ResetState();
                this->state = AppState::File;
            }
        }
    }

    void run_file(Displays &displays, Input &input) {
        // Clear the upper screen
        displays.Upper.Clear();
        runtime.Render(displays);

        if (runtime.GetState() == LispRuntimeState::Idle) {
            StringStream stream(RomFs::ReadAllText(this->script_path));
            runtime.ParseAndRun(stream);
        }

        if (input.JustPressed(KeyCodes::B)) {
            this->state = AppState::Menu;
        }
    }

    void repl(Displays &displays, Input &input) {
        // Clear the upper screen
        displays.Upper.Clear();
        runtime.Render(displays);

        if (input.JustPressed(KeyCodes::A)) {
            std::string line = input.Prompt("LISP Expression");
            StringStream stream(line);
            runtime.ParseAndRun(stream);
        }

        if (input.JustPressed(KeyCodes::B)) {
            runtime.ResetState();
            this->state = AppState::Menu;
        }
    }

    void cleanup() {

    }
};

int main() {
    Interpreter app;
    app.Run();
    return 0;
    /*
    LispRuntime runtime(1024);

    runtime.Reset();
    StringStream stream("(+ 4 5 6)"); 
    std::vector<LispRef> exprs = runtime.Parse(stream); // will be parsed as (+ (4 (5 (6 nil))))
    if (!runtime.DidParseSuccessfully()) {
        std::printf("Failed to parse\n");
        return 1;
    }

    std::printf("number of statements: %i\n", exprs.size());
    int i = 1;
    for (LispRef reference : exprs) {
        LispValue& val = runtime.ValueOf(reference);
        std::string str = runtime.Stringify(reference);
        std::printf("  %i: (%s) %s\n", i, Enum2String(val.Type).c_str(), str.c_str());
        i++;
    }
    LispRef result = runtime.EvalAll(exprs); // Hopefully will be the value 15, at the moment it' not, it's nil
    std::string fmt = runtime.Stringify(result);
    std::printf("result is: %s\n", fmt.c_str());

    return 0;*/
}