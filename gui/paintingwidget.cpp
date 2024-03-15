#include "paintingwidget.h"
#include "dropletgraphwidget.h"
#include "../core/data.h"
#include <QBoxLayout>
#include <QGroupBox>
#include "generic/launchwidget.h"
#include <QSlider>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QGridLayout>
#include "pointcloud.h"
#include "../core/colorscheme.h"
#include "../core/kernel.h"
#include "generic/flowlayout.h"
#include "generic/commandstack.h"
#include "plot/continuousaxis.h"

#include <atomic>
#include <execution>

#include <QProgressBar>
#include <QCheckBox>
#include <QToolButton>
#include <QButtonGroup>
#include <QLabel>
#include <QComboBox>
#include <QToolTip>
#include <QThread>
#include <QGuiApplication>
#include <QStyleHints>

#ifdef Q_OS_MACOS
#include <QtConcurrent>
#endif

PaintingWidget::PaintingWidget(Data * data, CommandStack * cmdStack, DropletGraphWidget * graph, QWidget *parent)
    : QWidget{parent},
    m_data(data),
    m_commandStack(cmdStack),
    m_graph(graph)
{
    QHBoxLayout * topLevelLayout = new QHBoxLayout;
    QGroupBox * paletteWidget = new QGroupBox;

    QVBoxLayout * layout = new QVBoxLayout;

    connect(data, &Data::colorCountChanged, this, &PaintingWidget::updatePaletteWidgets);
    connect(data, &Data::designChanged, this, &PaintingWidget::updatePaletteWidgets);
    connect(data, &Data::samplesAdded, this, &PaintingWidget::samplesAdded);

    m_paletteLayout = new FlowLayout;
    paletteWidget->setLayout(m_paletteLayout);
    topLevelLayout->addWidget(paletteWidget);

    QGridLayout * brushLayout = new QGridLayout;
    brushLayout->addWidget(new QLabel("Brush size"), 0, 0);
    QSlider * brushSizeSlider = new QSlider(Qt::Horizontal);
    brushSizeSlider->setRange(5, 200);
    brushSizeSlider->setValue(50);
    m_brushSize = 50;
    brushLayout->addWidget(brushSizeSlider, 0, 1);

    brushLayout->addWidget(new QLabel("Brush intensity"), 1, 0);
    QSlider * brushStrengthSlider = new QSlider(Qt::Horizontal);
    brushStrengthSlider->setRange(0, 100);
    brushStrengthSlider->setValue(100);
    brushLayout->addWidget(brushStrengthSlider, 1, 1);
    m_brushStrength = 100;

    brushLayout->addWidget(new QLabel("Flat Painting"), 2, 0);
    flatPainting = new QCheckBox;
    connect(flatPainting, &QCheckBox::clicked, this, &PaintingWidget::flatPaintingClicked);
    brushLayout->addWidget(flatPainting, 2, 1, Qt::AlignLeft);
    flatPainting->setEnabled(false);

    brushLayout->addWidget(new QLabel("Feathering"), 3,0);
    feathered = new QComboBox;
    feathered->addItems(QStringList() << "None" << "Epanechnikov" << "Quartic" << "Triweight" << "Tricube" << "Cosine");
    feathered->setCurrentIndex(1);
    feathered->setEnabled(false);
    brushLayout->addWidget(feathered, 3, 1, Qt::AlignLeft);

    layout->addLayout(brushLayout);
    connect(brushSizeSlider, &QSlider::valueChanged, this, &PaintingWidget::brushSizeSliderValueChanged);
    connect(brushSizeSlider, &QSlider::sliderPressed, this, &PaintingWidget::brushSizeSliderPressed);
    connect(brushSizeSlider, &QSlider::sliderReleased, this, &PaintingWidget::brushSizeSliderReleased);
    connect(brushStrengthSlider, &QSlider::valueChanged, this, &PaintingWidget::brushStrengthSliderValueChanged);

    QGroupBox * boxBlurBox = new QGroupBox("Circular Box Blur", this);
    QVBoxLayout * boxBlurLayout = new QVBoxLayout;
    m_boxBlurSlider = new QSlider(Qt::Horizontal);
    m_boxBlurSlider->setRange(20, 2000);
    m_boxBlurSlider->setValue(200);
    connect(m_boxBlurSlider, &QSlider::valueChanged, this, &PaintingWidget::boxBlurSliderValueChanged);
    connect(m_boxBlurSlider, &QSlider::sliderPressed, this, &PaintingWidget::boxBlurSliderPressed);
    connect(m_boxBlurSlider, &QSlider::sliderReleased, this, &PaintingWidget::boxBlurSliderReleased);
    boxBlurLayout->addWidget(m_boxBlurSlider);
    m_blurBoxlaunchWidget = new LaunchWidget;
    m_blurBoxlaunchWidget->setProperties(false, true);
    boxBlurLayout->addWidget(m_blurBoxlaunchWidget);
    connect(m_blurBoxlaunchWidget->runButton(), &QPushButton::clicked, this, &PaintingWidget::runBoxBlur);
    boxBlurBox->setLayout(boxBlurLayout);
    layout->addWidget(boxBlurBox);
    m_paletteButtonGroup = new QButtonGroup(this);
    connect(m_paletteButtonGroup, &QButtonGroup::idClicked, this, &PaintingWidget::paletteButtonClicked);

    QGroupBox * defuzzifyBox = new QGroupBox("Defuzzify", this);
    QHBoxLayout * defuzzifyBoxLayout = new QHBoxLayout;
    QPushButton * rbutton = new QPushButton("Probabilistic");
    QPushButton * dbutton = new QPushButton("Deterministic");
    connect(rbutton, &QPushButton::pressed, this, &PaintingWidget::randomlyDefuzzify);
    connect(dbutton, &QPushButton::pressed, this, &PaintingWidget::deterministicallyDefuzzify);
    defuzzifyBoxLayout->addWidget(rbutton);
    defuzzifyBoxLayout->addWidget(dbutton);
    defuzzifyBox->setLayout(defuzzifyBoxLayout);
    layout->addWidget(defuzzifyBox);

    QPushButton * clearButton = new QPushButton("Clear Assignments");
    connect(clearButton, &QPushButton::pressed, this, &PaintingWidget::clear);
    QHBoxLayout * hlayout = new QHBoxLayout;
    hlayout->addWidget(new QWidget);
    hlayout->addWidget(clearButton);
    hlayout->setStretch(0, 1);
    hlayout->setStretch(1, 0);
    layout->addLayout(hlayout);

    topLevelLayout->addLayout(layout);
    topLevelLayout->setStretch(0, 0);
    topLevelLayout->setStretch(1, 1);
    setLayout(topLevelLayout);
}

PaintingWidget::~PaintingWidget()
{
    if (boxBlurWorkerThread) {
        boxBlurWorkerThread->quit();
        boxBlurWorkerThread->wait();
        delete boxBlurWorkerThread;
    }
}

void PaintingWidget::colorsSetProgramatically()
{
    size_t count = 0;
    for (qsizetype i = 0; i < m_painted.size(); ++i) {
        m_painted[i] = (m_data->isSelected(i) && m_data->fuzzyColor(i) != m_prevColors[i]);
        count += m_painted[i];
    }
    if (count > 0) {
        m_commandStack->add(new PaintingWidget::PaintStrokeCommand(this, m_painted, m_prevColors), false);
        if (m_graph->convexHullsVisible())
            m_graph->updateConvexHulls();
        m_graph->repaintEntireGraph();
        beginPaintOperation();
    }
}

void PaintingWidget::randomlyDefuzzify()
{
    m_data->randomlyDefuzzifySelection();
    colorsSetProgramatically();
}

void PaintingWidget::deterministicallyDefuzzify()
{
    m_data->deterministicDefuzzifySelection();
    colorsSetProgramatically();
}

void PaintingWidget::clear()
{
    size_t count = 0;
    for (qsizetype i = 0; i < m_painted.size(); ++i) {
        m_painted[i] = (m_data->isSelected(i) && m_data->fuzzyColor(i).weight(0) != 1);
        if (m_painted[i]) {
            m_data->setColor(i, 0);
            ++count;
        }
    }
    if (count > 0) {
        colorsSetProgramatically();
        beginPaintOperation();
    }
}

void PaintingWidget::beginPaintOperation()
{
    m_prevColors = m_data->fuzzyColors();
#ifdef Q_OS_MACOS
    QtConcurrent::blockingMap(m_painted, [](bool & b) {b = false;});
#else
    std::fill(std::execution::par, m_painted.begin(), m_painted.end(), false);
#endif
}

void PaintingWidget::boxBlurSliderValueChanged(int value)
{
    m_graph->update();
}

void PaintingWidget::boxBlurSliderPressed()
{
    m_adjustingBoxBlurSize = true;
    m_graph->installEventFilter(this);
    m_graph->update();
}

void PaintingWidget::boxBlurSliderReleased()
{
    m_adjustingBoxBlurSize = false;
    m_graph->removeEventFilter(this);
    m_graph->update();
}

PaintingWidget::PaintStrokeCommand::PaintStrokeCommand(PaintingWidget * p, const QList<bool> & painted, const std::vector<FuzzyColor> & prevColors)
    : m_paintingWidget(p)
{
    size_t numModified = std::count_if(
#ifndef Q_OS_MACOS
        std::execution::par,
#endif
        painted.begin(), painted.end(), [](bool b){return b;});
    m_data.reserve(numModified);
    for (qsizetype i = 0; i < painted.size(); ++i) {
        if (painted[i]) {
            m_data.push_back({i, prevColors[i], p->data()->fuzzyColor(i)});
        }
    }
}

void PaintingWidget::PaintStrokeCommand::redo()
{
#ifndef Q_OS_MACOS
    std::for_each(std::execution::par, m_data.begin(), m_data.end(), [&](auto & tuple) {
        m_paintingWidget->data()->setColor(std::get<0>(tuple), std::get<2>(tuple));
    });
#else
    QtConcurrent::blockingMap(m_data.begin(), m_data.end(), [&](auto & tuple) {
        m_paintingWidget->data()->setColor(std::get<0>(tuple), std::get<2>(tuple));
    });
#endif

    m_paintingWidget->beginPaintOperation();
    if (m_paintingWidget->graph()->convexHullsVisible())
        m_paintingWidget->graph()->updateConvexHulls();
    m_paintingWidget->graph()->repaintEntireGraph();
}

void PaintingWidget::PaintStrokeCommand::undo()
{
#ifndef Q_OS_MACOS
    std::for_each(std::execution::par, m_data.begin(), m_data.end(), [&](auto & tuple) {
        m_paintingWidget->data()->setColor(std::get<0>(tuple), std::get<1>(tuple));
    });
#else
    QtConcurrent::blockingMap(m_data.begin(), m_data.end(), [&](auto & tuple) {
        m_paintingWidget->data()->setColor(std::get<0>(tuple), std::get<1>(tuple));
    });
#endif

    m_paintingWidget->beginPaintOperation();
    if (m_paintingWidget->graph()->convexHullsVisible())
        m_paintingWidget->graph()->updateConvexHulls();
    m_paintingWidget->graph()->repaintEntireGraph();
}

bool PaintingWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (m_graph->isEnabled() && event->type() == QEvent::MouseButtonRelease) {
        if (std::find(m_painted.begin(), m_painted.end(), true) != m_painted.end()) {
            PaintStrokeCommand * cmd = new PaintStrokeCommand(this, m_painted, m_prevColors);
            m_commandStack->add(cmd, false);
        }
        beginPaintOperation();
    } else if (event->type() == QEvent::Enter) {
        m_inWidget = true;
        m_graph->update();
    } else if (event->type() == QEvent::Leave) {
        m_inWidget = false;
        m_graph->update();
    } else if (m_graph->isEnabled() && (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress)) {
        QMouseEvent * me = (QMouseEvent*)event;
        m_prevMousePos = me->pos();
        if (m_paletteButtonId >= 0 && me->buttons().testFlag(Qt::LeftButton)) {

            auto viewport = m_graph->viewportRect();
            Point value (m_graph->horizontalAxis()->value(me->pos().x() - viewport.left()), m_graph->verticalAxis()->value(me->pos().y() - viewport.top()));
            double w = (m_brushSize/2) / (m_graph->horizontalAxis()->pixelLength() / m_graph->horizontalAxis()->valueLength());
            double h = (m_brushSize/2) / (m_graph->verticalAxis()->pixelLength() / m_graph->verticalAxis()->valueLength());
            double yMultiplier = pow(w,2)/pow(h,2);
            double w2 = pow(w, 2);
            OrthogonalRectangle rect(value + Point(-w, -h), 2*w, 2*h);
            viewport.translate(-viewport.topLeft());
            QList<size_t> items;

            if (m_brushStrength == 100 || flatPainting->isChecked()) { //unfuzzy painting
                items = m_data->rectangleSearchSelection(rect, [&](size_t i) {
                    return !m_painted[i]
                           && viewport.contains(QPointF(m_graph->horizontalAxis()->pixel(m_data->point(i).x()), m_graph->verticalAxis()->pixel(m_data->point(i).y())))
                           && pow(m_data->point(i).x() - value.x(),2) + pow((m_data->point(i).y() - value.y()),2) * yMultiplier < w2;
                });
            } else {
                items = m_data->rectangleSearchSelection(rect, [&](size_t i) {
                    return viewport.contains(QPointF(m_graph->horizontalAxis()->pixel(m_data->point(i).x()), m_graph->verticalAxis()->pixel(m_data->point(i).y())))
                           && pow(m_data->point(i).x() - value.x(),2) + pow((m_data->point(i).y() - value.y()),2) * yMultiplier < w2;
                });
            }

            if (items.size() > 0) {
                if (m_brushStrength == 100) { // flat and unfuzzy painting, no feathering
                    std::for_each(items.begin(), items.end(), [&](size_t i) {
                        m_data->setColor(i, m_paletteButtonId);
                        m_painted[i] = (m_data->fuzzyColor(i) != m_prevColors[i]);
                    });
                } else {
                    if (flatPainting->isChecked()) { // flat painting, no feathering
                        double weight = (double)m_brushStrength / 100;
                        std::for_each(items.begin(), items.end(), [&](size_t i) {
                            m_data->setWeightToColorComponent(i, m_paletteButtonId, weight);
                            m_painted[i] = (m_data->fuzzyColor(i) != m_prevColors[i]);
                        });
                    } else {
                        double weight = (double)m_brushStrength / 800;
                        if (feathered->currentIndex() == 0) { // additive painting, no feathering
                            std::for_each(items.begin(), items.end(), [&](size_t i) {
                                m_data->addWeightToColorComponent(i, m_paletteButtonId, weight);
                                if (!m_painted[i]) m_painted[i] = (m_data->fuzzyColor(i) != m_prevColors[i]);
                            });
                        } else {     // additive painting, feathered
                            Kernel<double> * kernel = 0;
                            switch (feathered->currentIndex()) {
                            case 1: kernel = new EpanechnikovKernel<double>();
                            case 2: kernel = new QuarticKernel<double>(); break;
                            case 3: kernel = new TriWeightKernel<double>(); break;
                            case 4: kernel = new TricubeKernel<double>(); break;
                            default: kernel = new CosineKernel<double>();
                            }
                            double maxWeight = (kernel->weight(value.x(), value.x(), w) + kernel->weight(value.y(), value.y(), h));
                            std::for_each(items.begin(), items.end(), [&](size_t i) {
                                double kernelWeight = (kernel->weight(m_data->point(i).x(), value.x(), w) + kernel->weight(m_data->point(i).y(), value.y(), h)) / maxWeight;;
                                m_data->addWeightToColorComponent(i, m_paletteButtonId, weight * kernelWeight);
                                if (!m_painted[i]) m_painted[i] = (m_data->fuzzyColor(i) != m_prevColors[i]);
                            });
                            delete kernel;
                        }
                    }
                }

                double maxSize = m_graph->pointCloud()->maxMarkerSize();
                m_graph->updateStaticPrimitives(QRect(me->pos().x() - m_brushSize / 2 - maxSize, me->pos().y() - m_brushSize / 2 - maxSize, m_brushSize + maxSize * 2, m_brushSize + maxSize * 2));
                if (m_graph->convexHullsVisible()) {
                    m_graph->updateConvexHulls();
                   m_graph->update();
                } else {
                    QRegion region;
                    region += QRect(m_prevMousePos - QPoint(m_brushSize / 2+1, m_brushSize / 2 + 1), QSize(2*(int)(m_brushSize / 2) + 2, 2*(int)(m_brushSize / 2) + 2));
                    region += QRect(me->pos() - QPoint(m_brushSize / 2+1, m_brushSize / 2 + 1), QSize(2*(int)(m_brushSize / 2) + 2, 2*(int)(m_brushSize / 2) + 2));
                    m_graph->update(region); //todo is this necessary??
                }
            }
        }

        m_graph->update();
        return true;
    } else if (event->type() == QEvent::Paint) {
        QPainter paint(m_graph);
        paint.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        paint.setClipRect(((QPaintEvent*)event)->rect());
        m_graph->render(paint);
        if (m_adjustingBoxBlurSize || m_adjustingBrushSize || (m_inWidget && m_paletteButtonId >= 0)) {
            paint.setClipRect(m_graph->viewportRect());
            paint.setPen(Qt::NoPen);
            if (m_adjustingBoxBlurSize) {
                QColor col(0,0,0,100);
                paint.setBrush(col);
                double w = m_graph->horizontalUnit() * m_boxBlurSlider->value();
                double h = m_graph->verticalUnit() * m_boxBlurSlider->value();
                paint.drawEllipse(QRectF(m_graph->rect().center().x() - w/2, m_graph->rect().center().y() - h/2, w, h));
            } else if (m_adjustingBrushSize || m_paletteButtonId >= 0) {
                QColor col = m_paletteButtonId == -1 ? QColor(0,0,0) : m_data->colorScheme()->color(m_paletteButtonId);
                col.setAlpha(100);
                paint.setBrush(col);
                if (m_adjustingBrushSize)
                    paint.drawEllipse(QRectF(m_graph->rect().center().x() - m_brushSize/2, m_graph->rect().center().y() - m_brushSize/2, m_brushSize, m_brushSize));
                else
                    paint.drawEllipse(QRectF(m_prevMousePos.x() - m_brushSize / 2, m_prevMousePos.y() - m_brushSize / 2, m_brushSize, m_brushSize));
            }
        }
        return true;
    }
    return QObject::eventFilter(obj, event);
}

void PaintingWidget::runBoxBlur()
{
    if (!boxBlurWorkerThread) {

        emit startParallelWork();
        setEnabled(false);

        beginPaintOperation();

        m_blurBoxlaunchWidget->setRunningMode();
        boxBlurWorkerThread = new QThread;
        BoxBlurWorker * worker = new BoxBlurWorker(m_data, m_graph, m_boxBlurSlider->value());
        worker->moveToThread(boxBlurWorkerThread);
        connect(boxBlurWorkerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &PaintingWidget::startBoxBlur, worker, &BoxBlurWorker::go);
        connect(worker, &BoxBlurWorker::finished, this, &PaintingWidget::boxBlurThreadFinished);
        connect(worker, &BoxBlurWorker::updateProgress, this, &PaintingWidget::updateProgress);
        boxBlurWorkerThread->start();
        emit startBoxBlur();
    }
}

void PaintingWidget::boxBlurThreadFinished()
{
    if (boxBlurWorkerThread) {
        boxBlurWorkerThread->quit();
        boxBlurWorkerThread->wait();
        boxBlurWorkerThread->deleteLater();
        boxBlurWorkerThread = nullptr;
        m_blurBoxlaunchWidget->setReadyMode();

        parallelWorkFinished();
        setEnabled(true);
        emit finishParallelWork();
    }
    for (size_t i = 0; i < m_data->pointCount(); ++i)
        m_painted[i] = (m_data->fuzzyColor(i) != m_prevColors[i]);
    m_commandStack->add(new PaintStrokeCommand(this, m_painted, m_prevColors), false);
    beginPaintOperation();
    m_graph->updateStaticPrimitives();
    m_graph->update();
}

void PaintingWidget::updateProgress(int i)
{
    m_blurBoxlaunchWidget->progressBar()->setValue(i);
}

BoxBlurWorker::BoxBlurWorker(Data * data, DropletGraphWidget * graph, int sliderValue)
    : m_data(data),
    m_graph(graph),
    m_sliderValue(sliderValue)
{
}

void BoxBlurWorker::go()
{
    emit updateProgress(0);

    QList<size_t> selectedPoints;
    selectedPoints.reserve(m_data->selectedPointCount());
    for (auto i : m_data->selectedSamples()) {
        for (size_t k = m_data->sampleIndices(i)[0]; k < m_data->sampleIndices(i)[1]; ++k) {
            selectedPoints.push_back(k);
        }
    }

    QList<FuzzyColor> newColors(selectedPoints.size(), FuzzyColor(m_data->colorComponentCount()));
    QList<size_t> iota(selectedPoints.size());
    std::iota(iota.begin(), iota.end(), 0);

    int pc = 0;
    std::atomic<int> count = 0;

#ifndef Q_OS_MACOS
    std::for_each(std::execution::par, iota.begin(), iota.end(), [&](size_t & id) {
#else
    QtConcurrent::blockingMap(iota.begin(), iota.end(), [&](size_t & id) {
#endif
        ++count;
        int newPc = 100 * count / (int)m_data->selectedPointCount();
        if (newPc != pc) {
            pc = newPc;
            emit updateProgress(pc);
        }
        double w = ((double)m_sliderValue/2);
        double w2 = pow(w, 2);
        auto value = m_data->point(selectedPoints[id]);
        OrthogonalRectangle rect(value + Point(-w, -w), 2*w, 2*w);
        QList<size_t> items;
        items = m_data->rectangleSearchSelection(rect, [&](size_t i) {
            return m_data->isSelected(i) && pow(m_data->point(i).x() - value.x(),2) + pow((m_data->point(i).y() - value.y()),2) < w2;
        });
        if (items.size() == 0) {
            newColors[id] = m_data->fuzzyColor(selectedPoints[id]);
        } else {
            for (auto & item : items) {
                for (auto component = 0; component < newColors[id].componentCount(); ++component)
                    newColors[id].setWeight(component, newColors[id].weight(component) + std::max(0.0, m_data->fuzzyColor(item).weight(component)));
            }
            newColors[id].normalize();
        }
    });


    for (auto id : iota) {
        m_data->setColor(selectedPoints[id], newColors[id]);
    }

    emit updateProgress(100);

    emit finished();
}

void PaintingWidget::updatePaletteWidgets()
{
    for (auto * w : m_paletteWidgets) {
        w->setVisible(false);
        m_paletteLayout->removeWidget(w);
        m_paletteButtonGroup->removeButton((QToolButton*)w);
        delete w;
    }

    m_paletteWidgets.clear();

    for (int i = 0; i < m_data->colorComponentCount(); ++i) {
        auto tb = new QToolButton;
        tb->setCheckable(true);
        //tb->setFixedSize(11 * devicePixelRatio(), 11 * devicePixelRatio());
        auto col = m_data->colorScheme()->color(i);
        auto col2 = (i == 0) ? Qt::red : QColor(col).darker(160);
        QString rgb = "rgb(" + QString::number(Color::red(col)) + "," + QString::number(Color::green(col)) + "," + QString::number(Color::blue(col)) + ")";
        QString rgb2 = "rgb("  + QString::number(col2.red()) + "," + QString::number(col2.green()) + "," + QString::number(col2.blue()) + ")";
        tb->setStyleSheet("QToolButton{background-color:" + rgb + ";border: 1px solid " + rgb2 + "; border-radius:4px;} QToolButton:checked {border: 2.5px solid " + rgb2 + "; border-radius:4px;}");
        tb->setCursor(Qt::PointingHandCursor);
        m_paletteLayout->addWidget(tb);
        m_paletteWidgets.push_back(tb);
        if (m_data->design()->isValid() && i > 0)
            tb->setToolTip(QString::fromStdString(m_data->design()->clusterLabel(i - 1)));
        if (i == 0)
            tb->setToolTip("Unassigned Droplets");
        m_paletteButtonGroup->addButton(tb,i);
    }

    m_paletteButtonId = -1;
}

void PaintingWidget::paletteButtonClicked(int id)
{
    if (id == m_paletteButtonId) {
        m_paletteButtonGroup->setExclusive(false);
        m_paletteButtonGroup->checkedButton()->setChecked(false);
        m_paletteButtonGroup->setExclusive(true);
        m_paletteButtonId = -1;
        m_graph->setCursor(Qt::ArrowCursor);
        m_graph->removeEventFilter(this);
    } else {
        m_paletteButtonId = id;
        m_graph->setCursor(Qt::BlankCursor);
        m_graph->installEventFilter(this);
    }
}

void PaintingWidget::brushSizeSliderValueChanged(int value)
{
    m_graph->update();
    m_brushSize = value;
}

void PaintingWidget::brushSizeSliderPressed()
{
    m_adjustingBrushSize = true;
    if (m_paletteButtonId == -1)
        m_graph->installEventFilter(this);
    m_graph->update();
}

void PaintingWidget::brushSizeSliderReleased()
{
    m_adjustingBrushSize = false;
    if (m_paletteButtonId == -1)
        m_graph->removeEventFilter(this);
    m_graph->update();
}

void PaintingWidget::brushStrengthSliderValueChanged(int value)
{
    m_brushStrength = value;
    feathered->setEnabled(!flatPainting->isChecked() && value < 100);
    flatPainting->setEnabled(value < 100);
    QToolTip::showText(QCursor::pos(), QString("%1%").arg(value), nullptr);
}

void PaintingWidget::samplesAdded()
{
    m_painted = QList<bool>(m_data->pointCount(), false);
    m_prevColors = m_data->fuzzyColors();
}

void PaintingWidget::flatPaintingClicked(bool checked)
{
    feathered->setEnabled(!checked && m_brushStrength < 100);
}

void PaintingWidget::parallelWorkStarted()
{
    setEnabled(false);
}

void PaintingWidget::parallelWorkFinished()
{
    setEnabled(true);
}
