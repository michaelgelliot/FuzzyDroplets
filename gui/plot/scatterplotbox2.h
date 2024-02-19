#ifndef FUZZYDROPLETS_GUI_PLOT_SCATTERPLOTBOX2_H
#define FUZZYDROPLETS_GUI_PLOT_SCATTERPLOTBOX2_H

#include "boxgraphbase.h"
#include "../../core/geometry.h"

class QPropertyAnimation;

namespace Plot
{

class ContinuousAxis;

class ScatterPlotBox2 : public BoxGraphBase
{
    Q_OBJECT

public:

    Q_PROPERTY(QRectF rangeRectF READ rangeRectF WRITE setRange);

    ScatterPlotBox2(QWidget *parent = nullptr);

    const ContinuousAxis * horizontalAxis() const {return (ContinuousAxis*)BoxGraphBase::bottomAxis();}
    const ContinuousAxis * verticalAxis() const {return (ContinuousAxis*)BoxGraphBase::leftAxis();}

    ContinuousAxis * horizontalAxis() {return (ContinuousAxis*)BoxGraphBase::bottomAxis();}
    ContinuousAxis * verticalAxis() {return (ContinuousAxis*)BoxGraphBase::leftAxis();}

    double horizontalUnit() const;
    double verticalUnit() const;

    void setRange(double xMin, double xMax, double yMin, double yMax);
    void setRange(OrthogonalRectangle rect);
    void setRange(QRectF rect);
    void setAbsoluteRange(double xMin, double xMax, double yMin, double yMax);
    OrthogonalRectangle range() const;
    QRectF rangeRectF() const;
    void setupFromDataRange(double xMin, double xMax, double yMin, double yMax);
    void setupFromDataRange(OrthogonalRectangle rect);

    void setZoomPanEnabled(bool enabled) {m_zoomPanEnabled = enabled;}
    bool zoomPanEnabled() const {return m_zoomPanEnabled;}

protected:

    OrthogonalRectangle visibleBounds() const;
    OrthogonalRectangle constrainedVisibleBounds(const OrthogonalRectangle & visibleBounds) const;
    void wheelEvent (QWheelEvent * e);

public slots:

    void resetZoom();
    void zoomIn();
    void zoomOut();

private:

    QRectF calcZoomedViewport(double factor);

    QPropertyAnimation * m_zoomAnimation;
    bool m_zoomPanEnabled {true};
    QPixmap m_staticPixmap;
};

} // namespace Plot

#endif // FUZZYDROPLETS_GUI_PLOT_SCATTERPLOTBOX2_H
