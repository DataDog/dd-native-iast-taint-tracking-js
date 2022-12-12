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
#include "../tainted/string_resource.h"
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
using iast::utils::GetLocalStringPointer;

namespace iast {
namespace api {
void substring(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();
    int argc = args.Length();

    if (argc < 4) {
        isolate->ThrowException(v8::Exception::TypeError(
                        v8::String::NewFromUtf8(isolate,
                        "Wrong number of arguments",
                        v8::NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    auto result = args[1];
    auto subject = args[2];
    auto start = args[3];
    auto end = args[4];

    auto transaction = GetTransaction(GetLocalStringPointer(args[0]));
    if (transaction == nullptr) {
        args.GetReturnValue().Set(result);
        return;
    }

    if (Local<String>::Cast(subject)->Length() <= 1) {
        args.GetReturnValue().Set(result);
        return;
    }

    try {
        uintptr_t subjectPointer = GetLocalStringPointer(subject);
        auto taintedObj = transaction->FindTaintedObject(subjectPointer);
        auto oRanges = taintedObj ? taintedObj->getRanges() : nullptr;

        int substringStart = start->ToInteger(context).ToLocalChecked()->Value();
        int substringEnd = argc > 4 ?
            end->ToInteger(context).ToLocalChecked()->Value() :
            subject->ToString(context).ToLocalChecked()->Length();

        Local<String> stringResult = result->ToString(context).ToLocalChecked();

        if (oRanges != nullptr) {
            auto newRanges = transaction->GetSharedVectorRange();
            for (auto it = oRanges->begin(); it != oRanges->end(); ++it) {
                auto oRange = *it;
                if (oRange->start >= substringEnd) {
                    break;
                }

                if (oRange->end <= substringStart) {
                    continue;
                }

                int rangeEnd = oRange->end - substringStart;
                auto stringLen = stringResult->Length();
                if (rangeEnd > stringLen) {
                    rangeEnd = stringLen;
                }

                if (substringStart > oRange->start && substringStart < oRange->end) {
                    if (substringStart != 0 || rangeEnd != oRange->end) {
                        auto newRange = transaction->GetRange(0, rangeEnd, oRange->inputInfo);
                        if (newRange) {
                            newRanges->PushBack(newRange);
                        } else {
                            break;
                        }
                    } else {
                        newRanges->PushBack(oRange);
                    }
                } else {
                    if (substringEnd > oRange->start && substringStart < oRange->end) {
                        if (substringStart != 0 || rangeEnd != oRange->end) {
                            int rangeStart = oRange->start - substringStart;
                            auto newRange = transaction->GetRange(rangeStart,
                                    rangeEnd,
                                    oRange->inputInfo);
                            if (newRange) {
                                newRanges->PushBack(newRange);
                            } else {
                                break;
                            }
                        } else {
                            newRanges->PushBack(oRange);
                        }
                    }
                }
            }

            if (newRanges->Size()) {
                // Uninternalize string
                if (Local<String>::Cast(result)->Length() == 1) {
                   result = tainted::NewExternalString(isolate, result);
                }
                auto key = GetLocalStringPointer(result);
                transaction->AddTainted(key, newRanges, result);
            }
        }
    } catch (const std::bad_alloc& err) {
    } catch (const container::QueuedPoolBadAlloc& err) {
    } catch (const container::PoolBadAlloc& err) {
    }

    args.GetReturnValue().Set(result);
}


void Substring::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "substring", substring);
}

}   // namespace api
}   // namespace iast
#endif
