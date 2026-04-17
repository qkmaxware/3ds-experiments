#include "../lib/ez3ds.hpp"

class HelloWorld: public CitrusApp {
public:
    void setup() override {
        Console::Println("Hello World!");
        Console::Println("Press START to quit");
    }

    void loop(Displays &displays, Input &input) override {

    }

    void cleanup() override {

    }
};

int main() {
    HelloWorld app;
    app.Run();
    return 0;
}