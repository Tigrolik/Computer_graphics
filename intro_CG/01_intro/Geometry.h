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
 */

#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include <algorithm>
#include "PPM_Image.h"

template <class T>
constexpr T sqr(const T val) {
    return val * val;
}

class Point; // forward declaration

class Shape {
public:
    virtual double length() const { return 0; } // length, perimeter...
    virtual double area() const { return 0; }
    virtual void draw(PPM_Image&, const PPM_Color&) const = 0;
    virtual void fill(PPM_Image&, const PPM_Color&) const = 0;
    virtual ~Shape() { }
};

class Point: public Shape {
public:
    constexpr Point(): x_{0}, y_{0} { }
    constexpr Point(const int xx, const int yy): x_{xx}, y_{yy} { }
    constexpr Point(const double xd, const double yd): x_(xd), y_(yd) { }
    constexpr Point(const Point &o): x_{o.x()}, y_{o.y()} { }
    Point &operator=(const Point&);

    ~Point() = default;

    constexpr int x() const { return x_; }
    constexpr int y() const { return y_; }

    constexpr double dist_to(const Point &o) const {
        return sqrt(sqr(x_ - o.x_) + sqr(y_ - o.y_));
    }
    void draw(PPM_Image&, const PPM_Color& = 255) const override;
    void fill(PPM_Image&, const PPM_Color& = 255) const override;

private:
    int x_;
    int y_;
};

class Line: public Shape {
public:
    Line(const Point&, const Point&);
    Line(const Point&, const int, const int);
    Line(const int, const int, const int, const int);
    Line(const Line&);
    Line &operator=(const Line&);

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
    Triangle &operator=(const Triangle&);

    ~Triangle() = default;

    double length() const override;
    double area() const override;
    void draw(PPM_Image&, const PPM_Color& = 255) const override;
    void fill(PPM_Image&, const PPM_Color& = 255) const override;

private:
    Point p1_;
    Point p2_;
    Point p3_;
};

#endif

