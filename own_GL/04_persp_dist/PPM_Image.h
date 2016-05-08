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
 * Removed:
 *      Fields width_ and height_;
 * Replaced:
 *      Field vals_ is now a nested vector (kind of matrix), width and height
 *      are now computed from vals_;
 *
 *      WARNING: our image now has a matrix form and its x coordinates represent
 *      rows and y coordinates represent columns. The meaning is that the image
 *      is 90 degrees rotated and when reading/writing image from/to a file the
 *      matrix is read/written correctly (kind of transposing the image) so that
 *      we can use x and y coordinates as we used to (x for columns and y for
 *      rows)
 *
 *      The write_ppm_image() function is replaced with method write_to():
 *      Pixel_color() replaced with set_color()
 * Added:
 *      Field bgcolor_ which keeps track of the background color
 *      Corresponding set_bgcolor() method.
 *      bgcolor() and color(int, int) return background color and color value at
 *      certain pixel respectively
 *      Index operator: I[24][79] = 0XFF00FF; // can assign unsigned int value
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
 *      PPM_Image I4 {"some_ppm_file.ppm"}; // image from a .ppm image file
 *          (replacement of old read_ppm_image("file.ppm") function)
 *      I2.write_to("output.ppm"); // save image to a file
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

    friend std::ostream &operator<<(std::ostream&, const PPM_Color&);

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
    PPM_Image(const std::string&);
    PPM_Image(const PPM_Image&);
    PPM_Image &operator=(const PPM_Image&);

    ~PPM_Image() = default;

    std::vector<uint> &operator[](const int);
    const std::vector<uint> &operator[](const int) const;

    int width() const { return vals_.size(); }
    int height() const { return vals_[0].size(); }
    uint bgcolor() const { return bgcolor_; }
    const PPM_Color color(const int, const int) const;

    void set_bgcolor(const PPM_Color&);
    void set_color(const int, const int, const PPM_Color& = PPM_Color{255});
    void write_to(const std::string&);

private:
    uint bgcolor_;
    std::vector<std::vector<uint>> vals_;
    // helper function: skip commment lines in the header of ppm image
    void skip_comment(std::istream&);
    // helper functions: extract channel values from uint values of the color
    uchar red(const uint c) const { return c >> 16 & 0xff; }
    uchar green(const uint c) const { return c >> 8 & 0xff; }
    uchar blue(const uint c) const { return c & 0xff; }

};

#endif

