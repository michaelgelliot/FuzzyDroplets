#ifndef FUZZYDROPLETS_GUI_ASSIGNMENTWIDGET_H
#define FUZZYDROPLETS_GUI_ASSIGNMENTWIDGET_H

#include <QWidget>
#include "../core/fuzzycolor.h"

class StackedWidget;
class LaunchWidget;
class Data;
class DropletGraphWidget;
class SampleListWidget;
class CommandStack;
class QComboBox;
class QCheckBox;
class QGroupBox;
class PaintingWidget;

class AssignmentWidget : public QWidget
{
    Q_OBJECT

public:

    explicit AssignmentWidget(Data * data, PaintingWidget * painting, DropletGraphWidget * graph, SampleListWidget * sampleList, CommandStack * commandStack, QWidget *parent = nullptr);

    QList<size_t> sourceIndices() const;
    QList<size_t> targetIndices() const;

public slots:

    void run();
    void cancel();
    void widgetChanged(QWidget * oldWidget, QWidget * newWidget);
    void hideLaunchWidget();
    void showLaunchWidget();
    void updateLaunchStatus();
    void startAssignmentActions();
    void endAssignmentActions();
    void sampleTypesChanged(std::vector<size_t> samples);
    void selectedSamplesChanged();

    void parallelWorkStarted();
    void parallelWorkFinished();

signals:

    void startParallelWork();
    void finishParallelWork();

private:

    bool hasMixedTypes();

    StackedWidget * m_stackedWidget;
    LaunchWidget * m_launchWidget;
    Data * m_data;
    PaintingWidget * m_painting;
    DropletGraphWidget * m_graph;
    CommandStack * m_commandStack;
    std::vector<FuzzyColor> m_originalColors;
    QComboBox * m_targetComboBox;
    QCheckBox * m_posCB;
    QCheckBox * m_negCB;
    QCheckBox * m_ntcCB;
    QCheckBox * m_unambCB;
    QGroupBox * m_sourceTargetGroup;
};

#endif // FUZZYDROPLETS_GUI_ASSIGNMENTWIDGET_H
