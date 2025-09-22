/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_TRANSACTION_MANAGER_H_
#define SRC_TRANSACTION_MANAGER_H_
#include <cstdint>
#include <map>
#include <iostream>
#include <atomic>
#include "container/queued_pool.h"


namespace iast {

template <typename T, typename U>
class TransactionManager {
 private:
    static std::atomic<uint64_t> next_id_;
    
 public:
    TransactionManager() = default;
    TransactionManager(TransactionManager const&) = delete;
    void operator=(TransactionManager const&) = delete;
    
    static U GenerateNewId() {
        U id = static_cast<U>(next_id_++);
        std::cout << "~ Generated new transaction ID: " << id << std::endl;
        return id;
    }
    
    std::pair<U, T*> NewWithGeneratedId() {
        if (_map.size() >= _maxItems) {
            std::cout << "~ Transaction max items reached " << std::endl;
            return {0, nullptr};
        }
        
        U id = GenerateNewId();
        T* item = _pool.Pop(id);
        _map[id] = item;
        std::cout << "~ Created transaction with generated ID: " << id << std::endl;
        return {id, item};
    }

    T* New(U id) {
        std::cout << "~ Transaction New " << id << std::endl;
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
        for (auto entry : _map) {
            std::cout << "~ Transaction Get for entry: " << entry.first << std::endl;
        }
        std::cout << "~ Transaction Get ~~~~~~~" << std::endl;
        auto found = _map.find(id);
        if (found == _map.end()) {
            std::cout << "~ Transaction Get - NO transaction found :'( " << id << std::endl;
            return nullptr;
        } else {
            std::cout << "~ Transaction Get - transaction found :))) " << id << std::endl;
            return found->second;
        }
    }

    void Remove(U id) noexcept {
        std::cout << "~ Transaction Remove " << id << std::endl;
        auto found = _map.find(id);
        if (found != _map.end()) {
            T* item = found->second;
            _map.erase(found);
            item->Clean();
            _pool.Push(item);
        }
    }

    void RehashAll(void) noexcept {
        std::cout << "~ Transaction RehashAll" << std::endl;
        for (auto entry : _map) {
            std::cout << "~ Transaction RehashAll entry: " << entry.first << std::endl;
            if (entry.second) {
                entry.second->RehashMap();
            }
        }
    }

    void Clear(void) noexcept {
        std::cout << "~ Transaction Clear" << std::endl;
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

template <typename T, typename U>
std::atomic<uint64_t> TransactionManager<T, U>::next_id_{1};

}   // namespace iast
#endif  // SRC_TRANSACTION_MANAGER_H_
