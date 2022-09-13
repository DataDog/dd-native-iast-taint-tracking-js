#ifndef TAINTED_OBJECT_H
#define TAINTED_OBJECT_H
#include <node.h>

#include <cstdint>
#include <vector>
#include <memory>

#include "../weakiface.h"
#include "../utils/string_utils.h"
#include "range.h"
#include "../container/pool.h"
#include "../container/weakmap.h"
#include "../container/shared_vector.h"
#include "../iastlimits.h"

namespace iast {
namespace tainted {
class TaintedObject: public iast::WeakObjIface<TaintedObject*> {
 using SharedRanges = iast::container::SharedVector<iast::tainted::Range*>;
 public:
    TaintedObject();
    TaintedObject(uintptr_t transactionIdPointer);
    TaintedObject(uintptr_t pointerToV8String, SharedRanges* ranges, TaintedObject* next);
    TaintedObject(uintptr_t pointerToV8String, SharedRanges* ranges);
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

    void SetWeak(v8::WeakCallbackInfo<TaintedObject>::Callback callback, v8::WeakCallbackType type) {
        // TODO(julio): define a closure?
        target.SetWeak(this, callback, type);
    }

    v8::Local<v8::Object> toJSObject(v8::Isolate*);
    SharedRanges* getRanges(void) { return _ranges; };
    void setRanges(SharedRanges* ranges) { _ranges = ranges; }
    uintptr_t getId(void) { return _transactionId; }
 private:
    uintptr_t _transactionId;
    SharedRanges* _ranges;
    v8::Persistent<v8::Value> target;

};

} // namespace tainted
} // namespace iast
#endif
