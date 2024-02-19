#include "data.h"
#include "design.h"
#include "colorscheme.h"
#include "round.h"
#include "mean.h"
#include "line_feeder.hpp"
#include "quadtree.h"
#include "hungarianalgorithm.h"

#include <charconv>

#ifdef FUZZY_QT

#include <QGuiApplication>
#include <QStyleHints>

Data::Data(QObject * parent)
    : QObject(parent),
    m_colorScheme(new ColorScheme)
{
    m_colorScheme->setColor(0, qGuiApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark ? Color::named::white : Color::named::black);
}
#else
Data::Data()
    : m_colorScheme(new ColorScheme)
{
}
#endif

Data::~Data()
{
    delete m_colorScheme;
}

void Data::setColor(size_t i, const FuzzyColor & color)
{
    assert(i < m_colors.size());
    m_colors[i] = color;
    m_rgba[i] = color.rgba(m_colorScheme->colors(0, m_colorComponentCount - 1));
}

void Data::addWeightToColorComponent(size_t i, size_t component, double weight)
{
    assert(i < m_colors.size());
    m_colors[i].setWeight(component, m_colors[i].weight(component) + weight);
    m_colors[i].normalize();
    m_rgba[i] = m_colors[i].rgba(m_colorScheme->colors(0, m_colorComponentCount - 1));
}

void Data::setWeightToColorComponent(size_t i, size_t component, double weight)
{
    assert(i < m_colors.size());
    if (component == 0) {
        if (m_colors[i].weight(0) == 1) return;
        m_colors[i].setWeight(0, 0);
        m_colors[i].normalize(std::max(0.0, 1.0 - weight));
        m_colors[i].setWeight(0, weight);
    } else {
        m_colors[i].setWeight(component, 0);
        m_colors[i].setWeight(0, 0);
        m_colors[i].normalize(std::max(0.0, 1.0 - weight));
        m_colors[i].setWeight(component, weight);
        m_colors[i].setWeight(0, std::max(0.0, 1.0-m_colors[i].totalWeight()));
    }
    m_rgba[i] = m_colors[i].rgba(m_colorScheme->colors(0, m_colorComponentCount - 1));

}

void Data::storeColor(size_t i, const FuzzyColor & color)
{
    assert(i < m_colors.size());
    m_colors[i] = color;
}

void Data::setColor(size_t i, size_t component)
{
    assert(i < m_colors.size());
    assert(component < m_colorComponentCount);
    m_colors[i].setFixedComponent(component);
    m_rgba[i] = m_colorScheme->color(component);
}

void Data::storeColor(size_t i, size_t component)
{
    assert(i < m_colors.size());
    m_colors[i].setFixedComponent(component);
}

void Data::updateRgbaInSelection()
{
    auto baseColors = m_colorScheme->colors(0, m_colorComponentCount - 1);
    for (auto i : m_selectionIndices) {
        auto iota = std::ranges::views::iota(m_samples[i][0], m_samples[i][1]);
        std::for_each(std::execution::par, iota.begin(), iota.end(), [&](size_t j) {
            m_rgba[j] = m_colors[j].rgba(baseColors);
        });
    }
}

void Data::updateRgba()
{
    if (m_points.size() > 0) {
        const auto baseColors = m_colorScheme->colors(0, m_colorComponentCount - 1);
        auto iota = std::ranges::views::iota((size_t)0, m_points.size());
        std::for_each(std::execution::par, iota.begin(), iota.end(), [&](size_t j) {
            m_rgba[j] = m_colors[j].rgba(baseColors);
        });
    }
}

void Data::setDesign(const Design & design)
{
    m_design = design;
    bool countChanged = false;
    if (m_colorComponentCount != m_design.clusterCount() + 1 ) {
        setColorComponentCount(m_design.clusterCount() + 1);
        countChanged = true;
    } else {
        updateRgba();
    }
    matchColorsToDesign(Data::SelectedAndUnselected);
    if (countChanged)
        emit colorCountChanged();
    emit designChanged();
}

void Data::updateDataBounds()
{
    m_dataBounds = std::accumulate(m_sampleDataBounds.begin(), m_sampleDataBounds.end(), OrthogonalRectangle({std::numeric_limits<double>::max(),std::numeric_limits<double>::max()},{-std::numeric_limits<double>::max(),-std::numeric_limits<double>::max()}), [](const auto & left, const auto & right) {return left.boundingBox(right);});;
}

void Data::setColorZOrder(size_t rgb, size_t pos)
{
    auto it = std::find(m_colorZOrder.begin(), m_colorZOrder.end(), rgb);
    if (it != m_colorZOrder.end()) {
        m_colorZOrder.erase(it);
        m_colorZOrder.insert(m_colorZOrder.begin() + std::clamp(pos, (size_t)0, (size_t)m_colorZOrder.size()), rgb);
#ifdef FUZZY_QT
        emit colorZOrderChanged(rgb, pos);
#endif
    }
}

int Data::colorZOrder(size_t color) const
{
    auto it = std::find(m_colorZOrder.begin(), m_colorZOrder.end(), color);
    return int(it == m_colorZOrder.end() ? -1 : it - m_colorZOrder.begin());
}

void Data::setColorComponentCount(size_t count)
{
    if (count != m_colorComponentCount) {
        if (count < m_colorComponentCount) {
            m_colorZOrder.erase(std::remove_if(m_colorZOrder.begin(), m_colorZOrder.end(), [&](size_t i){return i >= count;}), m_colorZOrder.end());
        } else {
            for (size_t i = m_colorComponentCount; i < count; ++i)
                m_colorZOrder.push_back(i);
        }
        m_colorComponentCount = count;
        auto baseColors = m_colorScheme->colors(0, m_colorComponentCount - 1);
        auto iota = std::ranges::views::iota((size_t)0, m_colors.size());
        std::for_each(std::execution::par, iota.begin(), iota.end(), [&](size_t i) {
            m_colors[i].setComponentCount(count);
            m_rgba[i] = m_colors[i].rgba(baseColors);
        });

#ifdef FUZZY_QT
        emit colorCountChanged();
#endif
    }
}

void Data::setNullColor(Color::Rgba color)
{
    if (m_colorScheme->color(0) != color) {
        m_colorScheme->setColor(0, color);
        updateRgba();
    }
}

Color::Rgba Data::nullColor() const
{
    return m_colorScheme->color(0);
}

void Data::setSelectedSamples(const std::vector<size_t> & indices)
{
    m_selectionIndices = indices;
    std::fill(m_selected.begin(), m_selected.end(), false);
    std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t i) {
        std::fill(m_selected.begin() + m_samples[i][0], m_selected.begin() + m_samples[i][1], true);
    });
#ifdef FUZZY_QT
    emit selectedSamplesChanged();
#endif
}

size_t Data::selectedPointCount() const
{
    size_t result = 0;
    for (auto i : m_selectionIndices)
        result += m_samples[i][1] - m_samples[i][0];
    return result;
}

std::vector<Point> Data::randomSelectedPoints(size_t i) const
{
    auto filter = std::views::zip(m_points, std::ranges::views::iota(0)) | std::views::filter([&](auto elem){return isSelected(std::get<1>(elem));}) | std::views::elements<0>;
    std::vector<Point> filtered;
    filtered.reserve(selectedPointCount());
    std::ranges::copy(filter, std::back_inserter(filtered));
    if (i > 0) {
        std::random_device seed;
        std::mt19937 rng(seed());
        size_t len = std::min(i, selectedPointCount());
        std::vector<Point> sample(len);
        std::ranges::copy(filter, std::back_inserter(filtered));
        std::ranges::sample(filtered, sample.begin(), len, rng);
        filtered = sample;
    }
    return filtered;
}

std::vector<Point> Data::centroidsByFuzzyColor(SelectionType type, std::vector<double> * count) const
{
    std::vector<WeightedArithmeticMean<Point>> means(m_colorComponentCount);
    auto iota = std::ranges::views::iota((size_t)0, m_colorComponentCount);
    std::for_each(std::execution::par, iota.begin(), iota.end(), [&](size_t k) {
        for (size_t i = 0; i < m_points.size(); ++i) {
            if (type == SelectedAndUnselected || (type == Selected && m_selected[i]) || (type == Unselected && !m_selected[i]))
                means[k].add(m_points[i], m_colors[i].weight(k));
        }
    });
    std::vector<Point> result(means.size());
    std::transform(means.begin(), means.end(), result.begin(), [](const auto & wam){return wam.count() == 0 ? Point(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()) : wam.mean();});
    if (count) {
        count->resize(means.size());
        std::transform(means.begin(), means.end(), count->begin(), [](const auto & wam){return wam.count();});
    }
    return result;
}

std::vector<Point> Data::centroidsByDominantColor(SelectionType type, std::vector<size_t> * count) const
{
    std::vector<ArithmeticMean<Point>> means(m_colorComponentCount);
    for (size_t i = 0; i < m_points.size(); ++i) {
        if (type == SelectedAndUnselected || (type == Selected && m_selected[i]) || (type == Unselected && !m_selected[i]))
            means[m_colors[i].dominantComponent()].add(m_points[i]);
    }
    std::vector<Point> result(means.size());
    std::transform(means.begin(), means.end(), result.begin(), [](const auto & am){return am.count() == 0 ? Point(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()) : am.mean();});
    if (count) {
        count->resize(means.size());
        std::transform(means.begin(), means.end(), count->begin(), [](const auto & am){return am.count();});
    }
    return result;
}

void Data::deterministicDefuzzifySelection() //todo parallelize
{
    for (auto i : m_selectionIndices) {
        for (size_t j = m_samples[i][0]; j < m_samples[i][1]; ++j) {
            if (fuzzyColor(j) != 0)
                setColor(j, fuzzyColor(j).dominantComponent());
        }
    }
}

void Data::randomlyDefuzzifySelection() // todo parallelize
{
    std::random_device seed;
    std::mt19937 rng(seed());
    std::uniform_real_distribution<double> dist(0, 1);
    for (auto i : m_selectionIndices) {
        for (size_t j = m_samples[i][0]; j < m_samples[i][1]; ++j) {
            if (fuzzyColor(j) != 0) {
                auto r = dist(rng);
                double total = 0;
                for (size_t k = 0; k < m_colorComponentCount; ++k) {
                    total += fuzzyColor(j).weight(k);
                    if (total >= r) {
                        setColor(j, k);
                        if (k == 0) qDebug("here");
                        break;
                    }
                }
            }
        }
    }
}



void Data::addSamples(const std::vector<std::string> & paths, std::string & error)
{
    if (paths.size() == 0) return;

    std::vector<size_t> addedSamples;

    for (auto path : paths) {

        bool fuzzy = false;
        int colorCount = 0;
        int columnCount = 0;
        bool ok = true;

        PhyloGenerics::LineFeeder feeder(path, PhyloGenerics::LineFeeder::SkipEmptyLines);
        if (!feeder.isValid()) {
            error += "Failed to open file: " + path + "\n\n";
            continue;
        }

        size_t lineCount = 0;
        while (!feeder.atEnd()) {
            double x, y;

            auto line = feeder.getLine();
            size_t pos1 = line.find(',');
            if (pos1 == std::string_view::npos) {
                ok = false;
                error += "Skipped file: " + path + "\n";
                error += "(found only one column)\n\n";
                break;
            }
            size_t pos2 = line.find(',', pos1 + 1);
            if (pos2 == std::string_view::npos) {
                pos2 = line.size();
            }

            auto err1 = std::from_chars(line.data(), line.data() + pos1, y);
            auto err2 = std::from_chars(line.data() + pos1 + 1, line.data() + pos2, x);

            if (err1.ec != std::errc() || err2.ec != std::errc()) {
                if (lineCount == 0)
                    continue;
                else {
                    error += "Skipped file: " + path + "\n";
                    std::stringstream ss;
                    ss << lineCount;
                    error += "(failed to convert the first two columns to numeric values in line " + ss.str() + ")\n\n";
                }
            }

            if (pos2 < line.size()) {
                auto commaCount = std::count(line.begin() + pos2 + 1, line.end(), ',');
                if (commaCount > 1) {
                    colorCount = (int)std::count(line.begin() + pos2 + 1, line.end(), ',') + 2;
                    fuzzy = true;
                    break;
                } else {
                    int cc;
                    auto err3 = std::from_chars(line.data() + pos2 + 1, line.data() + line.size(), cc);
                    if (err3.ec != std::errc() || cc < 0) {
                        continue;
                    }
                    colorCount = std::max(cc + 1, colorCount);
                }
            } else {
                colorCount = 1;
                columnCount = 0;
            }

            ++lineCount;
        }

        if (colorCount == 0) {
            ok = false;
            error += "Skipped file: " + path + "\n";
            error += "(failed to convert the third column to a positive integer value)\n\n";
        }

        if (!ok) continue;

        columnCount = fuzzy ? colorCount : 1;

        feeder.setPos(0);

        if (colorCount > m_colorComponentCount)
            setColorComponentCount(colorCount);

        const auto baseColors = m_colorScheme->colors(0, m_colorComponentCount - 1);
        FuzzyColor unassigned(m_colorComponentCount);
        unassigned.setFixedComponent(0);
        auto unassignedRgb = unassigned.rgba(baseColors);

        m_samples.push_back({m_points.size(), m_points.size()});

        lineCount = 0;

        size_t oldPointCount = pointCount();

        bool colorErrorInFile = false;

        while (!feeder.atEnd()) {

            double x, y;

            auto line = feeder.getLine();
            size_t pos1 = line.find(',');
            if (pos1 == std::string_view::npos) {
                m_samples.pop_back();
                m_points.resize(oldPointCount);
                error += "Skipped file: " + path + "\n";
                std::stringstream ss;
                ss << lineCount;
                error += "(found only one column in line " + ss.str() + ")\n\n";
                break;
            }
            size_t pos2 = line.find(',', pos1 + 1);
            if (pos2 == std::string_view::npos) {
                pos2 = line.size();
            }

            auto err1 = std::from_chars(line.data(), line.data() + pos1, y);
            auto err2 = std::from_chars(line.data() + pos1 + 1, line.data() + pos2, x);

            if (err1.ec != std::errc() || err2.ec != std::errc()) {
                if (lineCount == 0)
                    continue;
                else {
                    m_samples.pop_back();
                    m_points.resize(oldPointCount);
                    error += "Skipped file: " + path + "\n";
                    std::stringstream ss;
                    ss << lineCount;
                    error += "(failed to convert the first two columns to numeric values in line " + ss.str() + ")\n\n";
                    break;
                }
            }

            m_points.push_back({x,y});

            m_colors.push_back(unassigned);
            m_rgba.push_back(unassignedRgb);

            if (pos2 < line.size()) {
                std::string_view col(line.data() + pos2 + 1, line.data() + line.size());

                if (fuzzy) {
                    size_t offset = 0;
                    for (int i = 1; i < columnCount; ++i) {
                        auto pos2 = col.find(',', offset+1);
                        if (pos2 == std::string_view::npos)
                            pos2 = col.size();
                        double d;
                        auto err = std::from_chars(col.data() + offset, col.data() + pos2, d);
                        if (err.ec != std::errc()) {
                            m_colors.back().setWeight(i, 0);
                            colorErrorInFile = true;
                        } else {
                            m_colors.back().setWeight(i, d);
                        }
                        offset = pos2 + 1;
                    }
                    m_colors.back().setWeight(0, 1.0 - (m_colors.back().totalWeight() - m_colors.back().weight(0)));
                } else {
                    int fix;
                    auto err = std::from_chars(col.data(), col.data() + pos2, fix);
                    if (err.ec != std::errc()) {
                        m_colors.back().setFixedComponent(0);
                        colorErrorInFile = true;
                    } else {
                        m_colors.back().setFixedComponent(fix);
                    }
                }
            }

            m_colors.back().normalize();
            m_rgba.back() =  m_colors.back().rgba(baseColors);
            ++lineCount;
        }

        feeder.close();

        if (colorErrorInFile) {
            error += "Some assignments could not be read in file: " + path + "\n\n";
        }
        if (m_points.size() == m_samples.back()[0]) {
            m_samples.pop_back();
            error += "Skipped empty file: " + path + "\n\n";
        } else {
            addedSamples.push_back(m_samples.size() - 1);
            m_samples.back()[1] = m_points.size();
            m_samplePaths.push_back(path);
            m_sampleTypes.push_back(Experimental);
            auto xRange = std::minmax_element(m_points.begin() + m_samples.back()[0], m_points.begin() + m_samples.back()[1], [](const auto & left, const auto & right) {return left.x() < right.x();});
            auto yRange = std::minmax_element(m_points.begin() + m_samples.back()[0], m_points.begin() + m_samples.back()[1], [](const auto & left, const auto & right) {return left.y() < right.y();});
            m_sampleDataBounds.push_back({{(*xRange.first).x(), (*yRange.first).y()}, {xRange.second->x(), yRange.second->y()}});
        }
    }

    m_points.shrink_to_fit();
    m_colors.shrink_to_fit();
    m_selected.resize(m_points.size(), false);
    updateDataBounds();
    delete m_quadTree;
    m_quadTree = new QuadTree<Point>(m_points, [](const Point & p){return p.x();}, [&](const Point & p){return p.y();});

#ifdef FUZZY_QT
    if (addedSamples.size() > 0)
        emit samplesAdded(addedSamples);
#endif

}

void Data::matchSelectedColorToUnselectedColors()
{
    if (selectedPointCount() == pointCount()) return;

    std::vector<double> count;
    auto centroids = centroidsByFuzzyColor(Selected, &count);
    std::vector<double> unselectedCount;
    auto unselectedCentroids = centroidsByFuzzyColor(Unselected, &unselectedCount);
    count.erase(count.begin());
    centroids.erase(centroids.begin());
    unselectedCount.erase(unselectedCount.begin());
    unselectedCentroids.erase(unselectedCentroids.begin());

    double longDistance = 100 * (bounds().bottomLeft().distanceTo(bounds().topRight()));

    std::vector<std::vector<double>> distMat(centroids.size());
    for (int i = 0; i < centroids.size(); ++i) {
        std::vector<double> dist(unselectedCentroids.size());
        if (count[i] == 0) {
            dist = std::vector<double>(unselectedCentroids.size(), longDistance);
        } else {
            for (int j = 0; j < unselectedCentroids.size(); ++j)
                dist[j] = (unselectedCount[j] > 0) ? centroids[i].distanceTo(unselectedCentroids[j]) : longDistance;
        }
        distMat[i] = dist;
    }

    std::vector<int> assignment;
    HungarianAlgorithm ha;
    ha.Solve(distMat, assignment);

    auto iota = std::ranges::views::iota((size_t)0, m_points.size());
    std::for_each(std::execution::par, iota.begin(), iota.end(), [&](size_t i) {
        if (isSelected(i)) {
            auto color = fuzzyColor(i);
            FuzzyColor newColor(m_colorComponentCount);
            newColor.setWeight(0, color.weight(0));
            for (int k = 0; k < assignment.size(); ++k)
                newColor.setWeight(assignment[k] + 1, color.weight(k+1));
            setColor(i, newColor);
        }
    });

#ifdef FUZZY_QT
    emit fullRepaint();
#endif
}

void Data::matchColorsToDesign(SelectionType selection)
{
    if (!m_design.isValid()) return;

    std::vector<double> count;
    auto centroids = centroidsByFuzzyColor(selection, &count);
    count.erase(count.begin());
    centroids.erase(centroids.begin());

    std::vector<std::vector<double>> distMat(centroids.size());
    double longDistance = 100 * (bounds().bottomLeft().distanceTo(bounds().topRight()));
    for (int i = 0; i < centroids.size(); ++i) {
        std::vector<double> dist(m_design.clusterCount());
        if (count[i] == 0) {
            std::fill(dist.begin(), dist.end(), longDistance);
        } else {
            for (int j = 0; j < m_design.clusterCount(); ++j) {
                dist[j] = centroids[i].distanceTo(m_design.clusterCentroid(j));
            }
        }
        distMat[i] = dist;
    }

    std::vector<int> assignment;
    HungarianAlgorithm ha;
    ha.Solve(distMat, assignment);

    if (centroids.size() > m_design.clusterCount()) {
        // negatives present
        std::vector<int> dummy (centroids.size());
        std::iota(dummy.begin(), dummy.end(), 0);
        for (int i : assignment) {
            auto it = std::find(dummy.begin(), dummy.end(), i);
            if (it != dummy.end())
                dummy.erase(it);
        }
        int dummyPos = 0;
        for (int i = 0; i < assignment.size(); ++i) {
            if (assignment[i] == -1) {
                assignment[i] = dummy[dummyPos];
                ++dummyPos;
            }
        }
    }

    auto iota = std::ranges::views::iota((size_t)0, m_points.size());
    std::for_each(std::execution::par, iota.begin(), iota.end(), [&](size_t i) {
        if (selection == SelectedAndUnselected || (selection == Selected && isSelected(i)) || (selection == Unselected && !isSelected(i))) {
            auto color = fuzzyColor(i);
            FuzzyColor newColor(m_colorComponentCount);
            newColor.setWeight(0, color.weight(0));
            for (int k = 0; k < assignment.size(); ++k)
                newColor.setWeight(assignment[k] + 1, color.weight(k+1));
            setColor(i, newColor);
        }
    });

#ifdef FUZZY_QT
    emit fullRepaint();
#endif
}

std::vector<size_t> Data::rectangleSearchSelection(OrthogonalRectangle rect, const std::function<bool(size_t)> & filter) const
{
    if (m_selectionIndices.empty()) {
        return std::vector<size_t>();
    } else {
        return m_quadTree->rectangleSearch(m_points, rect, [&](size_t i) {return isSelected(i) && filter(i);});
    }
}

std::pair<size_t, double> Data::nearestNeighbourInSelection(Point target, double xScale, double yScale, const std::function<bool(size_t)> & filter)
{
    if (m_selectionIndices.empty()) {
        return {-1,std::numeric_limits<double>::max()};
    } else {
        auto result = m_quadTree->nearestNeighbor(m_points, target.x(), target.y(), xScale, yScale, [&](size_t i) {return isSelected(i) && filter(i);});
        return {result.first, result.second};
    }
}

void Data::setSampleType(std::vector<size_t> samples, SampleType type)
{
    for (auto sample : samples) {
        m_sampleTypes[sample] = type;
    }
#ifdef FUZZY_QT
    emit sampleTypesChanged(samples);
#endif
}

void Data::setSampleType(std::vector<size_t> samples, std::vector<SampleType> type)
{
    assert(samples.size() == type.size());
    for (int i = 0; i < samples.size(); ++i) {
        m_sampleTypes[samples[i]] = type[i];
    }
#ifdef FUZZY_QT
    emit sampleTypesChanged(samples);
#endif
}
