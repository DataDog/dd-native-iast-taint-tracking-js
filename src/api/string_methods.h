// Copyright 2022 Datadog, Inc.
#ifndef SRC_API_STRING_METHODS_H_
#define SRC_API_STRING_METHODS_H_

#include <node.h>

namespace iast {
namespace api {
using v8::Local;
using v8::Object;

class StringMethods {
 public:
    static void Init(Local<Object> exports);

 private:
    StringMethods();
    ~StringMethods();
};
}    // namespace api
}    // namespace iast

#endif  // SRC_API_STRING_METHODS_H_
