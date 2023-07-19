/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <node.h>

#include "../iast_node.h"
#include "range.h"
#include "input_info.h"
#include "../utils/jsobject_utils.h"

using v8::String;
using v8::NewStringType;
using v8::Isolate;

namespace iast {
namespace tainted {
bool labelsDefined = false;
v8::Persistent<v8::Value> startLabel;
v8::Persistent<v8::Value> endLabel;
v8::Persistent<v8::Value> iinfoLabel;
v8::Persistent<v8::Value> secureMarksLabel;

Range::Range(int start, int end, InputInfo *inputInfo, secure_marks_t secureMarks) {
    this->start = start;
    this->end = end;
    this->inputInfo = inputInfo;
    this->secureMarks = secureMarks;
}

Range::Range(const Range& range) {
    this->start = range.start;
    this->end = range.end;
    this->inputInfo = range.inputInfo;
    this->secureMarks = range.secureMarks;
}

Range::~Range() {}

v8::Local<v8::Object> Range::toJSObject(v8::Isolate* isolate) {
    auto context = isolate->GetCurrentContext();
    auto taintedRangev8Obj = v8::Object::New(isolate);
    v8::Local<v8::Value> startLabelLocal;
    v8::Local<v8::Value> endLabelLocal;
    v8::Local<v8::Value> iinfoLabelLocal;
    v8::Local<v8::Value> secureMarksLabelLocal;
    if (!labelsDefined) {
        startLabelLocal = utils::NewV8String(isolate, "start");
        endLabelLocal = utils::NewV8String(isolate, "end");
        iinfoLabelLocal = utils::NewV8String(isolate, "iinfo");
        secureMarksLabelLocal = utils::NewV8String(isolate, "secureMarks");
        startLabel.Reset(isolate, startLabelLocal);
        endLabel.Reset(isolate, endLabelLocal);
        iinfoLabel.Reset(isolate, iinfoLabelLocal);
        secureMarksLabel.Reset(isolate, secureMarksLabelLocal);
        labelsDefined = true;
    } else {
        startLabelLocal = v8::Local<v8::Value>::New(isolate, startLabel);
        endLabelLocal = v8::Local<v8::Value>::New(isolate, endLabel);
        iinfoLabelLocal = v8::Local<v8::Value>::New(isolate, iinfoLabel);
        secureMarksLabelLocal = v8::Local<v8::Value>::New(isolate, secureMarksLabel);
    }

    taintedRangev8Obj->Set(context, startLabelLocal, v8::Number::New(isolate, this->start)).Check();
    taintedRangev8Obj->Set(context, endLabelLocal, v8::Number::New(isolate, this->end)).Check();
    taintedRangev8Obj->Set(context,
            iinfoLabelLocal,
            GetJsObjectFromInputInfo(isolate, context, this->inputInfo)).Check();
    taintedRangev8Obj->Set(context, secureMarksLabelLocal, v8::Number::New(isolate, this->secureMarks)).Check();

    return taintedRangev8Obj;
}
}   // namespace tainted
}   // namespace iast
