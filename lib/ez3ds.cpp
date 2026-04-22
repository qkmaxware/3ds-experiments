#include "ez3ds.hpp"
#include <3ds.h>
#include <math.h>
#include <cstring>
#include <cstdio>

inline float toaxis(uint32_t held, KeyCodes positive, KeyCodes negative) {
    float p = (held & positive) != 0 ? 1 : 0;
    float n = (held & negative) != 0 ? -1 : 0;
    return p + n;
}

void Input::UpdateKeys(uint32_t down, uint32_t held, uint32_t released) {
    this->down = down;
    this->held = held;
    this->released = released;

    this->dpad.X = toaxis(held, KeyCodes::DPadRight, KeyCodes::DPadLeft);
    this->dpad.Y = toaxis(held, KeyCodes::DPadUp, KeyCodes::DPadDown);
}

void Input::UpdatePads(float circleX, float circleY, float nubX, float nubY) {
    this->circle.X = circleX;
    this->circle.Y = circleY;
    this->nub.X = nubX;
    this->nub.Y = nubY;
}

void Input::UpdateTouch(uint16_t x, uint16_t y, bool touched) {
    this->pad.SetTouch(x, y, touched);
}

bool Input::JustPressed(KeyCodes code) const {
    return (this->down & (uint32_t)code) != 0;
}

bool Input::Pressed(KeyCodes code) const {
    return (this->held & (uint32_t)code) != 0;
}

bool Input::Released(KeyCodes code) const {
    return (this->released & (uint32_t)code) != 0;
}

Joystick Input::GetAxes(JoystickNames stick) const {
    switch (stick) {
        case JoystickNames::DPad:
            return this->dpad;
        case JoystickNames::CirclePad:
            return this->circle;
        case JoystickNames::Nub: 
            return this->nub;
        default:
            return Joystick();           
    }
}

Touchpad& Input::GetTouchpad() {
    return this->pad;
}

std::string Input::Prompt(const std::string &question) const {
    SwkbdState swkbd;
    static char input[64]; // Buffer
    memset(input, 0, sizeof(input));
    
    swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 1, -1);
    swkbdSetHintText(&swkbd, question.c_str());

    SwkbdButton btn = swkbdInputText(&swkbd, input, sizeof(input));

    if (btn == SWKBD_BUTTON_CONFIRM) {
        return std::string(input);
    }
    
    return "";
}

int Input::PromptInt(const std::string &question) const {
    std::string result = Prompt(question);

    if (result.empty()) {
        return 0;
    }

    long int parsed = std::strtol(result.c_str(), nullptr, 10);
    return static_cast<int>(parsed);
}

float Input::PromptFloat(const std::string &question) const {
    std::string result = Prompt(question);

    if (result.empty()) {
        return 0.0f;
    }

    float parsed = std::strtof(result.c_str(), nullptr);
    return static_cast<float>(parsed);
}

namespace Console {
    void Clear() {
        consoleClear();
    }
    void Print(const char *format, ...) {
        std::printf(format);
    }
    void Println(const char *format, ...) {
        std::printf(format);
        std::puts("\n");
    }
};

Colour Colour::FromHSV(float h, float s, float v, uint8_t a) {
    if (s <= 0.0f) {
        uint8_t gray = static_cast<uint8_t>(round(v * 255.0f));
        return Colour(gray, gray, gray, a);
    }

    float hh = fmod(h, 360.0f);
    if (hh < 0.0f) hh += 360.0f;
    float c = v * s;
    float x = c * (1.0f - fabs(fmod(hh / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float rp=0.0f, gp=0.0f, bp=0.0f;
    if (hh < 60.0f) {
        rp = c; gp = x; bp = 0.0f;
    } else if (hh < 120.0f) {
        rp = x; gp = c; bp = 0.0f;
    } else if (hh < 180.0f) {
        rp = 0.0f; gp = c; bp = x;
    } else if (hh < 240.0f) {
        rp = 0.0f; gp = x; bp = c;
    } else if (hh < 300.0f) {
        rp = x; gp = 0.0f; bp = c;
    } else {
        rp = c; gp = 0.0f; bp = x;
    }

    uint8_t r = static_cast<uint8_t>(round((rp + m) * 255.0f));
    uint8_t g = static_cast<uint8_t>(round((gp + m) * 255.0f));
    uint8_t b = static_cast<uint8_t>(round((bp + m) * 255.0f));
    return {r, g, b, a};
}

inline void Colour::ToHsv(float &h, float &s, float &v) const {
    float rf = R / 255.0f;
    float gf = G / 255.0f;
    float bf = B / 255.0f;

    float mx = fmax(rf, fmax(gf, bf));
    float mn = fmin(rf, fmin(gf, bf));
    float d = mx - mn;

    v = mx;
    s = (mx <= 0.0f) ? 0.0f : (d / mx);

    if (d <= 1e-6f) {
        h = 0.0f;
        return;
    }

    if (mx == rf) {
        h = 60.0f * (fmod(((gf - bf) / d), 6.0f));
    } else if (mx == gf) {
        h = 60.0f * (((bf - rf) / d) + 2.0f);
    } else { // mx == bf
        h = 60.0f * (((rf - gf) / d) + 4.0f);
    }

    if (h < 0.0f) h += 360.0f;
}

int _frameBufferFormatToBytes(int frameBufferFormat) {
    switch (frameBufferFormat) {
        case GSP_RGBA8_OES: return 4;
        case GSP_BGR8_OES: return 3;	
        case GSP_RGB565_OES: return 2;	
        case GSP_RGB5_A1_OES: return 2;
        case GSP_RGBA4_OES: return 2;
        default: return 3;
    }
}

Screen::Screen(int screenId, int side, int frameBufferFormat, int width, int height): 
    frameBufferFormat(frameBufferFormat), 
    bytesPerPixel(_frameBufferFormatToBytes(frameBufferFormat)), 
    screenId(screenId),
    side(side),
    Width(width),
    Height(height)
{ }

void Screen::FetchFramebuffer() {
    u16 w, h;
    this->pixels = gfxGetFramebuffer(
        static_cast<gfxScreen_t>(this->screenId), 
        static_cast<gfx3dSide_t>(this->side), 
        &w, 
        &h
    );

    // This is affixed by the constructor
    //this->width = w;
    //Sthis->height = h;
}

void Screen::Clear()
{
    if (!pixels) return;

    int bytesPerPixel = this->bytesPerPixel;

    // Rotated framebuffer dimensions
    int fbWidth  = this->Height; // ex. 240
    int fbHeight = this->Width;  // ex. 400

    const size_t totalBytes = static_cast<size_t>(fbWidth) *
                              static_cast<size_t>(fbHeight) *
                              bytesPerPixel;

    memset(pixels, 0, totalBytes);
}

void Screen::Clear(Colour colour) {
    if (!pixels) return;

    int bytesPerPixel = this->bytesPerPixel;

    // Rotated framebuffer dimensions
    int fbWidth  = this->Height; // ex. 240
    int fbHeight = this->Width;  // ex. 400

    const size_t totalBytes = static_cast<size_t>(fbWidth) *
                              static_cast<size_t>(fbHeight) *
                              bytesPerPixel;

    auto buffer = this->pixels;
    for (size_t i = 0; i < totalBytes; i += bytesPerPixel) {
        buffer[i + 0] = colour.B;
        buffer[i + 1] = colour.G;
        buffer[i + 2] = colour.R;

        if (bytesPerPixel == 4)
            buffer[i + 3] = colour.A;
    }
}

void Screen::SetPixel(int x, int y, Colour colour)
{
    if (!pixels || !inbounds(x, y)) return;
    if (colour.A == 0) return; // Don't draw transparent pixels

    int fbWidth = this->Height; // Buffer rotated 90 degrees

    int fbX = fbWidth - 1 - y;
    int fbY = x;

    int pixelIndex = fbY * fbWidth + fbX;

    switch (frameBufferFormat)
    {
        case GSP_RGBA8_OES:
        {
            uint8_t* buffer = pixels + pixelIndex * 4;
            buffer[0] = colour.B;
            buffer[1] = colour.G;
            buffer[2] = colour.R;
            buffer[3] = colour.A;
            break;
        }

        case GSP_BGR8_OES:
        {
            uint8_t* buffer = pixels + pixelIndex * 3;
            buffer[0] = colour.B;
            buffer[1] = colour.G;
            buffer[2] = colour.R;
            break;
        }

        case GSP_RGB565_OES:
        {
            uint16_t* buffer = reinterpret_cast<uint16_t*>(pixels);
            
            uint16_t r = (colour.R >> 3) & 0x1F;
            uint16_t g = (colour.G >> 2) & 0x3F;
            uint16_t b = (colour.B >> 3) & 0x1F;

            buffer[pixelIndex] = (r << 11) | (g << 5) | b;
            break;
        }

        case GSP_RGB5_A1_OES:
        {
            uint16_t* buffer = reinterpret_cast<uint16_t*>(pixels);

            uint16_t r = (colour.R >> 3) & 0x1F;
            uint16_t g = (colour.G >> 3) & 0x1F;
            uint16_t b = (colour.B >> 3) & 0x1F;
            uint16_t a = (colour.A > 127) ? 1 : 0;

            buffer[pixelIndex] = (r << 11) | (g << 6) | (b << 1) | a;
            break;
        }

        case GSP_RGBA4_OES:
        {
            uint16_t* buffer = reinterpret_cast<uint16_t*>(pixels);

            uint16_t r = (colour.R >> 4) & 0x0F;
            uint16_t g = (colour.G >> 4) & 0x0F;
            uint16_t b = (colour.B >> 4) & 0x0F;
            uint16_t a = (colour.A >> 4) & 0x0F;

            buffer[pixelIndex] = (r << 12) | (g << 8) | (b << 4) | a;
            break;
        }
    }
}

void Screen::DrawLine(int x1, int y1, int x2, int y2, Colour stroke)
{
    int dx = abs(x2 - x1);
    int dy = -abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx + dy;

    while (true)
    {
        SetPixel(x1, y1, stroke);

        if (x1 == x2 && y1 == y2)
            break;

        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

void Screen::DrawRect(int x, int y, int width, int height, Colour stroke)
{
    // Top
    DrawLine(x, y, x + width - 1, y, stroke);

    // Bottom
    DrawLine(x, y + height - 1, x + width - 1, y + height - 1, stroke);

    // Left
    DrawLine(x, y, x, y + height - 1, stroke);

    // Right
    DrawLine(x + width - 1, y, x + width - 1, y + height - 1, stroke);
}

void Screen::FillRect(int x, int y, int width, int height, Colour stroke, Colour fill)
{
    // Fill interior
    for (int iy = 1; iy < height - 1; iy++)
    {
        for (int ix = 1; ix < width - 1; ix++)
        {
            SetPixel(x + ix, y + iy, fill);
        }
    }

    // Draw border
    DrawRect(x, y, width, height, stroke);
}

void Screen::DrawCircle(int cx, int cy, int radius, Colour stroke)
{
    int x = radius;
    int y = 0;
    int err = 1 - x;

    while (x >= y)
    {
        SetPixel(cx + x, cy + y, stroke);
        SetPixel(cx + y, cy + x, stroke);
        SetPixel(cx - y, cy + x, stroke);
        SetPixel(cx - x, cy + y, stroke);
        SetPixel(cx - x, cy - y, stroke);
        SetPixel(cx - y, cy - x, stroke);
        SetPixel(cx + y, cy - x, stroke);
        SetPixel(cx + x, cy - y, stroke);

        y++;
        if (err < 0)
        {
            err += 2 * y + 1;
        }
        else
        {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

void Screen::FillCircle(int cx, int cy, int radius, Colour stroke, Colour fill)
{
    int x = radius;
    int y = 0;
    int err = 1 - x;

    while (x >= y)
    {
        // Draw horizontal spans between −x..+x
        auto DrawSpan = [&](int sx, int ex, int py)
        {
            for (int px = sx; px <= ex; px++)
                SetPixel(px, py, fill);
        };

        // Horizontal spans for the filled area
        DrawSpan(cx - x, cx + x, cy + y);
        DrawSpan(cx - x, cx + x, cy - y);
        DrawSpan(cx - y, cx + y, cy + x);
        DrawSpan(cx - y, cx + y, cy - x);

        // Circle outline on top
        DrawCircle(cx, cy, radius, stroke);

        y++;
        if (err < 0)
            err += 2 * y + 1;
        else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

void Screen::StampGlyph(const Typeface::Glyph& glyph, int x, int y, int scale, Colour foreground, Colour background) {
    int xPtr = x;
    int yPtr = y;
    for (int row = 0; row < Typeface::Height; row++) {
        uint8_t row_data = glyph.Rows[row];
        
        for (int yrep = 0; yrep < scale; yrep++) {
            for (int col = 0; col < Typeface::Width; col++) {
                for (int xrep = 0; xrep < scale; xrep++) {
                    if ((row_data & (0x80 >> col)) != 0) {
                        this->SetPixel(xPtr++, yPtr, foreground);
                    }
                    else {
                        this->SetPixel(xPtr++, yPtr, background);
                    }
                }
            }
            xPtr = x;
            yPtr++;
        }
    }
}

void Screen::StampTexture(int x, int y, const Texture &texture) {
    for (int py = 0; py < texture.Height; py++) {
        for (int px = 0; px < texture.Width; px++) {
            Colour c = texture.GetPixel(px, py);
            SetPixel(x + px, y + py, c);
        }
    }
}


Displays::Displays(): Upper(GFX_TOP, GFX_LEFT, static_cast<int>(gfxGetScreenFormat(GFX_TOP)), 400, 240), Lower(GFX_BOTTOM, GFX_LEFT, static_cast<int>(gfxGetScreenFormat(GFX_BOTTOM)), 320, 240) {}


namespace Typeface {

Font _makeDefaultFont() {
    Font result {};

    result['a'] = Glyph{{0x00,0x00,0x3C,0x06,0x3E,0x66,0x66,0x3E,0x00,0x00,0x00,0x00}}; // a
    result['b'] = Glyph{{0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x7C,0x00,0x00,0x00,0x00}}; // b
    result['c'] = Glyph{{0x00,0x00,0x3C,0x66,0x60,0x60,0x66,0x3C,0x00,0x00,0x00,0x00}}; // c
    result['d'] = Glyph{{0x06,0x06,0x3E,0x66,0x66,0x66,0x66,0x3E,0x00,0x00,0x00,0x00}}; // d
    result['e'] = Glyph{{0x00,0x00,0x3C,0x66,0x7E,0x60,0x66,0x3C,0x00,0x00,0x00,0x00}}; // e
    result['f'] = Glyph{{0x1C,0x36,0x30,0x7C,0x30,0x30,0x30,0x30,0x00,0x00,0x00,0x00}}; // f
    result['g'] = Glyph{{0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x3C,0x00,0x00,0x00,0x00}}; // g
    result['h'] = Glyph{{0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00}}; // h
    result['i'] = Glyph{{0x18,0x00,0x38,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00}}; // i
    result['j'] = Glyph{{0x0C,0x00,0x1C,0x0C,0x0C,0x0C,0x6C,0x38,0x00,0x00,0x00,0x00}}; // j
    result['k'] = Glyph{{0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x66,0x00,0x00,0x00,0x00}}; // k
    result['l'] = Glyph{{0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00}}; // l
    result['m'] = Glyph{{0x00,0x00,0x6C,0x7E,0x7E,0x6A,0x6A,0x62,0x00,0x00,0x00,0x00}}; // m
    result['n'] = Glyph{{0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00}}; // n
    result['o'] = Glyph{{0x00,0x00,0x3C,0x66,0x66,0x66,0x66,0x3C,0x00,0x00,0x00,0x00}}; // o
    result['p'] = Glyph{{0x00,0x00,0x7C,0x66,0x66,0x7C,0x60,0x60,0x00,0x00,0x00,0x00}}; // p
    result['q'] = Glyph{{0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x06,0x00,0x00,0x00,0x00}}; // q
    result['r'] = Glyph{{0x00,0x00,0x6C,0x76,0x60,0x60,0x60,0x60,0x00,0x00,0x00,0x00}}; // r
    result['s'] = Glyph{{0x00,0x00,0x3E,0x60,0x3C,0x06,0x66,0x3C,0x00,0x00,0x00,0x00}}; // s
    result['t'] = Glyph{{0x30,0x30,0x7C,0x30,0x30,0x30,0x36,0x1C,0x00,0x00,0x00,0x00}}; // t
    result['u'] = Glyph{{0x00,0x00,0x66,0x66,0x66,0x66,0x66,0x3E,0x00,0x00,0x00,0x00}}; // u
    result['v'] = Glyph{{0x00,0x00,0x66,0x66,0x66,0x66,0x3C,0x18,0x00,0x00,0x00,0x00}}; // v
    result['w'] = Glyph{{0x00,0x00,0x62,0x6A,0x6A,0x7E,0x7E,0x34,0x00,0x00,0x00,0x00}}; // w
    result['x'] = Glyph{{0x00,0x00,0x66,0x3C,0x18,0x18,0x3C,0x66,0x00,0x00,0x00,0x00}}; // x
    result['y'] = Glyph{{0x00,0x00,0x66,0x66,0x3E,0x06,0x66,0x3C,0x00,0x00,0x00,0x00}}; // y
    result['z'] = Glyph{{0x00,0x00,0x7E,0x0C,0x18,0x30,0x60,0x7E,0x00,0x00,0x00,0x00}}; // z

    result['A'] = Glyph{{0x3C,0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00,0x00,0x00,0x00}}; // A
    result['B'] = Glyph{{0x7C,0x66,0x66,0x7C,0x66,0x66,0x66,0x7C,0x00,0x00,0x00,0x00}}; // B
    result['C'] = Glyph{{0x3C,0x66,0x60,0x60,0x60,0x60,0x66,0x3C,0x00,0x00,0x00,0x00}}; // C
    result['D'] = Glyph{{0x78,0x6C,0x66,0x66,0x66,0x66,0x6C,0x78,0x00,0x00,0x00,0x00}}; // D
    result['E'] = Glyph{{0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x7E,0x00,0x00,0x00,0x00}}; // E
    result['F'] = Glyph{{0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x60,0x00,0x00,0x00,0x00}}; // F
    result['G'] = Glyph{{0x3C,0x66,0x60,0x60,0x6E,0x66,0x66,0x3C,0x00,0x00,0x00,0x00}}; // G
    result['H'] = Glyph{{0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00}}; // H
    result['I'] = Glyph{{0x3C,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00}}; // I
    result['J'] = Glyph{{0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x6C,0x38,0x00,0x00,0x00,0x00}}; // J
    result['K'] = Glyph{{0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x66,0x00,0x00,0x00,0x00}}; // K
    result['L'] = Glyph{{0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00,0x00,0x00,0x00}}; // L
    result['M'] = Glyph{{0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x63,0x00,0x00,0x00,0x00}}; // M
    result['N'] = Glyph{{0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x66,0x00,0x00,0x00,0x00}}; // N
    result['O'] = Glyph{{0x3C,0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00,0x00,0x00,0x00}}; // O
    result['P'] = Glyph{{0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x60,0x00,0x00,0x00,0x00}}; // P
    result['Q'] = Glyph{{0x3C,0x66,0x66,0x66,0x66,0x6E,0x3C,0x0E,0x00,0x00,0x00,0x00}}; // Q
    result['R'] = Glyph{{0x7C,0x66,0x66,0x7C,0x78,0x6C,0x66,0x66,0x00,0x00,0x00,0x00}}; // R
    result['S'] = Glyph{{0x3C,0x66,0x60,0x3C,0x06,0x06,0x66,0x3C,0x00,0x00,0x00,0x00}}; // S
    result['T'] = Glyph{{0x7E,0x5A,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00}}; // T
    result['U'] = Glyph{{0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00,0x00,0x00,0x00}}; // U
    result['V'] = Glyph{{0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00,0x00,0x00,0x00}}; // V
    result['W'] = Glyph{{0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x63,0x00,0x00,0x00,0x00}}; // W
    result['X'] = Glyph{{0x66,0x66,0x3C,0x18,0x18,0x3C,0x66,0x66,0x00,0x00,0x00,0x00}}; // X
    result['Y'] = Glyph{{0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00}}; // Y
    result['Z'] = Glyph{{0x7E,0x06,0x0C,0x18,0x30,0x60,0x60,0x7E,0x00,0x00,0x00,0x00}}; // Z

    result['('] = Glyph{{0x1C,0x30,0x60,0x60,0x60,0x60,0x60,0x30,0x1C,0x00,0x00,0x00}}; // (
    result[')'] = Glyph{{0x38,0x0C,0x06,0x06,0x06,0x06,0x06,0x0C,0x38,0x00,0x00,0x00}}; // )
    result['<'] = Glyph{{0x0c,0x18,0x30,0x60,0x60,0x30,0x18,0x0c,0x00,0x00,0x00,0x00}}; // <
    result['>'] = Glyph{{0x30,0x18,0x0c,0x06,0x06,0x0c,0x18,0x30,0x00,0x00,0x00,0x00}}; // >

    result['0'] = Glyph{{0x3C,0x66,0x6E,0x76,0x76,0x6E,0x66,0x3C,0x00,0x00,0x00,0x00}}; // 0
    result['1'] = Glyph{{0x18,0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00}}; // 1
    result['2'] = Glyph{{0x3C,0x66,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00,0x00,0x00,0x00}}; // 2
    result['3'] = Glyph{{0x3C,0x66,0x06,0x1C,0x06,0x06,0x66,0x3C,0x00,0x00,0x00,0x00}}; // 3
    result['4'] = Glyph{{0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C,0x1E,0x00,0x00,0x00,0x00}}; // 4
    result['5'] = Glyph{{0x7E,0x60,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00,0x00,0x00,0x00}}; // 5
    result['6'] = Glyph{{0x3C,0x66,0x60,0x7C,0x66,0x66,0x66,0x3C,0x00,0x00,0x00,0x00}}; // 6
    result['7'] = Glyph{{0x7E,0x06,0x0C,0x18,0x18,0x30,0x30,0x30,0x00,0x00,0x00,0x00}}; // 7
    result['8'] = Glyph{{0x3C,0x66,0x66,0x3C,0x66,0x66,0x66,0x3C,0x00,0x00,0x00,0x00}}; // 8
    result['9'] = Glyph{{0x3C,0x66,0x66,0x66,0x3E,0x06,0x66,0x3C,0x00,0x00,0x00,0x00}}; // 9

    result['.'] = Glyph{{0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00}}; // .
    result[','] = Glyph{{0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x08,0x18,0x00,0x00}}; // ,

    result['-'] = Glyph{{0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}; // -
    result['+'] = Glyph{{0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00}}; // +

    result['*'] = Glyph{{0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00,0x00,0x00,0x00,0x00}}; // *
    result['/'] = Glyph{{0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00,0x00,0x00,0x00,0x00}}; // /

    result['!'] = Glyph{{0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00,0x00,0x00,0x00,0x00}}; // !
    result[':'] = Glyph{{0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00}}; // :
    result[';'] = Glyph{{0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x30,0x00,0x00,0x00,0x00}}; // ;

    result['@'] = Glyph{{0x3C,0x66,0x6E,0x6A,0x6E,0x60,0x3E,0x00,0x00,0x00,0x00,0x00}}; // @
    result['#'] = Glyph{{0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0x00,0x00,0x00,0x00,0x00}}; // #
    result['$'] = Glyph{{0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00,0x00,0x00,0x00,0x00}}; // $

    result['%'] = Glyph{{0x62,0x64,0x08,0x10,0x26,0x46,0x00,0x00,0x00,0x00,0x00,0x00}}; // %
    result['^'] = Glyph{{0x18,0x3C,0x66,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}; // ^
    result['&'] = Glyph{{0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00,0x00,0x00,0x00,0x00}}; // &

    return result;
}

const Font DefaultFont = _makeDefaultFont();

namespace SpecialGlyphs {

const Glyph Directory = Glyph{{0x78,0x78,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00}};
const Glyph ChevronUp = Glyph{{0x00,0x00,0x18,0x3c,0x7e,0x66,0xe7,0xc3,0x00,0x00,0x00,0x00}};
const Glyph ChevronDown = Glyph{{0x00,0x00,0xc3,0xc3,0x66,0x7e,0x3c,0x18,0x00,0x00,0x00,0x00}};
const Glyph LeftHalfBlock = Glyph{{0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0x00,0x00,0x00,0x00}};
const Glyph RightHalfBlock = Glyph{{0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x00,0x00,0x00,0x00}};
const Glyph Return = Glyph{{0x00,0x00,0x02,0x12,0x32,0x7e,0x30,0x10,0x00,0x00,0x00,0x00}};
const Glyph Backspace = Glyph{{0x00,0x3f,0x41,0x95,0x89,0x95,0x41,0x3f,0x00,0x00,0x00,0x00}};
}
}

int Imgui::WidthPixels() {
    return this->screen.Width;
}
int Imgui::WidthCharacters() {
    return this->screen.Width / (Typeface::Width + Typeface::Kerning);
}

int Imgui::HeightPixels() {
    return this->screen.Height;
}
int Imgui::HeightLines() {
    return this->screen.Height / Typeface::LineHeight;
}

void Imgui::BeginRow() {
    this->dir = Row;
}

void Imgui::EndRow() {
    this->dir = Column;
    Imgui::NextLine();
}

void Imgui::NextLine() {
    this->y += Typeface::LineHeight;
    this->x = 0;
}

const Imgui::LabelStyle Imgui::DefaultLabelStyle = Imgui::LabelStyle(Colour::White());
void Imgui::Label(const std::string &text, const LabelStyle &style) {
    Screen &screen = this->screen;
    int yStart = this->y;

    // Foreach character
    for (uint8_t ch : text) {
        if (ch >= Typeface::CharacterCount)
            ch = '?';

        Typeface::Glyph glyph = Typeface::DefaultFont[ch];
        
        // Draw character to screen at given x,y coordinate
        int xStart = this->x;
        for (int row = 0; row < Typeface::Height; row++) {
            uint8_t row_data = glyph.Rows[row];

            for (int col = 0; col < Typeface::Width; col++) {
                if ((row_data & (0x80 >> col)) != 0) {
                    screen.SetPixel(xStart + col, yStart + Typeface::VPad + row, style.FontColour);
                }
            }
        }

        this->x += Typeface::Width + Typeface::Kerning;
    }   
    if (this->dir == Column)
        NextLine(); // Auto-next line if column layout mode
}
void Imgui::Glyph(const Typeface::Glyph &glyph, const LabelStyle &style) {
    Screen &screen = this->screen;
    int yStart = this->y;
    
    // Draw character to screen at given x,y coordinate
    int xStart = this->x;
    for (int row = 0; row < Typeface::Height; row++) {
        uint8_t row_data = glyph.Rows[row];

        for (int col = 0; col < Typeface::Width; col++) {
            if ((row_data & (0x80 >> col)) != 0) {
                screen.SetPixel(xStart + col, yStart + Typeface::VPad + row, style.FontColour);
            }
        }
    }

    this->x += Typeface::Width + Typeface::Kerning;
     
    if (this->dir == Column)
        NextLine(); // Auto-next line if column layout mode
}

void Imgui::IndentCharacters(int characters) {
    this->x += (Typeface::Width + Typeface::Kerning) * characters;
}
void Imgui::IndentPixels(int px) {
    this->x += px;
}


const Imgui::ButtonStyle Imgui::DefaultButtonStyle = Imgui::ButtonStyle(Colour::White(), Colour::Grey(), Colour::DarkGrey(), Colour::White(), Colour::Grey(), Colour::Blue());
bool Imgui::Button(const std::string &text, const ButtonStyle &style) {
    // Draw a rectangle with the text centered inside it (note I'd have to know how big the text is for centering)
    int charCount = text.length();
    int width = 0;
    if (charCount > 0) {
        width = charCount * Typeface::Width + (charCount - 1) * Typeface::Kerning;
    }

    int rectBeginX = x, rectWidth = width + 2 * Typeface::VPad, rectEndX = x + width + 2 * Typeface::VPad;
    int rectBeginY = y, rectHeight = Typeface::LineHeight, rectEndY = y + Typeface::LineHeight;

    bool inArea = false; // No input checking if we have no input
    bool isPressed = false;
    if (input != NULL) {
        Touchpad& pad = input->GetTouchpad();
        inArea = pad.X >= rectBeginX && pad.X <= rectEndX && pad.Y >= rectBeginY && pad.Y <= rectEndY;
        isPressed = pad.IsJustPressed() && inArea;
    }

    // Draw button
    Colour font   = inArea ? style.HoverFontColour : style.NormalFontColour;
    Colour stroke = inArea ? style.HoverBorderColour : style.NormalBorderColour;
    Colour fill   = inArea ? style.HoverBackgroundColour : style.NormalBackgroundColour;
    screen.FillRect(x, y, rectWidth, rectHeight, stroke, fill);

    // Draw button text
    int xStart = x + Typeface::VPad;
    int yStart = y + Typeface::VPad;
    int xOffset = 0;
    for (uint8_t ch : text) {
        if (ch >= Typeface::CharacterCount)
            ch = '?';

        Typeface::Glyph glyph = Typeface::DefaultFont[ch];
        
        // Draw character to screen at given x,y coordinate
        for (int row = 0; row < Typeface::Height; row++) {
            uint8_t row_data = glyph.Rows[row];

            for (int col = 0; col < Typeface::Width; col++) {
                if ((row_data & (0x80 >> col)) != 0) {
                    screen.SetPixel(xStart + xOffset + col, yStart + row, font);
                }
            }
        }

        xOffset += Typeface::Width + Typeface::Kerning;
    }  

    this->x += rectWidth;
    if (this->dir == Column)
        NextLine(); // Auto-next line if column layout mode

    return isPressed;
}

void CitrusApp::Run() {
    // Initialize services
	gfxInitDefault();
    gfxSet3D (false);
    gfxSetDoubleBuffering(GFX_TOP, true);
    gfxSetDoubleBuffering(GFX_BOTTOM, true);
    fsInit();
    romfsInit();

    // User custom initialization
    this->setup();
    
    Displays displays;
    Input input;
    while (aptMainLoop()) {
        // Scan all the inputs. This should be done once for each frame
		hidScanInput();

        //Read the CirclePad position
        circlePosition pos;
		hidCircleRead(&pos);

        // Read the touchpad
        touchPosition touch;
        hidTouchRead(&touch);

        // hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		// hidKeysHeld returns information about which buttons have are held down in this frame
        // hidKeysUp returns information about which buttons have been just released
        u32 kHeld = hidKeysHeld();
        input.UpdateKeys(hidKeysDown(), kHeld, hidKeysUp());
        input.UpdatePads(pos.dx, pos.dy, 0, 0); // todo nub
        input.UpdateTouch(touch.px, touch.py, kHeld & KEY_TOUCH);

        if (input.JustPressed(KeyCodes::Start)) break; // break in order to return to hbmenu (standard practice is to exit when start is pressed)
        
        // Update the screen framebuffers
        displays.FetchFramebuffers();

        // User update loop
        this->loop(displays, input);

        // Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();

        // Wait for VBlank
		gspWaitForVBlank();
    }

    // User cleanup or deallocation
    this->cleanup();

    // Exit services
    romfsExit();
    fsExit();
	gfxExit();
}

void CitrusApp::EnableBottomConsole() {
    // Initialize console on bottom screen. Using NULL as the second argument tells the console library to use the internal console structure as current one
	consoleInit(GFX_BOTTOM, NULL);
}

MultiStateCitrusApp::MultiStateCitrusApp(MultiStateCitrusApp::StateId defaultState): CitrusApp(), hasEnteredCurrentState(false), currentStateId(defaultState) {}

void MultiStateCitrusApp::loop(Displays &displays, Input &input) {
    // Get the most recent state
    CitrusApp &state = this->GetState(currentStateId);
    
    // Test if we've just entered the state
    if (!hasEnteredCurrentState) {
        hasEnteredCurrentState = true;
        state.setup();
    }

    // Handle current state action
    state.loop(displays, input);

    // Handle state transitions / exiting current state
    StateId nextStateId = this->NextState(currentStateId, state);
    if (nextStateId != currentStateId) {
        state.cleanup();
        hasEnteredCurrentState = false;
        currentStateId = nextStateId;
    }
}

// Included here for RomFs and file management stuffs
#include <cstdio>      // FILE*, fopen, fclose, fread, fgetc, fscanf, ungetc
#include <cstdlib>     // std::runtime_error (or use <stdexcept> if you prefer)
#include <stdexcept>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <string>

bool list_dir(const std::string &root_dir, const std::string &path, std::vector<std::string> &out_files) {
    out_files.clear();

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        return false;
    }

    if (path != root_dir)
        out_files.push_back("..");

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        std::string name = entry->d_name;

        if (name == "." || name == "..")
            continue;

        out_files.push_back(name);
    }

    closedir(dir);
    return true;
}

bool is_dir(const std::string &current_dir, const std::string &filename) {
    struct stat s;
    std::string full_path = current_dir;

    if (full_path.back() != '/')
        full_path += '/';

    full_path += filename;

    if (stat(full_path.c_str(), &s) == 0) {
        return S_ISDIR(s.st_mode);
    }

    return false;
}

bool up_dir(const std::string &root_dir, std::string &current_dir, std::vector<std::string>::size_type &selected_index, std::vector<std::string> &files) {
    if (current_dir == root_dir) {
        return true; // We did change dir, we just changed to the current directory
    }

    // Remove trailing slash (except if root)
    std::string path = current_dir;
    if (path.size() > 1 && path.back() == '/') {
        path.pop_back();
    }

    // Find last slash
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        current_dir = root_dir;
    } else if (pos == 0) {
        current_dir = "/";
    } else {
        current_dir = path.substr(0, pos);
    }

    files.clear();
    list_dir(root_dir, current_dir, files);
    selected_index = 0;
    return true;
}

bool enter_dir(const std::string &root_dir, std::string &current_dir, const std::string &folder, std::vector<std::string>::size_type &selected_index, std::vector<std::string> &files) {
    if (!is_dir(current_dir, folder)) {
        return false;
    }

    std::string new_path = current_dir;

    if (new_path.back() != '/')
        new_path += '/';

    new_path += folder;

    // Optional: resolve to absolute path
    //char resolved[PATH_MAX];
    //if (realpath(new_path.c_str(), resolved)) {
        //current_dir = std::string(resolved);
    //} else {
        current_dir = new_path; // fallback
    //}

    files.clear();
    list_dir(root_dir, current_dir, files);
    selected_index = 0;
    return true;
}

const Imgui::LabelStyle FileBrowserCurDirStyle = Imgui::LabelStyle(Colour::FromRgb(140, 176, 245));
const Imgui::LabelStyle FileBrowserDirStyle = Imgui::LabelStyle(Colour::FromRgb(53, 118, 240));
const Imgui::LabelStyle FileBrowserFileStyle = Imgui::LabelStyle(Colour::White());
Imgui::FileBrowser::FileBrowser(): 
    root_dir("/"),
    current_dir("/"),
    selected_index(0),
    files(),
    listing_error(false),
    page_count(0),
    page_index(0),
    page_size(0)
{
    
}

bool Imgui::FileBrowser::HasErrorOccured() {
    return this->listing_error;
}

std::string Imgui::FileBrowser::CurrentDirectory() {
    return this->current_dir;
}

int Imgui::FileBrowser::FileCount() {
    return this->files.size();
}


bool starts_with(const std::string &str, const std::string &prefix) {
    return str.compare(0, prefix.size(), prefix) == 0;
}

const std::string& Imgui::FileBrowser::GetDir() {
    return this->current_dir;
}

void Imgui::FileBrowser::SetDir(const std::string &current_dir) {
    if (!starts_with(current_dir, this->root_dir))
        return; 
    this->current_dir = current_dir;
    this->listing_error = !list_dir(this->root_dir, this->current_dir, this->files);
}

void Imgui::FileBrowser::PageInfo(int &count, int &index, int &size) {
    count = this->page_count;
    index = this->page_index;
    size = this->page_size;
}

std::string Imgui::FileBrowser::Highlighted() {
    std::vector<std::string>::size_type ind = this->selected_index;
    std::vector<std::string> &files = this->files;
    if (ind < 0 || ind >= files.size())
        return "";

    std::string full_path = this->current_dir;

    if (full_path.back() != '/')
        full_path += '/';

    full_path += files[ind]; 
    return full_path;
}

bool Imgui::FileBrowser::SelectFile(Screen &display, Input &input) {
    const int screenHeight = display.Height;
    int page_size = (screenHeight / Typeface::LineHeight) - 3; // Assume a 3 line buffer
    std::vector<std::string>::size_type file_count = files.size();
    int pages = (file_count + (page_size - 1)) / page_size;

    this->page_size = page_size;
    this->page_count = pages;

    bool isFileSelected = false;

    // Handle inputs
    if (input.JustPressed(KeyCodes::DPadUp)) {
        selected_index--;
    }
    if (input.JustPressed(KeyCodes::DPadDown)) {
        selected_index++;
    }
    selected_index = (selected_index > (file_count - 1) ? (file_count - 1) : (selected_index < 0 ? 0 : selected_index));

    if (input.JustPressed(KeyCodes::A)) {
        // Enter directory
        if (selected_index >= 0 && selected_index < file_count) {
            std::string &selected = files[selected_index];
            if (selected == ".") {
                // Nothing
            } else if (selected == "..") {
                this->listing_error = !up_dir(this->root_dir, this->current_dir, this->selected_index, this->files);
            } else {
                if (is_dir(this->current_dir, selected)) {
                    this->listing_error = !enter_dir(this->root_dir, this->current_dir, selected, this->selected_index, this->files);
                } else {
                    isFileSelected = true;
                }
            }
        }
    } else if (input.JustPressed(KeyCodes::B)) {
        // Exit directory
        this->listing_error = !up_dir(this->root_dir, this->current_dir, this->selected_index, this->files);
    }

    int current_page = selected_index / page_size;
    //int offset_in_page = selected_index - current_page * page_size;
    this->page_index = current_page;

    // Draw items
    Imgui im(display);
    im.Label(this->current_dir, FileBrowserCurDirStyle);
    //im.Label("Up/Down-navigate A-select B-up directory", Imgui::DefaultLabelStyle);
    if (current_page > 0) {
        im.IndentCharacters(im.WidthCharacters() / 2 - 1);
        im.Glyph(Typeface::SpecialGlyphs::ChevronUp, Imgui::DefaultLabelStyle);
    } 
    else {
        im.NextLine();
    }

    for (int i = 0; i < page_size; i++) {
        size_t item_index = current_page * page_size + i;
        
        im.BeginRow();
        if (item_index == selected_index) {
            im.IndentCharacters(1);
            im.Glyph(Typeface::SpecialGlyphs::LeftHalfBlock, Imgui::DefaultLabelStyle);
            im.IndentCharacters(1);
        } else {
            im.IndentCharacters(3);
        }
        if (item_index >= 0 && item_index < files.size()) {
            std::string &item = files[item_index];
            bool dir = item_index == 0 || is_dir(this->current_dir, item);
            if (dir) {
                im.Glyph(Typeface::SpecialGlyphs::Directory, FileBrowserDirStyle);
                im.IndentCharacters(1);
            }
            im.Label(item, dir ? FileBrowserDirStyle : FileBrowserFileStyle);
            if (dir) {
                im.Glyph(Typeface::DefaultFont['/'], FileBrowserDirStyle);
            }
        } else {
            im.Label("", Imgui::DefaultLabelStyle);
        }
        im.EndRow();
    }

    auto padding = 0;
    if (current_page < (pages - 1)) {
        auto x = im.WidthCharacters() / 2 - 1;
        im.IndentCharacters(x);
        im.Glyph(Typeface::SpecialGlyphs::ChevronDown, Imgui::DefaultLabelStyle);

        padding += x + 1;
        
    }
    char index_str[10];
    auto chars_written = snprintf(index_str, sizeof(index_str), "%i/%i", selected_index + 1, file_count);
    padding = im.WidthCharacters() - (padding) - (chars_written);
    if (padding > 0)
        im.IndentCharacters(padding);
    im.Label(std::string(index_str) ,Imgui::DefaultLabelStyle);

    return isFileSelected;
}

namespace RomFs {

std::string ReadAllText(const std::string &path) {
    std::string result;

    FILE* f = fopen(path.c_str(), "rb"); // use binary to preserve exact contents
    if (!f)
        return result;

    char buffer[256];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), f)) > 0)
    {
        result.append(buffer, bytesRead);
    }

    fclose(f);
    return result;
}

std::vector<uint8_t> ReadAllBytes(const std::string &path) {
    std::vector<uint8_t> result;

    FILE* f = fopen(path.c_str(), "rb");
    if (!f)
        return result;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    if (size > 0) {
        result.resize(size);
        fread(result.data(), 1, size, f);
    }

    fclose(f);
    return result;
}

void ppm_skip_comments(FILE* f)
{
    int c;
    while ((c = fgetc(f)) == '#')
    {
        while (c != '\n' && c != EOF)
            c = fgetc(f);
    }
    ungetc(c, f);
}
bool TextureLoadPPM(const std::string &path, Texture &out_texture) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f)
        return false;

    char magic[2] = {(char)fgetc(f), (char)fgetc(f)};

    bool isP3 = strcmp(magic, "P3") == 0;
    bool isP6 = strcmp(magic, "P6") == 0;

    if (!isP3 && !isP6)
    {
        fclose(f);
        return false;
    }

    ppm_skip_comments(f);

    int width, height;
    fscanf(f, "%d %d", &width, &height);

    ppm_skip_comments(f);

    int maxVal;
    fscanf(f, "%d", &maxVal);

    fgetc(f); // consume single whitespace after maxVal

    if (maxVal <= 0)
    {
        fclose(f);
        return false;
    }

    std::vector<Colour> pixels;
    pixels.resize(width * height);

    float scale = 255.0f / maxVal;

    if (isP3)
    {
        // ASCII
        for (int i = 0; i < width * height; ++i)
        {
            int r, g, b;
            fscanf(f, "%d %d %d", &r, &g, &b);

            pixels[i] = Colour(
                (uint8_t)(r * scale),
                (uint8_t)(g * scale),
                (uint8_t)(b * scale)
            );
        }
    }
    else
    {
        // Binary (P6)
        if (maxVal <= 255)
        {
            for (int i = 0; i < width * height; ++i)
            {
                uint8_t rgb[3];
                fread(rgb, 1, 3, f);

                pixels[i] = Colour(
                    (uint8_t)(rgb[0] * scale),
                    (uint8_t)(rgb[1] * scale),
                    (uint8_t)(rgb[2] * scale)
                );
            }
        }
        else
        {
            // 16-bit big-endian
            for (int i = 0; i < width * height; ++i)
            {
                uint8_t bytes[6];
                fread(bytes, 1, 6, f);

                uint16_t r = (bytes[0] << 8) | bytes[1];
                uint16_t g = (bytes[2] << 8) | bytes[3];
                uint16_t b = (bytes[4] << 8) | bytes[5];

                pixels[i] = Colour(
                    (uint8_t)(r * scale),
                    (uint8_t)(g * scale),
                    (uint8_t)(b * scale)
                );
            }
        }
    }

    fclose(f);
    out_texture = Texture(width, height, std::move(pixels));
    return true;
}

/*
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Pixel;

typedef struct {
    int width;
    int height;
    Pixel *pixels;
} Image;

static int read_token(FILE *f, char *buf, int maxlen)
{
    int c;

    // Skip whitespace and comments
    while (1) {
        c = fgetc(f);
        if (c == '#') {
            while (c != '\n' && c != EOF)
                c = fgetc(f);
        } else if (!isspace(c)) {
            break;
        }
    }

    if (c == EOF)
        return 0;

    int i = 0;

    while (c != EOF && !isspace(c)) {
        if (i < maxlen - 1)
            buf[i++] = (char)c;
        c = fgetc(f);
    }

    buf[i] = '\0';
    return 1;
}

Image read_ppm(const char *filename)
{
    Image img = {0,0,NULL};
    FILE *f = fopen(filename, "rb");
    if (!f)
        return img;

    char tok[64];

    // Magic number
    if (!read_token(f, tok, sizeof(tok)) || tok[0] != 'P' || tok[1] != '6')
        goto fail;

    // Width 
    if (!read_token(f, tok, sizeof(tok)))
        goto fail;
    img.width = atoi(tok);

    // Height 
    if (!read_token(f, tok, sizeof(tok)))
        goto fail;
    img.height = atoi(tok);

    // Max color value 
    if (!read_token(f, tok, sizeof(tok)))
        goto fail;

    int maxval = atoi(tok);
    if (maxval != 255)
        goto fail;

    size_t count = (size_t)img.width * img.height;

    img.pixels = malloc(count * sizeof(Pixel));
    if (!img.pixels)
        goto fail;

    // Read pixel data 
    if (fread(img.pixels, sizeof(Pixel), count, f) != count)
        goto fail;

    fclose(f);
    return img;

fail:
    if (img.pixels)
        free(img.pixels);
    fclose(f);
    img.pixels = NULL;
    img.width = img.height = 0;
    return img;
}

void free_image(Image *img)
{
    free(img->pixels);
    img->pixels = NULL;
}
*/

}