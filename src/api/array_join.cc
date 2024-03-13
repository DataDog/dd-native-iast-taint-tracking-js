/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <new>
#include <vector>
#include <memory>
#include <string>

#include "array_join.h"
#include "../tainted/range.h"
#include "../tainted/string_resource.h"
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

const int DEFAULT_JOIN_SEPARATOR_LENGTH = 1;

void copyRangesWithOffset(Transaction* transaction,
        SharedRanges* origRanges,
        SharedRanges* destRanges,
        int offset) {
    if (origRanges != nullptr) {
        auto end = origRanges->end();
        for (auto it = origRanges->begin(); it != end; it++) {
            auto origRange = *it;
            auto newRange = transaction->GetRange(
                origRange->start + offset,
                origRange->end + offset,
                origRange->inputInfo,
                origRange->secureMarks);
            if (newRange != nullptr) {
                destRanges->PushBack(newRange);
            } else {
                break;
            }
        }
    }
}

SharedRanges* getJoinResultRanges(Isolate* isolate,
        Transaction* transaction, v8::Array* arr,
        SharedRanges* separatorRanges,
        int separatorLength) {
    auto length = arr->Length();
    int offset = 0;
    auto newRanges = transaction->GetSharedVectorRange();
    auto context = isolate->GetCurrentContext();
    for (uint32_t i = 0; i < length; i++) {
        if (i > 0) {
            copyRangesWithOffset(transaction, separatorRanges, newRanges, offset);
            offset += separatorLength;
        }
        auto maybeItem = arr->Get(context, i);
        if (!maybeItem.IsEmpty()) {
            auto item = maybeItem.ToLocalChecked();
            auto taintedItem = transaction->FindTaintedObject(utils::GetLocalPointer(item));
            auto itemRanges = taintedItem ? taintedItem->getRanges() : nullptr;
            copyRangesWithOffset(transaction, itemRanges, newRanges, offset);
            offset += utils::GetLength(isolate, item);
        }
    }
    if (newRanges->Size() == 0) {
        return nullptr;
    }
    return newRanges;
}

void ArrayJoinOperator(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    if (args.Length() < 3) {
        isolate->ThrowException(v8::Exception::TypeError(
                        v8::String::NewFromUtf8(isolate,
                        "Wrong number of arguments",
                        v8::NewStringType::kNormal).ToLocalChecked()));
        return;
    }
    
    auto result = args[1];
    
    if (!result->IsString()) {
        args.GetReturnValue().Set(result);
        return;
    }

    auto transaction = GetTransaction(utils::GetLocalPointer(args[0]));
    if (transaction == nullptr) {
        args.GetReturnValue().Set(result);
        return;
    }

    auto thisArg = args[2];
    if (thisArg->IsObject()) {
        auto arrObj = v8::Object::Cast(*thisArg);
        if (arrObj->IsArray()) {
            try {
                int separatorLength = DEFAULT_JOIN_SEPARATOR_LENGTH;
                SharedRanges* separatorRanges = nullptr;
                if (args.Length() > 3) {
                    auto separatorArg = args[3];
                    auto separatorValue = (*separatorArg);
                    if (!separatorValue->IsUndefined()) {
                        auto taintedSeparator = transaction->FindTaintedObject(utils::GetLocalPointer(separatorArg));
                        separatorRanges = taintedSeparator ? taintedSeparator->getRanges() : nullptr;
                        separatorLength = utils::GetCoercedLength(isolate, separatorArg);
                    }
                }
                auto arr = v8::Array::Cast(arrObj);

                auto newRanges = getJoinResultRanges(isolate, transaction, arr, separatorRanges, separatorLength);
                if (newRanges != nullptr) {
                    auto key = utils::GetLocalPointer(result);
                    transaction->AddTainted(key, newRanges, result);
                    args.GetReturnValue().Set(result);
                    return;
                }
            } catch (const std::bad_alloc& err) {
            } catch (const container::QueuedPoolBadAlloc& err) {
            } catch (const container::PoolBadAlloc& err) {
            }
        }
    }
    args.GetReturnValue().Set(args[1]);
}

void ArrayJoinOperations::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "arrayJoin", ArrayJoinOperator);
}
}   // namespace api
}   // namespace iast

