#ifndef NEARESTNEIGHBOURSWIDGET_H
#define NEARESTNEIGHBOURSWIDGET_H

#include "assignmentmethodwidget.h"

class Data;
class QCheckBox;
class QSpinBox;

class NearestNeighboursWorker : public QObject
{
    Q_OBJECT

public:

    NearestNeighboursWorker(Data * data);
    ~NearestNeighboursWorker();

    void setParams(int k, bool weighted, const QList<size_t> & source, const QList<size_t> & target) {m_k = k; m_weighted = weighted; m_sourceIndices = source; m_targetIndices = target;}
    void go();
    void cancel();

signals:

    void finished();
    void updateProgress(int);
    void finishedStep();

public slots:

    void nextStep();

private:

    Data * m_data;
    int m_k;
    bool m_weighted;
    QList<size_t> m_sourceIndices;
    QList<size_t> m_targetIndices;
    size_t m_iterStart {0};
    int m_percent {0};
    QuadTree<size_t> * m_tree;
    bool m_cancel {false};
};

class NearestNeighboursWidget : public AssignmentMethodWidget
{
    Q_OBJECT

public:

    NearestNeighboursWidget(Data * data, QWidget * parent = nullptr);

    virtual bool providesProgressUpdates() const {return true;}
    virtual bool canCancel() const {return true;}
    virtual void run(const QList<size_t> & sourceIndices, const QList<size_t> & targetIndices);
    virtual void cancel();

signals:

    void startAssignment();

public slots:

    void assignmentThreadFinished();

private:

    Data * m_data;
    QCheckBox * m_weightingCheckBox;
    QSpinBox * kSpinBox;

    QThread * assignmentWorkerThread {nullptr};
};

#endif // NEARESTNEIGHBOURSWIDGET_H
