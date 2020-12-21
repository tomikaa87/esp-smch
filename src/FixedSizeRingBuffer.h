#pragma once

#include <array>
#include <cstdlib>

template <typename ValueType, size_t Size>
class FixedSizeRingBuffer
{
public:
    // STL-compatibility aliases
    using value_type = ValueType;
    using reference = ValueType&;
    using const_reference = const ValueType&;
    using size_type = size_t;

    using Container = std::array<ValueType, Size>;

    void push_back(const value_type& val)
    {
        if (_pushed >= Size)
            return;

        _container[_inIdx] = val;
        _inIdx = (_inIdx + 1) % Size;
        ++_pushed;
    }

    void push_back(value_type&& val)
    {
        if (_pushed >= Size)
            return;

        _container[_inIdx] = std::move(val);
        _inIdx = (_inIdx + 1) % Size;
        ++_pushed;
    }

    reference front()
    {
        return _container[_outIdx];
    }

    const_reference front() const
    {
        return _container[_outIdx];
    }

    void pop_front()
    {
        if (empty())
            return;

        --_pushed;
        _outIdx = (_outIdx + 1) % Size;
    }

    bool empty() const
    {
        return _pushed == 0;
    }

    size_type size() const
    {
        return _pushed;
    }

private:
    size_type _inIdx = 0;
    size_type _outIdx = 0;
    size_type _pushed = 0;
    Container _container;
};