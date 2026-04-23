#include "../lib/ez3ds.hpp"

#ifndef NANO_BROWSE
#define NANO_BROWSE

enum class BrowseState {
    Browsing,
    SelectedFile
};

class Browse : public CitrusApp {
private:
    Imgui::FileBrowser fb;

public:
    BrowseState State;

    Browse(): fb(), State(BrowseState::Browsing) {}

    void oneTimeSetup() {
        fb.SetDir("/");
    }

    void setup() override {
        State = BrowseState::Browsing;
        fb.SetDir("/");
    }

    void loop(Displays &displays, Input &input) override { 
        // Browse
        if (fb.SelectFile(displays.Upper, input)) {
            State = BrowseState::SelectedFile;
        }
    }

    std::string GetSelectedFile() {
        return fb.Highlighted();
    }

    void cleanup() override {
        
    }
};

#endif