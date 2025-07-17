#pragma once

#include <memory>

namespace mute
{
    // WIPPPPP
    // template <typename T, bool Const, bool Ref, typename Allocator = std::allocator<T>>
    // struct BaseBuffer
    // {
    //     static constexpr bool IsConst = Const;
    //     static constexpr bool IsRef = Ref;
    //     using AllocatorType = Allocator;
    //     using ValueType = T;
    //     using PointerType = std::conditional_t<IsConst, const ValueType*, ValueType*>;
    //     using ConstPointerType = const ValueType*;

    //     using Iterator = PointerType;
    //     using ConstIterator = ConstPointerType;
    //     using ReverseIterator = std::reverse_iterator<Iterator>;
    //     using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

    //     AllocatorType _allocator;
    //     PointerType _data = nullptr;
    //     size_t _size = 0;

    //     BaseBuffer() = default;

    //     template <bool C, bool R>
    //     BaseBuffer(const BaseBuffer<T, C, R>& other)
    //         requires(IsConst || C == IsConst)
    //     {
    //         if (!IsRef)
    //         {
    //             _data = malloc
    //         }
    //     }

    //     BaseBuffer(BaseBuffer<T, false, Ref>&& other);

    //     BaseBuffer(PointerType data, size_t size);

    //     ~BaseBuffer()
    //     {
    //         if (!IsRef)
    //             free(_data);
    //     }

    //     ValueType& operator[](size_t index) requires(!IsConst) { return _data[index]; }
    //     const ValueType& operator[](size_t index) const { return _data[index]; }

    //     bool isref() const { return _capacity == -1; }
    //     bool isnull() const { return _data == nullptr; }
    //     size_t size() const { return _size; }
    //     bool empty() const { return _size == 0; }

    //     void resize(size_t newsize)
    //     {
    //         if (!IsRef)
    //         {
    //             _data = realloc()
    //         }
    //         _size = newsize;
    //     }
    //     void clear();

    //     Iterator begin() requires(!IsConst) { return _data; }
    //     Iterator end() requires(!IsConst) { return _data + _size; }
    //     ReverseIterator rbegin() requires(!IsConst) { return ReverseIterator(end()); }
    //     ReverseIterator rend() requires(!IsConst) { return ReverseIterator(begin()); }
        
    //     ConstIterator cbegin() const { return _data; }
    //     ConstIterator cend() const { return _data + _size; }
    //     ConstIterator begin() const { return cbegin(); }
    //     ConstIterator end() const { return cend(); }

    //     ConstReverseIterator rbegin() const { return ConstReverseIterator(end()); }
    //     ConstReverseIterator rend() const { return ConstReverseIterator(begin()); }
    //     ConstReverseIterator crbegin() const { return ConstReverseIterator(cend()); }
    //     ConstReverseIterator crend() const { return ConstReverseIterator(cbegin()); }
    // };

    // using Buffer = BaseBuffer<float, false, false>;
    // using BufferRef = BaseBuffer<float, false, true>;
    // using ConstBuffer = BaseBuffer<float, true, false>;
    // using ConstBufferRef = BaseBuffer<float, true, true>;
}