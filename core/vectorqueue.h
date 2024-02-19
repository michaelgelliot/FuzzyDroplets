#ifndef FUZZY_DROPLETS_VECTORQUEUE_HPP
#define FUZZY_DROPLETS_VECTORQUEUE_HPP

#include <vector>
#include <ranges>

template <typename T, typename Allocator = std::allocator<T>>
class VectorQueue
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
    using iterator = Vector::iterator;
    using const_iterator = Vector::const_iterator;
    using reverse_iterator = Vector::reverse_iterator;
    using const_reverse_iterator = Vector::const_reverse_iterator;
    using value_type = Vector::value_type;

    constexpr explicit VectorQueue(size_type max_offset = 1000, const Allocator& alloc = Allocator()) noexcept
        : m_vec(alloc),
          m_max_offset(max_offset)
    {
    }

    constexpr VectorQueue(size_type count, const T& value, size_type max_offset = 1000, const Allocator& alloc = Allocator())
        : m_vec(count, value, alloc),
        m_max_offset(max_offset)
    {
    }

    constexpr explicit VectorQueue(size_type count, size_type max_offset = 1000, const Allocator& alloc = Allocator())
        : m_vec(count, alloc),
        m_max_offset(max_offset)
    {
    }

    template< typename InputIt, typename Compare = std::less<T>>
    constexpr VectorQueue(InputIt first, InputIt last, size_type max_offset = 1000, const Allocator& alloc = Allocator())
        : m_vec(first, last, alloc),
        m_max_offset(max_offset)
    {
    }

    constexpr VectorQueue(VectorQueue&& other, const Allocator& alloc)
        : m_vec(std::move(other.m_vec), alloc),
          m_first(other.m_first),
          m_max_offset(other.m_max_offset)
    {
    }

    constexpr VectorQueue(std::initializer_list<T> init, size_type max_offset = 1000, const Allocator& alloc = Allocator())
        : m_vec(init, alloc),
        m_max_offset(max_offset)
    {
    }

    template <std::ranges::input_range R>
    constexpr VectorQueue(std::from_range_t, R&& rg, size_type max_offset = 1000, const Allocator& alloc = Allocator())
        : m_vec(std::forward<R>(rg), alloc),
        m_max_offset(max_offset)
    {
    }

    constexpr auto operator<=>(const VectorQueue&) const = default;

    constexpr VectorQueue& operator=(std::initializer_list<T> ilist)
    {
        m_vec = ilist;
        m_first = 0;
        return *this;
    }

    constexpr void assign(size_type count, const T& value)
    {
        m_vec.assign(count, value);
        m_first = 0;
    }

    template< class InputIt >
    constexpr void assign(InputIt first, InputIt last)
    {
        m_vec.assign(first, last);
        m_first = 0;
    }

    constexpr void assign(std::initializer_list<T> ilist)
    {
        m_vec.assign(ilist);
        m_first = 0;
    }

    template<typename R >
    constexpr void assign_range(R&& rg)
    {
        m_vec.assign(std::forward<R>(rg));
        m_first = 0;
    }

    constexpr allocator_type get_allocator() const noexcept
    {
        return m_vec.get_allocator();
    }

    constexpr reference at(size_type pos)
    {
        return m_vec.at(pos + m_first);
    }

    constexpr const_reference at(size_type pos) const
    {
        return m_vec.at(pos + m_first);
    }

    constexpr reference operator[](size_type pos) 
    {
        return m_vec[pos + m_first];
    }

    constexpr const_reference operator[](size_type pos) const
    {
        return m_vec[pos + m_first];
    }

    constexpr reference front() 
    {
        return m_vec[m_first];
    }

    constexpr const_reference front() const
    {
        return m_vec[m_first];
    }

    constexpr reference back() 
    {
        return m_vec.back();
    }

    constexpr const_reference back() const
    {
        return m_vec.back();
    }

    constexpr iterator begin() noexcept
    {
        return m_vec.begin() + m_first;
    }

    constexpr const_iterator begin() const noexcept
    {
        return m_vec.begin() + m_first;
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return m_vec.cbegin() + m_first;
    }

    constexpr iterator end() noexcept
    {
        return m_vec.end();
    }

    constexpr const_iterator end() const noexcept
    {
        return m_vec.end();
    }

    constexpr const_iterator cend() const noexcept
    {
        return m_vec.cend();
    }

    constexpr iterator rbegin() noexcept
    {
        return m_vec.rbegin();
    }

    constexpr const_iterator rbegin() const noexcept
    {
        return m_vec.rbegin();
    }

    constexpr const_iterator crbegin() const noexcept
    {
        return m_vec.crbegin();
    }

    constexpr iterator rend() noexcept
    {
        return m_vec.rend() - m_first;
    }

    constexpr const_iterator rend() const noexcept
    {
        return m_vec.rend() - m_first;
    }

    constexpr const_iterator crend() const noexcept
    {
        return m_vec.rend() - m_first;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return m_first >= m_vec.size();
    }

    constexpr size_type size() const noexcept
    {
        return m_vec.size() - m_first;
    }

    constexpr size_type max_size() const noexcept
    {
        return m_vec.max_size() - m_first;
    }

    constexpr size_type vector_size() const noexcept
    {
        return m_vec.size();
    }

    constexpr void reserve(size_type new_cap)
    {
        m_vec.reserve(new_cap + m_first);
    }

    constexpr size_type capacity() const noexcept
    {
        return m_vec.capacity() - m_first;
    }

    constexpr size_type vector_capacity() const noexcept
    {
        return m_vec.capacity();
    }

    constexpr void shrink_to_fit()
    {
        shrink_offset();
        m_vec.shrink_to_fit();
    }

    constexpr void clear() noexcept
    {
        m_vec.clear();
        m_first = 0;
    }

    constexpr iterator insert(const_iterator pos, const T & value)
    {
        return m_vec.insert(pos, value);
    }

    constexpr iterator insert(const_iterator pos, T && value)
    {
        return m_vec.insert(pos, std::move<T>(value));
    }
            
    constexpr iterator insert(const_iterator pos, size_type count, const T & value)
    {
        return m_vec.insert(pos, count, value);
    }

    template< class InputIt >
    constexpr iterator insert(const_iterator pos, InputIt first, InputIt last)
    {
        return m_vec.insert(pos, first, last);
    }
        
    constexpr iterator insert(const_iterator pos, std::initializer_list<T> ilist)
    {
        return m_vec.insert(pos, ilist);
    }

    template<typename R >
    constexpr iterator insert_range(const_iterator pos, R&& rg)
    {
        return m_vec.insert_range(pos, std::forward<R>(rg));
    }

    template< class... Args >
    constexpr iterator emplace(const_iterator pos, Args&&... args)
    {
        return m_vec.emplace(pos, std::forward<Args...>(args...));
    }

    constexpr iterator erase(const_iterator pos)
    {
        return m_vec.erase(pos);
    }

    constexpr iterator erase(const_iterator first, const_iterator last)
    {
        return m_vec.erase(first, last);
    }

    constexpr void push_back(const value_type& value)
    {
        m_vec.push_back(value);
    }

    constexpr void push_back(value_type&& value)
    {
        m_vec.push_back(std::move(value));
    }

    template< class... Args >
    constexpr reference emplace_back(Args&&... args)
    {
        return m_vec.emplace_back(args...);
    }

    template< typename R >
    constexpr void append_range(R&& rg)
    {
        m_vec.append_range(std::forward<R>(rg));
    }

    constexpr void pop_back()
    {
        m_vec.pop_back();
    }

    constexpr void pop_front()
    {
        ++m_first;
        if (m_first > m_max_offset)
            shrink_offset();
    }

    constexpr void push(const value_type& value)
    {
        m_vec.push_back(value);
    }

    constexpr void push(value_type&& value)
    {
        m_vec.push_back(std::move(value));
    }

    constexpr value_type pop()
    {
        auto result = m_vec[m_first];
        ++m_first;
        if (m_first > m_max_offset)
            shrink_offset();
        return result;
    }

    constexpr void resize(size_type count)
    {
        m_vec.resize(count + m_first);
    }

    constexpr void resize(size_type count, const value_type& value)
    {
        m_vec.resize(count + m_first, value);
    }

    constexpr void swap(VectorQueue& other) noexcept
    {
        m_vec.swap(other.m_vec);
        m_first.swap(other.m_first);
    }

    constexpr size_type erase(const value_type& value)
    {
        shrink_offset();
        return std::erase(m_vec.begin(), value);
    }

    template< typename Pred >
    constexpr size_type erase_if(Pred pred)
    {
        shrink_offset();
        return std::erase_if(m_vec, pred);
    }

    constexpr void shrink_offset()
    {
        m_vec.erase(m_vec.begin(), m_vec.begin() + m_first);
        m_first = 0;
    }

    constexpr size_type max_offset() const
    {
        return m_max_offset;
    }

    constexpr void set_max_offset(size_type offset)
    {
        m_max_offset = offset;
    }

private:

    Vector m_vec;
    size_type m_first{ 0 };
    size_type m_max_offset{ 1000 };
};

template <typename T, typename Alloc>
void swap(VectorQueue<T, Alloc>& left, VectorQueue<T, Alloc>& right) noexcept
{
    left.swap(right);
}

#endif // FUZZY_DROPLETS_VECTORQUEUE_HPP
