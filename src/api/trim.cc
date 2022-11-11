/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <new>
#include <vector>
#include <memory>

#include "trim.h"
#include "../tainted/range.h"
#include "../tainted/transaction.h"
#include "../iast.h"

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
        if (taintedObj == nullptr) {
            args.GetReturnValue().Set(args[1]);
            return;
        }

        auto ranges = taintedObj ? taintedObj->getRanges() : nullptr;
        if (ranges == nullptr) {
            args.GetReturnValue().Set(args[1]);
            return;
        }

        v8::String::Utf8Value selfStr(isolate, args[2]);
        std::string cSelf(*selfStr);
        int selfLength = cSelf.length();

        int left = 0;
        while (left < selfLength) {
            auto c = cSelf.at(left);
            if (isspace(c)) {
                left++;
            } else {
                break;
            }
        }

        v8::String::Utf8Value resultStr(isolate, args[1]);
        std::string cResultStr(*resultStr);
        int resultLength = cResultStr.length();

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
                auto newRange = transaction->GetRange(newRangeStart, newRangeEnd, range->inputInfo);
                resultRanges->PushBack(newRange);
            }
        }

        if (resultRanges != nullptr) {
            auto key = utils::GetLocalStringPointer(args[1]);
            transaction->AddTainted(key, resultRanges, args[1]);
            args.GetReturnValue().Set(args[1]);
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
        if (taintedObj == nullptr) {
            args.GetReturnValue().Set(args[1]);
            return;
        }

        auto ranges = taintedObj ? taintedObj->getRanges() : nullptr;
        if (ranges == nullptr) {
            args.GetReturnValue().Set(args[1]);
            return;
        }

        v8::String::Utf8Value resultStr(isolate, args[1]);
        std::string cResultStr(*resultStr);
        int resultLength = cResultStr.length();

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
                auto newRange = transaction->GetRange(newRangeStart, newRangeEnd, range->inputInfo);
                resultRanges->PushBack(newRange);
            }
        }

        if (resultRanges != nullptr) {
            auto key = utils::GetLocalStringPointer(args[1]);
            transaction->AddTainted(key, resultRanges, args[1]);
            args.GetReturnValue().Set(args[1]);
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
