#ifndef TAINTEDRANGE_CC
#define TAINTEDRANGE_CC

#include <v8.h>

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
    Range::Range(int start, int end, InputInfo *inputInfo) {
        this->start = start;
        this->end = end;
        this->inputInfo = inputInfo;
    }

    Range::Range(const Range& taintedRange) {
        this->start = taintedRange.start;
        this->end = taintedRange.end;
        this->inputInfo = taintedRange.inputInfo;
    }

    Range::~Range() {}

    v8::Local<v8::Object> GetJsObjectFromRange(Isolate* isolate, v8::Local<v8::Context> context, Range* taintedRange) {
        auto taintedRangev8Obj = v8::Object::New(isolate);
        v8::Local<v8::Value> startLabelLocal;
        v8::Local<v8::Value> endLabelLocal;
        v8::Local<v8::Value> iinfoLabelLocal;
        if (!labelsDefined) {
            startLabelLocal = utils::NewV8String(isolate, "start");
            endLabelLocal = utils::NewV8String(isolate, "end");
            iinfoLabelLocal = utils::NewV8String(isolate, "iinfo");
            startLabel.Reset(isolate, startLabelLocal);
            endLabel.Reset(isolate, endLabelLocal);
            iinfoLabel.Reset(isolate, iinfoLabelLocal);
            labelsDefined = true;
        } else {
            startLabelLocal = v8::Local<v8::Value>::New(isolate, startLabel);
            endLabelLocal = v8::Local<v8::Value>::New(isolate, endLabel);
            iinfoLabelLocal = v8::Local<v8::Value>::New(isolate, iinfoLabel);
        }
        taintedRangev8Obj->Set(context, startLabelLocal, v8::Number::New(isolate, taintedRange->start)).CHECK();
        taintedRangev8Obj->Set(context, endLabelLocal, v8::Number::New(isolate, taintedRange->end)).CHECK();
        taintedRangev8Obj->Set(context, iinfoLabelLocal, GetJsObjectFromInputInfo(isolate, context, taintedRange->inputInfo)).CHECK();

        return taintedRangev8Obj;
    }
} // namespace tainted
} // namespace iast
#endif
