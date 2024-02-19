#ifndef EXPERIMENTALDESIGNWIDGET_H
#define EXPERIMENTALDESIGNWIDGET_H

#include <QDialog>
#include "plot/scatterplotbox2.h"
#include "plot/textmarker.h"
#include "core/design.h"
#include "plot/ringmarker.h"
#include "gui/generic/commandstack.h"
#include "core/fuzzycolor.h"

class QFormLayout;
class QSpinBox;
class QVBoxLayout;
class Data;
class ColorScheme;
class QTableWidget;
class SampleListWidget;
class PointCloud;
class PaintingWidget;

namespace Plot
{
class Marker2D;
}

class ExperimentalDesignWizard : public QDialog
{
    Q_OBJECT

public:

    class SetDesignCommand : public Command<SetDesignCommand>
    {
    public:

        SetDesignCommand(Data * data, PaintingWidget * painting, Design newDesign, QList<QColor> newColors);

        void redo() override;
        void undo() override;

    private:

        Data * m_data;
        PaintingWidget * m_painting;
        Design m_newDesign;
        Design m_oldDesign;
        QList<QColor> m_newColors;
        QList<QColor> m_oldColors;
        std::vector<FuzzyColor> m_oldFuzzies;
    };

    enum Staggering
    {
        Unstaggered = 0,
        StaggeredCh1 = 1,
        StaggeredCh2 = 2
    };

    ExperimentalDesignWizard(Data * data, PaintingWidget * painting, CommandStack * cmdStack, Design * design, QWidget * parent);

    QColor color(int i) const {return (i >= m_colors.size()) ? Qt::black : m_colors[i];}
    int page() const {return m_page;}
    bool accepted() {return m_accepted;}
    QList<QColor> colors() const {return m_colors;}

public slots:

    void setCh1TargetCount(int count);
    void setCh2TargetCount(int count);
    void staggeredButtonClicked(int id);
    void next();
    void prev();
    void setColor();
    void currentColorChanged(const QColor & color);
    void updateLabels();
    void openFromFile();
    void saveToFile();

signals:

    void layoutChanged();
    void labelsChanged();
    void updateGraphColors();
    void startPositioningCentroids();
    void stopPositioningCentroids();

private:

    void setUpPage2();
    void setUpPage3();

    Data * m_data;
    PaintingWidget * m_painting;
    CommandStack * m_commandStack;
    QFormLayout * m_ch1Layout;
    QFormLayout * m_ch2Layout;
    QSpinBox * m_ch1SpinBox;
    QSpinBox * m_ch2SpinBox;
    QTableWidget * colorListTable;
    QList<QColor> m_colors;
    QList<QToolButton*> m_colorButtons;
    int m_selectedColor{-1};

    QPushButton * prevButton;
    QPushButton * nextButton;
    QPushButton * cancelButton;

    int m_page = 1;
    QWidget * page1;
    QWidget * page2;
    QWidget * page3;
    QVBoxLayout * mainLayout;
    Design * m_design;
    bool m_accepted{false};

    static const QString jsonPath;
};



class ExperimentalDesignWidget : public Plot::ScatterPlotBox2
{
    Q_OBJECT

public:

    ExperimentalDesignWidget(Data * data, PaintingWidget * painting, CommandStack * cmdStack, SampleListWidget * list, QWidget * parent = nullptr);

    void run();


protected:

    void mouseMoveEvent(QMouseEvent * e);
    void mousePressEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent * e);

public slots:

    void init();
    void labelsChanged();
    void layoutChanged();
    void updateGraphColors();
    void startPositioningCentroids();
    void stopPositioningCentroids();
    void selectedSamplesChanged();
    void wizardClosed();

signals:

    void closed();

private:

    ExperimentalDesignWizard * m_wizard {nullptr};
    QList<QList<Plot::Marker2D*>> m_markers;
    QList<Plot::TextMarker *> m_textMarkers;
    Design m_design;
    bool m_positioningCentroids{false};
    SampleListWidget * m_list;
    QList<Plot::RingMarker *> m_centroidMarkers;
    Data * m_data;
    PaintingWidget * m_painting;
    CommandStack * m_commandStack;
    PointCloud * m_pointCloud {nullptr};

    int m_selectedCentroid {-1};
    bool m_dragging {false};
    Point m_dragOffset;
};

#endif // EXPERIMENTALDESIGNWIDGET_H
