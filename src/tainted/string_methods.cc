#include <cstddef>
#include <node.h>
#include <string>
#include <map>
#include <locale>
#include <codecvt>
#include <list>
#include <vector>
#include <iterator>
#include <memory>

#include "string_methods.h"
#include "input_info.h"
#include "tainted_object.h"
#include "transaction.h"
#include "../utils/utils.h"
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

/* void NewExternalStringInstanceMethod(const FunctionCallbackInfo<Value>& args) { */
/*     Isolate* isolate = args.GetIsolate(); */
/*     args.GetReturnValue().Set(NewExternalString(isolate, args[0])); */
/* } */

void AddRangeInstanceMethod(const FunctionCallbackInfo<Value>& args) {
    if (!utils::ValidateMethodArguments(args, 3, "Wrong number of arguments, expected transactionId, originalString and taintedRange")) {
        return;
    }
    if (args[1]->IsString()) {
        uintptr_t transactionId = utils::GetLocalStringPointer(args[0]);
        Transaction* transactionRanges = IastManager::GetInstance().Get(transactionId);
        if (transactionRanges != nullptr) {
            uintptr_t ptrToFind = utils::GetLocalStringPointer(args[1]);
            auto ranges = transactionRanges->GetRanges(ptrToFind);
            if (ranges != nullptr) {
                auto isolate = args.GetIsolate();
                auto context = isolate->GetCurrentContext();
                auto jsTaintedRange = Object::Cast(*args[2]);
                auto taintedRangeToAdd = tainted::GetTaintedRangeFromJsObject(transactionRanges, jsTaintedRange, isolate, context);
                auto newRanges = transactionRanges->getAvailableSharedVector();
                newRanges->add(ranges);
                newRanges->push_back(taintedRangeToAdd);
                transactionRanges->UpdateRanges(ptrToFind, newRanges);
            }
        }
    }
}

void NewTaintedStringInstanceMethod(const FunctionCallbackInfo<Value>& args) {
    // Check the number of arguments passed.
    if (!utils::ValidateMethodArguments(args, 4, "Wrong number of arguments, expected transactionId, originalString, paramName and type")) {
        return;
    }

    auto transactionIdArgument = args[0];
    auto parameterValue = args[1];
    auto parameterName = args[2];
    auto type = args[3];

    if (parameterValue->IsString()) {
        uintptr_t transactionId = utils::GetLocalStringPointer(transactionIdArgument);
        Transaction *transactionRanges = IastManager::GetInstance().New(transactionId);
        if (transactionRanges != nullptr) {
            uintptr_t ptrToFind = utils::GetLocalStringPointer(parameterValue);
            auto existingRanges = transactionRanges->GetRanges(ptrToFind);
            if (existingRanges == nullptr) {
                auto result = parameterValue;
                InputInfo* inputInfo =
                    transactionRanges->createNewInputInfo(
                        parameterName, parameterValue, type);

                if (inputInfo == nullptr) {
                  args.GetReturnValue().Set(result);
                  return;
                }

                auto range = transactionRanges->getAvailableTaintedRange(0,
                                                          utils::GetLength(args.GetIsolate(), parameterValue),
                                                          inputInfo);
                if (range != nullptr) {
                    auto ranges = transactionRanges->getAvailableSharedVector();
                    ranges->push_back(range);
                    SaveTaintedRanges(result, ranges, transactionRanges);
                }
                args.GetReturnValue().Set(result);
                return;
            }
        }
    }
    args.GetReturnValue().Set(parameterValue);
}

void NewTaintedStringWithRangesInstanceMethod(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    // Check the number of arguments passed.
    if (args.Length() != 3) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate,
                "Wrong number of arguments, expected transactionId, originalString, TaintedRanges[]",
                NewStringType::kNormal).ToLocalChecked()));
        return;
    }
    try {
        uintptr_t transactionId = utils::GetLocalStringPointer(args[0]);
        if (args[1]->IsString()) {
            Transaction *transactionRanges = IastManager::GetInstance().New(transactionId);
            if (transactionRanges != nullptr) {
                uintptr_t ptrToFind = utils::GetLocalStringPointer(args[1]);
                auto existingRanges = transactionRanges->GetRanges(ptrToFind);
                if (existingRanges == nullptr) {
                    auto result = args[1];
                    auto jsTaintedRanges = Array::Cast(*args[2]);
                    auto taintedRangesLength = jsTaintedRanges->Length();
                    auto context = isolate->GetCurrentContext();
                    auto ranges = transactionRanges->getAvailableSharedVector();
                    for (size_t i = 0; i < taintedRangesLength; i++) {
                        auto jsTaintedRange = Object::Cast(
                                *(jsTaintedRanges->Get(context, i).ToLocalChecked()));
                        auto taintedRange = tainted::GetTaintedRangeFromJsObject(transactionRanges, jsTaintedRange, isolate, context);
                        if (taintedRange != nullptr) {
                            ranges->push_back(taintedRange);
                        }
                    }
                    SaveTaintedRanges(result, ranges, transactionRanges);
                    args.GetReturnValue().Set(result);
                    return;
                }
            }
        }
    } catch (const tainted::NotAvailableRangeVectorsException& err) {
        // do nothing, just continue without taint
    }
    args.GetReturnValue().Set(args[1]);
}

void IsTaintedInstanceMethod(const FunctionCallbackInfo<Value>& args) {
    if (!utils::ValidateMethodAtLeastArguments(args, 2, "Wrong number of arguments")) {
        return;
    }
    //Isolate* isolate = args.GetIsolate();
    uintptr_t transactionId = utils::GetLocalStringPointer(args[0]);
    auto transactionRanges = IastManager::GetInstance().Get(transactionId);
    if (transactionRanges != nullptr) {
        auto argsSize = args.Length();
        for (int i = 1; i < argsSize; i++) {
            uintptr_t ptr1 = utils::GetLocalStringPointer(args[i]);
            auto ranges = transactionRanges->GetRanges(ptr1);
            if (ranges != nullptr) {
                args.GetReturnValue().Set(true);
                return;
            }
        }
    }
    args.GetReturnValue().Set(false);
}

void GetRangesInstanceMethod(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    // Check the number of arguments passed.
    if (args.Length() != 2) {
        // Throw an Error that is passed back to JavaScript
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate,
                "Wrong number of arguments",
                NewStringType::kNormal).ToLocalChecked()));
        return;
    }
    uintptr_t transactionId = utils::GetLocalStringPointer(args[0]);
    auto transactionRanges = IastManager::GetInstance().Get(transactionId);
    if (transactionRanges != nullptr) {
        auto ptr = utils::GetLocalStringPointer(args[1]);
        auto ranges = transactionRanges->GetRanges(ptr);
        if (ranges != nullptr) {
            auto currentContext = isolate->GetCurrentContext();
            auto jsRanges = Array::New(isolate);
            int length = ranges->size();
            for (int i = 0; i < length; i++) {
                auto range = ranges->at(i);
                auto jsRange = GetJsObjectFromRange(isolate, currentContext, range);
                jsRanges->Set(currentContext, i, jsRange).CHECK();
            }
            args.GetReturnValue().Set(jsRanges);
            return;
        }
    }
    args.GetReturnValue().SetNull();
}

void GetAllTaintedObjectsInstanceMethod(const FunctionCallbackInfo<Value>& args) {
    if(!utils::ValidateMethodArguments(args, 1, "Wrong number of arguments")) {
        return;
    }
    Isolate* isolate = args.GetIsolate();
    auto transactionId = utils::GetLocalStringPointer(args[0]);
    auto transactionRanges = IastManager::GetInstance().Get(transactionId);
    if (transactionRanges != nullptr) {
        auto currentContext = isolate->GetCurrentContext();
        //TaintedMap* taintedMap = transactionRanges->taintedMap;
        auto jsTaintedObjects = Array::New(isolate);
        int i = 0;
        for (auto it = transactionRanges->taintedObjPool.begin(); it != transactionRanges->taintedObjPool.end(); ++it) {
            auto taintedObject = *it;
            while (taintedObject != nullptr) {
                if (!taintedObject->IsEmpty()) {
                    auto jsTaintedObject = taintedObject->toJSObject(isolate);
                    //auto jsTaintedObject = tainted::GetJsObjectFromTaintedObject(isolate, currentContext, taintedObject);
                    jsTaintedObjects->Set(currentContext, i, jsTaintedObject);
                    i++;
                    taintedObject = static_cast<TaintedObject*>(taintedObject->_next);
                }
            }
        }
        args.GetReturnValue().Set(jsTaintedObjects);
        return;
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
    IastManager::GetInstance().Remove(transactionId);
}

void StringMethods::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "addRange", AddRangeInstanceMethod);
    NODE_SET_METHOD(exports, "newTaintedString", NewTaintedStringInstanceMethod);
    NODE_SET_METHOD(exports, "newTaintedStringWithRanges", NewTaintedStringWithRangesInstanceMethod);
    NODE_SET_METHOD(exports, "isTainted", IsTaintedInstanceMethod);
    NODE_SET_METHOD(exports, "getRanges", GetRangesInstanceMethod);
    NODE_SET_METHOD(exports, "getAllTaintedObjects", GetAllTaintedObjectsInstanceMethod);
    NODE_SET_METHOD(exports, "endTransaction", EndTransactionInstanceMethod);
}
}  // namespace tainted
}  // namespace iast
