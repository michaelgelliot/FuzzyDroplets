#ifndef PHYLOGENERICS_UTILITY_MAPPED_FILE_HPP
#define PHYLOGENERICS_UTILITY_MAPPED_FILE_HPP

// this is a version of MemoryMapped modified by Mick Elliot
//////////////////////////////////////////////////////////
// MemoryMapped.h
// Copyright (c) 2013 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
/////

#include <stdexcept>
#include <cstdio>
#include <string_view>
#include <string>
#include <cstdint>
#include <QtGlobal>

#ifdef _MSC_VER
#define NOMINMAX
#include <windows.h>
#else
#include <stdint.h>
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#ifdef  _FILE_OFFSET_BITS
#undef  _FILE_OFFSET_BITS
#endif
#define _FILE_OFFSET_BITS 64
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#endif

namespace PhyloGenerics
{

class MappedFile
{
#ifdef _MSC_VER
    static constexpr bool Windows = true;
#else
    static constexpr bool Windows = false;
#endif

public:

    enum CacheHint
    {
        Normal,         ///< good overall performance
        SequentialScan, ///< read file only once with few seeks
        RandomAccess    ///< jump around
    };

    enum MapRange
    {
        WholeFile = 0   ///< everything ... be careful when file is larger than memory
    };

    MappedFile(const std::string & filename, size_t mappedBytes = WholeFile, CacheHint hint = Normal)
        : m_filesize(0),
          m_hint(hint),
          m_mappedBytes(mappedBytes),
          m_file(0),
          m_mappedFile (nullptr),
          m_mappedView (nullptr)
    {
#ifdef _MSC_VER
        // open file
        m_file = ::CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, m_hint == Normal ? FILE_ATTRIBUTE_NORMAL : (m_hint == SequentialScan ? FILE_FLAG_SEQUENTIAL_SCAN : FILE_FLAG_RANDOM_ACCESS), NULL);
        if (m_file) {
            LARGE_INTEGER result;
            if (GetFileSizeEx(m_file, &result)) {
                m_filesize = static_cast<uint64_t>(result.QuadPart);
                m_mappedFile = ::CreateFileMapping(m_file, NULL, PAGE_READONLY, 0, 0, NULL);
            }
        }
        if (!m_mappedFile)
            return;
#else
    // open file
#ifdef Q_OS_MACOS
        m_file = ::open(filename.c_str(), O_RDONLY);
#else
        m_file = ::open(filename.c_str(), O_RDONLY | O_LARGEFILE);
#endif
        if (m_file == -1)
        {
            m_file = 0;
            return;
        }

        // file size
        struct stat64 statInfo;
        if (fstat64(m_file, &statInfo) < 0)
            return;

        m_filesize = statInfo.st_size;
#endif

        // initial mapping
        remap(0, mappedBytes);
    }

    ~MappedFile()
    {
        close();
    }

    MappedFile(const MappedFile&) = delete;
    MappedFile& operator=(const MappedFile&) = delete;

    bool isValid() const
    {
        return m_mappedView != nullptr;
    }

    bool isOpen() const
    {
        return m_mappedView;
    }

    uint64_t size() const
    {
        return m_filesize;
    }

    void close()
    {
        // kill pointer
        if (m_mappedView)
        {
#ifdef _MSC_VER
            ::UnmapViewOfFile(m_mappedView);
#else
            ::munmap(m_mappedView, m_filesize);
#endif
            m_mappedView = NULL;
        }

#ifdef _MSC_VER
        if (m_mappedFile)
        {
            ::CloseHandle(m_mappedFile);
            m_mappedFile = NULL;
        }
#endif

        // close underlying file
        if (m_file)
        {
#ifdef _MSC_VER
            ::CloseHandle(m_file);
#else
            ::close(m_file);
#endif
            m_file = 0;
        }

        m_filesize = 0;
    }

    const char* data() const
    {
        return (const char*)m_mappedView;
    }

    std::string_view stringView()
    {
        return std::string_view((const char*)m_mappedView, m_filesize);
    }

    /// access position, no range checking (faster)
    char operator[](size_t offset) const
    {
        return ((unsigned char*)m_mappedView)[offset];
    }

    /// access position, including range checking
    char at (size_t offset) const
    {
        // checks
        if (!m_mappedView)
            throw std::invalid_argument("No view mapped");
        if (offset >= m_filesize)
            throw std::out_of_range("View is not large enough");
        return operator[](offset);
    }

    /// replace mapping by a new one of the same file, offset MUST be a multiple of the page size
    bool remap(uint64_t offset, size_t mappedBytes)
    {
        if (!m_file)
            return false;

        if (mappedBytes == WholeFile)
            mappedBytes = m_filesize;

        // close old mapping
        if (m_mappedView)
        {
#ifdef _MSC_VER
            ::UnmapViewOfFile(m_mappedView);
#else
            ::munmap(m_mappedView, m_mappedBytes);
#endif
            m_mappedView = NULL;
        }

        // don't go further than end of file
        if (offset > m_filesize)
            return false;
        if (offset + mappedBytes > m_filesize)
            mappedBytes = size_t(m_filesize - offset);

#ifdef _MSC_VER
        // Windows

        DWORD offsetLow  = DWORD(offset & 0xFFFFFFFF);
        DWORD offsetHigh = DWORD(offset >> 32);
        m_mappedBytes = mappedBytes;

        // get memory address
        m_mappedView = ::MapViewOfFile(m_mappedFile, FILE_MAP_READ, offsetHigh, offsetLow, mappedBytes);

        if (m_mappedView == NULL)
        {
            m_mappedBytes = 0;
            m_mappedView  = NULL;
            return false;
        }

        return true;

#else

#ifdef Q_OS_MACOS
        m_mappedView = ::mmap(NULL, mappedBytes, PROT_READ, MAP_SHARED, m_file, offset);
#else
        m_mappedView = ::mmap64(NULL, mappedBytes, PROT_READ, MAP_SHARED, m_file, offset);
#endif
        if (m_mappedView == MAP_FAILED)
        {
            m_mappedBytes = 0;
            m_mappedView  = NULL;
            return false;
        }

        m_mappedBytes = mappedBytes;

        // tweak performance
        int linuxHint = 0;
        switch (m_hint)
        {
        case Normal:         linuxHint = MADV_NORMAL;     break;
        case SequentialScan: linuxHint = MADV_SEQUENTIAL; break;
        case RandomAccess:   linuxHint = MADV_RANDOM;     break;
        default: break;
        }
        // assume that file will be accessed soon
        //linuxHint |= MADV_WILLNEED;
        // assume that file will be large
        //linuxHint |= MADV_HUGEPAGE;

        ::madvise(m_mappedView, m_mappedBytes, linuxHint);

        return true;
#endif
    }

private:

    uint64_t m_filesize;
    CacheHint m_hint;
    size_t m_mappedBytes;
    std::conditional_t<Windows, void*, int> m_file;
    void* m_mappedFile;
    void* m_mappedView;
};
}

#endif // PHYLOGENERICS_UTILITY_MAPPED_FILE_HPP
