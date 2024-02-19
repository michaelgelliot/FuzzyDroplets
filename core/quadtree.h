#ifndef FUZZY_DROPLETS_QUADTREE_HPP
#define FUZZY_DROPLETS_QUADTREE_HPP

#include <vector>
#include <numeric>
#include <algorithm>
#include <functional>
#include "geometry.h"
#include "median.h"
#include "vectorqueue.h"
#include "sortedvector.h"

template <typename T>
class QuadTree
{

public:

    struct Node
    {
        Node(size_t first, size_t last, OrthogonalRectangle boundingBox)
            : begin(first), end(last), rect(boundingBox)
        {}

        bool isTip() const {return ne == 0;}

        size_t ne {0};
        size_t se {0};
        size_t sw {0};
        size_t nw {0};
        size_t begin {0};
        size_t end {0};
        OrthogonalRectangle rect;
    };

    template <typename Data>
    auto kNearestNeighbors(const Data & container, size_t k, double x, double y, double xScale = 1, double yScale = 1, const std::function<bool(size_t)> & filter = [](size_t t){return true;}) const
    {

        auto compare = [](const std::pair<size_t, double> & a, const std::pair<size_t, double> & b){return a.second < b.second;};
        SortedVector<std::pair<size_t, double>, decltype(compare)> result;

        if (container.size() == 0 || m_nodes.size() == 0) return result;

        auto target = Point(x,y);
        VectorQueue<size_t> Q{0};
        while (!Q.empty()) {
            auto node = Q.pop();
            if (m_nodes[node].isTip()) {
                for (size_t i = m_nodes[node].begin; i < m_nodes[node].end; ++i) {
                    if (result.size() < k) {
                        result.insert({m_indices[i], Point(m_x(container[m_indices[i]]), m_y(container[m_indices[i]])).squaredDistanceTo(target, xScale, yScale)});
                    } else {
                        double d = Point(m_x(container[m_indices[i]]), m_y(container[m_indices[i]])).squaredDistanceTo(target, xScale, yScale);
                        if (d < result.back().second) {
                            result.insert({m_indices[i], d});
                            result.pop_back();
                        }
                    }
                }
            } else {
                if (result.size() < k) {
                    Q.push(m_nodes[node].nw);
                    Q.push(m_nodes[node].ne);
                    Q.push(m_nodes[node].se);
                    Q.push(m_nodes[node].sw);
                } else {
                    double closest = m_nodes[m_nodes[node].nw].rect.shortestSquaredDistanceFromPoint(target.x(), target.y(), xScale, yScale);
                    if (closest < result.back().second) Q.push(m_nodes[node].nw);
                    closest = m_nodes[m_nodes[node].ne].rect.shortestSquaredDistanceFromPoint(target.x(), target.y(), xScale, yScale);
                    if (closest < result.back().second) Q.push(m_nodes[node].ne);
                    closest = m_nodes[m_nodes[node].se].rect.shortestSquaredDistanceFromPoint(target.x(), target.y(), xScale, yScale);
                    if (closest < result.back().second) Q.push(m_nodes[node].se);
                    closest = m_nodes[m_nodes[node].sw].rect.shortestSquaredDistanceFromPoint(target.x(), target.y(), xScale, yScale);
                    if (closest < result.back().second) Q.push(m_nodes[node].sw);
                }
            }
        }

        return result;
    }

    template <typename Data>
    std::pair<size_t, double> nearestNeighbor(const Data & container, double x, double y, double xScale = 1, double yScale = 1, const std::function<bool(size_t)> & filter = [](size_t t){return true;}) const
    {
        if (container.size() == 0 || m_nodes.size() == 0) return {-1,-1};

        size_t result = -1;
        double distance = std::numeric_limits<double>::max();
        auto target = Point(x,y);
        VectorQueue<size_t> Q{0};
        while (!Q.empty()) {
            auto node = Q.pop();
            double closest = m_nodes[node].rect.shortestSquaredDistanceFromPoint(target.x(), target.y(), xScale, yScale);

            if (closest < distance) {

                if (m_nodes[node].isTip()) {
                    for (size_t i = m_nodes[node].begin; i < m_nodes[node].end; ++i) {
                        double d = Point(m_x(container[m_indices[i]]), m_y(container[m_indices[i]])).squaredDistanceTo(target, xScale, yScale);
                        if (d <= distance && filter(m_indices[i])) {
                            distance = d;
                            result = m_indices[i];
                        }
                    }
                } else {
                    closest = m_nodes[m_nodes[node].nw].rect.shortestSquaredDistanceFromPoint(target.x(), target.y(), xScale, yScale);
                    if (closest <= distance) {
                        Q.push(m_nodes[node].nw);
                    }
                    closest = m_nodes[m_nodes[node].ne].rect.shortestSquaredDistanceFromPoint(target.x(), target.y(), xScale, yScale);
                    if (closest <= distance) {
                        Q.push(m_nodes[node].ne);
                    }
                    closest = m_nodes[m_nodes[node].sw].rect.shortestSquaredDistanceFromPoint(target.x(), target.y(), xScale, yScale);
                    if (closest <= distance) {
                        Q.push(m_nodes[node].sw);
                    }
                    closest = m_nodes[m_nodes[node].se].rect.shortestSquaredDistanceFromPoint(target.x(), target.y(), xScale, yScale);
                    if (closest <= distance) {
                        Q.push(m_nodes[node].se);
                    }
                }
            }
        }

        return {result, distance};
    }

    template <typename Data>
    std::vector<size_t> rectangleSearch(const Data & container, OrthogonalRectangle rect, const std::function<bool(size_t)> & filter = [](size_t t){return true;}, size_t reserve = 100) const
    {

        std::vector<size_t> results;
        if (container.size() == 0 || m_nodes.size() == 0) return results;

        results.reserve(reserve);
        VectorQueue<size_t> Q{0};
        while (!Q.empty()) {
            auto node = Q.pop();
            if (m_nodes[node].rect.isContainedBy(rect)) {
                for (size_t i = m_nodes[node].begin; i < m_nodes[node].end; ++i) {
                    if (filter(m_indices[i]))
                        results.push_back(m_indices[i]);
                }
            } else {
                if (m_nodes[node].isTip()) {
                    for (size_t i = m_nodes[node].begin; i < m_nodes[node].end; ++i) {
                        if (m_x(container[m_indices[i]]) >= rect.left() && m_x(container[m_indices[i]]) <= rect.right() && m_y(container[m_indices[i]]) >= rect.bottom() && m_y(container[m_indices[i]]) <= rect.top() && filter(m_indices[i])) {
                            results.push_back(m_indices[i]);
                        }
                    }
                } else {
                    if (m_nodes[m_nodes[node].sw].rect.overlapsWith(rect)) Q.push(m_nodes[node].sw);
                    if (m_nodes[m_nodes[node].nw].rect.overlapsWith(rect)) Q.push(m_nodes[node].nw);
                    if (m_nodes[m_nodes[node].se].rect.overlapsWith(rect)) Q.push(m_nodes[node].se);
                    if (m_nodes[m_nodes[node].ne].rect.overlapsWith(rect)) Q.push(m_nodes[node].ne);
                }
            }
        }

        return results;
    }

    template <typename Data, typename XAccessor, typename YAccessor>
    QuadTree(const Data & data, XAccessor xAccessor, YAccessor yAccessor, size_t maxLeafSize = 20) :
        m_indices(data.size()),
        m_x(xAccessor),
        m_y(yAccessor)
    {
        if (data.size() == 0) return;

        std::iota(m_indices.begin(), m_indices.end(), 0);
        m_nodes.reserve(data.size() / maxLeafSize);
        m_nodes.emplace_back(0, data.size(), OrthogonalRectangle::boundingBox(data, xAccessor, yAccessor));
        size_t i = 0;
        while (i < m_nodes.size()) {
            if (m_nodes[i].end - m_nodes[i].begin > maxLeafSize) {
                double xPivot = m_x(data[*medianOfMedians(m_indices.begin() + m_nodes[i].begin, m_indices.begin() + m_nodes[i].end, 10, [&](const auto & left, const auto & right) {return m_x(data[left]) < m_x(data[right]);})]);
                double yPivot = m_y(data[*medianOfMedians(m_indices.begin() + m_nodes[i].begin, m_indices.begin() + m_nodes[i].end, 10, [&](const auto & left, const auto & right) {return m_y(data[left]) < m_y(data[right]);})]);
                auto yBound = std::partition(m_indices.begin() + m_nodes[i].begin, m_indices.begin() + m_nodes[i].end, [&](size_t k) {return m_y(data[k]) < yPivot;});
                auto xBound1 = std::partition(m_indices.begin() + m_nodes[i].begin, yBound, [&](size_t k) {return m_x(data[k]) < xPivot;});
                auto xBound2 = std::partition(yBound, m_indices.begin() + m_nodes[i].end, [&](size_t k) {return m_x(data[k]) < xPivot;});
                m_nodes[i].sw = m_nodes.size();
                m_nodes[i].se = m_nodes.size() + 1;
                m_nodes[i].nw = m_nodes.size() + 2;
                m_nodes[i].ne = m_nodes.size() + 3;
                m_nodes.emplace_back(m_nodes[i].begin, xBound1 - m_indices.begin(), OrthogonalRectangle(m_nodes[i].rect.bottomLeft(), {xPivot, yPivot}));
                m_nodes.emplace_back(xBound1 - m_indices.begin(), yBound - m_indices.begin(), OrthogonalRectangle({xPivot,  m_nodes[i].rect.bottom()}, {m_nodes[i].rect.right(), yPivot}));
                m_nodes.emplace_back(yBound - m_indices.begin(), xBound2 - m_indices.begin(), OrthogonalRectangle({m_nodes[i].rect.left(), yPivot}, {xPivot, m_nodes[i].rect.top()}));
                m_nodes.emplace_back(xBound2 - m_indices.begin(), m_nodes[i].end, OrthogonalRectangle({xPivot, yPivot}, m_nodes[i].rect.topRight()));
            }
            ++i;
        }
        m_nodes.shrink_to_fit();
    }

    void test() const
    {
        for (int i = 0; i < m_nodes.size(); ++i) {
            if (m_nodes[i].isTip()) {
                qDebug() << m_nodes[i].rect.width() << m_nodes[i].rect.height() << m_nodes[i].end - m_nodes[i].begin;
            }
        }
    }

    template <typename Data>
    void setKMeansLabels(const Data & container, const std::vector<Point> & centroids, std::function<void(size_t index, size_t color)> setLabel, std::function<bool(size_t)> filter) const
    {
        VectorQueue<size_t> Q{0};
        while (!Q.empty()) {

            auto node = Q.pop();

            if (m_nodes[node].isTip()) {

                for (size_t i = m_nodes[node].begin; i < m_nodes[node].end; ++i) {
                    if (filter(m_indices[i])) {
                        size_t closest = 0;
                        double len = centroids[0].squaredDistanceTo({m_x(container[m_indices[i]]), m_y(container[m_indices[i]])});
                        for (size_t j = 1; j < centroids.size(); ++j) {
                            double L = centroids[j].squaredDistanceTo({m_x(container[m_indices[i]]), m_y(container[m_indices[i]])});
                            if (L < len) {
                                len = L;
                                closest = j;
                            }
                        }
                        setLabel(m_indices[i], closest);
                    }
                }

            } else {

                size_t good {0};
                size_t bad {0};
                for (int i = 0; i < centroids.size(); ++i) {
                    double longest = m_nodes[node].rect.longestSquaredDistanceFromPoint(centroids[i].x(), centroids[i].y());
                    double shortest = longest * 2;
                    for (int j = 0 ; j < centroids.size(); ++j) {
                        if (i != j) {
                            shortest = std::min(shortest, m_nodes[node].rect.shortestSquaredDistanceFromPoint(centroids[j].x(), centroids[j].y()));
                        }
                        if (shortest < longest) {
                            break;
                        }
                    }
                    if (shortest < longest) {
                        ++bad;
                    } else {
                        good = i;
                    }
                }
                if (bad == centroids.size() - 1) {
                    for (size_t i = m_nodes[node].begin; i < m_nodes[node].end; ++i) {
                        if (filter(m_indices[i]))
                            setLabel(m_indices[i], good);
                    }
                } else {
                    Q.push(m_nodes[node].nw);
                    Q.push(m_nodes[node].ne);
                    Q.push(m_nodes[node].se);
                    Q.push(m_nodes[node].sw);
                }
            }
        }
    }

private:

    std::vector<size_t> m_indices;
    std::vector<Node> m_nodes;
    std::function<double(T)> m_x;
    std::function<double(T)> m_y;
};

#endif // FUZZY_DROPLETS_QUADTREE_HPP
