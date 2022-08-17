#ifndef _HDIV_POOL_H_
#define _HDIV_POOL_H_

//#include <type_traits>
#include <iostream>
#include <cstddef>
#include <memory>
#include <utility>

using std::size_t;
namespace container {

template<class T, size_t N>
class Pool final
{
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

        iterator(pointer p) : _ptr(p){};
        T* operator *() const { return reinterpret_cast<T*>(&(*_ptr)->storage); }
        T* operator ->() { return reinterpret_cast<T*>(&(*_ptr)->storage); };
        iterator& operator ++() {
            do {
                _ptr++;
            } while(!*_ptr && (_ptr != (_ptr + N)));
            return *this;
        }
        friend bool operator ==(const iterator& a, const iterator& b) { return a._ptr == b._ptr; }
        friend bool operator !=(const iterator& a, const iterator& b) { return a._ptr != b._ptr; };

    private:
        pointer _ptr;
    };
    
    Pool() {
        for (size_t i = 1; i < N; ++i) {
            _used[i - 1] = nullptr;
            _pool[i - 1].next = &_pool[i];
        }
        _nextAvail = &_pool[0];
    }
 
    Pool(const Pool&) = delete;
    Pool(Pool&&) = delete; 
    ~Pool() = default; //TODO: check if calling destructor for each element is needed.
 
    void clear() {
        for (size_t i = 1; i < N; ++i) {
            _used[i - 1] = nullptr;
            auto p = reinterpret_cast<T*>(&_pool[i - 1].storage);
            p->~T();
            _pool[i - 1].next = &_pool[i];
        }
        _nextAvail = &_pool[0];
    }

    template<class ...Args>
    T* pop(Args&& ...args) noexcept {
        auto element = _nextAvail;
        if (!element) {
            return nullptr;
        }

        _nextAvail = element->next;
        _used[element - &_pool[0]] = element;
        return new (reinterpret_cast<Element*>(&element->storage)) T(std::forward<Args>(args)...);
    }
 
    void push(T* p) noexcept {
        if (p == nullptr) {
            return;
        }

        auto element = reinterpret_cast<Element*>(p);
        if ((element < &_pool[0] || element > &_pool[N])) {
            return;
        }

        p->~T();
        element->next = _nextAvail;
        _nextAvail = element;
        _used[element - &_pool[0]] = nullptr;
    }

    iterator begin() { 
        auto first = &_used[0];
        while (!*first) {
            first++;
        }
        return iterator(first);
    }

    iterator end() { return iterator(&_used[N]); }

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

}
#endif /* ifndef _HDIV_POOL_H_ */
