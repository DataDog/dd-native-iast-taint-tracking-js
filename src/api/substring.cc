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

    if (!utils::ValidateMethodAtLeastArguments(args, 4, "Wrong number of arguments")) return;

    int argc = args.Length();

    auto vSubject = args[1];
    auto vResult = args[2];
    auto vSubstringStart = args[3];
    auto vSubstringEnd = args[4];
    try {
        uintptr_t transactionId = utils::GetLocalStringPointer(args[0]);
        auto transaction = GetTransaction(transactionId);
        if (transaction == nullptr) {
            args.GetReturnValue().Set(vResult);
            return;
        }

        uintptr_t subjectPointer = utils::GetLocalStringPointer(vSubject);
        auto oRanges = transaction->GetRanges(subjectPointer);

        int substringStart = vSubstringStart->ToInteger(context).ToLocalChecked()->Value();
        int substringEnd = argc > 4 ? vSubstringEnd->ToInteger(context).ToLocalChecked()->Value() : vSubject->ToString(context).ToLocalChecked()->Length();

        Local<String> result = vResult->ToString(context).ToLocalChecked();
        int resultLength = result->Length();

        if (oRanges != nullptr) {
            auto newRanges = transaction->GetAvailableSharedVector();
            for(auto it = oRanges->begin(); it != oRanges->end(); ++it) {
                auto oRange = *it;
                int rangeEnd = oRange->end - substringStart;
                if (rangeEnd > resultLength) {
                    rangeEnd = resultLength;
                }

                if (substringStart > oRange->start) {
                    if (substringStart < oRange->end) {
                        if (substringStart != 0 || rangeEnd != oRange->end) {
                            CREATE_RANGE_AND_INSERT_OR_BREAK(newRange, newRanges, transactionRanges, 0, rangeEnd, oRange->inputInfo)
                        } else {
                            newRanges->push_back(oRange);
                        }
                    }
                } else {
                    if (substringEnd > oRange->start) {
                        if (substringStart < oRange->end) {
                            if (substringStart != 0 || rangeEnd != oRange->end) {
                                int rangeStart = oRange->start - substringStart;
                                CREATE_RANGE_AND_INSERT_OR_BREAK(newRange, newRanges, transactionRanges, rangeStart, rangeEnd, oRange->inputInfo)
                            } else {
                                newRanges->push_back(oRange);
                            }
                        }
                    }
                }
            }

            if (newRanges->size() > 0) {
                SaveTaintedRanges(vResult, newRanges, transactionRanges);
            }
        }
    } catch (const tainted::NotAvailableRangeVectorsException& err) {
        // do nothing, just continue without taint
    }

    args.GetReturnValue().Set(vResult);
}


void StringSubstring::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "substring", substring);
}

}   // namespace api
}   // namespace iast
#endif
