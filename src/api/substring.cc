/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef STRINGSUBSTRINGOBJECT_CC
#define STRINGSUBSTRINGOBJECT_CC

#include <node.h>
#include <vector>

#include "substring.h"
#include "../iastlimits.h"
#include "../tainted/range.h"
#include "../tainted/transaction.h"
#include "../iast.h"



using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::NewStringType;
using v8::Object;
using v8::String;
using v8::Value;

using iast::tainted::Range;

namespace iast {
namespace api {
void substring(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();

    if (args.Length() < 4) {
        isolate->ThrowException(v8::Exception::TypeError(
                        v8::String::NewFromUtf8(isolate,
                        "Wrong number of arguments",
                        v8::NewStringType::kNormal).ToLocalChecked()));
        return;
    }


    int argc = args.Length();

    auto transactionId = utils::GetLocalStringPointer(args[0]);
    auto vSubject = args[1];
    auto vResult = args[2];
    auto vSubstringStart = args[3];
    auto vSubstringEnd = args[4];

    auto transaction = GetTransaction(transactionId);
    if (transaction == nullptr) {
        args.GetReturnValue().Set(vResult);
        return;
    }

    try {
        uintptr_t subjectPointer = utils::GetLocalStringPointer(vSubject);
        auto oRanges = transaction->GetRanges(subjectPointer);

        int substringStart = vSubstringStart->ToInteger(context).ToLocalChecked()->Value();
        int substringEnd = argc > 4 ? vSubstringEnd->ToInteger(context).ToLocalChecked()->Value() : vSubject->ToString(context).ToLocalChecked()->Length();

        Local<String> result = vResult->ToString(context).ToLocalChecked();

        if (oRanges != nullptr) {
            auto newRanges = transaction->GetAvailableSharedVector();
            for(auto it = oRanges->begin(); it != oRanges->end(); ++it) {
                auto oRange = *it;
                int rangeEnd = oRange->end - substringStart;
                if (rangeEnd > result->Length()) {
                    rangeEnd = result->Length();
                }

                if (substringStart > oRange->start && substringStart < oRange->end ) {
                    if (substringStart != 0 || rangeEnd != oRange->end) {
                        auto newRange = transaction->GetAvailableTaintedRange(0, rangeEnd, oRange->inputInfo);
                        if (newRange) {
                            newRanges->push_back(newRange);
                        } else {
                            break;
                        }
                    } else {
                        newRanges->push_back(oRange);
                    }
                } else {
                    if (substringEnd > oRange->start && substringStart < oRange->end) {
                        if (substringStart != 0 || rangeEnd != oRange->end) {
                            int rangeStart = oRange->start - substringStart;
                            auto newRange = transaction->GetAvailableTaintedRange(rangeStart, rangeEnd, oRange->inputInfo);
                            if (newRange) {
                                newRanges->push_back(newRange);
                            } else {
                                break;
                            }
                        } else {
                            newRanges->push_back(oRange);
                        }
                    }
                }
            }

            if (newRanges->size()) {
                auto key = utils::GetLocalStringPointer(vResult);
                transaction->AddTainted(key, newRanges, vResult);
            }
        }
    } catch (const tainted::NotAvailableRangeVectorsException& err) {
        // do nothing, just continue without taint
    }

    args.GetReturnValue().Set(vResult);
}


void Substring::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "substring", substring);
}

}   // namespace api
}   // namespace iast
#endif
