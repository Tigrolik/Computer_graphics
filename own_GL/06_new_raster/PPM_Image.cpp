#include "PPM_Image.h"
#include <iostream>
#include <stdexcept>
#include <memory>

/*
 * --------------------- PPM_Color implementation ---------------------
 */
PPM_Color::PPM_Color(): color_{0} {
}

// set color as a grey value
PPM_Color::PPM_Color(const uchar c): color_(c << 16 | c << 8 | c) { }

// set color using three values: red, green and blue
PPM_Color::PPM_Color(const uchar r, const uchar g, const uchar b):
    color_(r << 16 | g << 8 | b) {
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

std::ostream &operator<<(std::ostream &os, const PPM_Color &c) {
    return os << "red: " << +c.red() << ", green: " << +c.green() <<
        ", blue: " << +c.blue();
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

/*
 * --------------------- PPM_Image implementation ---------------------
 */
PPM_Image::PPM_Image(): bgcolor_{0}, vals_{} {
}

PPM_Image::PPM_Image(const int w, const int h, const PPM_Color& c):
    bgcolor_{c.color()},
    vals_{std::vector<std::vector<uint>>(w, std::vector<uint>(h, bgcolor_))} {
        if (w <= 0 || h <= 0)
            std::cerr << "warning: negative dimensions, empty image created\n";
    }

PPM_Image::PPM_Image(const PPM_Image &o): bgcolor_{o.bgcolor_}, vals_{o.vals_} {
}

// reading PPM_Image from a file
PPM_Image::PPM_Image(const std::string &fn): bgcolor_{0}, vals_{} {
    std::ifstream ifs {fn, std::ios_base::binary};
    if (!ifs)
        throw std::runtime_error("cannot open file " + fn);
    ifs.exceptions(ifs.exceptions() | std::ios_base::badbit);
    std::string header;
    ifs >> header;
    if (header != "P6")
        throw std::runtime_error("cannot read input file");
    skip_comment(ifs);
    int w, h, temp;
    ifs >> w >> h >> temp;
    skip_comment(ifs);
    const int num_values {w * h * 3};
    vals_ = std::vector<std::vector<uint>>(w, std::vector<uint>(h));
    char *v = new char[num_values];
    //std::unique_ptr<char> v {new char[num_values]};
    ifs.read(v, num_values);
    if (!ifs) delete [] v;
    for (int j {0}; j < h; ++j)
        for (int i {0}; i < w; ++i) {
            const int idx {(j * h + i) * 3};
            vals_[i][j] = static_cast<uchar>(v[idx]) << 16 |
                static_cast<uchar>(v[idx + 1]) << 8 |
                static_cast<uchar>(v[idx + 2]);
        }
    delete [] v;
}

PPM_Image &PPM_Image::operator=(const PPM_Image &o) {
    if (this != &o) {
        bgcolor_ = o.bgcolor_;
        vals_ = o.vals_;
    }
    return *this;
}

std::vector<uint> &PPM_Image::operator[](const int i) {
    return vals_[i];
}

const std::vector<uint> &PPM_Image::operator[](const int i) const {
    return vals_[i];
}

const PPM_Color PPM_Image::color(const int x, const int y) const {
    const uint c {vals_[x][y]};
    return PPM_Color {red(c), green(c), blue(c)};
}

void PPM_Image::set_bgcolor(const PPM_Color &c) {
    uint old_bgcolor_ {bgcolor_};
    bgcolor_ = c.color();
    for (auto &x: vals_)
        for (auto &y: x)
            if (y == old_bgcolor_)
                y = bgcolor_;
}

void PPM_Image::set_color(const int x, const int y, const PPM_Color &c) {
    if (x >= 0 && x <= width() && y >= 0 && y <= height())
        vals_[x][y] = c.color();
    else
        std::cerr << "error: coordinates are beyond the image\n";
}

void PPM_Image::write_to(const std::string &fn) {
    std::ofstream ofs {fn, std::ios_base::binary};
    ofs.exceptions(ofs.exceptions() | std::ios_base::badbit);
    ofs << "P6\n" << width() << ' ' << height() << "\n255\n"; // header
    const int w {width()}, h {height()};
    for (int y {0}; y < h; ++y)
        for (int x {0}; x < w; ++x) {
            const uint c = vals_[x][y];
            ofs << red(c) << green(c) << blue(c);
        }
    std::cout << "The result is saved to file: " << fn << '\n';
}

// helper function: skip commment lines in the header of ppm image
void PPM_Image::skip_comment(std::istream &is) {
    char c; is >> c;
    while (c == '#') { // skipping comment lines
        is.ignore(256, '\n');
        is >> c;
    }
    is.unget();
}

