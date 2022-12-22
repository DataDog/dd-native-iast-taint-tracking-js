/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_UTILS_VALIDATION_UTILS_H_
#define SRC_UTILS_VALIDATION_UTILS_H_

#include <node.h>

namespace iast {
namespace utils {
inline bool ValidateMethodArguments(const v8::FunctionCallbackInfo<v8::Value>& args, int argc, const char* message) {
    if (args.Length() != argc) {
        auto isolate = args.GetIsolate();
        isolate->ThrowException(v8::Exception::TypeError(
                v8::String::NewFromUtf8(isolate,
                                        message, v8::NewStringType::kNormal).ToLocalChecked()));
        return false;
    }
    return true;
}
}  // namespace utils
}  // namespace iast


#endif  // SRC_UTILS_VALIDATION_UTILS_H_
