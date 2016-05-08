/*
 * Very simple class for writing RGB PPM images
 * Uses 3 bytes (24 bits) per pixel: standard for PPM RGB images
 *
 * Class PPM_Color is used to create colors; stores color value as unsigned int
 * Examples:
 *      PPM_Color c1 {};    // black value = default
 *      PPM_Color c2 {128}; // gray level
 *      PPM_Color c3 {255, 75, 130}; // reg, green and blue values
 *      PPM_Color c4 {"AA50D4"}; // hex color code in string format
 *      PPM_Color c5 {Color_name::orange}; //use color name
 *
 * Class PPM_Image creates/reads/writes PPM RGB images
 * Examples:
 *      PPM_Image I1 {}; // empty image, not really useful :)
 *      PPM_Image I2 {600, 400}; // image with width 600 and height 400 with
 *          black background
 *      PPM_Image I3 {w, h, PPM_Color &c}; // image with width w and height h
 *          with background color specified (PPM_Color parameter might be
 *          given as one of PPM_Color constructors parameters and it should be
 *          implicitly converted: PPM_Image I3 {w, h, {"FF00FF"}}). Arguably,
 *          implicit conversion might be confusing, however, for now it seems to
 *          be convenient
 */

#ifndef _PPM_IMAGE_H_
#define _PPM_IMAGE_H_

#include <fstream>
#include <vector>

using uchar = unsigned char;
using uint = unsigned int;

// define a collection of color names
enum class Color_name {
    black = 0, red = 0xFF0000, green = 0x00FF00, blue = 0x0000FF,
    white = 0xFFFFFF, cyan = 0x00FFFF, magenta = 0xFF00FF, yellow = 0xFFFF00,
    orange = 0xFFA500, teal = 0x008080, brown = 0xA52A2A, khaki = 0xF0E68C
};

class PPM_Color {
public:
    PPM_Color(); // default
    PPM_Color(const uchar); // grey value
    PPM_Color(const uchar, const uchar, const uchar); // rgb values
    PPM_Color(const std::string&); // using string as color code in hex format
    PPM_Color(const Color_name&); // using predefined color name from enum class
    PPM_Color(const PPM_Color&);
    PPM_Color &operator=(const PPM_Color&);

    ~PPM_Color() = default;

    uint color() const { return color_; }
    uchar red() const { return color_ >> 16 & 0xff; }
    uchar green() const { return color_ >> 8 & 0xff; }
    uchar blue() const { return color_ & 0xff; }

private:
    uint color_;
    bool is_valid_hex(const std::string&);
};

class PPM_Image {
public:
    PPM_Image();
    // initialize an image with background color (default: black)
    PPM_Image(const int, const int, const PPM_Color& = PPM_Color{0});
    PPM_Image(const PPM_Image&);
    PPM_Image &operator=(const PPM_Image&);

    ~PPM_Image() = default;

    friend std::ostream &operator<<(std::ostream&, const PPM_Image&);

    int width() const { return width_; }
    int height() const { return height_; }
    std::vector<uint> values() const { return vals_; }

    PPM_Color get_color(const int, const int);
    // set color of individual pixel
    void pixel_color(int, int, const PPM_Color& = PPM_Color{255});
    void pixel_color(int, const PPM_Color& = PPM_Color{255});
    //void pixel_color(int, int, const Color_name&);

private:
    int width_;
    int height_;
    std::vector<uint> vals_;
    // helper functions: extract channel value from the color
    uchar red(const uint c) const { return c >> 16 & 0xff; }
    uchar green(const uint c) const { return c >> 8 & 0xff; }
    uchar blue(const uint c) const { return c & 0xff; }

};

// read PPM_Image from a file
PPM_Image read_ppm_image(const std::string&);
// save PPM_Image to a file
void write_ppm_image(const PPM_Image&, const std::string&);

#endif

