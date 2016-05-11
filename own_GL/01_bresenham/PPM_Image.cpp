#include "PPM_Image.h"
#include <iostream>
#include <stdexcept>

/*
 * ------------------ PPM_Color implementation ------------------
 */
PPM_Color::PPM_Color(): color_{0} {
}

// set color as a grey value
PPM_Color::PPM_Color(const uchar c): color_(c << 16 | c << 8 | c) { }

// set color using three values: red, green and blue
PPM_Color::PPM_Color(const uchar r, const uchar g, const uchar b):
    color_(r << 16 | g << 8 | b) {
    }

// helper function: check if input is six value hex
bool PPM_Color::is_valid_hex(const std::string &s) {
    int idx {0};
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        idx = 2;
    if (s.length() - idx < 6)
        return false;
    for (size_t i = idx; i < s.length(); ++i)
        if (!isxdigit(s[i]))
            return false;
    return true;
}

// set color using a color hex code as a string
PPM_Color::PPM_Color(const std::string &s): color_{0} {
    if (is_valid_hex(s))
        color_ = stoul(s, nullptr, 16);
    else
        std::cerr << "warning: invalid color code, color set to black\n";
}

// set color from enum class collection of color names
PPM_Color::PPM_Color(const Color_name &c): color_{static_cast<uint>(c)} {
}

PPM_Color::PPM_Color(const PPM_Color &o): color_{o.color()} {
}

PPM_Color &PPM_Color::operator=(const PPM_Color &o) {
    if (this != &o)
        color_ = o.color();
    return *this;
}

/*
 * ------------------ PPM_Image implementation ------------------
 */
PPM_Image::PPM_Image(): width_{0}, height_{0}, vals_{} {
}

PPM_Image::PPM_Image(const int w, const int h, const PPM_Color& c):
    width_{std::max(0, w)}, height_{std::max(0, h)},
    vals_{std::vector<uint>(width_ * height_, c.color())} {
        if (w <= 0 || h <= 0)
            std::cerr << "warning: negative dimensions, empty image created\n";
    }

PPM_Image::PPM_Image(const PPM_Image &o): width_{o.width()},
    height_{o.height()}, vals_{o.values()} {
    }

PPM_Image &PPM_Image::operator=(const PPM_Image &o) {
    if (this != &o) {
        width_ = o.width();
        height_ = o.height();
        vals_ = o.values();
    }
    return *this;
}

void skip_comment(std::istream &is) {
    char c; is >> c;
    while (c == '#') { // skipping comment lines
        is.ignore(256, '\n');
        is >> c;
    }
    is.unget();
}

PPM_Image read_ppm_image(const std::string &fn) {
    std::ifstream ifs {fn, std::ios_base::binary};
    if (!ifs)
        throw std::runtime_error("cannot open file " + fn);
    ifs.exceptions(ifs.exceptions() | std::ios_base::badbit);
    std::string header;
    ifs >> header;
    if (header != "P6")
        throw std::runtime_error("cannot read input file");
    skip_comment(ifs);
    int w, h, t;
    ifs >> w >> h >> t;
    skip_comment(ifs);
    PPM_Image img {w, h};
    const int num_pixels {w * h}, num_values {num_pixels * 3};
    char *v = new char[num_values];
    ifs.read(v, num_values);
    for (int i {0}; i < num_pixels; ++i)
        img.pixel_color(i, PPM_Color(v[i * 3], v[i * 3 + 1], v[i * 3 + 2]));
    delete [] v;
    return img;
}

std::ostream &operator<<(std::ostream &os, const PPM_Image &img) {
    const int w {img.width()}, h {img.height()};
    os << "P6\n" << w << ' ' << h << "\n255\n"; // header
    for (const auto x: img.values())
        os << img.red(x) << img.green(x) << img.blue(x);
    return os;
}

void write_ppm_image(const PPM_Image &img, const std::string &fn) {
    std::ofstream ofs {fn, std::ios_base::binary};
    ofs.exceptions(ofs.exceptions() | std::ios_base::badbit);
    ofs << img;
    std::cout << "The result is saved to file: " << fn << '\n';
}

void PPM_Image::pixel_color(int x, int y, const PPM_Color &c) {
    if (x >=0 && x <= width_ && y >= 0 && y <= height_)
        vals_[x + y * width_] = c.color();
    else
        std::cerr << "error: coordinates are beyond the image\n";
}

void PPM_Image::pixel_color(int idx, const PPM_Color &c) {
    if (idx >=0 && idx <= width_ * height_)
        vals_[idx] = c.color();
    else
        std::cerr << "error: coordinates are beyond the image\n";
}

