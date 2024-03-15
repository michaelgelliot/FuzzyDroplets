#ifndef FUZZYDROPLETS_GUI_DROPLETGRAPHWIDGET_H
#define FUZZYDROPLETS_GUI_DROPLETGRAPHWIDGET_H

#include "plot/scatterplotbox2.h"
#include "generic/command.h"

class Data;
class PointCloud;
class CommandStack;

namespace Plot {
class Polygon;
}

class DropletGraphWidget : public Plot::ScatterPlotBox2
{
    Q_OBJECT

    class SetZOrderCommand : public Command<SetZOrderCommand>
    {
    public:

        SetZOrderCommand(Data * data, size_t colorId, size_t oldZ, size_t newZ);

        void redo() override;
        void undo() override;

    private:

        Data * m_data;
        size_t m_colorId;
        size_t m_oldZ;
        size_t m_newZ;
    };

public:

    DropletGraphWidget(Data * data, CommandStack * cmdStack, QWidget * parent = nullptr);

    Data * data() {return m_data;}
    const Data * data() const {return m_data;}

    const PointCloud * pointCloud() const {return m_cloud;}
    PointCloud * pointCloud() {return m_cloud;}

    bool convexHullsVisible() const {return m_paintConvexHulls;}

protected:

    void contextMenuEvent(QContextMenuEvent * e);

public slots:

    void samplesAdded();
    void selectedSamplesChanged();
    void repaintEntireGraph();
    void parallelWorkStarted();
    void parallelWorkFinished();
    void setConvexHullsVisible(bool);
    void updateConvexHullCount();
    void updateConvexHullColors();
    void updateConvexHulls();
    void setMarkerSize(int);
    void setScaleFactor(double d);

private:

    size_t markerFromMousePos(const QPoint & pos) const;

    Data * m_data;
    CommandStack * m_commandStack;
    PointCloud * m_cloud;
    bool m_paintConvexHulls {false};
    QList<Plot::Polygon*> m_convexHulls;

};

#endif // FUZZYDROPLETS_GUI_DROPLETGRAPHWIDGET_H
