/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include "v8.h"
#include "slice.h"
#include "../tainted/range.h"
#include "../tainted/string_resource.h"
#include "../tainted/transaction.h"
#include "../iast.h"
#include "../utils/propagation.h"

namespace iast {
namespace api {

using v8::FunctionCallbackInfo;
using v8::Object;
using v8::Local;
using v8::Value;
using v8::Isolate;
using v8::String;
using v8::NewStringType;
using v8::Exception;
using utils::GetLocalStringPointer;
using utils::getRangesInSlice;

void slice(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    if (args.Length() < 4) {
        isolate->ThrowException(Exception::TypeError(
                        String::NewFromUtf8(isolate,
                        "Wrong number of arguments",
                        NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    auto context = isolate->GetCurrentContext();
    auto vResult = args[1];
    auto vSubject = args[2];

    int len =  v8::Local<v8::String>::Cast(vResult)->Length();
    if (len == 0) {
        args.GetReturnValue().Set(vResult);
        return;
    }

    int sliceStart = args[3]->IntegerValue(context).FromJust();

    Transaction* transaction = GetTransaction(GetLocalStringPointer(args[0]));
    if (transaction == nullptr) {
        args.GetReturnValue().Set(vResult);
        return;
    }

    auto taintedObj = transaction->FindTaintedObject(GetLocalStringPointer(vSubject));

    if (!taintedObj) {
        args.GetReturnValue().Set(vResult);
        return;
    }

    try {
        int subjectLength = Local<String>::Cast(vSubject)->Length();
        int resultLength = Local<String>::Cast(vResult)->Length();
        int sliceEnd = args.Length() > 4 ? args[4]->IntegerValue(context).FromJust() : subjectLength;
        sliceStart = sliceStart < 0 ? MAX(subjectLength + sliceStart, 0) : sliceStart;
        sliceEnd = sliceEnd < 0 ? subjectLength + sliceEnd : sliceEnd;
        auto newRanges = getRangesInSlice(transaction, taintedObj, sliceStart, sliceEnd);
        if (newRanges && newRanges->Size() > 0) {
            if (resultLength == 1) {
                vResult = tainted::NewExternalString(isolate, args[1]);
            }
            transaction->AddTainted(GetLocalStringPointer(vResult), newRanges, vResult);
        }
    } catch (const std::bad_alloc& err) {
    } catch (const container::QueuedPoolBadAlloc& err) {
    } catch (const container::PoolBadAlloc& err) {
    }

    args.GetReturnValue().Set(vResult);
}


void SliceOperations::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "slice", slice);
}
}   // namespace api
}   // namespace iast

