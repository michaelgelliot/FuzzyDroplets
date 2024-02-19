#ifndef RANDOMFORESTWIDGET_H
#define RANDOMFORESTWIDGET_H

#include "assignmentmethodwidget.h"

class Data;
class QSpinBox;

class RandomForestWorker : public QObject
{
    Q_OBJECT

public:

    RandomForestWorker(Data * data);
    ~RandomForestWorker();

    void go();
    void cancel();
    void setParams(int numTrees, const QList<size_t> & source, const QList<size_t> & target) {m_numTrees = numTrees; m_sourceIndices = source; m_targetIndices = target;}

    void useRanger(const std::string & rangerPath, bool connect);

signals:

    void finished();
    void updateProgress(int);

private:

    Data * m_data;
    QList<size_t> m_sourceIndices;
    QList<size_t> m_targetIndices;
    size_t m_iterStart {0};
    int m_percent {0};
    bool m_cancel {false};
    int m_numTrees;
};

class RandomForestWidget : public AssignmentMethodWidget
{
    Q_OBJECT

public:

    RandomForestWidget(Data * data, QWidget * parent = nullptr);

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
    QSpinBox * m_numTreesSpinBox;
};

#endif // RANDOMFORESTWIDGET_H
