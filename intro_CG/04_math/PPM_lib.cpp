#include "PPM_lib.h"
#include <stdexcept>
#include <iostream>

using namespace PPM_lib;

// helper function: skip commment lines in the header of the ppm image
void skip_comment(std::istream &is) {
    char c;
    is >> c;
    while (c == '#') { // skipping comment lines
        is.ignore(256, '\n');
        is >> c;
    }
    is.unget();
}

/*
 *Implementation of RGB_Image class
 */
RGB_Image::RGB_Image(const size_type w, const size_type h, const RGB_Color& rc):
    bgcolor_{rc.color()}, I_{mat(w, vec(h, bgcolor_))} {
    }

// reading an RGB image from a file
RGB_Image::RGB_Image(const std::string &fn): bgcolor_{0}, I_{} {
    using namespace std;
    ifstream ifs {fn, ios_base::binary};
    if (!ifs)
        throw runtime_error("cannot open file " + fn);
    ifs.exceptions(ifs.exceptions() | ios_base::badbit);
    string header;
    ifs >> header;
    if (header != "P6")
        throw runtime_error("cannot read input file");
    skip_comment(ifs);
    int w, h, temp;
    ifs >> w >> h >> temp;
    skip_comment(ifs);
    const int num_values {w * h * 3};
    I_ = mat(w, vec(h));
    char *v = new char[num_values];
    //std::unique_ptr<char> v {new char[num_values]}; // maybe later...
    ifs.read(v, num_values);
    if (!ifs) delete [] v;
    using gray_type = RGB_Color::gray_type;
    for (int y {0}; y < h; ++y)
        for (int x {0}; x < w; ++x) {
            const int idx {(y * h + x) * 3};
            I_[x][y] = gray_type(v[idx]) << 16 | gray_type(v[idx + 1]) << 8 |
                gray_type(v[idx + 2]);
        }
    delete [] v;
}

RGB_Image::RGB_Image(const GS_Image &R, const GS_Image &G, const GS_Image &B):
    RGB_Image(R.width(), R.height(),
            gray2rgb(R.bgcolor(), G.bgcolor(), B.bgcolor())) {
        const auto w = width(), h = height();
        // check images for equal size
        if (w != G.width() || w != B.width() ||
                h != G.height() || h != B.height())
            throw std::runtime_error("channel images have different sizes");
        // combine R, G and B channels into single image
        for (size_type y {0}; y < h; ++y)
            for (size_type x {0}; x < w; ++x)
                I_[x][y] = gray2rgb(R[x][y], G[x][y], B[x][y]);
    }

RGB_Image::RGB_Image(const RGB_Image &o): bgcolor_{o.bgcolor_}, I_{o.I_} { }

RGB_Image& RGB_Image::operator=(const RGB_Image &o) {
    if (this != &o) {
        bgcolor_ = o.bgcolor_;
        I_ = o.I_;
    }
    return *this;
}

// change the background color
void RGB_Image::set_bgcolor(const RGB_Color &rc) {
    auto old_bgcolor_ = bgcolor_;
    bgcolor_ = rc.color();
    for (auto &x: I_)
        for (auto &y: x)
            if (y == old_bgcolor_)
                y = bgcolor_;
}

// set color to the individual pixel
void RGB_Image::set_color(const int x, const int y, const RGB_Color &rc) {
    if (x >= 0 && x < int(width()) && y >= 0 && y < int(height()))
        I_[x][y] = rc.color();
}

void RGB_Image::write_to(const std::string &fn) {
    std::ofstream ofs {fn, std::ios_base::binary};
    ofs.exceptions(ofs.exceptions() | std::ios_base::badbit);
    const auto w = width(), h = height();
    ofs << "P6\n" << w << ' ' << h << "\n255\n"; // header
    for (size_type y {0}; y < h; ++y)
        for (size_type x {0}; x < w; ++x) {
            const RGB_Color rc = I_[x][y];
            ofs << rc.red() << rc.green() << rc.blue();
        }
    std::cout << "The result is saved to file: " << fn << '\n';
}

/*
 *Implementation of GS_Image class
 */
// construct an image with width, height and background color
GS_Image::GS_Image(const size_type w, const size_type h, const GS_Color &gc):
    bgcolor_{gc.color()}, I_{mat(w, vec(h, bgcolor_))} {
    }

// read a grayscale image from a file
GS_Image::GS_Image(const std::string &fn): bgcolor_{}, I_{} {
    using namespace std;
    ifstream ifs {fn, ios_base::binary};
    if (!ifs)
        throw runtime_error("cannot open file " + fn);
    ifs.exceptions(ifs.exceptions() | ios_base::badbit);
    string header;
    ifs >> header;
    // if the image is RGB then convert it to gray
    if (header == "P6") {
        *this = GS_Image{RGB_Image{fn}};
        return;
    }
    // throw if the format is wrong
    if (header != "P5")
        throw runtime_error("wrong input file format");
    skip_comment(ifs);
    int w, h, temp;
    ifs >> w >> h >> temp;
    skip_comment(ifs);
    const int num_values {w * h};
    I_ = mat(w, vec(h));
    char *v = new char[num_values];
    //std::unique_ptr<char> v {new char[num_values]}; // maybe later...
    ifs.read(v, num_values); // read data from the file
    if (!ifs)
        delete [] v;
    for (int y {0}; y < h; ++y)
        for (int x {0}; x < w; ++x)
            I_[x][y] = v[y * h + x];
    delete [] v;
}

// convert RGB image to a grayscale image
GS_Image::GS_Image(const RGB_Image& I):
    GS_Image(I.width(), I.height(), rgb2gray(I.bgcolor())) {
        const auto w = width();
        for (size_type y {0}; y < height(); ++y)
            for (size_type x {0}; x < w; ++x)
                I_[x][y] = rgb2gray(I[x][y]);
    }

GS_Image::GS_Image(const GS_Image &o): bgcolor_{o.bgcolor_}, I_{o.I_} { }

GS_Image& GS_Image::operator=(const GS_Image &o) {
    if (this != &o) {
        bgcolor_ = o.bgcolor_;
        I_ = o.I_;
    }
    return *this;
}

const GS_Image RGB_Image::red() const {
    const auto w = width(), h = height();
    GS_Image R {w, h};
    for (size_t x = 0; x < w; ++x)
        for (size_t y = 0; y < h; ++y)
            R[x][y] = color(x, y).red();
    return R;
}

const GS_Image RGB_Image::green() const {
    const auto w = width(), h = height();
    GS_Image R {w, h};
    for (size_t x = 0; x < w; ++x)
        for (size_t y = 0; y < h; ++y)
            R[x][y] = color(x, y).green();
    return R;
}

const GS_Image RGB_Image::blue() const {
    const auto w = width(), h = height();
    GS_Image R {w, h};
    for (size_t x = 0; x < w; ++x)
        for (size_t y = 0; y < h; ++y)
            R[x][y] = color(x, y).blue();
    return R;
}

// change the background color
void GS_Image::set_bgcolor(const GS_Color &gc) {
    auto old_bgcolor_ = bgcolor_;
    bgcolor_ = gc.color();
    for (auto &x: I_)
        for (auto &y: x)
            if (y == old_bgcolor_)
                y = bgcolor_;
}

// set color to the individual pixel
void GS_Image::set_color(const int x, const int y, const GS_Color &gc) {
    if (x >= 0 && x < int(width()) && y >= 0 && y < int(height()))
        I_[x][y] = gc.color();
}

// save the image into a file
void GS_Image::write_to(const std::string &fn) {
    std::ofstream ofs {fn, std::ios_base::binary};
    ofs.exceptions(ofs.exceptions() | std::ios_base::badbit);
    const auto w = width(), h = height();
    ofs << "P5\n" << w << ' ' << h << "\n255\n"; // header
    for (size_type y {0}; y < h; ++y)
        for (size_type x {0}; x < w; ++x)
            ofs << I_[x][y];
    std::cout << "The result is saved to file: " << fn << '\n';
}

