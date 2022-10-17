/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_CONTAINER_QUEUED_POOL_H_
#define SRC_CONTAINER_QUEUED_POOL_H_
#include <cstddef>
#include <cstdint>
#include <exception>
#include <queue>
#include <stdexcept>
#include <utility>
namespace iast {
namespace container {

class QueuedPoolBadAlloc : public std::exception {
};

template<typename T, size_t N = SIZE_MAX>
class QueuedPool {
 public:
     QueuedPool() {}
    ~QueuedPool() {
        Clear();
    }
    explicit QueuedPool(T const&) = delete;
    explicit QueuedPool(T&&) = delete;


    template<class ...Args>
    T* Pop(Args&&...args) {
        if (_pool.empty()) {
            if (_count >= N) {
                throw QueuedPoolBadAlloc();
            }
            _count++;
            return new T(std::forward<Args>(args)...);
        }

        T* item = _pool.front();
        _pool.pop();
        return item;
    }

    void Push(T* item) noexcept {
        if (item) {
            _pool.push(item);
        }
    }
    size_t Size() noexcept { return _count; }
    size_t Available() noexcept { return _pool.size(); }
    void Clear(void) noexcept {
        while (!_pool.empty()) {
            T* item = _pool.front();
            _pool.pop();
            delete item;
            _count--;
        }
    }

 private:
    size_t _count = 0;
    std::queue<T*> _pool;
};
}  // namespace container
}  // namespace iast
#endif  // SRC_CONTAINER_QUEUED_POOL_H_
