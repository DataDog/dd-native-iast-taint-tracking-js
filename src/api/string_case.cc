/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <new>
#include <vector>
#include <memory>
#include <string>

#include "string_case.h"
#include "../tainted/range.h"
#include "../tainted/string_resource.h"
#include "../tainted/transaction.h"
#include "../iast.h"

#define TO_V8STRING(arg) (v8::Local<v8::String>::Cast(arg))

using v8::FunctionCallbackInfo;
using v8::Value;
using v8::Local;
using v8::Isolate;
using v8::Object;
using v8::String;

using iast::tainted::Range;

namespace iast {
namespace api {

void StringCaseOperator(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    if (args.Length() < 3) {
        isolate->ThrowException(v8::Exception::TypeError(
                        v8::String::NewFromUtf8(isolate,
                        "Wrong number of arguments",
                        v8::NewStringType::kNormal).ToLocalChecked()));
        return;
    }
    if (!args[1]->IsString()) {
        args.GetReturnValue().Set(args[1]);
        return;
    }

    auto transaction = GetTransaction(utils::GetLocalPointer(args[0]));
    if (transaction == nullptr) {
        args.GetReturnValue().Set(args[1]);
        return;
    }
    
    if (args[1] == args[2]) {
        args.GetReturnValue().Set(args[1]);
        return;
    }
    
    auto taintedObj = transaction->FindTaintedObject(utils::GetLocalPointer(args[2]));
    if (!taintedObj) {
        args.GetReturnValue().Set(args[1]);
        return;
    }

    try {
        auto ranges = taintedObj ? taintedObj->getRanges() : nullptr;
        if (ranges == nullptr) {
            args.GetReturnValue().Set(args[1]);
            return;
        }
        
        auto res = args[1];
        int resultLength = TO_V8STRING(res)->Length();
        if (resultLength == 1) {
            res = tainted::NewExternalString(isolate, res);
        }
        auto key = utils::GetLocalPointer(res);
        transaction->AddTainted(key, ranges, res);
        args.GetReturnValue().Set(res);
        return;

    } catch (const std::bad_alloc& err) {
    } catch (const container::QueuedPoolBadAlloc& err) {
    } catch (const container::PoolBadAlloc& err) {
    }
    args.GetReturnValue().Set(args[1]);
}

void StringCaseOperations::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "stringCase", StringCaseOperator);
}
}   // namespace api
}   // namespace iast

