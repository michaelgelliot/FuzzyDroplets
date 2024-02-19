#ifndef GMMASSIGNMENTWIDGET_H
#define GMMASSIGNMENTWIDGET_H

#include "assignmentmethodwidget.h"
#include "core/binormal.h"

class Data;
class QCheckBox;

class GmmAssignmentWorker : public QObject
{
    Q_OBJECT

public:

    GmmAssignmentWorker(Data * data);
    ~GmmAssignmentWorker();

    void go();
    void cancel();
    void setParams(bool sharedScale, bool sharedRho, const QList<size_t> & source, const QList<size_t> & target) {m_sharedScale = sharedScale; m_sharedRho = sharedRho; m_sourceIndices = source; m_targetIndices = target;}

signals:

    void finished();
    void updateProgress(int);
    void finishedStep();

public slots:

    void nextStep();

private:

    Data * m_data;
    QList<size_t> m_sourceIndices;
    QList<size_t> m_targetIndices;
    size_t m_iterStart {0};
    int m_percent {0};
    bool m_cancel {false};
    bool m_sharedScale {false};
    bool m_sharedRho {false};
    std::vector<BinormalDistribution> m_distributions;
    std::vector<double> m_alpha;
};

class GmmAssignmentWidget : public AssignmentMethodWidget
{
    Q_OBJECT

public:

    GmmAssignmentWidget(Data * data, QWidget * parent = nullptr);

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
    QThread * assignmentWorkerThread {nullptr};
    QCheckBox * m_sharedScaleCB;
    QCheckBox * m_sharedRhoCB;
};

#endif // GMMASSIGNMENTWIDGET_H
