/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_TAINTED_TAINTED_OBJECT_H_
#define SRC_TAINTED_TAINTED_OBJECT_H_
#include <node.h>

#include <cstdint>
#include <vector>
#include <memory>

#include "range.h"
#include "../weakiface.h"
#include "../utils/string_utils.h"
#include "../container/pool.h"
#include "../container/weakmap.h"
#include "../container/shared_vector.h"
#include "../iastlimits.h"

using SharedRanges = iast::container::SharedVector<iast::tainted::Range*>;
namespace iast {
namespace tainted {
class TaintedObject: public iast::WeakObjIface<TaintedObject*> {
 public:
    TaintedObject();
    explicit TaintedObject(uintptr_t transactionIdPointer);
    TaintedObject(uintptr_t transactionId,
            uintptr_t pointerToV8String,
            SharedRanges* ranges,
            v8::Local<v8::Value> jsString);
    TaintedObject(const TaintedObject&) = delete;
    ~TaintedObject();

    bool IsEmpty() { return target.IsEmpty(); }

    uintptr_t Get() {
        return utils::GetLocalStringPointer(target.Get(v8::Isolate::GetCurrent()));
    }

    void Reset(v8::Local<v8::Value> v) {
        target.Reset(v8::Isolate::GetCurrent(), v);
    }

    void Reset() {
        target.Reset();
    }

    v8::Local<v8::Object> toJSObject(v8::Isolate* isolate);
    SharedRanges* getRanges(void) { return _ranges; }
    void setRanges(SharedRanges* ranges) { _ranges = ranges; }
    uintptr_t getId(void) { return _transactionId; }

 private:
    uintptr_t _transactionId;
    SharedRanges* _ranges;
    v8::Persistent<v8::Value> target;
};
}   // namespace tainted
}   // namespace iast
#endif  // SRC_TAINTED_TAINTED_OBJECT_H_
