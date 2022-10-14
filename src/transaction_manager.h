/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_TRANSACTION_MANAGER_H_
#define SRC_TRANSACTION_MANAGER_H_
#include <cstdint>
#include <map>
#include <iostream>
#include "container/queued_pool.h"


namespace iast {
using transaction_key_t = uintptr_t;

template <typename T>
class TransactionManager {
 public:
    TransactionManager() = default;
    TransactionManager(TransactionManager const&) = delete;
    void operator=(TransactionManager const&) = delete;

    T* New(transaction_key_t id) {
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

    T* Get(transaction_key_t id) {
        auto found = _map.find(id);
        if (found == _map.end()) {
            return nullptr;
        } else {
            return found->second;
        }
    }

    void Remove(transaction_key_t id) {
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
            if (entry.second) {
                entry.second->RehashMap();
            }
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
    std::map<transaction_key_t, T*> _map;
};

}   // namespace iast
#endif  // SRC_TRANSACTION_MANAGER_H_
