/*
 * The purpose of this exercise is to be able to draw a single dot on the image.
 * I am not creating any fancy graphics, but starting from the basics
 */

#include "Geometry.h"
#include <iostream>

using namespace std;

int main() {

    // initialize an image of width w and height h and black background
    constexpr int w {200}, h {100};
    PPM_Image I {w, h};

    // draw several dots in various ways
    // create a Point object and draw it onto the image with white (default)
    constexpr Point p1 {150, 75}, p2 {38.9024, 24.782}, p3 {p2};
    constexpr double d {p1.dist_to(p3)};
    std::cout << d << '\n';
    p1.draw(I);
    // draw a point right after constructing it
    Point{130, 30}.draw(I, PPM_Color{128, 240, 53});
    // use PPM_Image method
    I.set_color(w >> 1, h >> 1, Color_name::orange);
    I.set_color(50, 90, {"88AADD"});
    // draw via index operator and assigning an unsigned int in hex format
    I[25][10] = 0xFFAAFF;

    // save image to a file
    I.write_to("dots.ppm");

    return 0;
}

