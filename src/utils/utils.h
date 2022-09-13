#ifndef UTILS_H
#define UTILS_H

#include <node.h>

namespace iast {
namespace utils {
    inline bool ValidateMethodAtLeastArguments(const v8::FunctionCallbackInfo<v8::Value>& args, int argc, const char* message) {
        if (args.Length() < argc) {
            auto isolate = args.GetIsolate();
            isolate->ThrowException(v8::Exception::TypeError(
                    v8::String::NewFromUtf8(isolate,
                                            message, v8::NewStringType::kNormal).ToLocalChecked()));
            return false;
        }
        return true;
    };

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

} // namespace utils
} // namespace iast


#endif
