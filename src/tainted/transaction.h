/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_TAINTED_TRANSACTION_H_
#define SRC_TAINTED_TRANSACTION_H_

#include <stdint.h>
#include <node.h>
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
    explicit Transaction(transaction_key_t id, v8::Local<v8::Value> jsObject)
        : _id(id), _jsObjectRef(v8::Isolate::GetCurrent(), jsObject) {}
    ~Transaction() noexcept;
    void Clean(void) noexcept;

    InputInfo* createNewInputInfo(v8::Local<v8::Value> parameterName,
            v8::Local<v8::Value> parameterValue,
            v8::Local<v8::Value> type);

    Range* GetRange(int start, int end, InputInfo *inputInfo, secure_marks_t secureMarks) {
        return _rangesPool.Pop(start, end, inputInfo, secureMarks);
    }

    SharedRanges* GetSharedVectorRange(void) {
        auto sharedRanges = _sharedRangesPool.Pop();
        _usedSharedRanges.push(sharedRanges);
        return sharedRanges;
    }

    TaintedObject* FindTaintedObject(weak_key_t stringPointer) noexcept {
        return _taintedMap.Find(stringPointer);
    }

    int GetTaintedCount() {
        return _taintedMap.GetCount();
    }

    void RehashMap(void) noexcept {
        _taintedMap.Rehash();
    }

    void AddTainted(weak_key_t key, SharedRanges* ranges, v8::Local<v8::Value> jsValue) {
        // TODO(julio): trigger exception from the pool rather than a nullptr
        auto tainted = _taintedObjPool.Pop(key,
                ranges,
                jsValue);
        if (tainted) {
            _taintedMap.Insert(key, tainted);
        }
    }

    bool HasJsObjectReference() const noexcept {
        return !_jsObjectRef.IsEmpty();
    }

    transaction_key_t GetCurrentTransactionKey() const noexcept {
        if (_jsObjectRef.IsEmpty()) {
            return _id;
        }
        auto isolate = v8::Isolate::GetCurrent();
        if (!isolate) {
            return _id;
        }
        v8::HandleScope handle_scope(isolate);
        auto localRef = _jsObjectRef.Get(isolate);
        return *reinterpret_cast<uintptr_t*>(*localRef);
    }

    transaction_key_t GetOriginalTransactionKey() const noexcept {
        return _id;
    }

    void UpdateTransactionKey(transaction_key_t newKey) noexcept {
        _id = newKey;
    }

    void UpdateJsObjectReference(v8::Local<v8::Value> jsObject) noexcept {
        if (!_jsObjectRef.IsEmpty()) {
            _jsObjectRef.Reset();
        }
        _jsObjectRef.Reset(v8::Isolate::GetCurrent(), jsObject);
    }

    void Reinitialize(transaction_key_t id, v8::Local<v8::Value> jsObject) noexcept {
        Clean();

        _id = id;
        UpdateJsObjectReference(jsObject);
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
    v8::Persistent<v8::Value> _jsObjectRef;
};

}  // namespace tainted
}  // namespace iast
#endif  // SRC_TAINTED_TRANSACTION_H_
