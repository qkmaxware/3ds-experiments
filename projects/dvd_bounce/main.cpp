#include "../lib/ez3ds.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>

#include "texture.hpp"

class DvdBounce: public CitrusApp {
public:
    // For colour shift
    float hue = 0;
    float saturation = 1;
    float value = 1;

    // For bouncing
    float vx, vy = 0.0f;
    float px, py = 0.0f;
    int width = 64;
    int height = 20;

    void setup() override {
        EnableBottomConsole();
        Console::Println("Press START to quit");

        // Load resources
        auto welcome = RomFs::ReadAllText("romfs:/welcome.txt");
        Console::Println(welcome.c_str());
        width = TEX_IMG.Width;
        height = TEX_IMG.Height;

        // Seed random
        srand(time(NULL));

        // Random velocity between -5 and 5
        float speed = 10;
        vx = (rand() % 2) == 0 ? -speed : speed;
        vy = (rand() % 2) == 0 ? -speed : speed;

        // Random initial position
        px = (rand() % (400 - width));
        py = (rand() & (240 - height));
    }

    void loop(Displays &displays, Input &input) override {
        float dt = 0.1f;
        hue = std::fmod(hue + 5 * dt, 360);
        displays.Upper.Clear(Colour::FromHSV(hue, saturation, value));

        // Move cube
        px += vx * dt;
        py += vy * dt;
        
        // Handle bouncing
        if (px < 0 && vx < 0)
            vx = -vx;
        if (py < 0 && vy < 0)
            vy = -vy;
        if (px > (400 - width) && vx > 0)
            vx = -vx;
        if (py > (240 - height) && vy > 0)
            vy = -vy;

        // Draw rect
        displays.Upper.StampTexture(px, py, TEX_IMG);
    }

    void cleanup() override {

    }
};

int main() {
    DvdBounce app;
    app.Run();
    return 0;
}