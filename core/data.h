#ifndef FUZZY_DROPLETS_DATA_H
#define FUZZY_DROPLETS_DATA_H

#include "core/design.h"
#include "fuzzyqt.h"
#include "geometry.h"
#include "fuzzycolor.h"

class Design;
class ColorScheme;
template <typename> class QuadTree;

#ifdef FUZZY_QT
#include <QObject>
class Data: public QObject
#else
class Data
#endif
{
#ifdef FUZZY_QT
    Q_OBJECT
#endif
public:

    enum SampleType {
        Experimental,
        PositiveControl,
        NegativeControl,
        NonTemplateControl
    };

    enum SelectionType {
        Selected,
        Unselected,
        SelectedAndUnselected
    };

#ifdef FUZZY_QT
    Data(QObject * parent = nullptr);
#else
    Data();
#endif
    ~Data();

    size_t pointCount() const {return m_points.size();}
    const Point & point(size_t i) const {assert(i < m_points.size()); return m_points[i];}
    const std::vector<Point> & points() const {return  m_points;}
    const OrthogonalRectangle & bounds() const {return m_dataBounds;}

    Design * design() {return &m_design;}
    const Design * design() const {return &m_design;}
    void setDesign(const Design & design);

    ColorScheme * colorScheme() {return m_colorScheme;}
    const ColorScheme * colorScheme() const {return m_colorScheme;}

    const std::vector<FuzzyColor> & fuzzyColors() const {return m_colors;}
    const FuzzyColor & fuzzyColor(size_t i) const {assert(i < m_colors.size()); return m_colors[i];}
    void setColor(size_t i, const FuzzyColor & color);
    void setColor(size_t i, size_t component);
    void addWeightToColorComponent(size_t i, size_t component, double weight);
    void setWeightToColorComponent(size_t i, size_t component, double weight);
    void storeColor(size_t, const FuzzyColor & color);
    void storeColor(size_t, size_t component);
    void setColorComponentCount(size_t count);
    size_t colorComponentCount() const {return m_colorComponentCount;}
    void deterministicDefuzzifySelection();
    void randomlyDefuzzifySelection();
    void updateRgbaInSelection();
    void updateRgba();

    int colorZOrder(size_t color) const;
    const std::vector<size_t> & colorZOrder() const {return m_colorZOrder;}
    void setColorZOrder(size_t color, size_t pos);

    Color::Rgba rgba(size_t i) const {assert(i < m_rgba.size()); return m_rgba[i];}
    Color::Rgba nullColor() const;
    void setNullColor(Color::Rgba color);

    bool isSelected(size_t point) const {assert(point < m_selected.size()); return m_selected[point];}
    const std::vector<bool> & selectionFilter() const {return m_selected;}
    const std::vector<size_t> & selectedSamples() const {return m_selectionIndices;}
    void setSelectedSamples(const std::vector<size_t> & samples);
    size_t selectedPointCount() const;
    std::vector<Point> randomSelectedPoints(size_t count) const;

    const QuadTree<Point> * quadTree() const {return m_quadTree;}

    std::vector<Point> centroidsByFuzzyColor(SelectionType type, std::vector<double> * count = nullptr) const;
    std::vector<Point> centroidsByDominantColor(SelectionType type, std::vector<size_t> * count = nullptr) const;
    void matchColorsToDesign(SelectionType selection);
    void matchSelectedColorToUnselectedColors();

    void addSamples(const std::vector<std::string> & paths, std::string & error);
    size_t sampleCount() const {return m_samples.size();}
    std::array<size_t, 2> sampleIndices(size_t sample) {assert (sample < m_samples.size()); return m_samples[sample];}
    size_t sampleSize(size_t sample) const {assert(sample < m_samples.size()); return m_samples[sample][1] - m_samples[sample][0];}
    const std::string & samplePath(size_t sample) const {assert(sample < m_samplePaths.size()); return m_samplePaths[sample];}
    SampleType sampleType(size_t sample) const {assert(sample < m_sampleTypes.size()); return m_sampleTypes[sample];}
    void setSampleType(std::vector<size_t> samples, SampleType type);
    void setSampleType(std::vector<size_t> samples, std::vector<SampleType> type);
    const std::vector<std::string> & samplePaths() const {return m_samplePaths;}

    std::pair<size_t, double> nearestNeighbourInSelection(Point target, double xScale = 1, double yScale = 1, const std::function<bool(size_t)> & filter = [](size_t t){return true;});
    std::vector<size_t> rectangleSearchSelection(OrthogonalRectangle rect, const std::function<bool(size_t)> & filter = [](size_t t){return true;}) const;

#ifdef FUZZY_QT

signals:

    void samplesAdded(std::vector<size_t>);
    void selectedSamplesChanged();
    void colorZOrderChanged(size_t color, size_t pos);
    void colorCountChanged();
    void designChanged();
    void fullRepaint();
    void sampleTypesChanged(std::vector<size_t> samples);

#endif

private:

    std::array<int, 2> colorCountInFile(const std::string & path) const;
    void updateDataBounds();

    Design m_design;
    ColorScheme * m_colorScheme {nullptr};
    std::vector<Point> m_points;
    std::vector<FuzzyColor> m_colors;
    std::vector<Color::Rgba> m_rgba;
    std::vector<bool> m_selected;
    mutable QuadTree<Point> * m_quadTree {nullptr};

    size_t m_colorComponentCount {0};
    std::vector<size_t> m_colorZOrder;

    OrthogonalRectangle m_dataBounds {{std::numeric_limits<double>::max(), std::numeric_limits<double>::max()}, {-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()}};

    std::vector<size_t> m_selectionIndices;
    std::vector<OrthogonalRectangle> m_sampleDataBounds;
    std::vector<std::array<size_t, 2>> m_samples;
    std::vector<std::string> m_samplePaths;
    std::vector<SampleType> m_sampleTypes;
};

#endif // FUZZY_DROPLETS_DATA_H
