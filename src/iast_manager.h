#ifndef SRC_IAST_MANAGER_H_
#define SRC_IAST_MANAGER_H_
// Copyright 2022 Datadog, Inc.
#include <cstdint>
#include <map>
#include <iostream>
#include "container/queued_pool.h"

using iast_key_t = uintptr_t;

namespace iast {
template <typename T>
class IastManager {
 public:
    IastManager() = default;
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
                item->setId(id);
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
            item->clean();
            _pool.push(item);
        }
    }

    void RehashAll(void) {
        for (auto entry : _map) {
            if (entry.second)
                entry.second->RehashMap();
        }
    }

    void Clear(void) {
        for (auto it = _map.begin(); it != _map.end(); ++it) {
            _pool.push(it->second);
        }
        _map.clear();
        _pool.clear();
    }

    size_t Size() { return _map.size(); }
    void setMaxItems(size_t max) { _maxItems = max; }
    int getMaxItems(void) { return _maxItems; }

 private:
    size_t _maxItems = 2;
    container::QueuedPool<T> _pool;
    std::map<iast_key_t, T*> _map;
};

}   // namespace iast
#endif  // SRC_IAST_MANAGER_H_
