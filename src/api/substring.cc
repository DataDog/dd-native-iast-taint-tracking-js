/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef STRINGSUBSTRINGOBJECT_CC
#define STRINGSUBSTRINGOBJECT_CC

#include <node.h>
#include <vector>

#include "substring.h"
#include "../iastlimits.h"
#include "../tainted/range.h"
#include "../tainted/transaction.h"
#include "../tainted/string_resource.h"
#include "../iast.h"
#include "../utils/propagation.h"
#include "../utils/string_utils.h"
#include "v8.h"

#define SUBSTRING_MIN_ARGC  3
#define TO_V8STRING(arg) (v8::Local<v8::String>::Cast(arg))

using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::NewStringType;
using v8::Object;
using v8::String;
using v8::Value;

using iast::utils::GetLocalPointer;
using iast::utils::getRangesInSlice;
using iast::utils::ToIntegerOrInfinity;
using iast::utils::ClampIndex;
using iast::utils::ClampRelativeIndex;

namespace iast {
namespace api {
void substring(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();
    int argc = args.Length();

    if (argc < SUBSTRING_MIN_ARGC) {
        isolate->ThrowException(Exception::TypeError(
                        String::NewFromUtf8(isolate,
                        "Wrong number of arguments",
                        NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    auto result = args[1];
    if (argc == SUBSTRING_MIN_ARGC) {
        args.GetReturnValue().Set(result);
        return;
    }


    auto subject = args[2];
    int resultLen = TO_V8STRING(result)->Length();
    int subjectLen = TO_V8STRING(subject)->Length();
    if (resultLen == 0) {
        args.GetReturnValue().Set(result);
        return;
    }
    auto transaction = GetTransaction(GetLocalPointer(args[0]));
    if (transaction == nullptr) {
        args.GetReturnValue().Set(result);
        return;
    }

    if (subjectLen <= 1) {
        args.GetReturnValue().Set(result);
        return;
    }

    auto taintedObj = transaction->FindTaintedObject(GetLocalPointer(subject));
    if (!taintedObj) {
        args.GetReturnValue().Set(result);
        return;
    }

    double startArg;
    double endArg = subjectLen;
    if (!ToIntegerOrInfinity(context, args[3], &startArg) ||
        (argc > 4 && !args[4]->IsUndefined() && !ToIntegerOrInfinity(context, args[4], &endArg))) {
        args.GetReturnValue().Set(result);
        return;
    }

    try {
        int start = ClampIndex(startArg, subjectLen);
        int end = ClampIndex(endArg, subjectLen);
        auto newRanges = getRangesInSlice(transaction, taintedObj, MIN(start, end), MAX(start, end));
        if (newRanges && newRanges->Size() > 0) {
            if (resultLen == 1) {
                result = tainted::NewExternalString(isolate, args[1]);
            }
            transaction->AddTainted(GetLocalPointer(result), newRanges, result);
        }
    } catch (const std::bad_alloc& err) {
    } catch (const container::QueuedPoolBadAlloc& err) {
    } catch (const container::PoolBadAlloc& err) {
    }

    args.GetReturnValue().Set(result);
}

void substr(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();
    int argc = args.Length();

    if (argc < SUBSTRING_MIN_ARGC) {
        isolate->ThrowException(Exception::TypeError(
                        String::NewFromUtf8(isolate,
                        "Wrong number of arguments",
                        NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    auto result = args[1];
    int resultLen = TO_V8STRING(result)->Length();
    if (argc == SUBSTRING_MIN_ARGC) {
        args.GetReturnValue().Set(result);
        return;
    }

    if (resultLen == 0) {
        args.GetReturnValue().Set(result);
        return;
    }

    auto subject = args[2];
    int subjectLen = TO_V8STRING(subject)->Length();
    auto transaction = GetTransaction(GetLocalPointer(args[0]));
    if (transaction == nullptr) {
        args.GetReturnValue().Set(result);
        return;
    }

    if (subjectLen <= 1) {
        args.GetReturnValue().Set(result);
        return;
    }

    auto taintedObj = transaction->FindTaintedObject(GetLocalPointer(subject));
    if (!taintedObj) {
        args.GetReturnValue().Set(result);
        return;
    }

    double startArg;
    double lengthArg = subjectLen;
    if (!ToIntegerOrInfinity(context, args[3], &startArg) ||
        (argc > 4 && !args[4]->IsUndefined() && !ToIntegerOrInfinity(context, args[4], &lengthArg))) {
        args.GetReturnValue().Set(result);
        return;
    }

    try {
        int start = ClampRelativeIndex(startArg, subjectLen);
        int end = MIN(start + ClampIndex(lengthArg, subjectLen), subjectLen);

        auto newRanges = getRangesInSlice(transaction, taintedObj, start, end);
        if (newRanges && newRanges->Size() > 0) {
            if (resultLen == 1) {
                result = tainted::NewExternalString(isolate, args[1]);
            }
            transaction->AddTainted(GetLocalPointer(result), newRanges, result);
        }
    } catch (const std::bad_alloc& err) {
    } catch (const container::QueuedPoolBadAlloc& err) {
    } catch (const container::PoolBadAlloc& err) {
    }

    args.GetReturnValue().Set(result);
}


void Substring::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "substring", substring);
    NODE_SET_METHOD(exports, "substr", substr);
}

}   // namespace api
}   // namespace iast
#endif
