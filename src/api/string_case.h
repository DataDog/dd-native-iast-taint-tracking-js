/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_API_STRING_CASE_H_
#define SRC_API_STRING_CASE_H_

#include <node.h>
namespace iast {
namespace api {
class StringCaseOperations {
 public:
    static void Init(v8::Local<v8::Object> exports);

 private:
    StringCaseOperations();
    ~StringCaseOperations();
};
}   // namespace api
}   // namespace iast
#endif  // SRC_API_STRING_CASE_H_
