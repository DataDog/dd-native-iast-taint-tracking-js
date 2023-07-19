/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <new>
#include <vector>
#include <memory>
#include <string>

#include "trim.h"
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

void TaintTrimOperator(const FunctionCallbackInfo<Value>& args) {
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

    auto transaction = GetTransaction(utils::GetLocalStringPointer(args[0]));
    if (transaction == nullptr) {
        args.GetReturnValue().Set(args[1]);
        return;
    }

    try {
        auto taintedObj = transaction->FindTaintedObject(utils::GetLocalStringPointer(args[2]));
        auto ranges = taintedObj ? taintedObj->getRanges() : nullptr;
        if (ranges == nullptr) {
            args.GetReturnValue().Set(args[1]);
            return;
        }

        int left = 0;
        v8::String::Utf8Value selfStr(isolate, args[2]);

        char * selfCh = *selfStr;
        do {
            if (!isspace(*selfCh)) {
                break;
            }
            left++;
        } while (*selfCh++);

        int resultLength = TO_V8STRING(args[1])->Length();

        auto resultRanges = transaction->GetSharedVectorRange();
        auto end = ranges->end();
        for (auto it = ranges->begin(); it != end; it++) {
            auto range = *it;
            int newRangeEnd = range->end - left;
            int newRangeStart = range->start - left;

            if (newRangeStart < 0) {
                newRangeStart = 0;
            }

            if (newRangeEnd >= 0 && newRangeEnd > resultLength) {
                newRangeEnd = resultLength;
            }

            if (newRangeEnd > newRangeStart) {
                auto newRange = transaction->GetRange(newRangeStart, newRangeEnd, range->inputInfo, range->secureMarks);
                resultRanges->PushBack(newRange);
            }
        }

        if (resultRanges->Size() > 0) {
            auto res = args[1];
            if (resultLength == 1) {
                res = tainted::NewExternalString(isolate, res);
            }
            auto key = utils::GetLocalStringPointer(res);
            transaction->AddTainted(key, resultRanges, res);
            args.GetReturnValue().Set(res);
            return;
        }
    } catch (const std::bad_alloc& err) {
    } catch (const container::QueuedPoolBadAlloc& err) {
    } catch (const container::PoolBadAlloc& err) {
    }
    args.GetReturnValue().Set(args[1]);
}

void TaintTrimEndOperator(const FunctionCallbackInfo<Value>& args) {
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

    auto transaction = GetTransaction(utils::GetLocalStringPointer(args[0]));
    if (transaction == nullptr) {
        args.GetReturnValue().Set(args[1]);
        return;
    }

    try {
        auto taintedObj = transaction->FindTaintedObject(utils::GetLocalStringPointer(args[2]));
        auto ranges = taintedObj ? taintedObj->getRanges() : nullptr;
        if (ranges == nullptr) {
            args.GetReturnValue().Set(args[1]);
            return;
        }

        int resultLength = TO_V8STRING(args[1])->Length();

        auto resultRanges = transaction->GetSharedVectorRange();
        auto end = ranges->end();
        for (auto it = ranges->begin(); it != end; it++) {
            auto range = *it;
            int newRangeEnd = range->end;
            int newRangeStart = range->start;

            if (newRangeEnd > resultLength) {
                newRangeEnd = resultLength;
            }

            if (newRangeEnd > newRangeStart) {
                auto newRange = transaction->GetRange(newRangeStart, newRangeEnd, range->inputInfo, range->secureMarks);
                resultRanges->PushBack(newRange);
            }
        }

        if (resultRanges->Size() > 0) {
            auto res = args[1];
            if (resultLength == 1) {
                res = tainted::NewExternalString(isolate, res);
            }
            auto key = utils::GetLocalStringPointer(res);
            transaction->AddTainted(key, resultRanges, res);
            args.GetReturnValue().Set(res);
            return;
        }
    } catch (const std::bad_alloc& err) {
    } catch (const container::QueuedPoolBadAlloc& err) {
    } catch (const container::PoolBadAlloc& err) {
    }
    args.GetReturnValue().Set(args[1]);
}

void TrimOperations::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "trim", TaintTrimOperator);
    NODE_SET_METHOD(exports, "trimEnd", TaintTrimEndOperator);
}
}   // namespace api
}   // namespace iast

