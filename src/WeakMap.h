#ifndef TAINTED_MAP
#define TAINTED_MAP
//#define IMPROVE_INSERT

#include <cstdint>
#include <stddef.h>

#include "v8.h"


#define GET_INDEX(POINTER) (POINTER >> 3 & MASK)
#define INSERT_HANDLE_COLLISION(key, obj, arr)  do {                                                    \
                                                    auto index = GET_INDEX(key);                        \
                                                    auto taintedObjectInArray = arr[index];             \
                                                    if (taintedObjectInArray != nullptr) {              \
                                                        obj->next = taintedObjectInArray;               \
                                                    } else {                                            \
                                                        obj->next = nullptr;                            \
                                                    }                                                   \
                                                    arr[index] = obj;                                   \
                                                } while (0);

#define INSERT_COERCE_COLLISION(key, obj, arr)  do {                            \
                                                    arr[GET_INDEX(key)] = obj;  \
                                                } while (0);

#ifdef IMPROVE_INSERT
#define INSERT  INSERT_COERCE_COLLISION
#else
#define INSERT  INSERT_HANDLE_COLLISION
#endif

namespace tainted
{
    using weak_key_t = uintptr_t;

    template <typename T, size_t N>
    class WeakMap {
        public:
            WeakMap(){
                this->count = 0;
            }

            ~WeakMap(){
                for (int index = 0; index < N; index++) {
                    auto obj = this->items[index];
                    while (obj != nullptr) {
                        auto next = obj->next;
                        delete obj;
                        obj = next;
                    }
                }
            }

            void clean() {
                // TODO: check memset std::fill for performance.
                for (int i = 0; i < N; i++) {
                    this->items[i] = nullptr;
                }
            }

            inline T find(weak_key_t key) {
                auto index = GET_INDEX(key);
                auto obj = this->items[index];
                #ifndef IMPROVE_INSERT
                while (obj != nullptr) {
                    if(obj->pointerToV8String == key) {
                        return obj->target.IsEmpty() ? nullptr : obj;
                    } else {
                        obj = obj->next;
                    }
                }
                return nullptr;
                #else
                return obj != nullptr && obj->pointerToV8String == pointerToV8String ? obj : nullptr;
                #endif

            }

            inline void insert(weak_key_t key, T obj) {
                INSERT(key, obj, this->items);
            }

            void rehash() {
                for (int index = 0; index < N; index++) {
                    T prev = nullptr;
                    auto taintedObject = this->items[index];
                    while (taintedObject != nullptr) {
                        if (taintedObject->target.IsEmpty()) {
                            // removed by GC so prev remains the same
                            remove(index, prev, taintedObject);
                            auto toDelete = taintedObject;
                            taintedObject = taintedObject->next;
                            toDelete->next = nullptr;
                        } else {
                            auto newPointer = utils::GetLocalStringPointer(
                                    taintedObject->target.Get(Isolate::GetCurrent()));
                            if (newPointer != taintedObject->pointerToV8String) {
                                // moved by GC so prev remains the same
                                remove(index, prev, taintedObject);
                                auto toInsert = taintedObject;
                                taintedObject = taintedObject->next;

                                toInsert->next = nullptr;
                                toInsert->pointerToV8String = newPointer;
                                insert(newPointer, toInsert);
                            } else {
                                prev = taintedObject;
                                taintedObject = taintedObject->next;
                            }
                        }
                    }
                }
            }

        private:
            const int MASK = N - 1;
            int count;
            T items[N] = {};

            inline void remove(int index, T prev, T obj) {
                auto next = obj->next;
                if (prev == nullptr) {
                    this->items[index] = next;
                } else {
                    prev->next = next;
                }
            }
    };
} // namespace tainted
#endif
