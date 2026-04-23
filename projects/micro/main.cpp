#include "../lib/ez3ds.hpp"
#include "browse.hpp"
#include "editor.hpp"

enum class AppState {
    FileBrowser,
    TextEditor
};

class App : public MultiStateCitrusApp {
private:
    Browse browse;
    Editor editor;

public:
    App(): MultiStateCitrusApp(static_cast<MultiStateCitrusApp::StateId>(AppState::FileBrowser)), browse(), editor() {

    }

    void setup() override {
        // Global setup
        browse.oneTimeSetup();
        editor.oneTimeSetup();
    }

    // For debugging ONLY
    /*void loop(Displays &displays, Input &input) override {
        Keypad pad;
        pad.Repaint(displays, input);
    }*/

    void before_loop(Displays &displays, Input &input) override {
        displays.Upper.Clear();
        displays.Lower.Clear();
    }

    CitrusApp& GetState(MultiStateCitrusApp::StateId id) override {
        // Map stateid to an actual sub-app state
        switch (static_cast<AppState>(id)) {
            case AppState::FileBrowser:
                return this->browse;
            case AppState::TextEditor:
                return this->editor;

            default:
                return this->browse;
        }
    }

    MultiStateCitrusApp::StateId NextState(MultiStateCitrusApp::StateId currentId, CitrusApp &currentState) override {
        // Handle state transitions
        switch (static_cast<AppState>(currentId)) {
            case AppState::FileBrowser: {
                bool did_select = browse.State == BrowseState::SelectedFile;
                if (did_select) {
                    // Init the text-editor on transition
                    editor.load_file(browse.GetSelectedFile());
                }
                return did_select ? static_cast<MultiStateCitrusApp::StateId>(AppState::TextEditor) : currentId;
            } break;
            case AppState::TextEditor: {
                bool exit_requested = editor.State == EditorState::ExitRequested;
                return exit_requested ? static_cast<MultiStateCitrusApp::StateId>(AppState::FileBrowser) : currentId;
            } break;
            default:
                return currentId;
        }
        return currentId;
    }

    void cleanup() override {
        // Global cleanup
    }
};

int main() {
    App app;
    app.Run();
    return 0;
}