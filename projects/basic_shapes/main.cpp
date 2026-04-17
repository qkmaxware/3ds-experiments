#include "../lib/ez3ds.hpp"

class Shapes: public CitrusApp {
public:
    void setup() override {
        Console::Println("Press START to quit");
    }

    void loop(Displays &displays, Input &input) override {
        displays.Upper.Clear();


        const int buffer = 10;
        const int size = 48;
        const int halfsize = size/2;

        displays.Upper.DrawRect(buffer, buffer, size, size, Colour::FromRgb(255, 0, 0));
        displays.Upper.DrawCircle(buffer*2 + size + halfsize, buffer + halfsize, halfsize, Colour::FromRgb(0, 255, 0));
        displays.Upper.DrawLine(buffer*3 + size * 2, buffer, buffer*3 + size*3, buffer + size, Colour::FromRgb(0, 0, 255));

        displays.Upper.FillRect(buffer, buffer*2 + size, size, size, Colour::White(), Colour::FromRgb(255, 0, 0));
        displays.Upper.FillCircle(buffer*2 + size + halfsize, buffer*2 + size + halfsize, halfsize, Colour::White(), Colour::FromRgb(0, 255, 0));
    }

    void cleanup() override {

    }
};

int main() {
    Shapes app;
    app.Run();
    return 0;
}