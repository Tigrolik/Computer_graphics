/*
 * Geometry classes:
 * Using OOP techniques to implement geometrical primitives (points, lines,
 * triangles).
 * Shape is a virtual base class and all other classes are derived from it.
 * The Point here is implemented outside the Shape class for practice and test
 * (with probable addition of rotation around a point method)
 *
 * Shape declares methods for length (or perimeter), area of a shape,
 * drawing and filling shapes. Obviously area of a line is zero but I have
 * decided to keep it in the implementation in order not to remove it from
 * the Shape class. After all, it does not harm so far and might be useful
 * later... The same concerns the filling of lines (simply copy drawing, since
 * filling only makes sense for closed polygons: in our case, triangles only)
 * Added:
 *      - Cohen–Sutherland line clipping algorithm: clip the line so that the
 *      coordinates beyond image are not processed
 *      - Circle class
 *      - Half-space filling algorithm for triangle
 */

#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include "PPM_Image.h"
#include <algorithm>

// pi constant
static constexpr double pi {std::acos(-1)};

// function to compute a square value: x^2
template <class T>
constexpr T sqr(const T &val) { return val * val; }

class Point {
public:
    Point();
    Point(const int, const int);
    Point(const double, const double);
    Point(const Point&);
    Point& operator=(const Point&);
    ~Point() = default;

    int& operator[](const size_t i) { return (i < 1) ? x_ : y_; }
    const int& operator[](const size_t i) const { return (i < 1) ? x_ : y_; }

    int x() const { return x_; }
    int y() const { return y_; }

    double dist_to(const Point &o) const { return sqrt(sqr(x_ - o.x_) +
            sqr(y_ - o.y_)); }
    void draw(PPM_Image&, const PPM_Color& = 255) const;

private:
    int x_;
    int y_;
};

class Shape {
public:
    virtual double length() const = 0; // length, perimeter...
    virtual double area() const = 0;
    virtual void draw(PPM_Image&, const PPM_Color&) const = 0;
    virtual void fill(PPM_Image&, const PPM_Color&) const = 0;
};

class Line: public Shape {
public:
    Line(const Point&, const Point&);
    Line(const Point&, const int, const int);
    Line(const int, const int, const int, const int);
    Line(const Line&);
    Line& operator=(const Line&);
    ~Line() = default;

    double length() const override { return p1_.dist_to(p2_); }
    double area() const override { return 0.0; }
    void draw(PPM_Image&, const PPM_Color& = 255) const override;
    void fill(PPM_Image&, const PPM_Color& = 255) const override;

private:
    Point p1_;
    Point p2_;
};

class Triangle: public Shape {
public:
    Triangle(const Point&, const Point&, const Point&);
    Triangle(const int, const int, const int, const int, const int, const int);
    Triangle(const Triangle&);
    Triangle& operator=(const Triangle&);
    ~Triangle() = default;

    double length() const override;
    double area() const override;
    void draw(PPM_Image&, const PPM_Color& = 255) const override;
    void fill(PPM_Image&, const PPM_Color& = 255) const override;
    void fill_hs(PPM_Image&, const PPM_Color& = 255) const;

private:
    Point p1_;
    Point p2_;
    Point p3_;
};

class Circle: public Shape {
public:
    Circle(const Point&, const size_t);
    Circle(const int, const int, const size_t);
    Circle(const Circle&);
    Circle& operator=(const Circle&);
    ~Circle() = default;

    double length() const override { return pi * (r_ << 1); }
    double area() const override { return pi * sqr(r_); }
    void draw(PPM_Image&, const PPM_Color& = 255) const override;
    void fill(PPM_Image&, const PPM_Color& = 255) const override;

private:
    Point p_;
    size_t r_;
};

#endif

