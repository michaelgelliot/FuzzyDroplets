#ifndef FUZZY_DROPLETS_GEOMETRY_H
#define FUZZY_DROPLETS_GEOMETRY_H

#include "approximately.h"
#include <cmath>
#include <vector>
#include <functional>
#include <array>

class Point
{
public:

    Point() : m_x(std::numeric_limits<double>::quiet_NaN()), m_y(std::numeric_limits<double>::quiet_NaN()) {}
    Point(double x, double y) : m_x(x), m_y(y) {}

    auto operator<=>(const Point&) const = default;

    Point &	operator*=(double factor) {m_x *= factor; m_y *= factor; return *this;}
    Point operator*(double factor) const {return Point(m_x * factor, m_y * factor);}
    Point &	operator+=(const Point & point) {m_x += point.m_x; m_y += point.m_y; return *this;}
    Point operator+(const Point & point) const {return Point(m_x + point.m_x, m_y + point.m_y);}
    Point &	operator-=(const Point & point) {m_x -= point.m_x; m_y -= point.m_y; return *this;}
    Point operator-(const Point & point) const {return Point(m_x - point.m_x, m_y - point.m_y);}
    Point &	operator/=(double divisor) {m_x /= divisor; m_y /= divisor; return *this;}
    Point operator/(double divisor) const {return Point(m_x / divisor, m_y / divisor);}
    Point operator/(size_t divisor) const {return Point(m_x / divisor, m_y / divisor);}

    double x() const {return m_x;}
    double y() const {return m_y;}
    void setX(double x) {m_x = x;}
    void setY(double y) {m_y = y;}
    double & rx() {return m_x;}
    double & ry() {return m_y;}

    double distanceTo(const Point & other) const {return sqrt(squaredDistanceTo(other));}
    double squaredDistanceTo(const Point & other) const {return pow(other.m_x - m_x, 2) + pow(other.m_y - m_y, 2);}

    double distanceTo(const Point & other, double xScale, double yScale) const {return sqrt(squaredDistanceTo(other, xScale, yScale));}
    double squaredDistanceTo(const Point & other, double xScale, double yScale) const {return pow(xScale * (other.m_x - m_x), 2) + pow(yScale * (other.m_y - m_y), 2);}

    void transpose() {std::swap(m_x, m_y);}
    Point transposed() const {return Point{m_y, m_x};}

    bool isValid() const {return !std::isnan(m_x) && !std::isnan(m_y);}

private:

    double m_x;
    double m_y;
};

class Line
{
public:

    Line() {}
    Line(const Point & a, const Point & b) : m_point1{a}, m_point2{b} {}

    auto operator<=>(const Line&) const = default;

    double x1() const {return m_point1.x();}
    double x2() const {return m_point2.x();}
    double y1() const {return m_point1.y();}
    double y2() const {return m_point2.y();}

    void setLine(double x1, double y1, double x2, double y2)
    {
        m_point1.setX(x1);
        m_point1.setY(y1);
        m_point2.setX(x2);
        m_point2.setY(y2);
    }

    Point p1() const {return m_point1;}
    Point p2() const {return m_point2;}

    void setLine(const Point & p1, const Point & p2)
    {
        m_point1 = p1;
        m_point2 = p2;
    }

    double dx() const {return  x2() - x1();}
    double dy() const {return y2() - y1();}
    double gradient() const {return dy() / dx();}
    double intercept() const {return y1() - gradient() * x1();}

    double length() const {return sqrt(squaredLength());}
    double squaredLength() const {return pow(dx(), 2) + pow(dy(), 2);}
    double manhattanLength() const {return std::abs(dx()) + std::abs(dy());}

    Point intersectionWith(const Line & other) const
    {
        double s = (-dy() * (x1() - other.x1()) + dx() * (y1() - other.y1())) / (-other.dx() * dy() + dx() * other.dy());
        double t = ( other.dx() * (y1() - other.y1()) - other.dy() * (x1() - other.x1())) / (-other.dx() * dy() + dx() * other.dy());
        return (approximately::inRange(s, 0.0, 1.0) && approximately::inRange(t, 0.0, 1.0)) ? Point(m_point1.x() + (t * dx()), m_point1.y() + (t * dy())) : Point();
    }

    Point unboundedIntersectionWith(const Line & other) const
    {
        auto x = (gradient() - other.gradient());
        if (x != 0) {
            x = (other.intercept() - intercept()) / x;
            return Point(x, gradient() * x + intercept());
        }
        return Point();
    }

    bool isAbove(const Point & p) const
    {
        return approximately::lessThan(p.y(), (y2() * (x1() - p.x()) + y1() * (p.x() - x2())) / (x1() - x2()));
    }

    bool isRightOf(const Point & p) const
    {
        return approximately::lessThan(p.x(), (x2() * (y1() - p.y()) + x1() * (p.y() - y2())) / (y1() - y2()));
    }

    bool contains(const Point & p) const
    {
        return approximately::inRange(p.x(), x1(), x2()) && approximately::inRange(p.y(), y1(), y2()) && approximately::equalsZero(x2() * p.y() - y2() * p.x() + y1() * (p.x() - x2()) + x1() * (y2() - p.y()));
    }

    double distanceTo(const Point & p) const
    {
        return fabs((m_point2.x() - m_point1.x()) * (m_point1.y() - p.y()) - (m_point2.y() - m_point1.y()) * (m_point1.x() - p.x())) / m_point1.distanceTo(m_point2);
    }

    static bool isAbove(const Point & p, const Point & a, const Point & b)
    {
        return approximately::lessThan(p.y(), (b.y() * (a.x() - p.x()) + a.y() * (p.x() - b.x())) / (a.x() - b.x()));
    }

    static bool isRightOf(const Point & p, const Point & a, const Point & b)
    {
        return approximately::lessThan(p.x(), (b.x() * (a.y() - p.y()) + a.x() * (p.y() - b.y())) / (a.y() - b.y()));
    }

    static bool contains(const Point & p, const Point & a, const Point b)
    {
        return approximately::inRange(p.x(), a.x(), b.x()) && approximately::inRange(p.y(), a.y(), b.y()) && approximately::equalsZero(b.x() * p.y() - b.y() * p.x() + a.y() * (p.x() - b.x()) + a.x() * (b.y() - p.y()));
    }

private:

    Point m_point1;
    Point m_point2;
};

class Polygon
{
public:

    Polygon() {}

    Polygon(std::initializer_list<Point> points) : m_points(points)
    {
    }

    Polygon(const std::vector<Point> & points) : m_points(points)
    {
    }

    Polygon(std::vector<Point> && points) : m_points(std::move(points))
    {
    }

    auto operator<=>(const Polygon&) const = default;

    bool contains(const Point & p) const
    {
        bool c = false;
        for (size_t i = 0, j = m_points.size() - 1; i < m_points.size(); j = i++) {
            if (approximately::inRange(p.x(), m_points[i].x(), m_points[j].x())) {
                if (Line::contains(p, m_points[i], m_points[j])) return true;
                else if (!approximately::equals(m_points[i].x(), p.x()) && Line::isAbove(p, m_points[i], m_points[j])) c = !c;
            }
        }
        return c;
    }

    double area() const
    {
        double x = 0;
        for (size_t i = 0, j = m_points.size() - 1; i < m_points.size(); j = i++)
            x += (m_points[i].x() - m_points[j].x()) * (m_points[i].y() + m_points[j].y());
        return x / 2;
    }

    static Polygon convexHull(const std::vector<Point> & data)
    {
        if (data.empty()) {
            return Polygon();
        } else {
            std::function<void(const std::vector<Point>&, const Point&, const Point&, std::vector<Point>&)> quickHull = [&](const std::vector<Point>& v, const Point& a, const Point& b, std::vector<Point>& hull)
            {
                if (v.empty())
                    return;

                size_t idxMax = 0;
                double distMax = Line(a,b).distanceTo(v[idxMax]);
                for (size_t i = 1; i < v.size(); ++i) {
                    double distCurr = Line(a,b).distanceTo(v[i]);
                    if (distCurr > distMax) {
                        idxMax = i;
                        distMax = distCurr;
                    }
                }
                Point f = v[idxMax];

                std::vector<Point> left;
                for (auto & p : v) {
                    if ((f.x() - a.x()) * (p.y() - a.y()) - (f.y() - a.y()) * (p.x() - a.x()) > 0) {
                        left.push_back(p);
                    }
                }
                quickHull(left, a, f, hull);
                hull.push_back(f);
                std::vector<Point> right;
                for (auto & p : v) {
                    if ((b.x() - f.x()) * (p.y() - f.y()) - (b.y() - f.y()) * (p.x() - f.x()) > 0) {
                        right.push_back(p);
                    }
                }
                quickHull(right, f, b, hull);
            };

            std::vector<Point> hull;
            if (data.size() <= 2) return Polygon(hull);

            Point a = *min_element(data.begin(), data.end(), [](const Point & a, const Point & b) {return (a.x() < b.x() || (a.x() == b.x() && a.y() < b.y()));});
            Point b = *max_element(data.begin(), data.end(), [](const Point & a, const Point & b) {return (a.x() < b.x() || (a.x() == b.x() && a.y() < b.y()));});

            std::vector<Point> left, right;
            for (auto & p : data)
                ((b.x() - a.x()) * (p.y() - a.y()) - (b.y() - a.y()) * (p.x() - a.x())) > 0 ? left.push_back(p) : right.push_back(p);

            hull.push_back(a);
            quickHull(left, a, b, hull);
            hull.push_back(b);
            quickHull(right, b, a, hull);
            return Polygon(hull);
        }
    }

    const std::vector<Point> points() const
    {
        return m_points;
    }

private:

    std::vector<Point> m_points;
};

class OrthogonalRectangle
{
public:

    OrthogonalRectangle()
    {}

    OrthogonalRectangle(Point bottomLeft, double width, double height)
        : m_x(bottomLeft.x()), m_y(bottomLeft.y()), m_w(width), m_h(height)
    {}

    OrthogonalRectangle(Point bottomLeft, Point topRight)
        : m_x(bottomLeft.x()), m_y(bottomLeft.y()), m_w(topRight.x() - bottomLeft.x()), m_h(topRight.y() - bottomLeft.y())
    {}

    OrthogonalRectangle boundingBox(const OrthogonalRectangle & other) const
    {
        return OrthogonalRectangle( {std::min(left(), other.left()), std::min(bottom(), other.bottom())}, {std::max(right(), other.right()), std::max(top(), other.top())} );
    }

    template <typename Data, typename XAccessor, typename YAccessor>
    static OrthogonalRectangle boundingBox(const Data & data, XAccessor xAccessor, YAccessor yAccessor)
    {
        if (data.empty()) {
            return OrthogonalRectangle();
        } else {
            OrthogonalRectangle rect({xAccessor(data[0]), yAccessor(data[0])}, 0, 0);
            std::for_each(data.begin(), data.end(), [&](const auto & point) {
                if (xAccessor(point) < rect.m_x) {
                    rect.m_w += rect.m_x - xAccessor(point);
                    rect.m_x = xAccessor(point);
                } else if (xAccessor(point) > rect.m_x + rect.m_w) {
                    rect.m_w = xAccessor(point) - rect.m_x;
                }
                if (yAccessor(point) < rect.m_y) {
                    rect.m_h += rect.m_y - yAccessor(point);
                    rect.m_y = yAccessor(point);
                } else if (yAccessor(point) > rect.m_y + rect.m_h) {
                    rect.m_h = yAccessor(point) - rect.m_y;
                }
            });
            return rect;
        }
    }

    auto operator<=>(const OrthogonalRectangle&) const = default;

    inline bool contains(double x, double y) const
    {
        return x >= m_x && x <= m_x + m_w && y >= m_y && y <= m_y + m_h;
    }

    inline bool contains(const Point p) const
    {
        return p.x() >= m_x && p.x() <= m_x + m_w && p.y() >= m_y && p.y() <= m_y + m_h;
    }

    inline double area() const
    {
        return m_w * m_h;
    }

    void clip(const OrthogonalRectangle & r)
    {
        double R = std::min(r.right(), m_x + m_w);
        double T = std::min(r.top(), m_y + m_h);
        m_x = std::max(r.left(), m_x);
        m_y = std::max(r.bottom(), m_y);;
        m_w = R - m_x;
        m_h = T - m_y;
    }

    inline double shortestSquaredDistanceFromPoint(double x, double y) const
    {
        return pow(std::max(m_x - x, std::max(0.0, x - m_x - m_w)), 2)
               + pow(std::max(m_y - y, std::max(0.0, y - m_y - m_h)), 2);
    }

    inline double longestSquaredDistanceFromPoint(double x, double y) const
    {
        return std::max(pow(m_x - x, 2) + pow(m_y - y, 2),
                        std::max(pow(m_x + m_w - x, 2) + pow(m_y - y, 2),
                                 std::max(pow(m_x - x, 2) + pow(m_y + m_h - y, 2),
                                          pow(m_x + m_w - x, 2) + pow(m_y + m_h - y, 2))));
    }

    inline double shortestSquaredDistanceFromPoint(double x, double y, double xScale, double yScale) const
    {
        return pow(std::max(xScale * (m_x - x), std::max(0.0, xScale * (x - m_x - m_w))), 2)
               + pow(std::max(yScale * (m_y - y), std::max(0.0, yScale * (y - m_y - m_h))), 2);
    }

    inline double longestSquaredDistanceFromPoint(double x, double y, double xScale, double yScale) const
    {
        return std::max(pow(xScale * (m_x - x), 2) + pow(yScale * (m_y - y), 2),
                        std::max(pow(xScale * (m_x + m_w - x), 2) + pow(yScale * (m_y - y), 2),
                                 std::max(pow(xScale * (m_x - x), 2) + pow(yScale * (m_y + m_h - y), 2),
                                          pow(xScale * (m_x + m_w - x), 2) + pow(yScale * (m_y + m_h - y), 2))));
    }

    inline bool overlapsWith(const OrthogonalRectangle & other) const
    {
        return (m_x <= other.m_x + other.m_w
                && m_x + m_w >= other.m_x
                && m_y + m_h >= other.m_y
                && m_y <= other.m_y + other.m_h);
    }

    inline bool isContainedBy(const OrthogonalRectangle & other) const
    {
        return m_x >= other.m_x
               && m_x + m_w <= other.m_x + other.m_w
               && m_y >= other.m_y
               && m_y + m_h <= other.m_y + other.m_h;
    }

    inline double top() const {return m_y + m_h;}
    inline double bottom() const {return m_y;}
    inline double left() const {return m_x;}
    inline double right() const {return m_x + m_w;}
    inline double width() const {return m_w;}
    inline double height() const {return m_h;}

    inline Point topLeft() const {return {m_x, m_y + m_h};}
    inline Point topRight() const {return {m_x + m_w, m_y + m_h};}
    inline Point bottomLeft() const {return {m_x, m_y};}
    inline Point bottomRight() const {return {m_x + m_w, m_y};}
    inline Point center() const {return {m_x + m_w/2, m_y + m_h/2};}

    inline bool isValid() const {return m_w > 0 && m_y > 0;}

    inline void translate(double x, double y)
    {
        m_x += x;
        m_y += y;
    }

    inline void scale(double factor, Point origin)
    {
        m_x = origin.x() + (m_x - origin.x()) * factor;
        m_y = origin.y() + (m_y - origin.y()) * factor;
        m_w *= factor;
        m_h *= factor;
    }

    void setWidth(double w) {m_w = w;}
    void setHeight(double h) {m_h = h;}
    void setLeft(double left) {m_x = left;}
    void setBottom(double bottom) {m_y = bottom;}

private:

    double m_x {0};
    double m_y {0};
    double m_w {0};
    double m_h {0};
};

#endif // FUZZY_DROPLETS_GEOMETRY_H
