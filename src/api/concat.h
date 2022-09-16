// Copyright 2022 Datadog, Inc.
#ifndef SRC_API_CONCAT_H_
#define SRC_API_CONCAT_H_

#include <node.h>
namespace iast {
namespace api {
class ConcatOperations {
 public:
    static void Init(v8::Local<v8::Object> exports);

 private:
    ConcatOperations();
    ~ConcatOperations();
};
}   // namespace api
}   // namespace iast
#endif  // SRC_API_CONCAT_H_
