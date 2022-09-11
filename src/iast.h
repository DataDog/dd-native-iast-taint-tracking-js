#ifndef _IAST_H_
#define _IAST_H_

#include <map>
#include <cstdint>
#include <iostream>

#include "container/queued_pool.h"

namespace iast {

using iast_key_t = uintptr_t;

template<typename T>
class IastManager {
 public:
    static IastManager<T>& GetInstance(void) {
        static IastManager<T> _ctx;
        return _ctx;
    }

    IastManager(IastManager const&) = delete;
    void operator=(IastManager const&) = delete;

    T* New(iast_key_t id) {
        if (_map.size() >= _maxItems) {
            return nullptr;
        }

        auto found = _map.find(id);
        if (found == _map.end()) {
            T* item = _pool.pop();
            if (item != nullptr) {
                _map[id] = item;
                //item->setId(id);
            }
            return item;
        } else {
            auto item = found->second;
            return item;
        }
    }

    T* Get(iast_key_t id) {
        auto found = _map.find(id);
        if (found == _map.end()) {

            return nullptr;
        } else {
            return found->second;
        }
    }

    void Remove(iast_key_t id) {
        auto found = _map.find(id);
        if (found != _map.end()) {
            T* item = found->second;
            _map.erase(found);
            //item->clean();
            _pool.push(item);
        }
    }

    void RehashAll(void);
    void Clear(void) {
        for (auto it = _map.begin(); it != _map.end(); ++it) {
            _map.erase(it);
            delete it->second;
        }

        _pool.clear();
    }
    size_t Size() { return _map.size(); }
    void setMaxItems(int max) { _maxItems = max; }
    int getMaxItems(void) { return _maxItems; }

 private:
    IastManager() = default;
    int _maxItems = 2;
    container::QueuedPool<T> _pool;
    std::map<iast_key_t, T*> _map;
};
}  // namespace iast

#endif /* ifndef _IAST_H_ */

