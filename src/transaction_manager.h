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

template <typename T, typename U>
class TransactionManager {
 public:
    TransactionManager() = default;
    TransactionManager(TransactionManager const&) = delete;
    void operator=(TransactionManager const&) = delete;

    T* New(U id) {
        auto found = _map.find(id);
        if (found == _map.end()) {
            if (_map.size() >= _maxItems) {
                std::cout << "~ Transaction max items reached " << std::endl;
                return nullptr;
            }

            T* item = _pool.Pop(id);
            _map[id] = item;
            return item;
        } else {
            auto item = found->second;
            return item;
        }
    }

    T* Get(U id) {
        std::cout << "~ Transaction Get " << id << std::endl;
        auto found = _map.find(id);
        if (found == _map.end()) {
            std::cout << "~ Transaction Get - no transaction :(" << id << std::endl;
            return nullptr;
        } else {
            std::cout << "~ Transaction Get - found transaction :(" << id << std::endl;
            return found->second;
        }
    }

    void Remove(U id) noexcept {
        auto found = _map.find(id);
        if (found != _map.end()) {
            T* item = found->second;
            _map.erase(found);
            item->Clean();
            _pool.Push(item);
        }
    }

    void RehashAll(void) noexcept {
        for (auto entry : _map) {
            if (entry.second) {
                entry.second->RehashMap();
            }
        }
    }

    void Clear(void) noexcept {
        for (auto it = _map.begin(); it != _map.end(); ++it) {
            _pool.Push(it->second);
        }
        _map.clear();
        _pool.Clear();
    }

    size_t Size() noexcept { return _map.size(); }
    void setMaxItems(size_t max) noexcept { _maxItems = max; }
    int getMaxItems(void) noexcept { return _maxItems; }

 private:
    size_t _maxItems = 2;
    container::QueuedPool<T> _pool;
    std::map<U, T*> _map;
};

}   // namespace iast
#endif  // SRC_TRANSACTION_MANAGER_H_
