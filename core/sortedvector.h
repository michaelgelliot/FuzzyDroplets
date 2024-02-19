#ifndef SORTEDVECTOR_HPP
#define SORTEDVECTOR_HPP

#include <vector>
#include <ranges>
#include <algorithm>
#include <compare>

// use with lambda compare type:
// 	  auto compare = [](Point a, Point b) {return a.x() < b.x() || (a.x() == b.x() && a.y() < b.y());}
//    SortedVector<Point, decltype(compare)> vec;

template <typename T, typename Compare, typename Allocator = std::allocator<T>>
class SortedVector
{
    using Vector = std::vector<T, Allocator>;

public:

    using size_type = Vector::size_type;
    using allocator_type = Vector::allocator_type;
    using difference_type = Vector::difference_type;
    using reference = Vector::reference;
    using const_reference = Vector::const_reference;
    using pointer = Vector::pointer;
    using const_pointer = Vector::const_pointer;
    using iterator = Vector::const_iterator;
    using const_iterator = Vector::const_iterator;
    using reverse_iterator = Vector::const_reverse_iterator;
    using const_reverse_iterator = Vector::const_reverse_iterator;
    using value_type = Vector::value_type;

    explicit constexpr SortedVector(const Allocator& alloc = Allocator()) 
        : SortedVector(Compare(), alloc)
    {
    }

    explicit constexpr SortedVector(Compare compare, const Allocator& alloc = Allocator()) noexcept
        : m_vec(alloc),
          m_compare(compare)
    {
    }

    constexpr SortedVector(size_type count, const T& value, Compare compare = Compare(), const Allocator& alloc = Allocator())
        : m_vec(count, value, alloc),
          m_compare(compare)
    {
    }

    constexpr explicit SortedVector(size_type count, Compare compare = Compare(), const Allocator& alloc = Allocator())
        : m_vec(count, alloc),
          m_compare(compare)
    {
    }

    template< typename InputIt>
    constexpr SortedVector(InputIt first, InputIt last, Compare compare = Compare(), const Allocator& alloc = Allocator())
        : m_vec(last - first, alloc),
          m_compare(compare)
    {
        std::partial_sort_copy(first, last, m_vec.begin(), m_vec.end(), m_compare);
    }

    constexpr SortedVector(SortedVector&& other, const Allocator& alloc)
        : m_vec(std::move(other.m_vec), alloc),
          m_compare(std::move(other.m_compare))
    {
    }

    constexpr SortedVector(std::initializer_list<T> init, Compare compare = Compare(), const Allocator& alloc = Allocator())
        : m_vec(init.size(), alloc),
          m_compare(compare)
    {
        std::ranges::partial_sort_copy(init, m_vec, m_compare);
    }

    template <std::ranges::input_range R>
    constexpr SortedVector(std::from_range_t, R&& rg, Compare compare = Compare(), const Allocator& alloc = Allocator())
        : m_vec(std::ranges::distance(std::forward<R>(rg)), alloc),
          m_compare(compare)
    {
        std::ranges::partial_sort_copy(std::forward<R>(rg), m_vec, m_compare);
    }

    constexpr auto operator<=>(const SortedVector&) const = default;    

    constexpr SortedVector& operator=(std::initializer_list<T> ilist)
    {
        m_vec.resize(ilist.size());
        std::ranges::partial_sort_copy(ilist, m_vec, m_compare);
        return *this;
    }

    constexpr void assign(size_type count, const T& value)
    {
        m_vec.assign(count, value);
    }

    template< class InputIt >
    constexpr void assign(InputIt first, InputIt last)
    {
        m_vec.resize(last - first);
        std::ranges::partial_sort_copy(first, last, m_vec.begin(), m_vec.end(), m_compare);
    }

    constexpr void assign(std::initializer_list<T> ilist)
    {
        m_vec.resize(ilist.size());
        std::ranges::partial_sort_copy(ilist, m_vec, m_compare);
    }

    template<typename R >
    constexpr void assign_range(R&& rg)
    {
        m_vec.resize(std::ranges::distance(std::forward<R>(rg)));
        std::ranges::partial_sort_copy(std::forward<R>(rg), m_vec, m_compare);
    }

    constexpr allocator_type get_allocator() const noexcept
    {
        return m_vec.get_allocator();
    }

    constexpr const_reference at(size_type pos) const
    {
        return m_vec.at(pos);
    }

    constexpr const_reference operator[](size_type pos) const
    {
        return m_vec[pos];
    }

    constexpr const_reference front() const
    {
        return m_vec.front();
    }

    constexpr const_reference back() const
    {
        return m_vec.back();
    }

    constexpr const T* data() const noexcept
    {
        return m_vec.data();
    }

    constexpr iterator begin() noexcept
    {
        return m_vec.cbegin();
    }

    constexpr const_iterator begin() const noexcept
    {
        return m_vec.cbegin();
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return m_vec.cbegin();
    }

    constexpr iterator end() noexcept
    {
        return m_vec.cend();
    }

    constexpr const_iterator end() const noexcept
    {
        return m_vec.cend();
    }

    constexpr const_iterator cend() const noexcept
    {
        return m_vec.cend();
    }

    constexpr iterator rbegin() noexcept
    {
        return m_vec.crbegin();
    }

    constexpr const_iterator rbegin() const noexcept
    {
        return m_vec.crbegin();
    }

    constexpr const_iterator crbegin() const noexcept
    {
        return m_vec.crbegin();
    }

    constexpr iterator rend() noexcept
    {
        return m_vec.crend();
    }

    constexpr const_iterator rend() const noexcept
    {
        return m_vec.crend();
    }

    constexpr const_iterator crend() const noexcept
    {
        return m_vec.crend();
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return m_vec.empty();
    }

    constexpr size_type size() const noexcept
    {
        return m_vec.size();
    }

    constexpr size_type max_size() const noexcept
    {
        return m_vec.max_size();
    }

    constexpr void reserve(size_type new_cap)
    {
        m_vec.reserve(new_cap);
    }

    constexpr size_type capacity() const noexcept
    {
        return m_vec.capacity();
    }

    constexpr void shrink_to_fit()
    {
        m_vec.shrink_to_fit();
    }

    constexpr void clear() noexcept
    {
        m_vec.clear();
    }

    constexpr void pop_back() noexcept
    {
        m_vec.pop_back();
    }

    constexpr iterator insert(const T& value)
    {
        return m_vec.insert(std::ranges::upper_bound(m_vec, value, m_compare), value);
    }

    constexpr iterator insert(size_type count, const T& value)
    {
        return m_vec.insert(std::ranges::upper_bound(m_vec, value, m_compare), count, value);
    }

    template< class InputIt >
    constexpr iterator insert(InputIt first, InputIt last)
    {
        auto min = *(std::min_element(first, last, m_compare));
        m_vec.insert(m_vec.end(), first, last);
        auto it = std::upper_bound(m_vec.begin(), m_vec.end() - (last - first), min, m_compare);
        std::sort(it, m_vec.end(), m_compare);
        return it;
    }

    constexpr iterator insert(std::initializer_list<T> ilist)
    {
        auto min = *(std::ranges::min_element(ilist, m_compare));
        m_vec.insert(m_vec.end(), ilist);
        auto it = std::upper_bound(m_vec.begin(), m_vec.end() - ilist.size(), min, m_compare);
        std::sort(it, m_vec.end(), m_compare);
        return it;
    }

    template< typename R >
    constexpr iterator insert_range(R&& rg)
    {
        auto min = *(std::ranges::min_element(std::forward<R>(rg), m_compare));
        m_vec.insert_range(m_vec.end(), std::forward<R>(rg));
        auto it = std::upper_bound(m_vec.begin(), m_vec.end() - std::ranges::distance(std::forward<R>(rg)), min, m_compare);
        std::sort(it, m_vec.end(), m_compare);
        return it;
    }

    constexpr iterator erase(const_iterator pos)
    {
        m_vec.erase(pos);
    }

    constexpr iterator erase(const_iterator first, const_iterator last)
    {
        m_vec.erase(first, last);
    }

    constexpr void resize(size_type count, const value_type& value)
    {
        if (size() >= count) {
            m_vec.resize(count);
        } else {
            insert(count - size(), value);
        }
    }

    constexpr void swap(SortedVector& other) noexcept
    {
        m_vec.swap(other.m_vec);
        m_compare.swap(other.m_compare);
    }

    constexpr size_type erase(const value_type& value)
    {
        return std::erase(m_vec, value);
    }

    template< typename Pred >
    constexpr size_type erase_if(Pred pred)
    {
        return std::erase_if(m_vec, pred);
    }

private:

	Vector m_vec;
    Compare m_compare;
};

template <typename T, typename Alloc>
void swap(SortedVector<T, Alloc>& left, SortedVector<T, Alloc> & right) noexcept
{
    left.swap(right);
}

#endif // SORTEDVECTOR_HPP
