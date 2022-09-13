#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <node.h>
#include <string>
#include <locale>
#include <codecvt>

#include "../iast_node.h"

namespace iast {
namespace utils {
    const int COERCED_NULL_LENGTH = 4;
    const int COERCED_UNDEFINED_LENGTH = 9;

    inline uintptr_t GetLocalStringPointer(v8::Local<v8::Value> val) {
        return *reinterpret_cast<uintptr_t*>(*val);
    }

    inline int GetCoercedLength(v8::Isolate* isolate, v8::Local<v8::Value> val) {
        if(val->IsString()) {
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
        v8::String::Utf8Value VAL_STR(isolate, val);
        return *valStr ? *valStr : nullptr;
    }



    inline int GetLength(v8::Isolate* isolate, v8::Local<v8::Value> val) {
        if(val->IsString()) {
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
} // namespace utils
} // namespace iast
#endif
