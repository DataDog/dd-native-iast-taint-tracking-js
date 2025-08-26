/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <node.h>
#include <cstddef>
#include <string>
#include <map>
#include <locale>
#include <codecvt>
#include <list>
#include <iterator>
#include <memory>

#include "taint_methods.h"
#include "../tainted/string_resource.h"
#include "../tainted/input_info.h"
#include "../tainted/tainted_object.h"
#include "../tainted/transaction.h"
#include "../utils/jsobject_utils.h"
#include "../gc/gc.h"
#include "../iast.h"
#include "v8.h"
#include "../utils/string_utils.h"

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
using iast::tainted::secure_marks_t;

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

    if (!(args[0]->IsString()) || !Local<String>::Cast(args[0])->Length()) {
        args.GetReturnValue().Set(args[1]);
        return;
    }

    if (!(args[1]->IsString())) {
        args.GetReturnValue().Set(args[1]);
        return;
    }

    auto transactionIdArgument = args[0];
    auto parameterValue = args[1];
    auto parameterName = args[2];
    auto type = args[3];

    // if string length < 10 then make a new one in order to avoid cache issues.
    int len =  v8::Local<v8::String>::Cast(args[1])->Length();
    if (len == 0) {
        args.GetReturnValue().Set(args[1]);
        return;
    } else if (len == 1) {
        parameterValue = tainted::NewExternalString(isolate, args[1]);
    } else if (len < 10) {
        v8::String::Utf8Value param1(isolate, args[1]);
        std::string cppStr(*param1);
        auto newStr = v8::String::NewFromUtf8(isolate, cppStr.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
        parameterValue = newStr;
    }

    args.GetReturnValue().Set(parameterValue);

    uintptr_t transactionId = utils::GetLocalPointer(transactionIdArgument);

    try {
        auto transaction = NewTransaction(transactionId);
        if (transaction == nullptr) {
            return;
        }
        auto taintedObj = transaction->FindTaintedObject(utils::GetLocalPointer(parameterValue));
        if (taintedObj) {
            // Object already exist, nothing to do
            return;
        }

        InputInfo* inputInfo = transaction->createNewInputInfo(
                    parameterName, parameterValue, type);

        auto range = transaction->GetRange(0,
                utils::GetLength(args.GetIsolate(), parameterValue),
                inputInfo, 0);
        auto ranges = transaction->GetSharedVectorRange();
        ranges->PushBack(range);
        auto valuePointer = utils::GetLocalPointer(parameterValue);
        transaction->AddTainted(valuePointer, ranges, parameterValue);
    } catch (const std::bad_alloc& err) {
        // TODO(julio): log exception?
    } catch (const container::QueuedPoolBadAlloc& err) {
        // TODO(julio): log exception?
    } catch (const container::PoolBadAlloc& err) {
        // TODO(julio): log exception?
    }
}

void AddSecureMarksToTaintedString(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    if (args.Length() < 3) {
        isolate->ThrowException(v8::Exception::TypeError(
                v8::String::NewFromUtf8(isolate,
                                        "Wrong number of arguments",
                                        v8::NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    if (!(args[0]->IsString()) || !Local<String>::Cast(args[0])->Length()) {
        // invalid transaction id, return taintedString
        args.GetReturnValue().Set(args[1]);
        return;
    }

    if (!(args[1]->IsString())) {
        // invalid taintedString, return it
        args.GetReturnValue().Set(args[1]);
        return;
    }

    auto context = isolate->GetCurrentContext();

    auto transactionIdArgument = args[0];
    auto taintedString = args[1];
    auto secureMarksArgument = args[2];
    bool createNewTainted = args.Length() > 3 ? args[3]->BooleanValue(isolate) : true;

    args.GetReturnValue().Set(taintedString);

    secure_marks_t secureMarks = secureMarksArgument->IntegerValue(context).FromJust();
    if (secureMarks == 0) {
        // not secure marks to add
        return;
    }

    uintptr_t transactionId = utils::GetLocalPointer(transactionIdArgument);

    auto transaction = GetTransaction(transactionId);
    if (transaction == nullptr) {
        return;
    }
    auto taintedObj = transaction->FindTaintedObject(utils::GetLocalPointer(taintedString));
    if (!taintedObj) {
        // It is not a tainted object, do nothing
        return;
    }
    try {
        auto newRanges = transaction->GetSharedVectorRange();
        auto oRanges = taintedObj->getRanges();
        if (createNewTainted) {
            for (auto it = oRanges->begin(); it != oRanges->end(); ++it) {
                auto oRange = *it;
                auto start = oRange->start;
                auto end = oRange->end;
                auto oSecureMarks = oRange->secureMarks;
                newRanges->PushBack(transaction->GetRange(start, end, oRange->inputInfo, oSecureMarks | secureMarks));
            }
            taintedString = tainted::NewStringInstanceForNewTaintedObject
                    (isolate, v8::Local<v8::String>::Cast(taintedString));
            transaction->AddTainted(utils::GetLocalPointer(taintedString), newRanges, taintedString);
            args.GetReturnValue().Set(taintedString);
        } else {
            for (auto it = oRanges->begin(); it != oRanges->end(); ++it) {
                auto oRange = *it;
                auto oSecureMarks = oRange->secureMarks;
                oRange->secureMarks = oSecureMarks | secureMarks;
            }
        }
    } catch (const std::bad_alloc& err) {
    } catch (const container::QueuedPoolBadAlloc& err) {
    } catch (const container::PoolBadAlloc& err) {
    }
}

void IsTainted(const FunctionCallbackInfo<Value>& args) {
    auto argsLength = args.Length();
    if (argsLength < 2) {
        auto isolate = args.GetIsolate();
        isolate->ThrowException(v8::Exception::TypeError(
                        v8::String::NewFromUtf8(isolate,
                        "Wrong number of arguments",
                        v8::NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    uintptr_t transactionId = utils::GetLocalPointer(args[0]);
    auto transaction = GetTransaction(transactionId);
    if (!transaction) {
        args.GetReturnValue().Set(false);
        return;
    }
    for (auto i = 1; i < argsLength; i++) {
        auto taintedObj = transaction->FindTaintedObject(utils::GetLocalPointer(args[i]));
        if (taintedObj && taintedObj->getRanges()) {
            args.GetReturnValue().Set(true);
            return;
        }
    }
    args.GetReturnValue().Set(false);
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
    uintptr_t transactionId = utils::GetLocalPointer(args[0]);
    auto transaction = GetTransaction(transactionId);
    if (transaction != nullptr) {
        auto taintedObj = transaction->FindTaintedObject(utils::GetLocalPointer(args[1]));
        auto ranges = taintedObj ? taintedObj->getRanges() : nullptr;
        if (ranges != nullptr) {
            auto currentContext = isolate->GetCurrentContext();
            auto jsRanges = Array::New(isolate);
            int length = ranges->Size();
            for (int i = 0; i < length; i++) {
                auto jsRange = ranges->At(i)->toJSObject(isolate);
                jsRanges->Set(currentContext, i, jsRange).Check();
            }
            args.GetReturnValue().Set(jsRanges);
            return;
        }
    } else {
        std::cout << "~ GetRanges: no transaction" << std::endl;
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

    auto transactionId = utils::GetLocalPointer(args[0]);
    RemoveTransaction(transactionId);
}

void SetMaxTransactions(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    if (args.Length() != 1) {
        // Throw an Error that is passed back to JavaScript
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate,
                "Wrong number of arguments",
                NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    if (!args[0]->IsNumber()) {
        return;
    }

    iast::SetMaxTransactions(args[0]->IntegerValue(isolate->GetCurrentContext()).FromJust());
}

void NewTaintedObject(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    if (args.Length() < 4) {
        isolate->ThrowException(v8::Exception::TypeError(
                        v8::String::NewFromUtf8(isolate,
                        "Wrong number of arguments",
                        v8::NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    if (!(args[0]->IsString()) || !Local<String>::Cast(args[0])->Length()) {
        args.GetReturnValue().Set(args[1]);
        return;
    }

    if (args[1]->IsString()) {
        args.GetReturnValue().Set(args[1]);
        return;
    }

    auto transactionIdArgument = args[0];
    auto parameterValue = args[1];
    auto parameterName = args[2];
    auto type = args[3];

    args.GetReturnValue().Set(parameterValue);

    uintptr_t transactionId = utils::GetLocalPointer(transactionIdArgument);

    try {
        auto transaction = NewTransaction(transactionId);
        if (transaction == nullptr) {
            return;
        }
        auto taintedObj = transaction->FindTaintedObject(utils::GetLocalPointer(parameterValue));
        if (taintedObj) {
            // Object already exist, nothing to do
            return;
        }

        InputInfo* inputInfo = transaction->createNewInputInfo(
                    parameterName, parameterValue, type);

        auto range = transaction->GetRange(0,
            utils::GetLength(args.GetIsolate(), parameterValue),
            inputInfo, 0);
        auto ranges = transaction->GetSharedVectorRange();
        ranges->PushBack(range);

        auto valuePointer = utils::GetLocalPointer(parameterValue);
        transaction->AddTainted(valuePointer, ranges, parameterValue);
    } catch (const std::bad_alloc& err) {
        // TODO(julio): log exception?
    } catch (const container::QueuedPoolBadAlloc& err) {
        // TODO(julio): log exception?
    } catch (const container::PoolBadAlloc& err) {
        // TODO(julio): log exception?
    }
}

void TaintMethods::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "createTransaction", CreateTransaction);
    NODE_SET_METHOD(exports, "newTaintedString", NewTaintedString);
    NODE_SET_METHOD(exports, "addSecureMarksToTaintedString", AddSecureMarksToTaintedString);
    NODE_SET_METHOD(exports, "isTainted", IsTainted);  // TODO(julio): support several objects.
    NODE_SET_METHOD(exports, "getRanges", GetRanges);
    NODE_SET_METHOD(exports, "removeTransaction", DeleteTransaction);
    NODE_SET_METHOD(exports, "setMaxTransactions", SetMaxTransactions);
    NODE_SET_METHOD(exports, "newTaintedObject", NewTaintedObject);
}
}  // namespace api
}  // namespace iast
