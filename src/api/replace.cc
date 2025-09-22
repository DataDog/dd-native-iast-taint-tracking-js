/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <node.h>
#include <new>
#include <vector>
#include <memory>

#include "replace.h"
#include "../tainted/string_resource.h"
#include "../tainted/range.h"
#include "../tainted/transaction.h"
#include "../iast.h"
#include "../utils/validation_utils.h"
#include "../utils/string_utils.h"
#include "v8.h"

using v8::FunctionCallbackInfo;
using v8::Value;
using v8::Local;
using v8::Context;
using v8::Object;
using v8::Array;
using v8::String;
using v8::Local;
using v8::Integer;
using v8::Int32;

using iast::tainted::Range;
using iast::utils::GetLocalPointer;

namespace iast {
namespace api {

struct JsReplacementInfo {
    int64_t index;
    int64_t matchLength;
    int64_t offset;
};

struct MatcherArguments {
    Local<Value> result;
    Local<Value> self;
    Local<Value> matcher;
    Local<Value> replacer;
    Local<Value> replacements;
};

inline void addReplacerRanges(Transaction* transaction,
        SharedRanges* replacerRanges,
        int toReplaceStart,
        SharedRanges* newRanges) {
    if (replacerRanges) {
        if (toReplaceStart == 0) {
            newRanges->Add(replacerRanges);
        } else {
            auto replacerItEnd = replacerRanges->end();
            for (auto replacerIt = replacerRanges->begin(); replacerItEnd != replacerIt; replacerIt++) {
                auto range = (*replacerIt);
                newRanges->PushBack(transaction->GetRange(range->start + toReplaceStart,
                            range->end + toReplaceStart, range->inputInfo, range->secureMarks));
            }
        }
    }
}

inline SharedRanges* adjustReplacementRanges(Transaction* transaction,
        SharedRanges* subjectRanges,
        SharedRanges* replacerRanges,
        const MatcherArguments& args) {
    auto matcherLength = String::Cast(*(args.matcher))->Length();
    auto replacementLength = String::Cast(*(args.replacer))->Length();
    auto toReplaceStart = Integer::Cast(*(args.replacements))->Value();
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
                newRanges->PushBack(transaction->GetRange(range->start, toReplaceStart,
                                                          range->inputInfo, range->secureMarks));
                break;
            }
            ++subjectIt;
        }
    }

    addReplacerRanges(transaction, replacerRanges, toReplaceStart, newRanges);

    if (subjectRanges) {
        while (subjectIt != subjectItEnd) {
            auto range = (*subjectIt);
            if (range->end > toReplaceEnd) {
                if (range->start <= toReplaceEnd) {
                    newRanges->PushBack(transaction->GetRange(toReplaceEnd + offset,
                                range->end + offset,
                                range->inputInfo, range->secureMarks));
                } else if (offset == 0) {
                    newRanges->PushBack(range);
                } else {
                    newRanges->PushBack(transaction->GetRange(range->start + offset,
                                range->end + offset,
                                range->inputInfo, range->secureMarks));
                }
            }
            subjectIt++;
        }
    }

    return newRanges;
}

inline SharedRanges* adjustRegexReplacementRanges(Transaction* transaction,
        SharedRanges* subjectRanges,
        SharedRanges* replacerRanges,
        const MatcherArguments& args,
        Local<Context> context) {
    std::vector<Range*>::iterator subjectIt;
    std::vector<Range*>::iterator subjectItEnd;
    auto replacerLen = String::Cast(*(args.replacer))->Length();
    auto jsReplacements = Array::Cast(*args.replacements);
    auto newRanges = transaction->GetSharedVectorRange();

    if (subjectRanges != nullptr) {
        subjectIt = subjectRanges->begin();
        subjectItEnd = subjectRanges->end();
    }

    int offset = 0;
    int lastEnd = 0;
    for (unsigned int i = 0; i < jsReplacements->Length(); i++) {
        auto jsReplacement = Object::Cast(*(jsReplacements->Get(context, i).ToLocalChecked()));
        auto index = Int32::Cast(*(jsReplacement->Get(context, 0).ToLocalChecked()))->Value();
        auto matcherLength = String::Cast(*(jsReplacement->Get(context, 1).ToLocalChecked()))->Length();
        struct JsReplacementInfo currentReplacement = {index, matcherLength, replacerLen - matcherLength};

        if (subjectRanges) {
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
                        newRanges->PushBack(transaction->GetRange(start, end, range->inputInfo, range->secureMarks));
                    }
                }
                if (breakLoop) {
                    break;
                }
                ++subjectIt;
            }
        }

        addReplacerRanges(transaction, replacerRanges, index + offset, newRanges);

        lastEnd = currentReplacement.index + currentReplacement.matchLength;
        offset = offset + currentReplacement.offset;
    }

    if (subjectRanges) {
        while (subjectIt != subjectItEnd) {
            auto range = *subjectIt;
            if (lastEnd < range->end) {
                if (lastEnd > range->start) {
                    newRanges->PushBack(transaction->GetRange(lastEnd + offset, range->end + offset,
                                range->inputInfo, range->secureMarks));
                } else if (offset == 0) {
                    newRanges->PushBack(range);
                } else {
                    newRanges->PushBack(
                            transaction->GetRange(range->start + offset, range->end + offset,
                                range->inputInfo, range->secureMarks));
                }
            }
            ++subjectIt;
        }
    }
    return newRanges;
}

void TaintReplaceStringByStringMethod(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    if (!utils::ValidateMethodArguments(args, 6, "Wrong number of arguments")) {
        return;
    }
    auto replaceResult = args[1];
    if (!replaceResult->IsString()) {
        args.GetReturnValue().Set(replaceResult);
        return;
    }

    auto transaction = GetTransaction(utils::GetStringValue(isolate, args[0]));
    if (!transaction) {
        args.GetReturnValue().Set(replaceResult);
        return;
    }

    try {
        MatcherArguments methodArguments = {args[1], args[2], args[3], args[4], args[5]};

        auto taintedSubject = transaction->FindTaintedObject(GetLocalPointer(methodArguments.self));
        auto taintedReplacer = transaction->FindTaintedObject(GetLocalPointer(methodArguments.replacer));
        auto subjectRanges = (taintedSubject) ? taintedSubject->getRanges() : nullptr;
        auto replacerRanges = (taintedReplacer) ? taintedReplacer->getRanges() : nullptr;

        if (subjectRanges == nullptr && replacerRanges == nullptr) {
            args.GetReturnValue().Set(replaceResult);
            return;
        }

        auto newRanges = adjustReplacementRanges(transaction, subjectRanges, replacerRanges, methodArguments);
        if (newRanges->Size() > 0) {
            auto isolate = args.GetIsolate();
            auto resultString = replaceResult->ToString(isolate->GetCurrentContext()).ToLocalChecked();
            auto resultLength = resultString->Length();
            if (resultLength == 1) {
                replaceResult = tainted::NewExternalString(isolate, replaceResult);
            }
            auto key = GetLocalPointer(replaceResult);
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
    auto isolate = args.GetIsolate();
    if (!utils::ValidateMethodArguments(args, 6, "Wrong number of arguments")) {
        return;
    }
    auto replaceResult = args[1];
    if (!replaceResult->IsString()) {
        args.GetReturnValue().Set(replaceResult);
        return;
    }

    auto transaction = GetTransaction(utils::GetStringValue(isolate, args[0]));
    if (!transaction) {
        args.GetReturnValue().Set(replaceResult);
        return;
    }

    try {
        MatcherArguments methodArguments = {args[1], args[2], args[3], args[4], args[5]};

        auto taintedSubject = transaction->FindTaintedObject(GetLocalPointer(methodArguments.self));
        auto taintedReplacer = transaction->FindTaintedObject(GetLocalPointer(methodArguments.replacer));
        auto subjectRanges = (taintedSubject) ? taintedSubject->getRanges() : nullptr;
        auto replacerRanges = (taintedReplacer) ? taintedReplacer->getRanges() : nullptr;

        if (subjectRanges == nullptr && replacerRanges == nullptr) {
            args.GetReturnValue().Set(replaceResult);
            return;
        }

        auto newRanges = adjustRegexReplacementRanges(transaction,
                subjectRanges,
                replacerRanges,
                methodArguments,
                args.GetIsolate()->GetCurrentContext());

        if (newRanges->Size() > 0) {
            auto resultString = replaceResult->ToString(args.GetIsolate()->GetCurrentContext()).ToLocalChecked();
            auto resultLength = resultString->Length();
            if (resultLength == 1) {
                replaceResult = tainted::NewExternalString(args.GetIsolate(), replaceResult);
            }
            auto key = utils::GetLocalPointer(replaceResult);
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

