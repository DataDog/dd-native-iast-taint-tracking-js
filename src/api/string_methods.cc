// Copyright 2022 Datadog, Inc.
#include <node.h>
#include <cstddef>
#include <string>
#include <map>
#include <locale>
#include <codecvt>
#include <list>
#include <iterator>
#include <memory>

#include "string_methods.h"
#include "../tainted/string_resource.h"
#include "../tainted/input_info.h"
#include "../tainted/tainted_object.h"
#include "../tainted/transaction.h"
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

using iast::tainted::InputInfo;

namespace iast {
namespace api {

void CreateTransaction(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    args.GetReturnValue().Set(tainted::NewExternalString(isolate, args[0]));
}

void NewTaintedString(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    if (args.Length() < 4) {
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

    if (!(args[1]->IsString())) {
        args.GetReturnValue().Set(args[1]);
        return;
    }

    // if string length == 1 then make a new one in order to avoid chache issues.
    if (v8::Local<v8::String>::Cast(args[1])->Length() == 1) {
        v8::String::Utf8Value param1(isolate, args[1]);
        std::string cppStr(*param1);
        v8::Local<v8::String> newStr = v8::String::NewFromUtf8(isolate, cppStr.c_str()).ToLocalChecked();
        parameterValue = newStr;
    }

    args.GetReturnValue().Set(parameterValue);
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

                auto range = transaction->GetAvailableTaintedRange(0,
                        utils::GetLength(args.GetIsolate(), parameterValue),
                        inputInfo);
                if (range != nullptr) {
                    auto ranges = transaction->GetAvailableSharedVector();
                    ranges->push_back(range);
                    auto stringPointer = utils::GetLocalStringPointer(result);
                    transaction->AddTainted(stringPointer, ranges, result);
                }
            }
        }
    } catch (const tainted::NotAvailableRangeVectorsException& err) {
        // TODO(julio): propagate exception to JS?
    }
}

void IsTainted(const FunctionCallbackInfo<Value>& args) {
    if (args.Length() < 2) {
        auto isolate = args.GetIsolate();
        isolate->ThrowException(v8::Exception::TypeError(
                        v8::String::NewFromUtf8(isolate,
                        "Wrong number of arguments",
                        v8::NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    uintptr_t transactionId = utils::GetLocalStringPointer(args[0]);
    auto transaction = GetTransaction(transactionId);
    if (!transaction) {
        args.GetReturnValue().Set(false);
        return;
    }

    uintptr_t ptr1 = utils::GetLocalStringPointer(args[1]);
    if (transaction->GetRanges(ptr1)) {
        args.GetReturnValue().Set(true);
    } else {
        args.GetReturnValue().Set(false);
    }
}

void GetRanges(const FunctionCallbackInfo<Value>& args) {
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

void DeleteTransaction(const FunctionCallbackInfo<Value>& args) {
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
    NODE_SET_METHOD(exports, "createTransaction", CreateTransaction);
    NODE_SET_METHOD(exports, "newTaintedString", NewTaintedString);
    NODE_SET_METHOD(exports, "isTainted", IsTainted);  // TODO(julio): support several objects.
    NODE_SET_METHOD(exports, "getRanges", GetRanges);
    NODE_SET_METHOD(exports, "removeTransaction", DeleteTransaction);
}
}  // namespace api
}  // namespace iast
