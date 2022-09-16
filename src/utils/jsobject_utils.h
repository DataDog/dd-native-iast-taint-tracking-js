// Copyright 2022 Datadog, Inc.
#ifndef SRC_UTILS_JSOBJECT_UTILS_H_
#define SRC_UTILS_JSOBJECT_UTILS_H_

#include <node.h>

namespace iast {
namespace utils {
v8::Local<v8::Value> NewV8String(v8::Isolate* isolate, const char* keyName);
char* GetCharsFromV8Value(v8::Isolate* isolate, v8::Local<v8::Value> value);
char* GetCharsFromV8ObjectProperty(v8::Isolate* isolate, v8::Local<v8::Context> context,
        v8::Object* v8Object, const char* propertyName);
int GetIntFromV8ObjectProperty(v8::Isolate* isolate, v8::Local<v8::Context> context,
        v8::Object* v8Object, const char* propertyName);
bool IsExternal(v8::String* originalString);
void CopyCharArrToUint16Arr(const char* charArr, uint16_t* result);
}   // namespace utils
}   // namespace iast
#endif  // SRC_UTILS_JSOBJECT_UTILS_H_
