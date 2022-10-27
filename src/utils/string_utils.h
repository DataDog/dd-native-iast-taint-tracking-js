/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_UTILS_STRING_UTILS_H_
#define SRC_UTILS_STRING_UTILS_H_

#include <node.h>
#include <string>
#include <locale>
#include <codecvt>

namespace iast {
namespace utils {
const int COERCED_NULL_LENGTH = 4;
const int COERCED_UNDEFINED_LENGTH = 9;

inline uintptr_t GetLocalStringPointer(v8::Local<v8::Value> val) {
    return *reinterpret_cast<uintptr_t*>(*val);
}

inline int GetCoercedLength(v8::Isolate* isolate, v8::Local<v8::Value> val) {
    if (val->IsString()) {
        return v8::String::Cast(*val)->Length();
    } else {
        if (val->IsUndefined()) {
            return COERCED_UNDEFINED_LENGTH;
        } else if (val->IsNull()) {
            return COERCED_NULL_LENGTH;
        } else {
            auto context = isolate->GetCurrentContext();
            auto firstLocalString =  val->ToString(context).ToLocalChecked();
            return firstLocalString->Length();
        }
    }
}

inline std::string GetCString(v8::Isolate* isolate, v8::Local<v8::Value> val) {
    v8::String::Utf8Value valStr(isolate, val);
    return *valStr ? *valStr : nullptr;
}



inline int GetLength(v8::Isolate* isolate, v8::Local<v8::Value> val) {
    if (val->IsString()) {
        return v8::String::Cast(*val)->Length();
    } else {
        if (val->IsUndefined()) {
            return 0;
        } else if (val->IsNull()) {
            return 0;
        } else {
            auto context = isolate->GetCurrentContext();
            auto firstLocalString =  val->ToString(context).ToLocalChecked();
            return firstLocalString->Length();
        }
    }
}
}   // namespace utils
}   // namespace iast
#endif  // SRC_UTILS_STRING_UTILS_H_
