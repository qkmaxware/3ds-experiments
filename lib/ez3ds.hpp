#include <stdint.h>
#include <string>
#include <array>
#include <vector>

#ifndef QKMAXWARE_EZ3DS
#define QKMAXWARE_EZ3DS

// Enum for key codes (not including axis names)
/*
./services/hid.h:       KEY_A       = BIT(0),       ///< A
./services/hid.h:       KEY_B       = BIT(1),       ///< B
./services/hid.h:       KEY_SELECT  = BIT(2),       ///< Select
./services/hid.h:       KEY_START   = BIT(3),       ///< Start
./services/hid.h:       KEY_DRIGHT  = BIT(4),       ///< D-Pad Right
./services/hid.h:       KEY_DLEFT   = BIT(5),       ///< D-Pad Left
./services/hid.h:       KEY_DUP     = BIT(6),       ///< D-Pad Up
./services/hid.h:       KEY_DDOWN   = BIT(7),       ///< D-Pad Down
./services/hid.h:       KEY_R       = BIT(8),       ///< R
./services/hid.h:       KEY_L       = BIT(9),       ///< L
./services/hid.h:       KEY_X       = BIT(10),      ///< X
./services/hid.h:       KEY_Y       = BIT(11),      ///< Y
./services/hid.h:       KEY_ZL      = BIT(14),      ///< ZL (New 3DS only)
./services/hid.h:       KEY_ZR      = BIT(15),      ///< ZR (New 3DS only)
./services/hid.h:       KEY_TOUCH   = BIT(20),      ///< Touch (Not actually provided by HID)
./services/hid.h:       KEY_CSTICK_RIGHT = BIT(24), ///< C-Stick Right (New 3DS only)
./services/hid.h:       KEY_CSTICK_LEFT  = BIT(25), ///< C-Stick Left (New 3DS only)
./services/hid.h:       KEY_CSTICK_UP    = BIT(26), ///< C-Stick Up (New 3DS only)
./services/hid.h:       KEY_CSTICK_DOWN  = BIT(27), ///< C-Stick Down (New 3DS only)
./services/hid.h:       KEY_CPAD_RIGHT = BIT(28),   ///< Circle Pad Right
./services/hid.h:       KEY_CPAD_LEFT  = BIT(29),   ///< Circle Pad Left
./services/hid.h:       KEY_CPAD_UP    = BIT(30),   ///< Circle Pad Up
./services/hid.h:       KEY_CPAD_DOWN  = BIT(31),   ///< Circle Pad Down
./services/hid.h:       KEY_UP    = KEY_DUP    | KEY_CPAD_UP,    ///< D-Pad Up or Circle Pad Up
./services/hid.h:       KEY_DOWN  = KEY_DDOWN  | KEY_CPAD_DOWN,  ///< D-Pad Down or Circle Pad Down
./services/hid.h:       KEY_LEFT  = KEY_DLEFT  | KEY_CPAD_LEFT,  ///< D-Pad Left or Circle Pad Left
./services/hid.h:       KEY_RIGHT = KEY_DRIGHT | KEY_CPAD_RIGHT, ///< D-Pad Right or Circle Pad Right
./services/ps.h:        PS_KEYSLOT_0D,      ///< Key slot 0x0D.
./services/ps.h:        PS_KEYSLOT_2D,      ///< Key slot 0x2D.
./services/ps.h:        PS_KEYSLOT_38,      ///< Key slot 0x38.
./services/ps.h:        PS_KEYSLOT_39_DLP,  ///< Key slot 0x39. (DLP)
./services/ps.h:        PS_KEYSLOT_2E,      ///< Key slot 0x2E.
./services/ps.h:        PS_KEYSLOT_INVALID, ///< Invalid key slot.
./services/ps.h:        PS_KEYSLOT_36,      ///< Key slot 0x36.
./services/ps.h:        PS_KEYSLOT_39_NFC   ///< Key slot 0x39. (NFC)
*/
typedef enum {
    A = 1U << 0,
    B = 1U << 1,
    Select = 1U << 2,
    Start = 1U << 3,
    DPadRight = 1U << 4,
    DPadLeft = 1U << 5,
    DPadUp = 1U << 6,
    DPadDown = 1U << 7,
    R = 1U << 8,
    L = 1U << 9,
    X = 1U << 10,
    Y = 1U << 11,
    ZL = 1U << 14,
    ZR = 1U << 15,
} KeyCodes;

typedef enum {
    DPad,
    CirclePad,
    Nub
} JoystickNames;

struct Joystick {
public:
    float X;
    float Y;

    Joystick(): X(0), Y(0) 
    {}
};

struct Touchpad {
private:
    bool wasTouched;
    bool isTouched;
public:
    uint16_t X;
    uint16_t Y;

    Touchpad(): wasTouched(false), isTouched(false), X(0), Y(0)
    {}

    inline bool IsPressed() {
        return this->isTouched;
    }

    inline bool IsJustPressed() {
        return this->isTouched && !this->wasTouched;
    }

    inline bool IsJustReleased() {
        return !this->isTouched && this->wasTouched;
    }

    inline void SetTouch(uint16_t x, uint16_t y, bool touched) {
        this->X = x;
        this->Y = y;
        this->wasTouched = this->isTouched;
        this->isTouched = touched;
    }
};

/// @brief A struct for organized input access
struct Input {
private:
    // Keys
    uint32_t down;
    uint32_t held;
    uint32_t released;
    
    Joystick dpad;
    Joystick circle;
    Joystick nub;

    Touchpad pad;
public:
    Input(): down(0), held(0), released(0), dpad(), circle(), nub(), pad()
    {}

    void UpdateKeys(uint32_t down, uint32_t held, uint32_t released);

    void UpdatePads(float circleX, float circleY, float nubX, float nubY);

    void UpdateTouch(uint16_t x, uint16_t y, bool touched);

    bool JustPressed(KeyCodes code) const;

    bool Pressed(KeyCodes code) const;

    bool Released(KeyCodes code) const;

    Joystick GetAxes(JoystickNames stick) const;
    Touchpad GetTouchpad() const;

    std::string Prompt(const std::string &question) const;
    int PromptInt(const std::string &question) const;
    float PromptFloat(const std::string &question) const;
};

/// @brief Console debugging utilities
namespace Console {
    void Clear();
    void Print(const char *format, ...);
    void Println(const char *format, ...);
};

/// @brief 24 bit colour plus transparency
struct Colour {
public:
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t A;

    constexpr Colour() noexcept: R(0), G(0), B(0), A(0)
    {}

    constexpr Colour(uint8_t r, uint8_t g, uint8_t b) noexcept: R(r), G(g), B(b), A(255)
    {}

    constexpr Colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept: R(r), G(g), B(b), A(a)
    {}

    /// @brief Create a colour with r,g,b components. IE for red -> Colour::FromRgb(255, 0, 0);
    static constexpr Colour FromRgb(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) { return { r, g, b, a }; }
    /// @brief Create a colour with r,g,b from a hex integer components. IE for red ->  Colour::FromHex(0xFF0000);
    static constexpr Colour FromHex(uint32_t hex, uint8_t a = 255) { return { uint8_t(hex >> 16), uint8_t(hex >> 8), uint8_t(hex >> 0), a }; }
    /// @brief Create a colour from HSV values. H in degrees [0,360), S and V in [0,1].
    static Colour FromHSV(float h, float s, float v, uint8_t a = 255);

    static constexpr Colour Black() { return Colour(0, 0, 0, 255); }
    static constexpr Colour White() { return Colour(255, 255, 255, 255); }
    static constexpr Colour Red() { return Colour(255, 0, 0, 255); }
    static constexpr Colour Green() { return Colour(0, 255, 0, 255); }
    static constexpr Colour Blue() { return Colour(0, 0, 255, 255); }
    static constexpr Colour Yellow() { return Colour(255, 255, 0, 255); }
    static constexpr Colour Cyan() { return Colour(0, 255, 255, 255); }
    static constexpr Colour Magenta() { return Colour(255, 0, 255, 255); }
    static constexpr Colour Grey() { return Colour(128, 128, 128, 255); }
    static constexpr Colour DarkGrey() { return Colour(64, 64, 64, 255); }
    static constexpr Colour Transparent() { return Colour(0, 0, 0, 0); }

    inline void ToHsv(float &h, float &s, float &v) const;

    constexpr uint32_t PackABRG() const {
        return (uint32_t(A) << 24) | (uint32_t(B) << 16) | (uint32_t(R) << 8) | (uint32_t(G));
    }
};

struct Texture {
private:
    std::vector<Colour> pixels;

public:
    int Width;
    int Height;

    Texture(const int width, const int height) noexcept: pixels(width * height), Width(width), Height(height) {}

    Texture(const int width, const int height, std::vector<Colour> &&pixels): pixels(std::move(pixels)), Width(width), Height(height) 
    { }

    Colour GetPixel(const int x, const int y) const noexcept {
        int index = y * Width + x;
        return index >= 0 && (unsigned int)index < pixels.size() ? pixels[index] : Colour::White();
    }

    Colour GetPixel(const int index) const noexcept {
        return index >= 0 && (unsigned int)index < pixels.size() ? pixels[index] : Colour::White();
    }

    void SetPixel(const int x, const int y, const Colour &colour) noexcept {
        int index = y * Width + x;
        if (index >= 0 && (unsigned int)index < pixels.size()) {
            pixels[index] = colour;
        }
    }

    void SetPixel(const int index, const Colour &colour) noexcept {
        if (index >= 0 && (unsigned int)index < pixels.size()) {
            pixels[index] = colour;
        }
    }

};

namespace Typeface {
struct Glyph {
public:
    uint8_t Rows[12];
};
}

struct Screen {
private:
    const int frameBufferFormat;
    const int bytesPerPixel;
    const int screenId;
    const int side;

    uint8_t *pixels = nullptr;

    inline bool inbounds(int x, int y) const noexcept {
        return (x >= 0 && x < this->Width
             && y >= 0 && y < this->Height);
    }

public:
    const int Width;
    const int Height;

    Screen(int screenId, int side, int frameBufferFormat, int width, int height);

    void FetchFramebuffer();

    /// @brief Fill the screen with black
    void Clear();
    /// @brief Fill the screen with the given colour
    void Clear(Colour colour);
    void SetPixel(int x, int y, Colour colour); 
    void DrawLine(int x1, int y1, int x2, int y2, Colour stroke);
    void DrawRect(int x, int y, int width, int height, Colour stroke);
    void FillRect(int x, int y, int width, int height, Colour stroke, Colour fill);
    void DrawCircle(int cx, int cy, int radius, Colour stroke);
    void FillCircle(int cx, int cy, int radius, Colour stroke, Colour fill);
    /// @brief Draw a glyph at the given coordinates
    /// @param glyph glyph to draw
    /// @param x top left corner to start drawing at 
    /// @param y top left corner to start drawing at
    /// @param scale glyph scale >= 1
    /// @param foreground colour to use for foreground
    /// @param background colour to use for background
    void StampGlyph(const Glyph& glyph, int x, int y, int scale, Colour foreground, Colour background);
    /// @brief Stamp a texture onto the screen at the given x,y coordinates
    /// @param x top left corner to start drawing at
    /// @param y top left corner to start drawing at
    /// @param texture texture to draw
    void StampTexture(int x, int y, const Texture &texture);
};

struct Displays {
public:
    Screen Upper;
    Screen Lower;

    Displays();

    void FetchFramebuffers() {
        Upper.FetchFramebuffer();
        Lower.FetchFramebuffer();
    }
};

// example usage
/* 
    Imgui imgui(Screen.Lower);

    imgui.Label("Hello World");
    if (imgui.Button("Press To Exit")) {
        // Do something
    }
*/

namespace Typeface {

const int Width = 8;
const int Kerning = 1;
const int Height = 12;
const int LineHeight = 16;
const int VPad = 2;

const int CharacterCount = 128;
using Font = std::array<Glyph, CharacterCount>;
extern const Font DefaultFont;

namespace SpecialGlyphs {
/// @brief Special glyph representing a directory/folder
extern const Glyph Directory;
extern const Glyph ChevronUp;
extern const Glyph ChevronDown;
extern const Glyph LeftHalfBlock;
extern const Glyph RightHalfBlock;
}
}

/// @brief Simple immediate mode gui rendering class built on-top of the framebuffer
struct Imgui {
private:
    Screen screen;
    Input *input;

    enum FlowDirection {
        Column, Row
    };
    
    FlowDirection dir;
    int x;
    int y;

public:
    int WidthPixels();
    int WidthCharacters();

    int HeightPixels();
    int HeightLines();

    Imgui(Screen &screen): screen(screen), input(NULL), dir(Column), x(0), y(0)
    { }
    Imgui(Screen &screen, Input *input): screen(screen), input(input), dir(Column), x(0), y(0)
    { }

    // Layout items
    void BeginRow();
    void EndRow();
    void NextLine();

    // Display items
    struct LabelStyle {
        Colour FontColour;
        constexpr LabelStyle(const Colour &fontColour): FontColour(fontColour)
        { }
    };
    static const LabelStyle DefaultLabelStyle;
    void Label(const std::string &text, const LabelStyle &style);
    void Glyph(const Typeface::Glyph &glyph, const LabelStyle &style);

    void IndentCharacters(int characters);
    void IndentPixels(int px);
    
    struct ButtonStyle {
        Colour NormalFontColour;
        Colour NormalBorderColour;
        Colour NormalBackgroundColour;

        Colour HoverFontColour;
        Colour HoverBorderColour;
        Colour HoverBackgroundColour;

        constexpr ButtonStyle(const Colour &normalFont, const Colour &normalBorder, const Colour &normalBg, const Colour &hoverFont, const Colour &hoverBorder, const Colour &hoverBg): 
            NormalFontColour(normalFont), NormalBorderColour(normalBorder), NormalBackgroundColour(normalBg),
            HoverFontColour(hoverFont), HoverBorderColour(hoverBorder), HoverBackgroundColour(hoverBg)
        { }
    };
    static const ButtonStyle DefaultButtonStyle;
    bool Button(const std::string &text, const ButtonStyle &style);
    // TODO Draw a teture at the given position (have to determine what a "texture" looks like in this framework)
    //void Texture();

    class FileBrowser {
        private:
            std::string root_dir;
            std::string current_dir;
            std::vector<std::string>::size_type selected_index;
            std::vector<std::string> files;    
            bool listing_error;

            int page_count;
            int page_index;
            int page_size;

        public:
            FileBrowser();
            bool HasErrorOccured();
            std::string CurrentDirectory();
            int FileCount();
            const std::string& GetDir();
            void SetDir(const std::string &current_dir);
            bool SelectFile(Screen &display, Input &input);
            std::string Highlighted();
            void PageInfo(int &count, int &index, int &size);
    };
};

namespace RomFs {

/// @brief Read all text from the file at the given path
std::string ReadAllText(const std::string &path);
/// @brief Read all the bytes from the file at the given path
std::vector<uint8_t> ReadAllBytes(const std::string &path);
/// @brief Load a PPM texture from the file at the given path
bool TextureLoadPPM(const std::string &path, Texture &texture);

}

// -----------------------------------------------------------------------------------------
// Final Component!!
// -----------------------------------------------------------------------------------------
 
class CitrusApp {

public:
    void Run();
    
    virtual void setup() = 0;
    virtual void loop(Displays &displays, Input &input) = 0;
    virtual void cleanup() = 0;

};

/// @brief A CitrusApp that uses sub-apps to represent diffent states where the app can transition from one sub-app to another.
class MultiStateCitrusApp: public CitrusApp {
public:
    using StateId = size_t;

private:
    bool hasEnteredCurrentState;
    StateId currentStateId;

public:

    MultiStateCitrusApp(StateId defaultState);

    void loop(Displays &displays, Input &input) override;
    virtual CitrusApp& GetState(StateId id) = 0;
    virtual StateId NextState(StateId currentId, CitrusApp &currentState) = 0;
};

#endif