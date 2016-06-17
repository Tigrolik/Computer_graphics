/*
 * Practicing with colors:
 *      converting RGB to Grayscale, filtering, extracting color channels,
 *      representing various color spaces, displaying color palettes
 */


#include "PPM_lib.h"
#include "Geometry.h"
#include <iostream>
#include <array>
#include <chrono>
#include <bitset>
#include <algorithm>

// define file (path) separator depending on OS
#ifdef _WIN32
static const std::string os_filesep {"\\"};
#else
static const std::string os_filesep {"/"};
#endif

// convert gray-scale image to black and white image using a threshold value
void gray2bw(PPM_lib::GS_Image &I, const unsigned char t = 128) {
    for (auto &x: I) for (auto &y: x)
        y = (y & 0xFF) > t ? 255 : 0;
}

template <class C>
void disp_array_int(const C &A) {
    for (const auto &x: A) {
        for (const auto &y: x)
            std::cout << +y << ' ';
        std::cout << '\n';
    }
}

// apply ordered dither to a grayscale image
void ordered_dither(PPM_lib::GS_Image &I) {
    using namespace PPM_lib;
    using namespace std;
    using gray_t = GS_Image::value_type;
    // Bayer matrix
    static constexpr array<array<gray_t, 2>, 2> M2 {
        array<gray_t, 2> {0, 2}, array<gray_t, 2> {3, 1}};
    array<array<gray_t, 4>, 4> M4;
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j) {
            M4[i][j] = M2[i][j] << 2;
            M4[i][j + 2] = (M2[i][j] << 2) + 2;
            M4[i + 2][j] = (M2[i][j] << 2) + 3;
            M4[i + 2][j + 2] = (M2[i][j] << 2) + 1;
        }
    //disp_array_int(M2);
    //disp_array_int(M4);

    const auto h = I.height();
    for (size_t x {0}; x < I.width(); ++x)
        for (size_t y {0}; y < h; ++y)
            I[x][y] = (I[x][y] * 17 >> 8) > M4[x % 4][y % 4] ? 255 : 0;
    //I[x][y] = (I[x][y] * 5 >> 8) > M2[x % 2][y % 2] ? 255 : 0;
}

void ordered_dither(PPM_lib::RGB_Image &I) {
    PPM_lib::GS_Image R {I.red()};
    PPM_lib::GS_Image G {I.green()};
    PPM_lib::GS_Image B {I.blue()};
    ordered_dither(R);
    ordered_dither(G);
    ordered_dither(B);
    I = PPM_lib::RGB_Image{R, G, B};
}

void error_diffusion(PPM_lib::GS_Image &I) {
    const auto h = I.height() - 1;
    for (size_t x {2}; x < I.width() - 2; ++x)
        for (size_t y {0}; y < h; ++y) {
            auto clr = I[x][y];
            I[x][y] = clr;
            auto g_clr = clr & 0xFF;
            auto temp = (g_clr > 128) ? 255 : 0;
            auto q_err = g_clr - temp;
            I[x + 1][y] += q_err >> 2;
            I[x + 2][y] += (q_err * 3) >> 4;
            I[x - 2][y + 1] += q_err >> 4;
            I[x - 1][y + 1] += q_err >> 3;
            I[x][y + 1] += (q_err * 3) >> 4;
            I[x + 1][y + 1] += q_err >> 3;
            I[x + 2][y + 1] += q_err >> 4;
        }
}

void error_diffusion(PPM_lib::RGB_Image &I) {
    PPM_lib::GS_Image R {I.red()};
    PPM_lib::GS_Image G {I.green()};
    PPM_lib::GS_Image B {I.blue()};
    error_diffusion(R);
    error_diffusion(G);
    error_diffusion(B);
    I = PPM_lib::RGB_Image{R, G, B};
}

// construct a name for a new file
std::string create_outname(const std::string &fn, const std::string &ins) {
    std::string s = fn.substr(fn.find_last_of(os_filesep) + 1);
    //return s.insert(s.find_last_of('.'), ins);
    return s.insert(0, ins);
}

void test_gray() {
    using namespace PPM_lib;
    // read image
    const std::string fn {"../imgs/baboon.ppm"};
    GS_Image I {fn};
    I.write_to(create_outname(fn, "gray_"));
    gray2bw(I, 128);
    //gray2bw(I, std::rand() % 255);
    I.write_to(create_outname(fn, "bw_"));
}

void test_dither() {
    //const std::string fn {"../imgs/baboon.ppm"};
    const std::string fn {"../imgs/building.ppm"};
    //PPM_lib::GS_Image I {fn};
    PPM_lib::RGB_Image I {fn};
    ordered_dither(I);
    //I.write_to("baboon_dither.ppm");
    I.write_to(create_outname(fn, "dither_"));
}

void test_error_diffusion() {
    //const std::string fn {"../imgs/baboon.ppm"};
    const std::string fn {"../imgs/building.ppm"};
    //PPM_lib::GS_Image I {fn};
    PPM_lib::RGB_Image I {fn};
    error_diffusion(I);
    I.write_to(create_outname(fn, "err_diff_"));
}

void display_rgb_channels(const std::string &fn) {
    PPM_lib::RGB_Image I {fn};
    const auto w = I.width(), h = I.height();
    PPM_lib::GS_Image IR {w, h};
    PPM_lib::GS_Image IG {w, h};
    PPM_lib::GS_Image IB {w, h};
    for (size_t x = 0; x < w; ++x)
        for (size_t y = 0; y < h; ++y) {
            IR[x][y] = I.color(x, y).red();
            IG[x][y] = I.color(x, y).green();
            IB[x][y] = I.color(x, y).blue();
        }
    IR.write_to(create_outname(fn, "red_"));
    IG.write_to(create_outname(fn, "green_"));
    IB.write_to(create_outname(fn, "blue_"));
    PPM_lib::RGB_Image{IR, IG, IB}.write_to("rgb.ppm");
}

std::array<double, 4> rgb2cmyk(const unsigned char r, const unsigned char g,
        const unsigned char b) {
    const double c {r / 255.0}, m {g / 255.0}, y {b / 255.0};
    const double k {std::max({c, m, y})}; // 1 - k
    return k == 0 ? std::array<double, 4> {0, 0, 0, 1 - k} :
        std::array<double, 4> {1 - c / k, 1 - m / k, 1 - y / k, 1 - k};
}

std::array<double, 4> rgb2cmyk(const std::array<unsigned char, 3> &a) {
    return rgb2cmyk(a[0], a[1], a[2]);
}

constexpr std::array<unsigned char, 3> cmyk2rgb(const double c, const double m,
        const double y, const double k) {
    return std::array<unsigned char, 3> {
        static_cast<unsigned char>((1 - c) * (1 - k) * 255),
        static_cast<unsigned char>((1 - m) * (1 - k) * 255),
        static_cast<unsigned char>((1 - y) * (1 - k) * 255)
    };
}

constexpr std::array<unsigned char, 3> cmyk2rgb(const std::array<double, 4> &a){
    return cmyk2rgb(a[0], a[1], a[2], a[3]);
}

void display_cmyk_channels(const std::string &fn) {
    PPM_lib::RGB_Image I {fn};
    const auto w = I.width(), h = I.height();
    PPM_lib::GS_Image IC {w, h};
    PPM_lib::GS_Image IM {w, h};
    PPM_lib::GS_Image IY {w, h};
    PPM_lib::GS_Image IK {w, h};
    PPM_lib::GS_Image IR {w, h};
    PPM_lib::GS_Image IG {w, h};
    PPM_lib::GS_Image IB {w, h};
    for (size_t x = 0; x < w; ++x)
        for (size_t y = 0; y < h; ++y) {
            const std::array<double, 4> cmyk = rgb2cmyk(I.color(x, y).red(),
                    I.color(x, y).green(), I.color(x, y).blue());
            //const double r = I.color(x, y).red() / 255.0;
            //const double g = I.color(x, y).green() / 255.0;
            //const double b = I.color(x, y).blue() / 255.0;
            //const double k = std::max({r, g, b}); // 1 - k
            //IC[x][y] = k > 0 ? (1 - r / k) * 255 : 0;
            //IM[x][y] = k > 0 ? (1 - g / k) * 255 : 0;
            //IY[x][y] = k > 0 ? (1 - b / k) * 255 : 0;
            //IK[x][y] = (1 - k) * 255;
            IC[x][y] = cmyk[0] * 255;
            IM[x][y] = cmyk[1] * 255;
            IY[x][y] = cmyk[2] * 255;
            IK[x][y] = cmyk[3] * 255;
            const std::array<unsigned char, 3> rgb = cmyk2rgb(cmyk);
            IR[x][y] = rgb[0];
            IG[x][y] = rgb[1];
            IB[x][y] = rgb[2];
        }
    IC.write_to(create_outname(fn, "cyan_"));
    IM.write_to(create_outname(fn, "magenta_"));
    IY.write_to(create_outname(fn, "yellow_"));
    IK.write_to(create_outname(fn, "black_"));
    PPM_lib::RGB_Image{IR, IG, IB}.write_to(create_outname(fn, "rgb_"));
}

std::array<double, 3> rgb2hsv(const unsigned char r, const unsigned char g,
        const unsigned char b) {
    const double c_max = std::max({r, g, b});
    const double delta = c_max - std::min({r, g, b});
    //const double S = (c_max > 0) ? (delta / c_max) : 0;
    double H = 0;
    if (delta > 0) {
        if (r == c_max)
            H = fmod((g - b) / delta, 6.0); // between yellow and magenta
        else if (g == c_max)
            H = 2 +  (b - r) / delta; // between cyan and yellow
        else
            H = 4 + (r - g) / delta; // between magenta and cyan
    }
    return {H * 60, (c_max > 0) ? (delta / c_max) : 0, c_max};
}

std::array<double, 3> rgb2hsv(const std::array<unsigned char, 3> &a) {
    return rgb2hsv(a[0], a[1], a[2]);
}

std::array<unsigned char, 3> hsv2rgb(double H, const double S,
        const double V) {
    using gray_t = unsigned char;
    if (S == 0) // gray
        return {gray_t(V), gray_t(V), gray_t(V)};
    H /= 60.0;
    const int n = H;
    const double frac = H - n;
    const double c1 = V * (1 - S);
    const double c2 = V * (1 - S * frac);
    const double c3 = V * (1 - S * (1 - frac));
    switch (n) {
        case 0:
            return {gray_t(V), gray_t(c3), gray_t(c1)};
        case 1:
            return {gray_t(c2), gray_t(V), gray_t(c1)};
        case 2:
            return {gray_t(c1), gray_t(V), gray_t(c3)};
        case 3:
            return {gray_t(c1), gray_t(c2), gray_t(V)};
        case 4:
            return {gray_t(c3), gray_t(c1), gray_t(V)};
        case 5:
        default:
            return {gray_t(V), gray_t(c1), gray_t(c2)};
    }
}

std::array<unsigned char, 3> hsv2rgb(const std::array<double, 3> &a) {
    return hsv2rgb(a[0], a[1], a[2]);
}

constexpr std::array<unsigned char, 3> rgb2ycbcr(const unsigned char r,
        const unsigned char g, const unsigned char b) {
    return {static_cast<unsigned char>(0.299 * r + 0.587 * g + 0.114 * b),
        static_cast<unsigned char>(128 - 0.168736 * r - 0.33126 * g + 0.5 * b),
        static_cast<unsigned char>(128 + 0.5 * r - 0.41869 * g - 0.08131 * b)};
}

constexpr std::array<unsigned char, 3> rgb2ycbcr(const
        std::array<unsigned char, 3> &a) {
    return rgb2ycbcr(a[0], a[1], a[2]);
}

constexpr std::array<unsigned char, 3> ycbcr2rgb(const unsigned char y,
        const unsigned char cb, const unsigned char cr) {
    return {static_cast<unsigned char>(y + 1.402 * (cr - 128)),
        static_cast<unsigned char>(y - 0.344136 * (cb - 128) -
                0.714136 * (cr - 128)),
        static_cast<unsigned char>(y + 1.772 * (cb - 128))};
}

constexpr std::array<unsigned char, 3> ycbcr2rgb(const
        std::array<unsigned char, 3> &a) {
    return ycbcr2rgb(a[0], a[1], a[2]);
}

void display_hsv_channels(const std::string &fn) {
    PPM_lib::RGB_Image I {fn};
    const auto w = I.width(), h = I.height();
    PPM_lib::GS_Image H {w, h};
    PPM_lib::GS_Image S {w, h};
    PPM_lib::GS_Image V {w, h};
    PPM_lib::GS_Image IR {w, h};
    PPM_lib::GS_Image IG {w, h};
    PPM_lib::GS_Image IB {w, h};
    for (size_t x = 0; x < w; ++x)
        for (size_t y = 0; y < h; ++y) {
            const std::array<double, 3> hsv = rgb2hsv(I.color(x, y).red(),
                    I.color(x, y).green(), I.color(x, y).blue());
            H[x][y] = hsv[0] / 360.0 * 255;
            S[x][y] = hsv[1] * 255;
            V[x][y] = hsv[2];

            const std::array<unsigned char, 3> rgb = hsv2rgb(hsv);
            IR[x][y] = rgb[0];
            IG[x][y] = rgb[1];
            IB[x][y] = rgb[2];
        }
    H.write_to(create_outname(fn, "hue_"));
    S.write_to(create_outname(fn, "sat_"));
    V.write_to(create_outname(fn, "val_"));
    PPM_lib::RGB_Image{IR, IG, IB}.write_to(create_outname(fn, "rgb_"));
}

void display_ycbcr_channels(const std::string &fn) {
    PPM_lib::RGB_Image I {fn};
    const auto w = I.width(), h = I.height();
    PPM_lib::GS_Image Y {w, h};
    PPM_lib::GS_Image Cb {w, h};
    PPM_lib::GS_Image Cr {w, h};
    PPM_lib::GS_Image IR {w, h};
    PPM_lib::GS_Image IG {w, h};
    PPM_lib::GS_Image IB {w, h};
    for (size_t x = 0; x < w; ++x)
        for (size_t y = 0; y < h; ++y) {
            const std::array<unsigned char, 3> ycbcr =
                rgb2ycbcr(I.color(x, y).red(), I.color(x, y).green(),
                        I.color(x, y).blue());
            Y[x][y] = ycbcr[0];
            Cb[x][y] = ycbcr[1];
            Cr[x][y] = ycbcr[2];

            const std::array<unsigned char, 3> rgb = ycbcr2rgb(ycbcr);
            IR[x][y] = rgb[0];
            IG[x][y] = rgb[1];
            IB[x][y] = rgb[2];
        }
    Y.write_to(create_outname(fn, "y_"));
    Cb.write_to(create_outname(fn, "cb_"));
    Cr.write_to(create_outname(fn, "cr_"));
    PPM_lib::RGB_Image{IR, IG, IB}.write_to(create_outname(fn, "rgb_"));
}

void test_rgb_channels() {
    //display_rgb_channels("../imgs/building.ppm");
    display_rgb_channels("../imgs/baboon.ppm");
}

void test_cmyk_channels() {
    //display_rgb_channels("../imgs/building.ppm");
    display_cmyk_channels("../imgs/baboon.ppm");
}

void test_hsv_channels() {
    //display_rgb_channels("../imgs/building.ppm");
    display_hsv_channels("../imgs/baboon.ppm");
}

void test_ycbcr_channels() {
    //display_rgb_channels("../imgs/building.ppm");
    display_ycbcr_channels("../imgs/baboon.ppm");
}

// input parameter: number of colors for each (r, g, b) channel
void draw_uniform_palette(const int n) {
    if (n < 1)
        throw std::runtime_error("input should be a positive number");
    const int r {n - 1}; // auxilliary coefficient
    constexpr int a {32}; // size of a square to be painted
    constexpr int margin {10}; // margin for padding the rectangle a little
    constexpr int pad {margin >> 1}; // padding value
    const int num_sq {n * n * n}; // number of squares
    constexpr int cols {16}; // limit the number of squares in a row

    using namespace PPM_lib;
    constexpr int w = cols * a + margin; // image width
    const int h = ((num_sq - 1)/ cols + 1) * a + margin; // image height
    RGB_Image I (w, h, Color_name::white); // white background
    int idx {0}; // current square index
    // traversing colors in the reverse order to display the palette correctly
    for (int i = n; i--;) // red
        for (int j = n; j--;) // green
            for (int k = n; k--;) { // blue
                const int x = idx % cols;
                const int y = idx / cols;
                Rectangle rect {Point{pad + x * a, pad + y * a}, a, a};
                rect.fill(I, RGB_Color(i / double(r) * 255,
                            j / double(r) * 255, k / double(r) * 255));
                rect.draw(I, Color_name::white);
                ++idx;
            }
    I.write_to("color_palette_" + std::to_string(n) + ".ppm");
}

// input parameter: how many shaded steps to apply
void draw_shaded_palette(const int n) {
    const int r {n - 1}; // auxilliary coefficient
    constexpr int a {32}; // size of a square to be painted
    constexpr int margin {10}; // margin for padding the rectangle a little
    constexpr int pad {margin >> 1}; // padding value
    const double step {255.0 / n};

    using namespace PPM_lib;
    const int w = n * a + margin; // image width
    const int h = 3 * a + margin; // image height
    RGB_Image I (w, h, Color_name::white); // white background
    for (int i = n; i--;) {
        const int x = pad + (r - i) * a;
        const unsigned char val = i * step;
        // draw reddish squares
        Rectangle rect_red {Point{x, pad}, a, a};
        rect_red.fill(I, RGB_Color{val, 0, 0});
        rect_red.draw(I, Color_name::white);
        // draw greenish squares
        Rectangle rect_green {Point{x, pad + a}, a, a};
        rect_green.fill(I, RGB_Color{0, val, 0});
        rect_green.draw(I, Color_name::white);
        // draw blueish squares
        Rectangle rect_blue {Point{x, pad + (a << 1)}, a, a};
        rect_blue.fill(I, RGB_Color{0, 0, val});
        rect_blue.draw(I, Color_name::white);
    }
    I.write_to("shaded_palette_" + std::to_string(n) + ".ppm");
}

void test_palette() {
    // testing uniform palette
    //draw_uniform_palette(1);
    //draw_uniform_palette(2);
    //draw_uniform_palette(3);
    //draw_uniform_palette(4);
    //draw_uniform_palette(5);
    draw_uniform_palette(6);
    // testing shaded palette
    draw_shaded_palette(16);
}

int main() {

    //test_gray();
    //test_dither();
    //test_error_diffusion();
    //test_rgb_channels();
    //test_cmyk_channels();
    //test_hsv_channels();
    test_ycbcr_channels();
    //test_colors();
    //test_palette();

    return 0;
}

