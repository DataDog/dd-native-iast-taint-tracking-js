/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_TRANSACTION_MANAGER_H_
#define SRC_TRANSACTION_MANAGER_H_
#include <cstdint>
#include <map>
#include <iostream>
#include <vector>
#include <node.h>
#include "container/queued_pool.h"


namespace iast {

template <typename T, typename U>
class TransactionManager {
 public:
    TransactionManager() = default;
    TransactionManager(TransactionManager const&) = delete;
    void operator=(TransactionManager const&) = delete;

    T* New(U id, v8::Local<v8::Value> jsObject) {
        auto found = _map.find(id);
        if (found == _map.end()) {
            if (_map.size() >= _maxItems) {
                return nullptr;
            }

            T* item;
            if (_pool.Available() > 0) {
                item = _pool.Pop();
                item->Reinitialize(id, jsObject);
            } else {
                item = _pool.Pop(id, jsObject);
            }
            _map[id] = item;
            return item;
        } else {
            auto item = found->second;
            if (item) {
                item->UpdateJsObjectReference(jsObject);
            }
            return item;
        }
    }

    T* Get(U id) {
        auto found = _map.find(id);
        if (found == _map.end()) {
            return nullptr;
        } else {
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

        RehashTransactionKeys();
    }

    void RehashTransactionKeys(void) noexcept {
        std::vector<std::pair<U, T*>> toReinsert;

        // Find transactions whose keys have changed due to GC
        for (auto it = _map.begin(); it != _map.end();) {
            auto transaction = it->second;
            if (transaction && transaction->HasJsObjectReference()) {
                auto currentKey = transaction->GetCurrentTransactionKey();
                auto originalKey = transaction->GetOriginalTransactionKey();

                if (currentKey != originalKey) {
                    // Key has changed due to GC, need to re-insert with new key
                    toReinsert.push_back({currentKey, transaction});
                    transaction->UpdateTransactionKey(currentKey);
                    it = _map.erase(it);
                } else {
                    ++it;
                }
            } else {
                ++it;
            }
        }

        // Re-insert transactions with updated keys
        for (auto& pair : toReinsert) {
            _map[pair.first] = pair.second;
        }
    }

    void Clear(void) noexcept {
        for (auto it = _map.begin(); it != _map.end(); ++it) {
            it->second->Clean();
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
