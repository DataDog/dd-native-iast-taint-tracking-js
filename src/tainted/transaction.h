// Copyright 2022 Datadog, Inc.
#ifndef SRC_TAINTED_TRANSACTION_H_
#define SRC_TAINTED_TRANSACTION_H_

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
using RangePool = iast::container::Pool<iast::tainted::Range, iast::Limits::MAX_TAINTED_RANGES>;

namespace iast {
namespace tainted {

struct NotAvailableRangeVectorsException { };

class Transaction {
 public:
    Transaction();
    ~Transaction();

    void clean(void);

    inline void setId(uintptr_t id) { transactionId = id;}
    InputInfo* createNewInputInfo(v8::Local<v8::Value> parameterName,
            v8::Local<v8::Value> parameterValue,
            v8::Local<v8::Value> type);
    void cleanInputInfos(void);
    void cleanSharedVectors(void);

    inline TaintedObject* GetAvailableTaintedObject(void) {
        return taintedObjPool.pop(this->transactionId);
    }

    inline void returnTaintedObject(TaintedObject* taintedObject) {
        if (!taintedObject) {
            return;
        }

        taintedObjPool.push(taintedObject);
    }

    inline Range* GetAvailableTaintedRange(int start, int end, InputInfo *inputInfo) {
        return availableTaintedRanges.pop(start, end, inputInfo);
    }

    inline SharedRanges* GetAvailableSharedVector(void) {
        SharedRanges* taintedRangeVector = nullptr;
        if (!this->availableSharedVectors->empty()) {
            taintedRangeVector = this->availableSharedVectors->front();
            taintedRangeVector->clear();
            this->availableSharedVectors->pop();
        } else if (this->createdSharedVectors < Limits::MAX_TAINTED_RANGE_VECTORS) {
            taintedRangeVector = new SharedRanges(this->createdSharedVectors++);
        }
        if (taintedRangeVector != nullptr) {
            inUseSharedVectors[taintedRangeVector->getId()] = taintedRangeVector;
            return taintedRangeVector;
        }
        throw NotAvailableRangeVectorsException();
    }

    inline SharedRanges* GetRanges(uintptr_t stringPointer) {
        auto taintedObject = taintedMap->find(stringPointer);
        if (taintedObject != nullptr) {
            return taintedObject->getRanges();
        }
        return nullptr;
    }

    inline void UpdateRanges(uintptr_t stringPointer, SharedRanges* taintedRanges) {
        auto taintedObject = taintedMap->find(stringPointer);
        if (taintedObject != nullptr) {
            taintedObject->setRanges(taintedRanges);
        }
    }

    inline void RehashMap(void) {
        taintedMap->rehash();
    }
    inline void AddTainted(uintptr_t key, TaintedObject* tainted) {
        taintedMap->insert(key, tainted);
    }

    inline void ReturnSharedVector(SharedRanges* ranges) {
        availableSharedVectors->push(ranges);
        inUseSharedVectors[ranges->getId()] = nullptr;
    }


    TaintedPool taintedObjPool;

 private:
    SharedRanges* inUseSharedVectors[Limits::MAX_TAINTED_OBJECTS] = {};
    std::queue<SharedRanges*>* availableSharedVectors;
    WeakMap* taintedMap;
    int createdSharedVectors;
    RangePool availableTaintedRanges;
    std::vector<InputInfo*> inputInfoVector;
    uintptr_t transactionId;
};

}  // namespace tainted
}  // namespace iast
#endif  // SRC_TAINTED_TRANSACTION_H_
