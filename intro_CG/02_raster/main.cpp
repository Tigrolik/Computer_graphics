/*
 * The purpose of this exercise is to be able to draw lines and circles
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
    const size_t n {10}; // number of reps for testing

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
    constexpr Point p1 {0, 0}, p2 {100, 100};
    constexpr int r {1};
    const Line s {p1, p2};
    const Circle c1 {p1, r};
    std::cout << "Line(0, 0, 100, 100) length: " << s.length() <<
        "\nCircle(0, 0, 20) length: " << c1.length() << ", and area: " <<
        c1.area() << '\n';
}

void test_filling_tri() {
    constexpr int w {600}, h {400};
    PPM_Image I {w, h};

    using namespace std::chrono;
    const size_t n {10}; // number of reps for testing

    const Triangle t1 {{10, 10}, {10, 30}, {45, 10}};
    const Triangle t2 {{50, 50}, {590, 30}, {40, 390}};
    const Triangle t3 {{595, 70}, {595, 370}, {90, 395}};

    std::cout << "Testing Triangle.fill()\n";
    auto t = high_resolution_clock::now();
    for (size_t i {n}; i--;) {
        t1.fill(I, Color_name::red);
        t2.fill(I, Color_name::green);
        t3.fill(I, Color_name::blue);
    }
    std::cout << duration<double>(high_resolution_clock::now() - t).count() <<
        " seconds\n";

    std::cout << "Testing Triangle.fill_halfs():\n";
    t = high_resolution_clock::now();
    for (size_t i {n}; i--;) {
        t1.fill_hs(I, Color_name::red);
        t2.fill_hs(I, Color_name::green);
        t3.fill_hs(I, Color_name::blue);
    }
    std::cout << duration<double>(high_resolution_clock::now() - t).count() <<
        " seconds\n";

    t1.draw(I, Color_name::yellow);
    t2.draw(I, Color_name::yellow);
    t3.draw(I, Color_name::yellow);

    I.write_to("tri.ppm");
}

void test_point_array() {
    constexpr int w {600}, h {400};
    PPM_Image I {w, h};
    //constexpr Point p1 {10, 10}, p2 {100, 250}, p3 {530, 150};
    //std::vector<Point> vp {p1, p2, p3};
    Point_array pa {{10, 10}, {100, 250}, {530, 150}};
    pa.push_back({300, 10});
    Point_array pa2 {std::move(pa)};
    pa2.fill(I, Color_name::yellow);
    Polyline pl1 {pa2.points()};
    Polygon pg1 {pl1.points()};
    std::cout << pg1.length() << ' ' << pl1.length() << '\n';
    std::cout << pg1.area() << ' ' << pl1.area() << '\n';
    Polygon pg2 {{50, 10}, {10, 139}, {153, 30}};
    Triangle t1 {{50, 10}, {153, 30}, {10, 139}};
    std::cout << pg2.area() << ' ' << t1.area() << '\n';
    Polygon pg3 {{450, 250}, {450, 350}, {550, 350}, {550, 250}, {500, 300}};
    std::cout << pg3.area() << '\n';
    pg3.draw(I, Color_name::red);
    pg1.draw(I, Color_name::red);
    pl1.push_back({500, 100});
    Polyline pl2 {std::move(pl1)};
    pl2.draw(I, Color_name::green);
    Polyline pl3 = pl2;
    pl3.fill(I, Color_name::cyan);
    pg2.draw(I, Color_name::green);
    I.write_to("dots.ppm");
}

void test_random() {
    constexpr int w {600}, h {400}, wc {w >> 1}, hc {h >> 1};
    PPM_Image I {w, h};
    constexpr size_t n {20};
    auto gen = std::bind(std::normal_distribution<double> {n, 4.0},
            std::default_random_engine {});
    std::vector<int> hist (n << 1);
    for (size_t i {0}; i < 1000; ++i)
        ++hist[int(round(gen()))];
    Point_array pa;
    for (size_t i {0}; i < hist.size(); ++i)
        pa.push_back({int(i + wc - n), hc - hist[i]});
    Polyline pl {pa.points()};
    pl.draw(I, Color_name::yellow);
    pa.draw(I, Color_name::red);

    I.write_to("normal_dist.ppm");
}

bool is_in_poly(const Point p, const Polygon &pg) {
    bool res {false};
    const int x {p.x()}, y {p.y()};
    for (size_t i {0}, j {pg.size() - 1}; i < pg.size(); j = i++) {
        const Point pgi {pg[i]}, pgj {pg[j]};
        const int xi {pgi.x()}, xj {pgj.x()}, yi {pgi.y()}, yj {pgj.y()};
        if (((yi < y && yj >= y) || (yj < y && yi >= y)) && (xi <= x || xj <=x))
            res ^= (xi + (double(y) - yi) / (yj - yi) * (xj - xi) < x);
    }
    return res;
}

void fill_poly_scan(PPM_Image &I, const Polygon &pg, const PPM_Color &c) {
    const size_t n {pg.size()};
    if (n < 1) return;
    int ymin {pg[0].y()}, ymax {ymin};
    for (size_t i {1}; i < n; ++i) {
        const int y {pg[i].y()};
        if (y < ymin) ymin = y;
        else if (y > ymax) ymax = y;
    }
    // building a vector of nodes, sorting them and filling the pixels
    const int w {I.width()}, h {I.height()};
    for (auto y = std::min(std::max(0, ymin), h);
            y < std::min(std::max(0, ymax), h); ++y) {
        std::vector<int> nodes;
        for (size_t i {0}, j {n - 1}; i < n; j = i++) {
            const int yi {pg[i].y()}, yj {pg[j].y()};
            if ((yi < y && yj >= y) || (yj < y && yi >= y)) {
                const int xi {pg[i].x()};
                nodes.push_back(xi + (double(y) - yi) / (yj - yi) *
                        (pg[j].x()- xi));
            }
        }
        sort(std::begin(nodes), std::end(nodes));
        for (size_t i {0}; i < nodes.size(); i += 2) {
            for (auto x = std::min(std::max(0, nodes[i]), w);
                    x < std::min(std::max(0, nodes[i + 1]), w); ++x)
                I[x][y] = c.color();
        }
    }
}

void test_poly() {
    const Polygon pg1 {{450, 250}, {450, 350}, {550, 350}, {550, 250},
        {500, 300}};
    constexpr Point p1 {500, 325}, p2 {400, 100}, p3 {500, 275};

    constexpr int w {600}, h {400};
    PPM_Image I {w, h};
    pg1.draw(I, Color_name::red);
    p1.draw(I);
    p2.draw(I);
    p3.draw(I);
    std::cout << (is_in_poly(p1, pg1) ? "inside\n" : "outside\n");
    std::cout << (is_in_poly(p2, pg1) ? "inside\n" : "outside\n");
    std::cout << (is_in_poly(p3, pg1) ? "inside\n" : "outside\n");
    fill_poly_scan(I, pg1, Color_name::yellow);

    I.write_to("poly.ppm");
}

void test_fill_poly() {
    constexpr int w {600}, h {400};
    PPM_Image I {w, h};

    const Polygon pg1 {{-10, 30}, {70, -10}, {90, 60}, {-30, 80}};
    fill_poly_scan(I, pg1, PPM_Color{255, 160, 125});
    pg1.draw(I, PPM_Color{53, 216, 185});

    const Polygon pg2 {{500, 10}, {610, -50}, {700, -5}, {590, 70}, {550, 60}};
    fill_poly_scan(I, pg2, PPM_Color{195, 83, 216});
    pg2.draw(I, PPM_Color{142, 216, 52});

    Polygon pg3 {{100, 80}, {120, 40}, {200, 10}, {350, 75}, {310, 85}};
    pg3.push_back({255, 40}); pg3.push_back({225, 90});
    pg3.push_back({185, 90}); pg3.push_back({205, 45});
    pg3.push_back({215, 85}); pg3.push_back({235, 35});
    pg3.push_back({165, 30}); pg3.push_back({150, 80});
    pg3.push_back({135, 95}); pg3.push_back({115, 100});
    fill_poly_scan(I, pg3, PPM_Color{60, 90, 255});
    pg3.draw(I, PPM_Color{216, 174, 52});

    Polygon pg4 {{50, 300}, {80, 200}, {140, 200}, {140, 330}, {100, 330},
        {90, 270}, {130, 245}, {125, 210}, {110, 205}, {115, 315}};
    //fill_poly_scan(I, pg4, PPM_Color{8, 216, 82});
    pg4.fill(I, PPM_Color{8, 216, 82});
    pg4.draw(I, PPM_Color{216, 35, 28});

    I.write_to("filled_polygons.ppm");
}

void test_rect() {
    constexpr int w {600}, h {400};
    PPM_Image I {w, h};

    Rectangle rect1 {{100, 100}, 200, 50};
    rect1.fill(I, Color_name::cyan);
    rect1.draw(I, Color_name::red);

    Rectangle rect2 {{-10, 300}, 500, 150};
    //rect2.fill(I, Color_name::magenta);
    rect2.draw(I, Color_name::green);

    Rectangle rect3 {{510, 300}, 200, 150};
    rect3.draw(I, Color_name::magenta);

    Rectangle rect4 {{-10, -30}, 90, 150};
    rect4.draw(I, Color_name::khaki);

    Rectangle rect5 {{410, -90}, 250, 150};
    rect5.draw(I, Color_name::red);

    I.write_to("rect.ppm");
}

int main() {

    //test_lines();
    //test_circles();
    //test_lenght_n_area();
    //test_filling_tri();
    //test_point_array();
    //test_random();
    //test_poly();
    //test_fill_poly();
    test_rect();

    return 0;
}

