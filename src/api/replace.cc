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

struct StringMatcherArguments {
    Transaction* transaction;
    v8::Local<Value> result;
    v8::Local<Value> self;
    v8::Local<Value> matcher;
    v8::Local<Value> replacer;
    v8::Local<Value> replacement;
};

struct RegexMatcherArguments {
    Transaction* transaction;
    v8::Local<Value> result;
    v8::Local<Value> self;
    v8::Local<Value> matcher;
    v8::Local<Value> replacer;
    v8::Local<Value> replacements;
};

inline StringMatcherArguments ReadStringMatcherMethodArguments(const FunctionCallbackInfo<Value>& args) {
    auto replaceResult = args[1];
    if (!replaceResult->IsString()) {
        return StringMatcherArguments {
            nullptr, replaceResult, args[2], args[3], args[4], args[5]
        };
    } else {
        auto transaction = GetTransaction(utils::GetLocalStringPointer(args[0]));
        return StringMatcherArguments {
                transaction, replaceResult, args[2], args[3], args[4], args[5]
        };
    }
}

inline RegexMatcherArguments ReadRegexMatcherMethodArguments(const FunctionCallbackInfo<Value>& args) {
    auto replaceResult = args[1];
    if (!replaceResult->IsString()) {
        return RegexMatcherArguments {
            nullptr, replaceResult, args[2], args[3], args[4], args[5]
        };
    } else {
        auto transaction = GetTransaction(utils::GetLocalStringPointer(args[0]));
        return RegexMatcherArguments {
                transaction, replaceResult, args[2], args[3], args[4], args[5]
        };
    }
}

inline SharedRanges* GetTaintedRangesFromLocal(Transaction* transaction, v8::Local<Value> value) {
    auto pointer = utils::GetLocalStringPointer(value);
    auto taintedObject = transaction->FindTaintedObject(pointer);
    if (taintedObject) {
        return taintedObject->getRanges();
    }
    return nullptr;
}

#define INIT_STRING_MATCHER_PROPERTIES \
auto matcherLength = v8::String::Cast(*(methodArguments.matcher))->Length(); \
auto replacementLength = v8::String::Cast(*(methodArguments.replacer))->Length(); \
auto toReplaceStart = v8::Integer::Cast(*(methodArguments.replacement))->Value(); \
int toReplaceEnd = toReplaceStart + matcherLength; \
int offset = replacementLength - matcherLength; \
auto newRanges = methodArguments.transaction->GetSharedVectorRange();

#define ADD_BEFORE_REPLACEMENT_RANGES \
while (subjectIt != subjectItEnd && (*subjectIt)->start < toReplaceStart) { \
    auto range = (*subjectIt); \
    if (range->end <= toReplaceStart) { \
        newRanges->PushBack(range); \
    } else if (range->end > toReplaceStart) { \
        newRanges->PushBack(methodArguments.transaction->GetRange(range->start, toReplaceStart, range->inputInfo)); \
        break; \
    } \
    ++subjectIt; \
}

#define ADD_REPLACEMENT_RANGES \
if (replacerRanges) { \
    if (toReplaceStart == 0) { \
        newRanges->Add(replacerRanges); \
    } else { \
        auto replacerItEnd = replacerRanges->end(); \
        for (auto replacerIt = replacerRanges->begin(); replacerItEnd != replacerIt; replacerIt++) { \
            auto range = (*replacerIt); \
            newRanges->PushBack(methodArguments.transaction->GetRange(range->start + toReplaceStart, \
            range->end + toReplaceStart, range->inputInfo)); \
        } \
    } \
}

#define ADD_AFTER_REPLACEMENT_RANGES \
while (subjectIt != subjectItEnd) { \
    auto range = (*subjectIt); \
    if (range->end > toReplaceEnd) { \
        if (range->start <= toReplaceEnd) { \
            newRanges->PushBack(methodArguments.transaction->GetRange(toReplaceEnd + offset, \
                range->end + offset, \
                range->inputInfo)); \
        } else if (offset == 0) { \
            newRanges->PushBack(range); \
        } else { \
            newRanges->PushBack(methodArguments.transaction->GetRange(range->start + offset, \
                range->end + offset, \
                range->inputInfo)); \
        } \
    } \
    subjectIt++; \
}

void TaintReplaceStringByStringMethod(const FunctionCallbackInfo<Value>& args) {
    if (!utils::ValidateMethodArguments(args, 6, "Wrong number of arguments")) {
        return;
    }
    auto replaceResult = args[1];
    if (!replaceResult->IsString()) {
        args.GetReturnValue().Set(replaceResult);
        return;
    }
    try {
        auto methodArguments = ReadStringMatcherMethodArguments(args);
        if (methodArguments.transaction == nullptr) {
            args.GetReturnValue().Set(methodArguments.result);
            return;
        }

        auto subjectRanges = GetTaintedRangesFromLocal(methodArguments.transaction, methodArguments.self);
        auto replacerRanges = GetTaintedRangesFromLocal(methodArguments.transaction, methodArguments.replacer);
        if (subjectRanges == nullptr && replacerRanges == nullptr) {
            args.GetReturnValue().Set(replaceResult);
            return;
        }

        INIT_STRING_MATCHER_PROPERTIES

        std::vector<Range*>::iterator subjectIt;
        std::vector<Range*>::iterator subjectItEnd;
        if (subjectRanges) {
            subjectIt = subjectRanges->begin();
            subjectItEnd = subjectRanges->end();
            ADD_BEFORE_REPLACEMENT_RANGES
        }

        ADD_REPLACEMENT_RANGES

        if (subjectRanges) {
            ADD_AFTER_REPLACEMENT_RANGES
        }

        if (newRanges->Size() > 0) {
            auto isolate = args.GetIsolate();
            auto resultString = replaceResult->ToString(isolate->GetCurrentContext()).ToLocalChecked();
            auto resultLength = resultString->Length();
            if (resultLength == 1) {
                replaceResult = tainted::NewExternalString(isolate, replaceResult);
            }
            auto key = utils::GetLocalStringPointer(replaceResult);
            methodArguments.transaction->AddTainted(key, newRanges, replaceResult);
        }
    } catch (const std::bad_alloc& err) {
    } catch (const container::QueuedPoolBadAlloc& err) {
    } catch (const container::PoolBadAlloc& err) {
    }
    args.GetReturnValue().Set(replaceResult);
}
#undef INIT_STRING_MATCHER_PROPERTIES
#undef ADD_BEFORE_REPLACEMENT_RANGES
#undef ADD_REPLACEMENT_RANGES
#undef ADD_AFTER_REPLACEMENT_RANGES

#define INIT_REGEX_REPLACE_PROPERTIES \
auto isolate = args.GetIsolate(); \
auto context = isolate->GetCurrentContext(); \
auto jsReplacements = v8::Array::Cast(*replacements); \
auto jsReplacementsLength = jsReplacements->Length(); \
auto replacerLength = v8::String::Cast(*(methodArguments.replacer))->Length(); \
std::vector<Range*>::iterator subjectIt; \
std::vector<Range*>::iterator subjectItEnd; \

#define ADD_RANGES_BEFORE_REPLACEMENT \
while (subjectIt != subjectItEnd && (*subjectIt)->start < index) { \
    auto breakLoop = false; \
    auto range = *subjectIt; \
    if (lastEnd < range->end) { \
        auto start = range->start; \
        auto end = range->end; \
        if (lastEnd > range->start) { \
            start = lastEnd; \
        } \
        start += offset; \
        if (range->end > index) { \
            breakLoop = true; \
            end = index; \
        } \
        end += offset; \
        if (start == range->start && end == range->end) { \
            newRanges->PushBack(range); \
        } else { \
            newRanges->PushBack(methodArguments.transaction->GetRange(start, end, range->inputInfo)); \
        } \
    } \
    if (breakLoop) { \
        break; \
    } \
    ++subjectIt; \
}
#define ADD_REPLACEMENT_RANGES \
if (replacerRanges != nullptr) { \
    auto toReplaceStart =  index + offset; \
    if (toReplaceStart == 0) { \
        newRanges->Add(replacerRanges); \
    } else { \
        auto replacerItEnd = replacerRanges->end(); \
        for (auto replacerIt = replacerRanges->begin(); replacerItEnd != replacerIt; replacerIt++) { \
            auto range = (*replacerIt); \
            newRanges->PushBack(methodArguments.transaction->GetRange(range->start + toReplaceStart, \
                range->end + toReplaceStart, range->inputInfo)); \
        } \
    } \
}

#define ADD_RANGES_AFTER_ALL_REPLACEMENTS \
while (subjectIt != subjectItEnd) { \
    auto range = *subjectIt; \
    if (lastEnd < range->end) { \
        if (lastEnd > range->start) { \
            newRanges->PushBack(methodArguments.transaction->GetRange(lastEnd + offset, range->end + offset, \
                range->inputInfo)); \
        } else if (offset == 0) { \
            newRanges->PushBack(range); \
        } else { \
            newRanges->PushBack( \
                methodArguments.transaction->GetRange(range->start + offset, range->end + offset, \
                range->inputInfo)); \
        } \
    } \
    ++subjectIt; \
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
    auto replacements = args[5];

    try {
        auto methodArguments = ReadRegexMatcherMethodArguments(args);
        if (methodArguments.transaction == nullptr) {
            args.GetReturnValue().Set(replaceResult);
            return;
        }
        auto subjectRanges = GetTaintedRangesFromLocal(methodArguments.transaction, methodArguments.self);
        auto replacerRanges = GetTaintedRangesFromLocal(methodArguments.transaction, methodArguments.replacer);

        if (subjectRanges == nullptr && replacerRanges == nullptr) {
            args.GetReturnValue().Set(replaceResult);
            return;
        }

        INIT_REGEX_REPLACE_PROPERTIES

        if (subjectRanges != nullptr) {
            subjectIt = subjectRanges->begin();
            subjectItEnd = subjectRanges->end();
        }

        auto newRanges = methodArguments.transaction->GetSharedVectorRange();
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
                ADD_RANGES_BEFORE_REPLACEMENT
            }
            ADD_REPLACEMENT_RANGES

            lastEnd = currentReplacement.index + currentReplacement.matchLength;
            offset = offset + currentReplacement.offset;
        }
        if (subjectRanges != nullptr) {
            ADD_RANGES_AFTER_ALL_REPLACEMENTS
        }

        if (newRanges->Size() > 0) {
            auto resultString = replaceResult->ToString(isolate->GetCurrentContext()).ToLocalChecked();
            auto resultLength = resultString->Length();
            if (resultLength == 1) {
                replaceResult = tainted::NewExternalString(isolate, replaceResult);
            }
            auto key = utils::GetLocalStringPointer(replaceResult);
            methodArguments.transaction->AddTainted(key, newRanges, replaceResult);
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

