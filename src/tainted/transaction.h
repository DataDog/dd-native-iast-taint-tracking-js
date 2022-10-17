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
using transaction_key_t = uintptr_t;

class Transaction {
 public:
    Transaction() {}
    explicit Transaction(transaction_key_t id) : _id(id) {}
    ~Transaction() noexcept;
    void Clean(void) noexcept;

    InputInfo* createNewInputInfo(v8::Local<v8::Value> parameterName,
            v8::Local<v8::Value> parameterValue,
            v8::Local<v8::Value> type);

    Range* GetRange(int start, int end, InputInfo *inputInfo) {
        return _rangesPool.pop(start, end, inputInfo);
    }

    SharedRanges* GetSharedVectorRange(void) {
        auto sharedRanges = _sharedRangesPool.pop();
        _usedSharedRanges.push(sharedRanges);
        return sharedRanges;
    }

    TaintedObject* FindTaintedObject(weak_key_t stringPointer) noexcept {
        return _taintedMap.find(stringPointer);
    }

    void RehashMap(void) noexcept {
        _taintedMap.rehash();
    }

    void AddTainted(weak_key_t key, SharedRanges* ranges, v8::Local<v8::Value> jsString) {
        // TODO(julio): trigger exception from the pool rather than a nullptr
        auto tainted = _taintedObjPool.pop(key,
                ranges,
                jsString);
        if (tainted) {
            _taintedMap.insert(key, tainted);
        }
    }

 private:
    void cleanSharedVectors(void);
    void cleanInputInfos(void) noexcept;
    TaintedPool _taintedObjPool;
    std::queue<SharedRanges*> _usedSharedRanges;
    SharedRangesPool _sharedRangesPool;
    WeakMap _taintedMap;
    RangePool _rangesPool;
    std::vector<InputInfo*> _usedInputInfo;
    transaction_key_t _id;
};

}  // namespace tainted
}  // namespace iast
#endif  // SRC_TAINTED_TRANSACTION_H_
