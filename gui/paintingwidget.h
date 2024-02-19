#ifndef PAINTINGWIDGET_H
#define PAINTINGWIDGET_H

#include "core/fuzzycolor.h"
#include "gui/generic/command.h"
#include <QWidget>
class CommandStack;
class DropletGraphWidget;
class Data;
class QSlider;
class LaunchWidget;
class FlowLayout;
class QButtonGroup;
class QAbstractButton;
class QLabel;
class QComboBox;
class QCheckBox;

class PaintingWidget : public QWidget
{
    Q_OBJECT

public:

    class PaintStrokeCommand : public Command<PaintStrokeCommand>
    {
    public:

        PaintStrokeCommand(PaintingWidget * p, const std::vector<bool> & painted, const std::vector<FuzzyColor> & prevColors);

        void redo() override;
        void undo() override;

    private:

        PaintingWidget * m_paintingWidget;
        std::vector<std::tuple<size_t, FuzzyColor, FuzzyColor>> m_data;
    };

    explicit PaintingWidget(Data * data, CommandStack * cmdStack, DropletGraphWidget * graph, QWidget *parent = nullptr);
    ~PaintingWidget();

    Data * data() {return m_data;}
    const Data * data() const {return m_data;}

    DropletGraphWidget * graph() {return m_graph;}
    const DropletGraphWidget * graph() const {return m_graph;}

    void beginPaintOperation();

public slots:

    void boxBlurSliderValueChanged(int value);
    void boxBlurSliderPressed();
    void boxBlurSliderReleased();
    void brushSizeSliderValueChanged(int value);
    void brushSizeSliderPressed();
    void brushSizeSliderReleased();
    void brushStrengthSliderValueChanged(int value);
    void runBoxBlur();
    void updatePaletteWidgets();
    void paletteButtonClicked(int id);
    void samplesAdded();
    void flatPaintingClicked(bool checked);
    void randomlyDefuzzify();
    void deterministicallyDefuzzify();
    void clear();
    void boxBlurThreadFinished();
    void updateProgress(int);
    void parallelWorkStarted();
    void parallelWorkFinished();
    void colorsSetProgramatically();

protected:

    bool eventFilter(QObject *obj, QEvent *event) override;

signals:

    void startBoxBlur();
    void startParallelWork();
    void finishParallelWork();

private:

    bool m_adjustingBoxBlurSize {false};
    Data * m_data;
    CommandStack * m_commandStack;
    DropletGraphWidget * m_graph;
    QSlider * m_boxBlurSlider;
    LaunchWidget * m_blurBoxlaunchWidget;

    FlowLayout * m_paletteLayout;
    QList<QWidget*> m_paletteWidgets;
    QButtonGroup * m_paletteButtonGroup;
    int m_paletteButtonId {-1};
    bool m_adjustingBrushSize {false};
    double m_brushSize;
    int m_brushStrength;
    bool m_inWidget {false};
    QComboBox * feathered;
    QCheckBox * flatPainting;

    QPoint m_prevMousePos;
    std::vector<bool> m_painted;
    std::vector<FuzzyColor> m_prevColors;

    QThread * boxBlurWorkerThread {nullptr};

};

class BoxBlurWorker : public QObject
{
    Q_OBJECT

public:

    BoxBlurWorker(Data * data, DropletGraphWidget * graph, int sliderValue);
    ~BoxBlurWorker() {}

public slots:

    void go();

signals:

    void updateProgress(int);
    void finished();

private:

    Data * m_data;
    DropletGraphWidget * m_graph;
    int m_sliderValue;
    int m_percent;
};

#endif // PAINTINGWIDGET_H
