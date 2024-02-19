#ifndef FUZZYDROPLETS_GUI_CLUSTERINGWIDGET_H
#define FUZZYDROPLETS_GUI_CLUSTERINGWIDGET_H

#include <QWidget>
#include "gui/generic/command.h"
#include "core/fuzzycolor.h"

class StackedWidget;
class LaunchWidget;
class Data;
class DropletGraphWidget;
class SampleListWidget;
class CommandStack;
class PaintingWidget;

class ClusteringWidget : public QWidget
{
    Q_OBJECT

public:

    explicit ClusteringWidget(Data * data, PaintingWidget * painting, DropletGraphWidget * graph, SampleListWidget * sampleList, CommandStack * commandStack, QWidget *parent = nullptr);

public slots:

    void run();
    void cancel();
    void widgetChanged(QWidget * oldWidget, QWidget * newWidget);
    void cleanUpAfterClustering();
    void hideLaunchWidget();
    void showLaunchWidget();
    void parallelWorkStarted();
    void parallelWorkFinished();

signals:

    void startParallelWork();
    void finishParallelWork();
    void showCentroids(bool);

private:

    StackedWidget * m_stackedWidget;
    LaunchWidget * m_launchWidget;
    Data * m_data;
    PaintingWidget * m_painting;
    DropletGraphWidget * m_graph;
    CommandStack * m_commandStack;
    std::vector<FuzzyColor> m_originalColors;
};

#endif // FUZZYDROPLETS_GUI_CLUSTERINGWIDGET_H
