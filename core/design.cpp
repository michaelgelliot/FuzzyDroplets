#include "design.h"
#include <sstream>

void Design::setCh1TargetCount(int num)
{
    assert(num >= 0 && num <= 2);
    int count = (int)m_ch1TargetLabels.size();
    m_ch1TargetLabels.resize(num);
    for (int i = count; i < num; ++i) {
        std::stringstream ss;
        ss << "Ch1." << (i+1);
        m_ch1TargetLabels[i] = ss.str();
    }
    updateClusters();
}

void Design::setCh2TargetCount(int num)
{
    assert (num >= 0 && num <= 2);
    int count = (int)m_ch2TargetLabels.size();
    m_ch2TargetLabels.resize(num);
    for (int i = count; i < num; ++i) {
        std::stringstream ss;
        ss << "Ch2." << (i+1);
        m_ch2TargetLabels[i] = ss.str();
    }
    updateClusters();
}

void Design::setTargetCounts(int ch1TargetCount, int ch2TargetCount)
{
    assert(ch1TargetCount >= 0 && ch1TargetCount <= 2);
    assert(ch2TargetCount >= 0 && ch2TargetCount <= 2);
    int count = (int)m_ch1TargetLabels.size();
    m_ch1TargetLabels.resize(ch1TargetCount);
    for (int i = count; i < ch1TargetCount; ++i) {
        std::stringstream ss;
        ss << "Ch1." << (i+1);
        m_ch1TargetLabels[i] = ss.str();
    }
    count = (int)m_ch2TargetLabels.size();
    m_ch2TargetLabels.resize(ch2TargetCount);
    for (int i = count; i < ch2TargetCount; ++i) {
        std::stringstream ss;
        ss << "Ch2." << (i+1);
        m_ch2TargetLabels[i] = ss.str();
    }
    updateClusters();
}

void Design::updateClusters()
{
    std::vector<std::string> label1{""};
    label1.reserve(rowCount());
    for (int j = 1; j < rowCount(); ++j) {
        std::string line;
        for (int i = 0; i < rowCount(); ++i) {
            int pos = (1 << i);
            if ((j&pos)==pos) {
                if (line.size() > 0) line += "+";
                line += m_ch1TargetLabels[i];
            }
        }
        label1.push_back(line);
    }

    std::vector<std::string> label2{""};
    label2.reserve(colCount());
    for (int j = 1; j < colCount(); ++j) {
        std::string line;
        for (int i = 0; i < colCount(); ++i) {
            int pos = (1 << i);
            if ((j&pos)==pos) {
                if (line.size() > 0) line += "+";
                line += m_ch2TargetLabels[i];
            }
        }
        label2.push_back(line);
    }

    m_clusterLabels.clear();
    for (int i2 = 0; i2 < label2.size(); ++i2) {
        for (int i1 = 0; i1 < label1.size(); ++i1) {
            if (i1 == 0) {
                if (i2 == 0)
                    m_clusterLabels.push_back("No target");
                else
                    m_clusterLabels.push_back(label2[i2]);
            } else if (i2 == 0) {
                m_clusterLabels.push_back(label1[i1]);
            } else {
                m_clusterLabels.push_back(label1[i1] + "+" + label2[i2]);
            }
        }
    }

    m_clusterCentroids.resize(m_clusterLabels.size(), Point(0,0));
}

void Design::initCentroids(OrthogonalRectangle dataBounds)
{
    if (m_staggering == Unstaggered) {
        double rowGap = dataBounds.height() / (rowCount() + 1);
        double colGap = dataBounds.width() / (colCount() + 1);
        for (int r = 0; r < rowCount(); ++r) {
            for (int c = 0; c < colCount(); ++c)
                setClusterCentroid(r, c, Point(dataBounds.left() + (c + 1) * colGap, dataBounds.bottom() + (r + 1) * rowGap));
        }
    } else if (m_staggering == StaggeredInChannel2) {
        double rowGap = dataBounds.height() / (rowCount() + 1);
        double colGap = dataBounds.width() / (2*colCount() + 1);
        for (int r = 0; r < rowCount(); ++r) {
            for (int c = 0; c < colCount(); ++c)
                setClusterCentroid(r, c, Point(dataBounds.left() +(c + 1) * colGap * 2 - (r%2 == 0) * colGap, dataBounds.bottom() +(r + 1) * rowGap));
        }
    } else if (m_staggering == StaggeredInChannel1) {
        double rowGap = dataBounds.height() / (2*rowCount() + 1);
        double colGap = dataBounds.width() / (colCount() + 1);
        for (int r = 0; r < rowCount(); ++r) {
            for (int c = 0; c < colCount(); ++c)
                setClusterCentroid(r, c, Point(dataBounds.left() +(c + 1) * colGap, dataBounds.bottom() + (r + 1) * rowGap * 2 - (c%2 == 0) * rowGap));
        }
    }
}
