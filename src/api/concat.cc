/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <new>
#include <vector>
#include <memory>

#include "concat.h"
#include "../iastlimits.h"
#include "../tainted/range.h"
#include "../tainted/transaction.h"
#include "../iast.h"

using v8::FunctionCallbackInfo;
using v8::Value;
using v8::Local;
using v8::Isolate;
using v8::Object;

using iast::tainted::Range;

namespace iast {
namespace api {
void TaintConcatOperator(const FunctionCallbackInfo<Value>& args) {
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

    try {
        auto argsSize = args.Length();
        auto taintedObj = transaction->FindTaintedObject(utils::GetLocalPointer(args[2]));
        auto ranges = taintedObj ? taintedObj->getRanges() : nullptr;
        bool usingFirstParamRanges = ranges != nullptr;

        if (ranges == nullptr || ranges->Size() < Limits::MAX_RANGES) {
            int offset = utils::GetCoercedLength(isolate, args[2]);
            for (int i = 3; i < argsSize; i++) {
                auto taintedObj = transaction->FindTaintedObject(
                        utils::GetLocalPointer(args[i]));
                auto argRanges = taintedObj ? taintedObj->getRanges() : nullptr;
                if (argRanges != nullptr) {
                    if (ranges == nullptr) {
                        ranges = transaction->GetSharedVectorRange();
                    } else if (usingFirstParamRanges) {
                        usingFirstParamRanges = false;
                        auto tmpRanges = ranges;
                        ranges = transaction->GetSharedVectorRange();
                        ranges->Add(tmpRanges);
                    }
                    auto end = argRanges->end();
                    if (offset != 0) {
                        for (auto it = argRanges->begin(); it != end; it++) {
                            auto argRange = *it;
                            auto newRange = transaction->GetRange(offset + argRange->start
                                    , offset + argRange->end,
                                    argRange->inputInfo, argRange->secureMarks);
                            ranges->PushBack(newRange);
                        }
                    } else {
                        for (auto it = argRanges->begin(); it != end; it++) {
                            auto argRange = *it;
                            ranges->PushBack(argRange);
                        }
                    }
                }
                offset += utils::GetCoercedLength(isolate, args[i]);
            }
        }

        if (ranges != nullptr) {
            auto key = utils::GetLocalPointer(args[1]);
            transaction->AddTainted(key, ranges, args[1]);
            args.GetReturnValue().Set(args[1]);
            return;
        }
    } catch (const std::bad_alloc& err) {
    } catch (const container::QueuedPoolBadAlloc& err) {
    } catch (const container::PoolBadAlloc& err) {
    }
    args.GetReturnValue().Set(args[1]);
}

void ConcatOperations::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "concat", TaintConcatOperator);
}
}   // namespace api
}   // namespace iast

