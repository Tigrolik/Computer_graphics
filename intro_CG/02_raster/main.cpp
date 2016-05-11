/*
 * The purpose of this exercise is to be able to draw a single dot on the image.
 * I am not creating any fancy graphics, but starting from the basics
 *
 * Added:
 *      Cohen–Sutherland line clipping algorithm: clip the line so that the
 *      coordinates beyond image are not processed
 */

#include "Geometry.h"
#include <iostream>
#include <chrono>

/*
 * draw lines with a straightforward increment of both x and y coordinates
 * might stop before one of coordinates reaches the end (WARNING: not good)
 * For testing we assume that x1 < x2
 */
void drawline_steps(const int x1, const int y1, const int x2, const int y2,
        PPM_Image &I, const PPM_Color &c = 255) {
    int x {x1}, y {y1};
    while (x <= x2 && y <= y2)
        I.set_color(++x, ++y, c);
}

/*
 * digital differential analyzer: using slope variable
 * For testing we assume that x1 < x2
 */
void drawline_dda(const int x1, const int y1, const int x2, const int y2,
        PPM_Image &I, const PPM_Color &c = 255) {
    const double slope {static_cast<double>(y2 - y1) / (x2 - x1)};
    double y {y1 + 0.5};
    for (int x {x1}; x <= x2; ++x, y += slope)
        I.set_color(x, y, c);
}

/*
 * digital differential analyzer fixed point: using bitwise shift
 * For testing we assume that x1 < x2
 */
void drawline_dda_fp(const int x1, const int y1, const int x2, const int y2,
        PPM_Image &I, const PPM_Color &c = 255) {
    //int x = x1, count = x2 - x1;
    const long slope {((y2 - y1) << 16) / (x2 - x1)};
    long y {y1 << 16};

    I.set_color(x1, y1, c);
    for (int x {x1 + 1}; x <= x2; ++x) {
        y += slope;
        I.set_color(x, y >> 16, c);
    }
}

/*
 * Bresenham algorithm: first try, using floating point values yet
 * using function (x - x1) * dy - (y - y1 + 0.5) * dx to define whether to use
 * current y value or to increment (++y). The 0.5 value is used as a point
 * between y and y + 1.
 * For testing we assume that x1 < x2
 */
void drawline_bresenham(const int x1, const int y1, const int x2, const int y2,
        PPM_Image &I, const PPM_Color &c = 255) {
    const int dx {x2 - x1}, dy {y2 - y1};
    I.set_color(x1, y1, c);
    for (int x {x1}, y {y1}; x < x2;
            (++x - x1) * dy - (y - y1 + 0.5) * dx > 0 ?
            I.set_color(x, ++y, c) : I.set_color(x, y, c)) { }
}

/*
 * Bresenham algorithm: second try, enhancing, getting rid of floating points
 * values
 * For testing we assume that x1 < x2
 */
void drawline_bresenham_enh(const int x1, const int y1, const int x2,
        const int y2, PPM_Image &I, const PPM_Color &c = 255) {
    const int dx {x2 - x1}, dy {y2 - y1}, incdx {dx << 1}, incdy {dy << 1};
    for (int x {x1}, y {y1}, e {dx}; x <= x2; ++x) {
        I.set_color(x, y, c);
        if ((e -= incdy) < 0) {
            ++y;
            e += incdx;
        }
    }
}

void test_lines() {
    // initialize an image of width w and height h and black background
    constexpr int w {600}, h {400};
    PPM_Image I {w, h};

    using namespace std::chrono;
    // measure time
    const size_t n {100}; // number of reps for testing

    std::cout << "Testing drawline_steps() (wrong):\twhite lines\n";
    auto t = high_resolution_clock::now();
    for (size_t i {n}; i--;) {
        drawline_steps(10, 20, 570, 80, I);
        drawline_steps(10, 30, 570, 90, I);
    }
    std::cout << duration<double>(high_resolution_clock::now() - t).count() <<
        " seconds\n";

    t = high_resolution_clock::now();
    std::cout << "Testing drawline_dda():\t\t\tyellow lines\n";
    for (size_t i {n}; i--;) {
        drawline_dda(10, 40, 570, 100, I, Color_name::yellow);
        drawline_dda(10, 50, 570, 110, I, Color_name::yellow);
    }
    std::cout << duration<double>(high_resolution_clock::now() - t).count() <<
        " seconds\n";

    t = high_resolution_clock::now();
    std::cout << "Testing drawline_dda_fp():\t\tgreen lines\n";
    for (size_t i {n}; i--;) {
        drawline_dda_fp(10, 60, 570, 120, I, Color_name::green);
        drawline_dda_fp(10, 70, 570, 130, I, Color_name::green);
    }
    std::cout << duration<double>(high_resolution_clock::now() - t).count() <<
        " seconds\n";

    t = high_resolution_clock::now();
    std::cout << "Testing drawline_bresenham():\t\tred lines\n";
    for (size_t i {n}; i--;) {
        drawline_bresenham(10, 80, 570, 140, I, Color_name::red);
        drawline_bresenham(10, 90, 570, 150, I, Color_name::red);
    }
    std::cout << duration<double>(high_resolution_clock::now() - t).count() <<
        " seconds\n";

    t = high_resolution_clock::now();
    std::cout << "Testing drawline_bresenham_enh():\tcyan lines\n";
    for (size_t i {n}; i--;) {
        drawline_bresenham_enh(10, 100, 570, 160, I, Color_name::cyan);
        drawline_bresenham_enh(10, 110, 570, 170, I, Color_name::cyan);
    }
    std::cout << duration<double>(high_resolution_clock::now() - t).count() <<
        " seconds\n";

    t = high_resolution_clock::now();
    std::cout << "Testing bresenham from Line.draw():\tmagenta lines\n";
    for (size_t i {n}; i--;) {
        Line{10, 120, 570, 180}.draw(I, Color_name::magenta);
        Line{10, 130, 570, 190}.draw(I, Color_name::magenta);
    }
    std::cout << duration<double>(high_resolution_clock::now() - t).count() <<
        " seconds\n";

    // test for clipping
    Line{100, -100, 600, 400}.draw(I, Color_name::magenta);
    Line{0, 0, 800, 400}.draw(I, Color_name::magenta);
    Line{-50, 330, 850, 330}.draw(I, Color_name::magenta);
    Line{150, -130, 150, 1330}.draw(I, Color_name::magenta);
    Line{550, -10, 15, 430}.draw(I, Color_name::magenta);
    I.write_to("lines.ppm");
}

void draw8points(const int x1, const int y1, const int x, const int y,
        PPM_Image &I, const PPM_Color &c) {
    // draw 8 points to reperesent the entire circle (except the 'corners')
    I.set_color(x1 + x, y1 + y, c);
    I.set_color(x1 - x, y1 + y, c);
    I.set_color(x1 + x, y1 - y, c);
    I.set_color(x1 - x, y1 - y, c);
    I.set_color(x1 + y, y1 + x, c);
    I.set_color(x1 - y, y1 + x, c);
    I.set_color(x1 + y, y1 - x, c);
    I.set_color(x1 - y, y1 - x, c);
}

void draw4points(const int x1, const int y1, const int r, PPM_Image &I,
        const PPM_Color &c) {
    // draw 4 'corners' of a circle
    I.set_color(x1, y1 + r, c);
    I.set_color(x1, y1 - r, c);
    I.set_color(x1 + r, y1, c);
    I.set_color(x1 - r, y1, c);
}

void drawcircle_bresenham(const int x1, const int y1, const int r,
        PPM_Image &I, const PPM_Color &c = 255) {
    int x {0}, y {r};
    const int r2 {sqr(r)};
    while (x <= y) {
        if (sqr(++x) + sqr(y - 0.5) - r2 > 0)
            --y;
        draw8points(x1, y1, x, y, I, c);
    }
    draw4points(x1, y1, r, I, c);
}

void drawcircle_bresenham_enh(const int x1, const int y1, const int r,
        PPM_Image &I, const PPM_Color &c = 255) {
    int x {0}, y {r}, f {1 - r}, incE {3}, incSE {5 - (r << 1)};
    while (x <= y) {
        if (f > 0) {
            --y;
            f += incSE;
            incSE += 4;
        } else {
            f += incE;
            incSE += 2;
        }
        incE += 2;
        draw8points(x1, y1, ++x, y, I, c);
    }
    draw4points(x1, y1, r, I, c);
}

void drawcircle_bresenham_mod(const int x1, const int y1, const int r,
        PPM_Image &I, const PPM_Color &c = 255) {
    int x {0}, y {r}, f {1 - r};
    while (x <= y) {
        f < 0 ? f += (++x << 1) + 3 : f += (++x << 1) - (--y << 1) + 5;
        draw8points(x1, y1, x, y, I, c);
    }
    draw4points(x1, y1, r, I, c);
}


void test_circles() {
    // initialize an image of width w and height h and black background
    constexpr int w {600}, h {400}, w2 {w >> 1}, h2 {h >> 1};
    PPM_Image I {w, h};

    using namespace std::chrono;
    // measure time
    const size_t n {1000}; // number of reps for testing

    std::cout << "Testing drawcircle_bresenham():\t\twhite color\n";
    auto t = high_resolution_clock::now();
    for (size_t i {n}; i--;) {
        drawcircle_bresenham(w2, h2, 50, I);
        drawcircle_bresenham(w2, h2, 90, I);
    }
    std::cout << duration<double>(high_resolution_clock::now() - t).count() <<
        " seconds\n";

    std::cout << "Testing drawcircle_bresenham_enh():\tred color\n";
    t = high_resolution_clock::now();
    for (size_t i {n}; i--;) {
        drawcircle_bresenham_enh(w2, h2, 51, I, Color_name::red);
        drawcircle_bresenham_enh(w2, h2, 91, I, Color_name::red);
    }
    std::cout << duration<double>(high_resolution_clock::now() - t).count() <<
        " seconds\n";

    std::cout << "Testing drawcircle_bresenham_mod():\tgreen color\n";
    t = high_resolution_clock::now();
    for (size_t i {n}; i--;) {
        drawcircle_bresenham_mod(w2, h2, 52, I, Color_name::green);
        drawcircle_bresenham_mod(w2, h2, 92, I, Color_name::green);
    }
    std::cout << duration<double>(high_resolution_clock::now() - t).count() <<
        " seconds\n";

    std::cout << "Testing bresenham for Circle.draw():\tcyan color\n";
    t = high_resolution_clock::now();
    for (size_t i {n}; i--;) {
        Circle{w2, h2, 53}.draw(I, Color_name::cyan);
        Circle{w2, h2, 93}.draw(I, Color_name::cyan);
    }
    std::cout << duration<double>(high_resolution_clock::now() - t).count() <<
        " seconds\n";

    // test for filling
    Circle c {50, 50, 20};
    c.fill(I, Color_name::khaki);

    I.write_to("circles.ppm");
}

void test_lenght_n_area() {
    const Point p1 {0, 0}, p2 {100, 100};
    constexpr int r {1};
    const Line s {p1, p2};
    const Circle c1 {p1, r};
    std::cout << "Line(0, 0, 100, 100) length: " << s.length() <<
        "\nCircle(0, 0, 20) length: " << c1.length() << ", and area: " <<
        c1.area() << '\n';
}

int main() {

    //test_lines();
    test_circles();
    //test_lenght_n_area();

    return 0;
}

