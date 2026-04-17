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
const int OUTPUT_LINES = (240 / 16);
const int OUTPUT_CHARS_PER_LINE = (400 / 9);
const int OUTPUT_LENGTH = (OUTPUT_LINES) * (OUTPUT_CHARS_PER_LINE);

class Interpreter {
private:
    InterpreterState state;
    std::vector<uint8_t> script;
    std::vector<uint8_t>::size_type text_ptr;
    std::vector<uint8_t> data;
    std::vector<uint8_t>::size_type data_ptr;
    std::vector<uint8_t> output_buffer;
    std::vector<uint8_t>::size_type output_ptr;

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
                    this->output_buffer[this->output_ptr] = toPrint;
                    this->output_ptr += 1;
                    if (this->output_ptr >= OUTPUT_LENGTH) {
                        this->output_ptr = 0;
                    }
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
    Interpreter(): state(InterpreterState::Okay), script(), text_ptr(0), data(TAPE_LENGTH), data_ptr(0), output_buffer(OUTPUT_LENGTH) {

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
        std::fill(this->output_buffer.begin(), this->output_buffer.end(), 0);
        this->output_ptr = 0;
    }

    uint8_t GetBufferedChar(int index) {
        if (index >= 0 && index < OUTPUT_LENGTH) {
            return this->output_buffer[index];
        }
        return 0;
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
};

class Runtime: public CitrusApp {
private:
    std::string script_path;
    Imgui::FileBrowser fb;
    Interpreter interpreter;

public:
    Runtime(): script_path(), fb(), interpreter() { }

    void setup() override {
        fb.SetDir("/");
        Console::Println("Press START to quit");
    }

    void loop(Displays &displays, Input &input) override { 
        if (this->script_path.length() == 0) {
            // Clear the upper screen
            displays.Upper.Clear();

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

                    Console::Clear();
                    Console::Println("Program Running");
                    Console::Println("file: %s", selected_file.c_str());
                    Console::Println("size: %d bytes", interpreter.ProgramLength());
                    Console::Println("");
                    Console::Println("Press START to quit");
                }
            }
        } else {
            // Check if interpreter is done
            if (interpreter.IsDone()) {
                if (input.Pressed(KeyCodes::A) && input.Pressed(KeyCodes::B)) {
                    // Clear selected file and return to the file browser
                    this->script_path = "";
                }
                return;
            }
            
            // Interpreter Step
            interpreter.Step(input, 12); // Do x steps per iteration

            // Draw any generated outputs
            displays.Upper.Clear();
            Imgui im(displays.Upper);
            for (int i = 0, line = 0; line < OUTPUT_LINES; line++) {
                im.BeginRow();
                for (int col = 0; col < OUTPUT_CHARS_PER_LINE; col++, i++) {
                    uint8_t ch = interpreter.GetBufferedChar(i);
                    im.Glyph(Typeface::DefaultFont[ch], Imgui::DefaultLabelStyle);
                }
                im.EndRow();
            }

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
    }

    void cleanup() override {

  
    }
};

int main() {
    Runtime app;
    app.Run();
    return 0;
}