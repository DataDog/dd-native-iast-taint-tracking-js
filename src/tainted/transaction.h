/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_TAINTED_TRANSACTION_H_
#define SRC_TAINTED_TRANSACTION_H_

#include <cstddef>
#include <cstdint>
#include <map>
#include <queue>
#include <unordered_map>
#include <vector>

#include "../tainted/range.h"
#include "../tainted/tainted_object.h"
#include "../container/queued_pool.h"
#include "../container/shared_vector.h"

using SharedRanges = iast::container::SharedVector<iast::tainted::Range*>;
using WeakMap = iast::container::WeakMap<iast::tainted::TaintedObject*, iast::Limits::MAX_TAINTED_OBJECTS>;
using TaintedPool = iast::container::Pool<iast::tainted::TaintedObject, iast::Limits::MAX_TAINTED_OBJECTS>;
using RangePool = iast::container::Pool<iast::tainted::Range, iast::Limits::MAX_GLOBAL_TAINTED_RANGES>;
using SharedRangesPool = iast::container::QueuedPool<SharedRanges, iast::Limits::MAX_TAINTED_OBJECTS>;

namespace iast {
namespace tainted {

class Transaction {
 public:
    Transaction();
    ~Transaction();

    void clean(void);

    void setId(uintptr_t id) { transactionId = id;}
    InputInfo* createNewInputInfo(v8::Local<v8::Value> parameterName,
            v8::Local<v8::Value> parameterValue,
            v8::Local<v8::Value> type);
    void cleanInputInfos(void);
    void cleanSharedVectors(void);

    TaintedObject* GetTaintedObject(void) {
        return taintedObjPool.pop();
    }

    void ReturnTaintedObject(TaintedObject* taintedObject) {
        if (!taintedObject) {
            return;
        }

        taintedObjPool.push(taintedObject);
    }

    Range* GetRange(int start, int end, InputInfo *inputInfo) {
        return availableTaintedRanges.pop(start, end, inputInfo);
    }

    SharedRanges* GetSharedVectorRange(void) {
        auto sharedRanges = _sharedRangesPool.pop();
        _usedSharedRanges.push(sharedRanges);
        return sharedRanges;
    }

    TaintedObject* FindTaintedObject(weak_key_t stringPointer) {
        return taintedMap->find(stringPointer);
    }

    void UpdateRanges(weak_key_t stringPointer, SharedRanges* taintedRanges) {
        auto taintedObject = taintedMap->find(stringPointer);
        if (taintedObject != nullptr) {
            taintedObject->setRanges(taintedRanges);
        }
    }

    void RehashMap(void) {
        taintedMap->rehash();
    }
    void AddTainted(weak_key_t key, TaintedObject* tainted) {
        taintedMap->insert(key, tainted);
    }

    void AddTainted(weak_key_t key, SharedRanges* ranges, v8::Local<v8::Value> jsString) {
        // TODO(julio): trigger exception from the pool rather than a nullptr
        auto tainted = taintedObjPool.pop(key,
                ranges,
                jsString);
        if (tainted) {
            taintedMap->insert(key, tainted);
        }
    }

 private:
    TaintedPool taintedObjPool;
    std::queue<SharedRanges*> _usedSharedRanges;
    SharedRangesPool _sharedRangesPool;
    WeakMap* taintedMap;
    RangePool availableTaintedRanges;
    std::vector<InputInfo*> inputInfoVector;
    uintptr_t transactionId;
};

}  // namespace tainted
}  // namespace iast
#endif  // SRC_TAINTED_TRANSACTION_H_
