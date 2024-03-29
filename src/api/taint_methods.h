/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_API_TAINT_METHODS_H_
#define SRC_API_TAINT_METHODS_H_

#include <node.h>

namespace iast {
namespace api {
using v8::Local;
using v8::Object;

class TaintMethods {
 public:
    static void Init(Local<Object> exports);

 private:
    TaintMethods();
    ~TaintMethods();
};
}    // namespace api
}    // namespace iast

#endif  // SRC_API_TAINT_METHODS_H_
