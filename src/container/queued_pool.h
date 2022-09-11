#include <cstddef>
#include <cstdint>
#include <queue>
#include <utility>
#ifndef SRC_CONTAINER_QUEUEDPOOL_H_
#define SRC_CONTAINER_QUEUEDPOOL_H_
namespace iast {
namespace container {
using pool_id_t = uintptr_t ;

template<typename T>
class QueuedPool {
    public:
        QueuedPool() {};
        ~QueuedPool() {
            clear();
        }
        QueuedPool(T const&) = delete;
        QueuedPool(T&&) = delete;


        template<class ...Args>
        T* pop(Args&&...args) {
            if (_pool.empty()) {
                _count++;
                return new T(std::forward<Args>(args)...);
            }

            T* item = _pool.front();
            _pool.pop();
            return item;
        }

        void push(T* item) {
            if (item) {
                _pool.push(item);
            }
        }
        size_t size() { return _count; }
        size_t available() { return _pool.size(); }
        void clear(void) {
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
#endif // SRC_CONTAINER_QUEUEDPOOL_H_
