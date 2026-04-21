#include "../lib/ez3ds.hpp"

#ifndef NANO_BROWSE
#define NANO_BROWSE

class Browse : public CitrusApp {
private:
    Imgui::FileBrowser fb;

public:
    Browse(): fb() {}

    void oneTimeSetup() {
        fb.SetDir("/");
    }

    void setup() override {
        fb.SetDir("/");
    }

    void loop(Displays &displays, Input &input) override { 
        // Clear the upper screen
        displays.Upper.Clear();

        // Browse
        if (fb.SelectFile(displays.Upper, input)) {
            
        }
    }

    void cleanup() override {
        
    }
};

#endif