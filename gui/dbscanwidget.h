#ifndef DBSCANWIDGET_H
#define DBSCANWIDGET_H

#include "gui/clustermethodwidget.h"
#include "core/vectorqueue.h"

class Data;
class DropletGraphWidget;
class QSpinBox;
class QFormLayout;

class DBScanWorker : public QObject
{
    Q_OBJECT

    static const size_t Undefined = std::numeric_limits<size_t>::max();


public:

    DBScanWorker(Data * data, double epsilon, int minPts, QObject * parent = nullptr);

    void increaseCompletedPointCount();

public slots:

    void go();
    void cancel();
    void proceed();

signals:

    void finished();
    void keepGoing();
    void updateProgress(int);

private:

    double m_epsilon;
    int m_minPts;
    double m_epsilon2;
    Data * m_data;
    bool m_cancelled {false};
    int m_completedPointCount {0};
    int m_currentCluster {0};
    int m_percent;
    size_t m_curPos {0};
    std::vector<size_t> m_assignment;
    size_t m_selectedPointCount {0};
};

class DBScanWidget : public ClusterMethodWidget
{
    Q_OBJECT

public:

    explicit DBScanWidget(Data * data, DropletGraphWidget * graph, QWidget *parent = nullptr);
    virtual ~DBScanWidget();

    virtual bool providesProgressUpdates() const override {return true;}
    virtual bool canCancel() const override {return true;}

    virtual void run() override;
    virtual void cancel() override;

public slots:

    void threadFinished();

signals:

    void startThread();
    void cancelThread();

protected:

    Data * m_data;
    DropletGraphWidget * m_graph;

    QSpinBox * m_epsilon;
    QSpinBox * m_minPts;

    QThread * workerThread {nullptr};
};

#endif // DBSCANWIDGET_H
