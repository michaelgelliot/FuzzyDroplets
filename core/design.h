#ifndef FUZZY_DROPLETS_DESIGN_H
#define FUZZY_DROPLETS_DESIGN_H

#include <cassert>
#include <string>
#include <array>
#include "geometry.h"

// Maintains the experimental design of a digital pcr experiment
// The number and names of the targets on channel one
// The number and names of the tagets on channel two
// The resultant number, names and positions of the clusters in a two dimensional amplitude multiplex experiment
// A stored centroid for each cluster
// Information about whether the centroids are staggered
// The number of targets per channel can be zero, one or two

class Design
{
public:

    enum Staggering
    {
        Unstaggered,
        StaggeredInChannel1,
        StaggeredInChannel2
    };

    Design() {}
    Design(int ch1TargetCount, int ch2TargetCount) {setTargetCounts(ch1TargetCount, ch2TargetCount);}
    Design(const std::vector<std::string> & ch1Labels, const std::vector<std::string> & ch2Labels) {setTargetLabels(ch1Labels, ch2Labels);}

    friend auto operator<=>(const Design &, const Design &) = default;

    bool isValid() const {return m_ch1TargetLabels.size() > 0 || m_ch2TargetLabels.size() > 0;}
    int ch1TargetCount() const {return (int)m_ch1TargetLabels.size();}
    int ch2TargetCount() const {return (int)m_ch2TargetLabels.size();}

    int rowCount() const {return 1 << m_ch1TargetLabels.size();}
    int colCount() const {return 1 << m_ch2TargetLabels.size();}
    int clusterCount() const {return rowCount() * colCount();}

    void setCh1TargetCount(int num);
    void setCh2TargetCount(int num);
    void setTargetCounts(int numCh1, int numCh2);

    void setCh1TargetLabel(int i, const std::string & label) {assert(i >= 0 && i < m_ch1TargetLabels.size()); m_ch1TargetLabels[i] = label; updateClusters();}
    void setCh2TargetLabel(int i, const std::string & label) {assert(i >= 0 && i < m_ch2TargetLabels.size()); m_ch2TargetLabels[i] = label; updateClusters();}

    const std::string & ch1TargetLabel(int i) const {return m_ch1TargetLabels[i];}
    const std::string & ch2TargetLabel(int i) const {return m_ch2TargetLabels[i];}
    void setCh1TargetLabels(const std::vector<std::string> & labels) {assert (labels.size() <= 2); m_ch1TargetLabels = labels; updateClusters();}
    void setCh2TargetLabels(const std::vector<std::string> & labels) {assert (labels.size() <= 2); m_ch2TargetLabels = labels; updateClusters();}
    void setTargetLabels(const std::vector<std::string> & ch1Labels, const std::vector<std::string> & ch2Labels) {assert (ch1Labels.size() <= 2); assert (ch2Labels.size() <= 2); m_ch1TargetLabels = ch1Labels; m_ch2TargetLabels = ch2Labels; updateClusters();}

    int cluster(int row, int col) const {assert (row >= 0 && row < rowCount() && col >= 0 && col < colCount()); return col * rowCount() + row;}
    std::array<int, 2> pos(int cluster) const {assert (cluster >= 0 && cluster < clusterCount()); return {cluster % rowCount(), cluster / rowCount()};}

    const std::string & clusterLabel(int cluster) {assert (cluster >= 0 && cluster < clusterCount()); return m_clusterLabels[cluster];}
    const std::string & clusterLabel(int row, int col) {return m_clusterLabels[cluster(row, col)];}
    const std::vector<std::string> & clusterLabels() const {return m_clusterLabels;}

    const Point & clusterCentroid(int cluster) const {assert (cluster >= 0 && cluster < clusterCount()); return m_clusterCentroids[cluster];}
    const Point & clusterCentroid(int row, int col) const {return m_clusterCentroids[cluster(row, col)];}
    const std::vector<Point> & clusterCentroids() const {return m_clusterCentroids;}

    void setClusterCentroids(const std::vector<Point> & centroids) {m_clusterCentroids = centroids;}
    void setClusterCentroid(int cluster, Point point) {assert (cluster >= 0 && cluster < clusterCount()); m_clusterCentroids[cluster] = point;}
    void setClusterCentroid(int row, int col, Point point) {m_clusterCentroids[cluster(row, col)] = point;}

    Staggering staggering() const {return m_staggering;}
    void setStaggering(Staggering staggering) {m_staggering = staggering;}

    void initCentroids(OrthogonalRectangle dataBounds);

private:

    void updateClusters();

    std::vector<std::string> m_ch1TargetLabels;
    std::vector<std::string> m_ch2TargetLabels;
    std::vector<std::string> m_clusterLabels;
    std::vector<Point> m_clusterCentroids;
    Staggering m_staggering {Unstaggered};
};

#endif // FUZZY_DROPLETS_DESIGN_H
