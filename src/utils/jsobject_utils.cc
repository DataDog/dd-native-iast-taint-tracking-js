#ifndef JS_OBJECT_UTILS_CC
#define JS_OBJECT_UTILS_CC
#include <node.h>
#include <string.h>
#include <locale>
#include <codecvt>
#include "jsobject_utils.h"
#include "../iast_node.h"


namespace iast {
namespace utils {
    v8::Local<v8::Value> NewV8String(v8::Isolate* isolate, const char* keyName) {
        return v8::String::NewFromUtf8(isolate, keyName, v8::NewStringType::kNormal).ToLocalChecked();
    }

    char* GetCharsFromV8Value(v8::Isolate* isolate, v8::Local<v8::Value> value) {
        const v8::String::Utf8Value UTF8_VALUE(isolate, value);
        return strdup(*utf8Value);
    }

    char* GetCharsFromV8ObjectProperty(v8::Isolate* isolate, v8::Local<v8::Context> context,
            v8::Object* v8Object, const char* propertyName) {
        auto jsPropertyValue = v8Object->Get(context, NewV8String(isolate, propertyName)).ToLocalChecked();
        return GetCharsFromV8Value(isolate, jsPropertyValue);
    }

    int GetIntFromV8ObjectProperty(v8::Isolate* isolate, v8::Local<v8::Context> context,
            v8::Object* v8Object, const char* propertyName) {
        auto jsPropertyValue = v8Object->Get(context, NewV8String(isolate, propertyName)).ToLocalChecked();
        return v8::Number::Cast(*jsPropertyValue)->Value();
    }

    bool IsExternal(v8::String* originalString) {
        return originalString->IS_EXTERNAL();
    }

    void CopyCharArrToUint16Arr(const char* charArr, uint16_t* result) {
        std::string originalString(charArr);
        std::u16string utf16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(originalString.data());
        int i = 0;
        for (char16_t c : utf16) {
            result[i] = c;
            i++;
        }
    }

} // namespace utils
} // namespace iast
#endif
