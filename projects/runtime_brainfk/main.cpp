#include "../lib/ez3ds.hpp"
#include <algorithm>

bool ends_with(const std::string &str, const std::string &suffix) {
    if (suffix.size() > str.size()) 
        return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

enum class InterpreterState {
    Okay,
    ErrStackOverflow,
    ErrStackUnderflow,
    ErrUnclosedLoop,
};

enum class StepType {
    NoStep = 0,
    NoRedraw = 1,
    Redraw = 2
};

const std::vector<uint8_t>::size_type TAPE_LENGTH = 30000;

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

class Interpreter {
private:
    InterpreterState state;
    std::vector<uint8_t> script;
    std::vector<uint8_t>::size_type text_ptr;
    std::vector<uint8_t> data;
    std::vector<uint8_t>::size_type data_ptr;
    ConsoleDisplayBuffer console;

    StepType Step(Input &input) {
        // Don't step if done
        if (IsDone())
            return StepType::NoStep;
        
        // Fetch
        uint8_t instr = this->script[this->text_ptr];
        //Console::Print("%c", instr);

        // Decode
        StepType type = StepType::NoRedraw;
        switch (instr) {
            case '>':
                if (this->data_ptr >= TAPE_LENGTH) {
                    this->state = InterpreterState::ErrStackOverflow;
                    return StepType::NoStep;
                }

                this->data_ptr += 1;
                break;
            case '<':
                if (this->data_ptr <= 0) {
                    this->state = InterpreterState::ErrStackUnderflow;
                    return StepType::NoStep;
                }

                this->data_ptr -= 1;
                break;
            case '+':
                if (this->data_ptr >= 0 && this->data_ptr < TAPE_LENGTH) {
                    this->data[this->data_ptr] += 1;
                }
                break;
            case '-':
                if (this->data_ptr >= 0 && this->data_ptr < TAPE_LENGTH) {
                    this->data[this->data_ptr] -= 1;
                }
                break;
            case '.':
                {
                    uint8_t toPrint = 0;
                    if (this->data_ptr >= 0 && this->data_ptr < TAPE_LENGTH) {
                        toPrint = this->data[this->data_ptr];
                    }
                    console.Write(static_cast<char>(toPrint));
                    type = StepType::Redraw;
                }
                break;
            case ',':
                {
                    uint8_t val = static_cast<uint8_t>(input.PromptInt("Byte Value"));
                    if (this->data_ptr >= 0 && this->data_ptr < TAPE_LENGTH) {
                        this->data[this->data_ptr] = val;
                    }
                }
                break;
            case '[':
                if (this->data[this->data_ptr] == 0) {
                    // Jump forward to matching ']'
                    int depth = 1;
                    while (depth > 0) {
                        this->text_ptr += 1;

                        if (this->text_ptr >= this->script.size()) {
                            this->state = InterpreterState::ErrUnclosedLoop;
                            return StepType::NoStep;
                        }

                        if (this->script[this->text_ptr] == '[') {
                            depth += 1;
                        }
                        else if (this->script[this->text_ptr] == ']') {
                            depth -= 1;
                        }
                    }
                }
                break;
            case ']':
                if (this->data[this->data_ptr] != 0) {
                    int depth = 1;
                    while (depth > 0) {
                        if (this->text_ptr == 0) {
                            this->state = InterpreterState::ErrUnclosedLoop;
                            return StepType::NoStep;
                        }

                        this->text_ptr -= 1;

                        if (this->script[this->text_ptr] == ']') {
                            depth += 1;
                        }
                        else if (this->script[this->text_ptr] == '[') {
                            depth -= 1;
                        }
                    }
                }
                break;
        }
        
        // Increment
        this->text_ptr += 1;

        return type;
    }

public:
    Interpreter(): state(InterpreterState::Okay), script(), text_ptr(0), data(TAPE_LENGTH), data_ptr(0), console() {

    }

    std::vector<uint8_t>::size_type ProgramLength() {
        return this->script.size();
    }

    InterpreterState GetState() {
        return this->state;
    }

    void Initialize(const std::vector<uint8_t> &script) {
        this->state = InterpreterState::Okay;
        this->script = script;
        this->text_ptr = 0;
        std::fill(this->data.begin(), this->data.end(), 0);
        this->data_ptr = 0;
        console.Clear();
    }

    ConsoleDisplayBuffer& GetConsole() {
        return this->console;
    }

    bool IsDone() {
        return this->state != InterpreterState::Okay || (this->text_ptr >= this->script.size());
    }

    StepType Step(Input &input, int steps) {
        StepType type = StepType::NoStep;
        for (int i = 0; i < steps; i++) {
            StepType type2 = Step(input);
            if (type2 > type)
                type = type2;
        }
        return type;
    }

    StepType RunToEnd(Input &input) {
        StepType type = StepType::NoStep;
        while (!IsDone()) {
            StepType type2 = Step(input, 1);
            if (type2 > type)
                type = type2;
        }
        return type;
    }
};

enum class AppState {
    Browse,
    RunFile,
    RunInteractive,
};

class Runtime: public CitrusApp {
private:
    AppState state;
    std::string script_path;
    Imgui::FileBrowser fb;
    Interpreter interpreter;

public:
    Runtime(): state(AppState::Browse), script_path(), fb(), interpreter() { }

    void setup() override {
        EnableBottomConsole();
        fb.SetDir("/");
        Console::Println("Press SELECT to enter interactive mode");
        Console::Println("Press START to quit");
    }

    void loop(Displays &displays, Input &input) override { 
        if (this->state == AppState::Browse) {
            // Clear the upper screen
            displays.Upper.Clear();

            if (input.JustPressed(KeyCodes::Select)) {
                state = AppState::RunInteractive;
                return;
            }

            // Browse
            if (fb.SelectFile(displays.Upper, input)) {
                // Test if selected file is a valid BF script
                auto selected_file = fb.Highlighted();
                bool is_bf = ends_with(selected_file, ".bf");

                if (is_bf) {
                    displays.Upper.Clear();
                    this->script_path = selected_file;
                    
                    // Load the script and init the interpreter
                    interpreter.Initialize(RomFs::ReadAllBytes(selected_file));
                    this->state = AppState::RunFile;

                    Console::Clear();
                    Console::Println("Program Running");
                    Console::Println("file: %s", selected_file.c_str());
                    Console::Println("size: %d bytes", interpreter.ProgramLength());
                    Console::Println("");
                    Console::Println("Press START to quit");
                }
            }
        } else if (state == AppState::RunFile) {
            // Check if interpreter is done
            if (interpreter.IsDone()) {
                if (input.Pressed(KeyCodes::A) && input.Pressed(KeyCodes::B)) {
                    // Clear selected file and return to the file browser
                    this->script_path = "";
                    this->state = AppState:: Browse;
                }
                return;
            }
            
            // Interpreter Step
            interpreter.Step(input, 12); // Do x steps per iteration

            // Draw any generated outputs
            interpreter.GetConsole().Flush(displays);

            // Final output step
            if (interpreter.IsDone()) {
                Console::Clear();
                Console::Println("Program Halted");
                Console::Println("file: %s", this->script_path.c_str());
                Console::Println("size: %d bytes", interpreter.ProgramLength());
                Console::Println("");

                if (interpreter.GetState() != InterpreterState::Okay) {
                    Console::Print("An ERROR has occurred [");
                    switch (interpreter.GetState()) {
                        case InterpreterState::ErrStackOverflow:
                            Console::Print("Stack Overflow"); break;
                        case InterpreterState::ErrStackUnderflow:
                            Console::Print("Stack Underflow"); break;
                        case InterpreterState::ErrUnclosedLoop:
                            Console::Print("Malformed Loops"); break;
                        default:
                            Console::Print("Unknown Error");
                            break;
                    }
                    Console::Println("]");
                } else {
                    Console::Println("Program completed successfully");
                }   
                
                Console::Println("Press A+B to return to file selection");
                Console::Println("Press START to quit");
            }
        } 
        else if (state == AppState::RunInteractive) {
            Console::Clear();
            Console::Println("Press X or Y to type");
            Console::Println("Press A+B to return to file selection");

            if (interpreter.GetState() != InterpreterState::Okay) {
                Console::Println("");
                Console::Print("An ERROR has occurred [");
                switch (interpreter.GetState()) {
                    case InterpreterState::ErrStackOverflow:
                        Console::Print("Stack Overflow"); break;
                    case InterpreterState::ErrStackUnderflow:
                        Console::Print("Stack Underflow"); break;
                    case InterpreterState::ErrUnclosedLoop:
                        Console::Print("Malformed Loops"); break;
                    default:
                        Console::Print("Unknown Error");
                        break;
                }
                Console::Println("]");
            }

            // Draw any generated outputs
            interpreter.GetConsole().Flush(displays);

            if (input.Pressed(KeyCodes::A) && input.Pressed(KeyCodes::B)) {
                state = AppState::Browse;
                Console::Clear();
                Console::Println("Press SELECT to enter interactive mode");
                Console::Println("Press START to quit");
                return;
            }

            if (input.JustPressed(KeyCodes::X) || input.JustPressed(KeyCodes::Y)) {
                // TODO
                std::string line = input.Prompt("BrainF**k Script");
                if (line.length() == 0) {
                    return;
                }

                std::vector<uint8_t> bytes(line.begin(), line.end());
                interpreter.Initialize(bytes);

                // Write Script Line
                ConsoleDisplayBuffer &console = interpreter.GetConsole();
                console.Write('>'); console.Write(' ');
                console.Writeln(line); 
                console.Write('\n');

                // Interpret
                interpreter.RunToEnd(input);
            }
        }
    }

    void cleanup() override {

  
    }
};

int main() {
    Runtime app;
    app.Run();
    return 0;
}