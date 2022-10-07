/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
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
    try {
        if (args[1]->IsString()) {
            uintptr_t transactionId = utils::GetLocalStringPointer(args[0]);
            auto transaction = GetTransaction(transactionId);
            if (transaction != nullptr) {
                auto ptr1 = utils::GetLocalStringPointer(args[2]);
                auto argsSize = args.Length();

                auto ranges = transaction->GetRanges(ptr1);
                bool usingFirstParamRanges = ranges != nullptr;

                if (ranges == nullptr || ranges->size() < Limits::MAX_RANGES) {
                    int offset = utils::GetCoercedLength(isolate, args[2]);
                    for (int i = 3; i < argsSize; i++) {
                        auto argPointer = utils::GetLocalStringPointer(args[i]);
                        auto argRanges = transaction->GetRanges(argPointer);
                        if (argRanges != nullptr) {
                            if (ranges == nullptr) {
                                ranges = transaction->GetAvailableSharedVector();
                            } else if (usingFirstParamRanges) {
                                usingFirstParamRanges = false;
                                auto tmpRanges = ranges;
                                ranges = transaction->GetAvailableSharedVector();
                                ranges->add(tmpRanges);
                            }
                            auto end = argRanges->end();
                            if (offset != 0) {
                                for (auto it = argRanges->begin(); it != end; it++) {
                                    auto argRange = *it;
                                    auto newRange = transaction->GetAvailableTaintedRange(offset + argRange->start
                                            , offset + argRange->end,
                                            argRange->inputInfo);
                                    if (newRange) {
                                        ranges->push_back(newRange);
                                    } else {
                                        break;
                                    }
                                }
                            } else {
                                for (auto it = argRanges->begin(); it != end; it++) {
                                    auto argRange = *it;
                                    ranges->push_back(argRange);
                                }
                            }
                        }
                        offset += utils::GetCoercedLength(isolate, args[i]);
                    }
                }

                if (ranges != nullptr) {
                    auto key = utils::GetLocalStringPointer(args[1]);
                    transaction->AddTainted(key, ranges, args[1]);
                    args.GetReturnValue().Set(args[1]);
                    return;
                }
            }
        }
    } catch (const tainted::NotAvailableRangeVectorsException& err) {
        // do nothing, just continue without taint
    }
    args.GetReturnValue().Set(args[1]);
}

void ConcatOperations::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "concat", TaintConcatOperator);
}
}   // namespace api
}   // namespace iast

