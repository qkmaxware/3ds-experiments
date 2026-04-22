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
    App(): MultiStateCitrusApp(static_cast<MultiStateCitrusApp::StateId>(AppState::TextEditor)), browse(), editor() {

    }

    void setup() override {
        // Global setup
        browse.oneTimeSetup();
        editor.oneTimeSetup();
    }

    void loop(Displays &displays, Input &input) override {
        Keypad pad;
        pad.Repaint(displays, input);
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