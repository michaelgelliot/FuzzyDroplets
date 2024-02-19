#ifndef PHYLOGENERICS_UTILITY_LINE_FEEDER_HPP
#define PHYLOGENERICS_UTILITY_LINE_FEEDER_HPP

#include "mapped_file.hpp"
#include "trim_string_view.hpp"

namespace PhyloGenerics
{

// Memory maps a file containing an array of chars, and provides sequential access to the lines, treating CR ('\r'), LF ('\n') or CRLF ("\r\n") as line endings.
// Does not do any string copying at all, simply returns string_view objects pointing at the raw memory underlying each line in the memory mapped file
// Provide an EmptyLinePolicy to decide how to treat empty lines:
// * KeepAllLines does not skip any lines.
// * SkipNullLines skips lines of zero length
// * SkipEmptyLines skips lines of zero length or lines that contain only spaces (std::isspace).
// You can use isValid() to test whether the file was opened succesfully
// While the file is iterating, you can find the position at which the current line starts using pos()
// You can set this value using setPos() to skip to a different position (setPos(0) will return to the beginning)
// The size of the file is given by size()
// After you have finished, you can clear memory by calling close() (this will happen automatically upon destruction)

// Usage:
//
// LineFeeder file("data.txt", SkipEmptyLines);
// while (!file.atEnd()) {
//     auto line = file.getLine();
//     ... do something with the line
// }

class LineFeeder
{
public:

    enum EmptyLinePolicy
    {
        KeepAllLines,
        SkipEmptyLines,
        SkipNullLines
    };

    LineFeeder(const std::string & fileName, EmptyLinePolicy policy = KeepAllLines)
        : m_file(fileName, MappedFile::WholeFile, MappedFile::SequentialScan),
          m_pos(0),
          m_policy(policy)
    {
        m_size = m_file.size();
        if (policy == SkipEmptyLines) {
            while (m_size > 1 && std::isspace(m_file[m_size - 1]))
                --m_size;
        } else if (policy == SkipNullLines) {
            while (m_size > 1 && m_file[m_size - 1] == '\n' || m_file[m_size - 1] == '\r')
                --m_size;
        }
    }

    std::string_view getLine()
    {
        size_t k = m_pos;
        while (k < m_size && m_file[k] != '\n' && m_file[k] != '\r')
            ++k;
        auto result = std::string_view(m_file.data() + m_pos, m_file.data() + k);
        m_pos = k + 1 + (k < m_size - 1 && m_file[k] == '\r' && m_file[k+1] == '\n');
        return ((m_policy == SkipNullLines && result.size() == 0) || (m_policy == SkipEmptyLines && trim(result).size() == 0)) ? getLine() : result;
    }

    bool isValid() const  {return m_file.isValid() && m_file.isOpen();}
    bool atEnd() const    {return !isValid() || m_pos >= m_size;}
    size_t size() const   {return m_size;}
    size_t pos() const    {return m_pos;}

    void close()          {m_file.close();}
    void setPos(size_t i) {m_pos = i;}

private:

    MappedFile m_file;
    size_t m_pos;
    size_t m_size;
    EmptyLinePolicy m_policy;
};

}

#endif // PHYLOGENERICS_UTILITY_LINE_FEEDER_HPP
