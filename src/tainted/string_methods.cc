// Copyright 2022 Datadog, Inc.
#include <node.h>
#include <cstddef>
#include <string>
#include <map>
#include <locale>
#include <codecvt>
#include <list>
#include <vector>
#include <iterator>
#include <iostream>
#include <memory>

#include "string_methods.h"
#include "input_info.h"
#include "tainted_object.h"
#include "transaction.h"
#include "../gc/gc.h"
#include "../iast.h"

using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::NewStringType;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Array;

namespace iast {
namespace tainted {

void SaveTaintedRanges(v8::Local<v8::Value> string, SharedRanges* taintedRanges, Transaction* transaction) {
    auto stringPointer = utils::GetLocalStringPointer(string);
    auto tainted = transaction->getAvailableTaintedObject();
    if (tainted != nullptr) {
        tainted->_key = stringPointer;
        tainted->setRanges(taintedRanges);
        tainted->Reset(string);
        tainted->SetWeak(iast::gc::OnGarbageCollected, v8::WeakCallbackType::kParameter);
        transaction->AddTainted(stringPointer, tainted);
    }
}

void NewTaintedStringInstanceMethod(const FunctionCallbackInfo<Value>& args) {
    if (args.Length() < 4) {
        auto isolate = args.GetIsolate();
        isolate->ThrowException(v8::Exception::TypeError(
                        v8::String::NewFromUtf8(isolate,
                        "Wrong number of arguments",
                        v8::NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    auto transactionIdArgument = args[0];
    auto parameterValue = args[1];
    auto parameterName = args[2];
    auto type = args[3];

    args.GetReturnValue().Set(parameterValue);
    if (!parameterValue->IsString()) {
        return;
    }
    try {
    uintptr_t transactionId = utils::GetLocalStringPointer(transactionIdArgument);
    auto transaction = NewTransaction(transactionId);
    if (transaction != nullptr) {
        uintptr_t ptrToFind = utils::GetLocalStringPointer(parameterValue);
        auto existingRanges = transaction->GetRanges(ptrToFind);
        if (existingRanges == nullptr) {
            auto result = parameterValue;
            InputInfo* inputInfo =
                transaction->createNewInputInfo(
                        parameterName, parameterValue, type);

            if (inputInfo == nullptr) {
                return;
            }

            auto range = transaction->getAvailableTaintedRange(0,
                    utils::GetLength(args.GetIsolate(), parameterValue),
                    inputInfo);
            if (range != nullptr) {
                auto ranges = transaction->getAvailableSharedVector();
                ranges->push_back(range);
                SaveTaintedRanges(result, ranges, transaction);
            }
        }
    }
    } catch (const tainted::NotAvailableRangeVectorsException& err) {
        // TODO(julio): propagate exception to JS?
    }
}

void IsTaintedInstanceMethod(const FunctionCallbackInfo<Value>& args) {
    if (args.Length() < 2) {
        auto isolate = args.GetIsolate();
        isolate->ThrowException(v8::Exception::TypeError(
                        v8::String::NewFromUtf8(isolate,
                        "Wrong number of arguments",
                        v8::NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    args.GetReturnValue().Set(false);
    uintptr_t transactionId = utils::GetLocalStringPointer(args[0]);
    auto transaction = GetTransaction(transactionId);
    if (!transaction) {
        return;
    }

    uintptr_t ptr1 = utils::GetLocalStringPointer(args[1]);
    if (transaction->GetRanges(ptr1)) {
        args.GetReturnValue().Set(true);
    }
}

void GetRangesInstanceMethod(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    if (args.Length() != 2) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate,
                "Wrong number of arguments",
                NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    uintptr_t transactionId = utils::GetLocalStringPointer(args[0]);
    auto transaction = GetTransaction(transactionId);
    if (transaction != nullptr) {
        auto ptr = utils::GetLocalStringPointer(args[1]);
        auto ranges = transaction->GetRanges(ptr);
        if (ranges != nullptr) {
            auto currentContext = isolate->GetCurrentContext();
            auto jsRanges = Array::New(isolate);
            int length = ranges->size();
            for (int i = 0; i < length; i++) {
                auto jsRange = ranges->at(i)->toJSObject(isolate);
                jsRanges->Set(currentContext, i, jsRange).CHECK();
            }
            args.GetReturnValue().Set(jsRanges);
            return;
        }
    }
    args.GetReturnValue().SetNull();
}

void EndTransactionInstanceMethod(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    if (args.Length() != 1) {
        // Throw an Error that is passed back to JavaScript
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate,
                "Wrong number of arguments",
                NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    auto transactionId = utils::GetLocalStringPointer(args[0]);
    RemoveTransaction(transactionId);
}

void StringMethods::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "newTaintedString", NewTaintedStringInstanceMethod);
    NODE_SET_METHOD(exports, "isTainted", IsTaintedInstanceMethod);
    NODE_SET_METHOD(exports, "getRanges", GetRangesInstanceMethod);
    NODE_SET_METHOD(exports, "endTransaction", EndTransactionInstanceMethod);
}
}  // namespace tainted
}  // namespace iast
