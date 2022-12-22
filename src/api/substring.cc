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
#include "v8.h"

#define SUBSTRING_MIN_ARGC  3
#define TO_V8STRING(arg) (v8::Local<v8::String>::Cast(arg))
#define TO_INTEGER_VALUE(arg, v8_ctx) (arg->IntegerValue(v8_ctx).FromJust())

using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::NewStringType;
using v8::Object;
using v8::String;
using v8::Value;

using iast::utils::GetLocalStringPointer;
using iast::utils::getRangesInSlice;

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
    int start = TO_INTEGER_VALUE(args[3], context);
    int end = subjectLen;
    if (argc > 4) {
        end = TO_INTEGER_VALUE(args[4], context);
    }

    if (resultLen == 0) {
        args.GetReturnValue().Set(result);
        return;
    }
    auto transaction = GetTransaction(GetLocalStringPointer(args[0]));
    if (transaction == nullptr) {
        args.GetReturnValue().Set(result);
        return;
    }

    if (subjectLen <= 1) {
        args.GetReturnValue().Set(result);
        return;
    }

    auto taintedObj = transaction->FindTaintedObject(GetLocalStringPointer(subject));
    if (!taintedObj) {
        args.GetReturnValue().Set(result);
        return;
    }

    try {
        start = MAX(start, 0);
        auto newRanges = getRangesInSlice(transaction, taintedObj, MIN(start, end), MAX(start, end));
        if (newRanges && newRanges->Size() > 0) {
            if (resultLen == 1) {
                result = tainted::NewExternalString(isolate, args[1]);
            }
            transaction->AddTainted(GetLocalStringPointer(result), newRanges, result);
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
    int start = TO_INTEGER_VALUE(args[3], context);
    int end = subjectLen;
    if (argc > 4) {
        end = TO_INTEGER_VALUE(args[4], context);
    }

    auto transaction = GetTransaction(GetLocalStringPointer(args[0]));
    if (transaction == nullptr) {
        args.GetReturnValue().Set(result);
        return;
    }

    if (subjectLen <= 1) {
        args.GetReturnValue().Set(result);
        return;
    }

    auto taintedObj = transaction->FindTaintedObject(GetLocalStringPointer(subject));
    if (!taintedObj) {
        args.GetReturnValue().Set(result);
        return;
    }

    try {
        start = (start >= 0) ? start : subjectLen + start;
        end = ((start + end) >= subjectLen) ? subjectLen : start + end;

        auto newRanges = getRangesInSlice(transaction, taintedObj, start, end);
        if (newRanges && newRanges->Size() > 0) {
            if (resultLen == 1) {
                result = tainted::NewExternalString(isolate, args[1]);
            }
            transaction->AddTainted(GetLocalStringPointer(result), newRanges, result);
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
