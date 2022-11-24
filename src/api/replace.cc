/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <new>
#include <vector>
#include <memory>

#include "replace.h"
#include "../tainted/string_resource.h"
#include "../tainted/range.h"
#include "../tainted/transaction.h"
#include "../iast.h"
#include "../utils/validation_utils.h"

using v8::FunctionCallbackInfo;
using v8::Value;
using v8::Local;
using v8::Isolate;
using v8::Object;

using iast::tainted::Range;

namespace iast {
namespace api {
void TaintReplaceStringByStringMethod(const FunctionCallbackInfo<Value>& args) {
    // transactionId, result, subject, matcher, replacer, index
    if (!utils::ValidateMethodArguments(args, 6, "Wrong number of arguments")) {
        return;
    }
    auto isolate = args.GetIsolate();
    auto replaceResult = args[1];
    if (!replaceResult->IsString()) {
        args.GetReturnValue().Set(replaceResult);
        return;
    }

    auto transactionId = args[0];
    auto self = args[2];
    auto matcher = args[3];
    auto replacer = args[4];
    auto replacement = args[5];

    auto transaction = GetTransaction(utils::GetLocalStringPointer(transactionId));
    if (transaction == nullptr) {
        args.GetReturnValue().Set(args[1]);
        return;
    }

    try {
        auto subjectPointer = utils::GetLocalStringPointer(self);
        auto replacerPointer = utils::GetLocalStringPointer(replacer);

        auto subjectTaintedObject = transaction->FindTaintedObject(subjectPointer);
        auto replacerTaintedObject = transaction->FindTaintedObject(replacerPointer);
        auto subjectRanges = subjectTaintedObject ? subjectTaintedObject->getRanges() : nullptr;
        auto replacerRanges = replacerTaintedObject ? replacerTaintedObject->getRanges() : nullptr;

        if (subjectRanges == nullptr && replacerRanges == nullptr) {
            args.GetReturnValue().Set(replaceResult);
            return;
        }

        auto matcherLength = v8::String::Cast(*matcher)->Length();
        auto replacementLength = v8::String::Cast(*replacer)->Length();
        auto toReplaceStart = v8::Integer::Cast(*replacement)->Value();
        int toReplaceEnd = toReplaceStart + matcherLength;
        int offset = replacementLength - matcherLength;
        auto newRanges = transaction->GetSharedVectorRange();
        std::vector<Range*>::iterator subjectIt;
        std::vector<Range*>::iterator subjectItEnd;
        if (subjectRanges) {
            subjectIt = subjectRanges->begin();
            subjectItEnd = subjectRanges->end();
            while (subjectIt != subjectItEnd && (*subjectIt)->start < toReplaceStart) {
                auto range = (*subjectIt);
                if (range->end <= toReplaceStart) {
                    newRanges->PushBack(range);
                } else if (range->end > toReplaceStart) {
                    newRanges->PushBack(transaction->GetRange(range->start, toReplaceStart, range->inputInfo));
                    break;
                }
                ++subjectIt;
            }
        }

        if (replacerRanges) {
            if (toReplaceStart == 0) {
                newRanges->Add(replacerRanges);
            } else {
                auto replacerItEnd = replacerRanges->end();
                for (auto replacerIt = replacerRanges->begin(); replacerItEnd != replacerIt; replacerIt++) {
                    auto range = (*replacerIt);
                    newRanges->PushBack(transaction->GetRange(range->start + toReplaceStart,
                                                              range->end + toReplaceStart, range->inputInfo));
                }
            }
        }

        if (subjectRanges) {
            while (subjectIt != subjectItEnd) {
                auto range = (*subjectIt);
                if (range->end > toReplaceEnd) {
                    if (range->start <= toReplaceEnd) {
                        newRanges->PushBack(transaction->GetRange(toReplaceEnd + offset,
                                                                  range->end + offset,
                                                                  range->inputInfo));
                    } else if (offset == 0) {
                        newRanges->PushBack(range);
                    } else {
                        newRanges->PushBack(transaction->GetRange(range->start + offset,
                                                                  range->end + offset,
                                                                  range->inputInfo));
                    }
                }
                subjectIt++;
            }
        }

        if (newRanges->Size() > 0) {
            auto resultString = replaceResult->ToString(isolate->GetCurrentContext()).ToLocalChecked();
            auto resultLength = resultString->Length();
            if (resultLength == 1) {
                replaceResult = tainted::NewExternalString(isolate, replaceResult);
            }
            auto key = utils::GetLocalStringPointer(replaceResult);
            transaction->AddTainted(key, newRanges, replaceResult);
        }
    } catch (const std::bad_alloc& err) {
    } catch (const container::QueuedPoolBadAlloc& err) {
    } catch (const container::PoolBadAlloc& err) {
    }
    args.GetReturnValue().Set(replaceResult);
}

void ReplaceOperations::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "replaceStringByString", TaintReplaceStringByStringMethod);
}
}   // namespace api
}   // namespace iast

