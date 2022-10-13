/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_API_SUBSTRING_H_
#define SRC_API_SUBSTRING_H_

#include <node.h>

using v8::Local;
using v8::Object;

namespace iast {
namespace api {
class Substring {
 public:
    static void Init(Local<Object> exports);

 private:
    explicit Substring();
    ~Substring();
};
}   // namespace api
}   // namespace iast
#endif  // SRC_API_SUBSTRING_H_
