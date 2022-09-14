#include <node.h>

#include "../utils/jsobject_utils.h"

#include "tainted_object.h"
#include "range.h"

namespace iast {
namespace tainted {
    TaintedObject::TaintedObject() {
        this->_transactionId = 0;
        this->_key = 0;
        this->_ranges = nullptr;
        this->_next = nullptr;
    }
    TaintedObject::TaintedObject(uintptr_t transactionIdPointer) : _transactionId(transactionIdPointer) {
        this->_key = 0;
        this->_ranges = nullptr;
    }
    TaintedObject::TaintedObject(uintptr_t pointerToV8String, SharedRanges* ranges) {
        this->_key = pointerToV8String;
        this->_ranges = ranges;
        this->_next = nullptr;
    }

    TaintedObject::TaintedObject(uintptr_t pointerToV8String, SharedRanges* ranges, TaintedObject* next) {
        this->_key = pointerToV8String;
        this->_ranges = ranges;
        this->_next = next;
    }

    TaintedObject::~TaintedObject() {
        if (!this->target.IsEmpty()) {
            this->target.Reset();
        }
        this->_ranges = nullptr;
        this->_next = nullptr;
        this->_key = 0;
    }

    v8::Local<v8::Object> TaintedObject::toJSObject(v8::Isolate* isolate) {
        auto context = isolate->GetCurrentContext();
        auto taintedObjectv8Obj = v8::Object::New(isolate);
        auto local = v8::Local<v8::Value>::New(isolate, this->target);
        taintedObjectv8Obj->Set(context, utils::NewV8String(isolate, "value"), local);

        auto jsRanges = v8::Array::New(isolate);
        int length = this->_ranges->size();
        for (int i = 0; i < length; i++) {
            auto range = this->_ranges->at(i);
            auto jsRange = range->toJSObject(isolate);
            jsRanges->Set(context, i, jsRange);
        }

        taintedObjectv8Obj->Set(context, utils::NewV8String(isolate, "ranges"), jsRanges);
        return taintedObjectv8Obj;
    }
} // namespace tainted
} // namespace iast
