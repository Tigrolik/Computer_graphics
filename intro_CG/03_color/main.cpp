/*
 *
 */

#include "Geometry.h"
#include <iostream>
#include <chrono>

// define file (path) separator depending on OS
#ifdef _WIN32
static const std::string os_filesep {"\\"};
#else
static const std::string os_filesep {"/"};
#endif

/*
 * appends string ins to the filename, keeping the extension and ignoring the
 * relative path (that is, the new file name is for the current folder)
 */
std::string create_outname(const std::string &fn, const std::string &ins) {
    std::string s = fn.substr(fn.find_last_of(os_filesep) + 1);
    return s.insert(s.find_last_of('.'), ins);
}

// convert single unsigned int color value to gray color value
uint rgb2gray(const uint val) {
    static constexpr double r_w {0.2126}, g_w {0.7152}, b_w {0.0722};
    const uchar gray_color = (val >> 16 & 0xFF) * r_w +
        (val >> 8 & 0xFF) * g_w + (val & 0xFF) * b_w;
    return gray_color << 16 | gray_color << 8 | gray_color;
}

constexpr uint gray2rbg(const uchar val) {
    return val << 16 | val << 8 | val;
}

// check if unsigned int color value is gray
constexpr bool is_gray(const uint y) {
    return (y >> 8 & 0xFF) == (y & 0xFF) && (y >> 16 & 0xFF) == (y & 0xFF);
}

// convert image to grayscale
void rgb2gray(PPM_Image &I) {
    for (auto &x: I) for (auto &y: x)
        y = rgb2gray(y);
}

// assuming that the image is already grayscale
void gray2bw(PPM_Image &I, const uchar t = 128) {
    for (auto &x: I) for (auto &y: x) {
        if (!is_gray(y)) // fix color value
            y = rgb2gray(y);
        if ((y & 0xFF) > t)
            y = 0xFFFFFF;
        else
            y = 0;
    }

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
void ordered_dither(PPM_Image &I) {
    // Bayer matrix
    static constexpr std::array<std::array<uchar, 2>, 2> M2 {
        std::array<uchar, 2> {0, 2}, std::array<uchar, 2> {3, 1}};
    disp_array_int(M2);
    std::array<std::array<uchar, 4>, 4> M4;
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j) {
            M4[i][j] = M2[i][j] << 2;
            M4[i][j + 2] = (M2[i][j] << 2) + 2;
            M4[i + 2][j] = (M2[i][j] << 2) + 3;
            M4[i + 2][j + 2] = (M2[i][j] << 2) + 1;
        }
    disp_array_int(M4);

    const int h {I.height()};
    for (int x {0}; x < I.width(); ++x)
        for (int y {0}; y < h; ++y) {
            if (!is_gray(I[x][y])) // fix color value
                I[x][y] = rgb2gray(I[x][y]);
            if ((((I[x][y] & 0xFF) * 5) >> 8) > M2[x % 2][y % 2])
            //if ((((I[x][y] & 0xFF) * 17) >> 8) > M4[x % 4][y % 4])
                I[x][y] = 0xFFFFFF;
            else
                I[x][y] = 0;
        }
}

void error_diffusion(PPM_Image &I) {
    const int h {I.height() - 1};
    for (int x {2}; x < I.width() - 2; ++x)
        for (int y {0}; y < h; ++y) {
            uint clr {I[x][y]};
            if (!is_gray(clr)) // fix color value
                clr = rgb2gray(clr);
            I[x][y] = clr;
            uchar g_clr = clr & 0xFF;
            auto temp = (g_clr > 128) ? 255 : 0;
            auto q_err = g_clr - temp;
            I[x + 1][y] += gray2rbg(q_err >> 2);
            I[x + 2][y] += gray2rbg((q_err * 3) >> 4);
            I[x - 2][y + 1] += gray2rbg(q_err >> 4);
            I[x - 1][y + 1] += gray2rbg(q_err >> 3);
            I[x][y + 1] += gray2rbg((q_err * 3) >> 4);
            I[x + 1][y + 1] += gray2rbg(q_err >> 3);
            I[x + 2][y + 1] += gray2rbg(q_err >> 4);
        }
}

void test_gray() {
    // read image
    const std::string fn {".." + os_filesep + "imgs" + os_filesep +
        "baboon.ppm"};
    PPM_Image I {fn};
    rgb2gray(I);
    //std::string s {create_outname(fn, "_gc")};
    I.write_to(create_outname(fn, "_gc"));
    gray2bw(I, 128);
    //gray2bw(I, std::rand() % 255);
    I.write_to(create_outname(fn, "_bw"));

    //using namespace std::chrono;
    // measure time
    //const size_t n {100}; // number of reps for testing
    //std::cout << "Testing drawline_steps() (wrong):\twhite lines\n";
    //auto t = high_resolution_clock::now();
    //for (size_t i {n}; i--;) {
    //}
    //std::cout << duration<double>(high_resolution_clock::now() - t).count() <<
    //    " seconds\n";

}

void test_dither() {
    //const std::string fn {".." + os_filesep + "imgs" + os_filesep +
    //"baboon.ppm"};
    //const std::string fn {".." + os_filesep + "imgs" + os_filesep +
    //"building.ppm"};
    const std::string fn {".." + os_filesep + "imgs" + os_filesep +
        "building.ppm"};
    PPM_Image I {fn};
    rgb2gray(I);
    ordered_dither(I);
    //I.write_to("baboon_dither.ppm");
    I.write_to(create_outname(fn, "_dither"));
}

void test_error_diffusion() {
    //const std::string fn {".." + os_filesep + "imgs" + os_filesep +
    //"baboon.ppm"};
    const std::string fn {".." + os_filesep + "imgs" + os_filesep +
        "baboon.ppm"};
    //const std::string fn {".." + os_filesep + "imgs" + os_filesep +
    //"building.ppm"};
    PPM_Image I {fn};
    rgb2gray(I);
    error_diffusion(I);
    I.write_to(create_outname(fn, "_err_diff"));
}

void display_rgb(const std::string &fn) {
    //PPM_Image I {fn};
    std::string s {create_outname(fn, "_gc")};
    std::cout << s + "\n";
}

int main() {

    test_gray();
    //test_dither();
    //test_error_diffusion();
    //display_rgb(".." + os_filesep + "imgs" + os_filesep + "baboon.ppm");
    //display_rgb("baboon.ppm");

    return 0;
}

