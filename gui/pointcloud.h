#ifndef FUZZYDROPLETS_GUI_FASTPOINTCLOUD_H
#define FUZZYDROPLETS_GUI_FASTPOINTCLOUD_H

#include "plot/continuousaxis.h"
#include "core/geometry.h"

class Data;
template <typename T> class QuadTree;

class PointCloud : public Plot::Primitive
{
public:

    PointCloud(Data * data, Plot::ContinuousAxis * xAxis, Plot::ContinuousAxis * yAxis);

    void render(QPainter & painter);

    void setScaleFactor(double scaleFactor) {m_scaleFactor = std::clamp(scaleFactor, 0.0, 1.0);}
    double scaleFactor() const {return m_scaleFactor;}

    void setBaseSize(double d) {m_baseSize = d;}
    double baseSize() const {return m_baseSize;}
    double maxMarkerSize() const {return m_baseSize * (1.0 + m_scaleFactor * (m_xAxis->absoluteValueLength() / m_xAxis->valueLength() - 1.0));}

    void setQuadTree(const QuadTree<Point> * tree) {m_quadTree = tree;}

    void setRoundSvgMarkers(bool b) {m_roundSVGMarkers = b;}

private:

    inline void drawSquare(int S, int S2, size_t x, size_t y, QRgb rgb, std::vector<QRgb> & pixels);

    Plot::ContinuousAxis * m_xAxis;
    Plot::ContinuousAxis * m_yAxis;
    Data * m_data;

    std::vector<size_t> m_iota;
    double m_scaleFactor {0};
    double m_baseSize {2};

    int viewWidth{0};
    int viewHeight{0};
    std::vector<QRgb> m_pixelData;

    const QuadTree<Point> * m_quadTree {nullptr};

    bool m_roundSVGMarkers {false};
};


#endif // FUZZYDROPLETS_GUI_FASTPOINTCLOUD_H
