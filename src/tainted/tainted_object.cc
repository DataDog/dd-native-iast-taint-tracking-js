// Copyright 2022 Datadog, Inc.
#include <node.h>
#include <cstdint>
#include <functional>

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
    this->_next = nullptr;
}

TaintedObject::TaintedObject(uintptr_t transactionId,
        uintptr_t pointerToV8String,
        SharedRanges* ranges,
        v8::Local<v8::Value> jsString): _transactionId(transactionId), _ranges(ranges) {
    this->_key = pointerToV8String;
    this->_next = nullptr;
    this->target.Reset(v8::Isolate::GetCurrent(), jsString);
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
}   // namespace tainted
}   // namespace iast
