#ifndef FUZZYDROPLETS_GUI_ASSIGNMENTMETHODWIDGET_H
#define FUZZYDROPLETS_GUI_ASSIGNMENTMETHODWIDGET_H

#include <QWidget>
#include "core/geometry.h"
#include "core/quadtree.h"

class Droplet;
class Data;

class AssignmentMethodWidget : public QWidget
{
    Q_OBJECT

public:

    AssignmentMethodWidget(QWidget *parent = nullptr);
    virtual ~AssignmentMethodWidget();

    virtual void run(const QList<size_t> & sourceIndices, const QList<size_t> & targetIndices) {};
    virtual void cancel() {}

    virtual bool requiresLaunchWidget() const {return true;}        // reimplement to request no launch widget, continuous updates instead
    virtual bool providesProgressUpdates() const {return false;}    // reimplement to request a progress bar
    virtual bool canCancel() const {return false;}                  // reimplement to request a cancel button while running

    virtual void activate() {}
    virtual void deactivate() {}

signals:

    void beginAssignment();
    void updateProgress(int percent);
    void endAssignment();
    void cancelAssignment();
    void hideLaunchWidget();
    void showLaunchWidget();
    void setDocumentModified(bool);
};

#endif // FUZZYDROPLETS_GUI_ASSIGNMENTMETHODWIDGET_H
