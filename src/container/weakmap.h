/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/

#ifndef SRC_CONTAINER_WEAKMAP_H_
#define SRC_CONTAINER_WEAKMAP_H_

#include <stddef.h>
#include <cstdint>
#include <iostream>

#include "../weakiface.h"

#define GET_INDEX(ptr, mask) (ptr >> 3 & (mask))

enum {
    WEAK_MAP_SUCCESS = 0,
    WEAK_MAP_INVALID_ARG,
    WEAK_MAP_MAX_ELEM,
    WEAK_MAP_MAX
};

namespace iast {
namespace container {
template <typename T, size_t N>
class WeakMap {
 public:
    WeakMap() {
        this->_count = 0;
    }

    ~WeakMap() {
        for (size_t index = 0; index < N; index++) {
            auto obj = this->items[index];
            while (obj != nullptr) {
                auto next = obj->_next;
                delete obj;
                obj = next;
            }
        }
    }

    void Clean() {
        // TODO(julio): check memset std::fill for performance.
        for (size_t i = 0; i < N; i++) {
            this->items[i] = nullptr;
        }
        _count = 0;
    }

    T Find(weak_key_t key) {
        T ret = nullptr;
        auto index = GET_INDEX(key, MASK);
        std::cout << "~ Find " << index << std::endl;
        auto obj = static_cast<T>(this->items[index]);
        while (obj != nullptr) {
            if (obj->_key == key) {
                ret = obj->IsEmpty() ? nullptr : static_cast<T>(obj);
                break;
            } else {
                obj = static_cast<T>(obj->_next);
            }
        }
        return ret;
    }

    int Insert(weak_key_t key, T obj) {
        if (!obj) {
            return WEAK_MAP_INVALID_ARG;
        }

        // TODO(julio): check policy for insertion:
        // * If max elements are exhausted then error (current).
        // * If all buckets are exhausted then error (Likely to be selected).
        // * Replace old element with the new one.
        if (_count >= N) {
            return WEAK_MAP_MAX_ELEM;
        }

        auto index = GET_INDEX(key, MASK);
        std::cout << "~ Insert " << index << std::endl;
        auto taintedObjectInArray = this->items[index];
        if (taintedObjectInArray != nullptr) {
            obj->_next = taintedObjectInArray;
        } else {
            obj->_next = nullptr;
        }
        this->items[index] = obj;
        _count++;
        return WEAK_MAP_SUCCESS;
    }

    void Del(weak_key_t key) {
        auto index = GET_INDEX(key, MASK);
        std::cout << "~ Del " << index << std::endl;
        auto root = this->items[index];
        auto prev = root;
        if (root->_key == key) {
            this->items[index] = root->_next;
            delete root;
            _count--;
        } else {
            for (root = root->_next; root != nullptr; root = root->_next) {
                if (root->_key == key) {
                    prev->_next = root->_next;
                    delete root;
                    _count--;
                    break;
                }
                prev = root;
            }
        }
    }

    void Rehash() {
        for (size_t index = 0; index < N; index++) {
            T prev = nullptr;
            auto obj = static_cast<T>(this->items[index]);
            while (obj != nullptr) {
                if (obj->IsEmpty()) {
                    // removed by GC so prev remains the same
                    remove(index, prev, obj);
                    auto toDelete = obj;
                    obj = static_cast<T>(obj->_next);
                    toDelete->_next = nullptr;
                } else {
                    auto newPointer = obj->Get();
                    if (newPointer != obj->_key) {
                        // moved by GC so prev remains the same
                        remove(index, prev, obj);
                        auto toInsert = obj;
                        obj = static_cast<T>(obj->_next);

                        toInsert->_next = nullptr;
                        toInsert->_key = newPointer;
                        Insert(newPointer, toInsert);
                    } else {
                        prev = obj;
                        obj = static_cast<T>(obj->_next);
                    }
                }
            }
        }
    }

    int GetCount(void) { return _count; }

 private:
    const int MASK = N - 1;
    size_t _count;
    WeakObjIface<T>* items[N] = {};

    void remove(int index, T prev, T obj) {
        auto next = obj->_next;
        if (prev == nullptr) {
            this->items[index] = next;
        } else {
            prev->_next = next;
        }
        _count--;
    }
};
}  // namespace container
}  // namespace iast
#endif  // SRC_CONTAINER_WEAKMAP_H_
