#ifndef PPM_LIB_H
#define PPM_LIB_H

#include <fstream>
#include <vector>

namespace PPM_lib { // general namespace for this small library

// Possible future implementation using Format class
enum class Format {GS, RGB};

// define a collection of color names
enum class Color_name {
    black = 0, red = 0xFF0000, green = 0x00FF00, blue = 0x0000FF,
    white = 0xFFFFFF, cyan = 0x00FFFF, magenta = 0xFF00FF, yellow = 0xFFFF00,
    orange = 0xFFA500, teal = 0x008080, brown = 0xA52A2A, khaki = 0xF0E68C
};

// weight values for converting rgb color to gray color
static constexpr double red_coeff {0.2126};
static constexpr double green_coeff {0.7152};
static constexpr double blue_coeff {0.0722};

constexpr unsigned char rgb2gray(const unsigned int clr) {
    return (clr >> 16 & 0xFF) * red_coeff + (clr >> 8 & 0xFF) * green_coeff +
        (clr & 0xFF) * blue_coeff;
}

constexpr unsigned int gray2rgb(const unsigned char r, const unsigned char g,
        const unsigned char b) {
    return r << 16 | g << 8 | b;
}

/*
 * Definition of class RGB_Color
 */
class RGB_Color {
public:
    using value_type = unsigned int;
    using gray_type = unsigned char;

    constexpr RGB_Color(const value_type = 0);
    constexpr RGB_Color(const gray_type, const gray_type, const gray_type);
    constexpr RGB_Color(const Color_name&);
    constexpr RGB_Color(const RGB_Color&);
    RGB_Color& operator=(const RGB_Color&);

    ~RGB_Color() = default;

    constexpr value_type color() const { return color_; }
    constexpr gray_type red() const { return color_ >> 16 & 0xFF; }
    constexpr gray_type green() const { return color_ >> 8 & 0xFF; }
    constexpr gray_type blue() const { return color_ & 0xFF; }

private:
    value_type color_;
};

/*
 * Implementation of class RGB_Color
 */
// constexpr constructor: init color from a value
constexpr RGB_Color::RGB_Color(const value_type clr): color_{clr} { }

constexpr RGB_Color::RGB_Color(const gray_type r, const gray_type g,
        const gray_type b): color_(gray2rgb(r, g, b)) {
}

// constexpr constructor: init color from the collection of color names
constexpr RGB_Color::RGB_Color(const Color_name &o): color_{value_type(o)} { }

// constexpr copy constructor
constexpr RGB_Color::RGB_Color(const RGB_Color &o): color_{o.color_} { }

// copy assignment
inline RGB_Color& RGB_Color::operator=(const RGB_Color &o) {
    color_ = o.color_;
    return *this;
}

/*
 * Definition of class GS_Color
 */
class GS_Color {
public:
    // simple grayscale images use values from 0 to 255: unsigned char range
    using value_type = unsigned char;

    constexpr GS_Color(const value_type = 0); // gray value
    constexpr GS_Color(const RGB_Color&); // rgb
    constexpr GS_Color(const GS_Color&);
    GS_Color& operator=(const GS_Color&);

    ~GS_Color() = default;

    constexpr value_type color() const { return color_; }

private:
    value_type color_;
};

/*
 * Implementation of class GS_Color
 */
// constexpr constructor: init color from a value
constexpr GS_Color::GS_Color(const value_type clr): color_{clr} { }

// constructor: init gray color from rgb color
constexpr GS_Color::GS_Color(const RGB_Color &o): color_{rgb2gray(o.color())} {
}

// constexpr copy constructor
constexpr GS_Color::GS_Color(const GS_Color &o): color_{o.color_} { }

// copy assignment
inline GS_Color& GS_Color::operator=(const GS_Color &o) {
    color_ = o.color_;
    return *this;
}

class GS_Image; // forward declaration

/*
 * Definition of class RGB_Image
 */
class RGB_Image {
public:
    using value_type = RGB_Color::value_type;
    using size_type = std::size_t;
    using vec = std::vector<value_type>;
    using mat = std::vector<vec>;
    using iterator = typename mat::iterator;
    using const_iterator = typename mat::const_iterator;

    RGB_Image(const size_type, const size_type, const RGB_Color& = {});
    RGB_Image(const std::string&);
    RGB_Image(const GS_Image&, const GS_Image&, const GS_Image&);
    RGB_Image(const RGB_Image&);
    RGB_Image& operator=(const RGB_Image&);

    ~RGB_Image() = default;

    iterator begin() { return I_.begin(); }
    const_iterator begin() const { return I_.begin(); }
    iterator end() { return I_.end(); }
    const_iterator end() const { return I_.end(); }

    vec& operator[](const int i) { return I_[i]; }
    const vec& operator[](const int i) const { return I_[i]; }

    size_type width() const { return I_.size(); }
    size_type height() const { return I_[0].size(); }
    value_type bgcolor() const { return bgcolor_; }
    const RGB_Color color(const int x, const int y) const { return {I_[x][y]}; }

    const GS_Image red() const;
    const GS_Image green() const;
    const GS_Image blue() const;

    void set_bgcolor(const RGB_Color&);
    void set_color(const int, const int, const RGB_Color& = 0xFFFFFF);

    void write_to(const std::string&);

private:
    value_type bgcolor_;
    mat I_;
};

/*
 * Definition of class GS_Image
 */
class GS_Image {
public:
    using value_type = GS_Color::value_type;
    using size_type = std::size_t;
    using vec = std::vector<value_type>;
    using mat = std::vector<vec>;
    using iterator = typename mat::iterator;
    using const_iterator = typename mat::const_iterator;

    GS_Image(const size_type, const size_type, const GS_Color& = {});
    GS_Image(const std::string&);
    GS_Image(const RGB_Image&);
    GS_Image(const GS_Image&);
    GS_Image &operator=(const GS_Image&);

    ~GS_Image() = default;

    iterator begin() { return I_.begin(); }
    const_iterator begin() const { return I_.begin(); }
    iterator end() { return I_.end(); }
    const_iterator end() const { return I_.end(); }

    vec& operator[](const int i) { return I_[i]; }
    const vec& operator[](const int i) const { return I_[i]; }

    size_type width() const { return I_.size(); }
    size_type height() const { return I_[0].size(); }
    value_type bgcolor() const { return bgcolor_; }
    const GS_Color color(const int x, const int y) const { return {I_[x][y]}; }

    void set_bgcolor(const GS_Color&);
    void set_color(const int, const int, const GS_Color& = 255);

    void write_to(const std::string&);

private:
    value_type bgcolor_;
    mat I_;
};

} // end namespace PPM_lib

#endif

