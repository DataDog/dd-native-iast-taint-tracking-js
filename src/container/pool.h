/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_CONTAINER_POOL_H_
#define SRC_CONTAINER_POOL_H_

#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <utility>

using std::size_t;

namespace iast {
namespace container {

class PoolBadAlloc : public std::exception {
};

template<class T, size_t N>
class Pool final {
 public:
    union Element {
        alignas(T) uint8_t storage[sizeof(T)];
        Element* next;
    };

    class iterator {
     public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = size_t;
        using value_type        = Element*;
        using pointer           = Element**;
        using reference         = Element&;

        iterator(pointer p, pointer end) : _ptr(p), _end(end) {}
        T* operator *() const { return reinterpret_cast<T*>(&(*_ptr)->storage); }
        T* operator ->() { return reinterpret_cast<T*>(&(*_ptr)->storage); }
        iterator& operator ++() {
            do {
                _ptr++;
            } while ((_ptr < _end) && !*_ptr);
            return *this;
        }
        friend bool operator ==(const iterator& a, const iterator& b) { return a._ptr == b._ptr; }
        friend bool operator !=(const iterator& a, const iterator& b) { return a._ptr != b._ptr; }

     private:
        pointer _ptr;
        pointer _end;
    };

    Pool() {
        for (size_t i = 1; i < N; ++i) {
            _pool[i - 1].next = &_pool[i];
        }
        _nextAvail = &_pool[0];
    }

    Pool(const Pool&) = delete;
    Pool(Pool&&) = delete;
    ~Pool() = default;

    void Clear() {
        for (size_t i = 1; i < N; ++i) {
            if (_used[i - 1]) {
                _used[i - 1] = nullptr;
                auto p = reinterpret_cast<T*>(&_pool[i - 1].storage);
                p->~T();
            }
            _pool[i - 1].next = &_pool[i];
        }
        _used[N - 1] = nullptr;
        _nextAvail = &_pool[0];
    }

    template<class ...Args>
        T* Pop(Args&& ...args) {
            auto element = _nextAvail;
            if (!element) {
                throw PoolBadAlloc();
            }

            _nextAvail = element->next;
            _used[element - &_pool[0]] = element;
            return new (reinterpret_cast<Element*>(&element->storage)) T(std::forward<Args>(args)...);
        }

    void Push(T* p) noexcept {
        if (p == nullptr) {
            return;
        }

        auto element = reinterpret_cast<Element*>(p);
        if ((element < &_pool[0] || element > &_pool[N - 1])) {
            return;
        }

        p->~T();
        element->next = _nextAvail;
        _nextAvail = element;
        _used[element - &_pool[0]] = nullptr;
    }

    iterator begin() {
        auto first = &_used[0];
        while (first < &_used[N - 1] && !*first) {
            first++;
        }
        return iterator(first, &_used[N - 1]);
    }

    iterator end() {
        return iterator(&_used[N - 1], &_used[N - 1]); }

    Pool& operator =(const Pool&) = delete;

    Pool& operator =(Pool&& other) noexcept {
        if (this == &other)
            return *this;

        _pool = std::move(other._pool);
        _used = std::move(other._used);
        _nextAvail = other._nextAvail;

        other._nextAvail = nullptr;
        return *this;
    }

 private:
    std::unique_ptr<Element[]> _pool = std::make_unique<Element[]>(N);
    std::unique_ptr<Element*[]> _used = std::make_unique<Element*[]>(N);
    Element* _nextAvail = nullptr;
};

}  // namespace container
}  // namespace iast
#endif  // SRC_CONTAINER_POOL_H_
