/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <node.h>
#include <string>

#include "metrics.h"

#include "../iast.h"
#include "../utils/string_utils.h"
#include "../utils/jsobject_utils.h"

using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Local;
using v8::NewStringType;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Number;

namespace iast {
namespace api {

void GetMetrics(const FunctionCallbackInfo<Value>& args) {
    auto argsLength = args.Length();
    auto isolate = args.GetIsolate();

    if (argsLength < 2) {
        isolate->ThrowException(Exception::TypeError(
                        String::NewFromUtf8(isolate,
                        "Wrong number of arguments",
                        NewStringType::kNormal).ToLocalChecked()));
        return;
    }


    auto telemetryVerbosity = args[1];
    if (!args[1]->IsNumber()) {
        args.GetReturnValue().SetNull();
        return;
    }

    std::string transactionId = utils::GetStringValue(args.GetIsolate(), args[0]);
    auto transaction = GetTransaction(transactionId);
    if (!transaction) {
        args.GetReturnValue().SetNull();
        return;
    }

    auto context = isolate->GetCurrentContext();
    auto jsMetrics = Object::New(isolate);
    switch (static_cast<TelemetryVerbosity>(telemetryVerbosity->IntegerValue(context).FromJust())) {
        case TelemetryVerbosity::DEBUG:
        case TelemetryVerbosity::INFORMATION:
            jsMetrics->Set(context,
                    utils::NewV8String(isolate, "requestCount"),
                    Number::New(isolate, transaction->GetTaintedCount()))
            .Check();

            args.GetReturnValue().Set(jsMetrics);
            break;
        default:
            args.GetReturnValue().SetNull();
            break;
    }
}

void Metrics::Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "getMetrics", GetMetrics);
}
}   // namespace api
}   // namespace iast
