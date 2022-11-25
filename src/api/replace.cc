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
struct JsReplacementInfo {
    int64_t index;
    int64_t matchLength;
    int64_t offset;
};
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

    try {
        auto transaction = GetTransaction(utils::GetLocalStringPointer(transactionId));
        if (transaction == nullptr) {
            args.GetReturnValue().Set(args[1]);
            return;
        }
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

void TaintReplaceStringByStringUsingRegexMethod(const FunctionCallbackInfo<Value>& args) {
    // transactionId, result, subject, matcher, replacer, replacements
    if (!utils::ValidateMethodArguments(args, 6, "Wrong number of arguments")) {
        return;
    }
    auto replaceResult = args[1];
    if (!replaceResult->IsString()) {
        args.GetReturnValue().Set(replaceResult);
        return;
    }
    auto transactionId = args[0];
    auto self = args[2];
    auto matcher = args[3];
    auto replacer = args[4];
    auto replacements = args[5];

    try {
        auto transaction = GetTransaction(utils::GetLocalStringPointer(transactionId));
        if (transaction == nullptr) {
            args.GetReturnValue().Set(replaceResult);
            return;
        }
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
        auto isolate = args.GetIsolate();
        auto context = isolate->GetCurrentContext();
        auto jsReplacements = v8::Array::Cast(*replacements);
        auto jsReplacementsLength = jsReplacements->Length();
        auto replacerLength = v8::String::Cast(*replacer)->Length();
        std::vector<Range*>::iterator subjectIt;
        std::vector<Range*>::iterator subjectItEnd;

        if (subjectRanges != nullptr) {
            subjectIt = subjectRanges->begin();
            subjectItEnd = subjectRanges->end();
        }

        auto newRanges = transaction->GetSharedVectorRange();
        int offset = 0;
        int lastEnd = 0;
        for (int i = 0; i < jsReplacementsLength; i++) {
            auto jsReplacement = v8::Object::Cast(*(jsReplacements->Get(context, i).ToLocalChecked()));
            auto index = v8::Int32::Cast(*(jsReplacement->Get(context, 0).ToLocalChecked()))->Value();
            auto matcherLength = v8::String::Cast(*(jsReplacement->Get(context, 1).ToLocalChecked()))->Length();
            struct JsReplacementInfo currentReplacement = {
                    index, matcherLength, replacerLength - matcherLength
            };
            if (subjectRanges != nullptr) {
                while (subjectIt != subjectItEnd && (*subjectIt)->start < index) {
                    auto breakLoop = false;
                    auto range = *subjectIt;
                    if (lastEnd < range->end) {
                        auto start = range->start;
                        auto end = range->end;
                        if (lastEnd > range->start) {
                            start = lastEnd;
                        }
                        start += offset;
                        if (range->end > index) {
                            breakLoop = true;
                            end = index;
                        }
                        end += offset;
                        if (start == range->start && end == range->end) {
                            newRanges->PushBack(range);
                        } else {
                            newRanges->PushBack(transaction->GetRange(start, end, range->inputInfo));
                        }
                    }
                    if (breakLoop) {
                        break;
                    }
                    ++subjectIt;
                }
            }
            if (replacerRanges != nullptr) {
                auto toReplaceStart =  index + offset;
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
            lastEnd = currentReplacement.index + currentReplacement.matchLength;
            offset = offset + currentReplacement.offset;
        }
        while (subjectIt != subjectItEnd) {
            auto range = *subjectIt;
            if (lastEnd < range->end) {
                if (lastEnd > range->start) {
                    newRanges->PushBack(transaction->GetRange(lastEnd + offset, range->end + offset,
                                                              range->inputInfo));
                } else if (offset == 0) {
                    newRanges->PushBack(range);
                } else {
                    newRanges->PushBack(transaction->GetRange(range->start + offset, range->end + offset,
                                                              range->inputInfo));
                }
            }
            ++subjectIt;
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
    NODE_SET_METHOD(exports, "replaceStringByStringUsingRegex", TaintReplaceStringByStringUsingRegexMethod);
}
}   // namespace api
}   // namespace iast

