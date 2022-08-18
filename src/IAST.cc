// Copyright 2021 Datadog, Inc.

#include <node.h>

namespace IAST {

    void Init(v8::Local<v8::Object> exports) {
    }

    NODE_MODULE(NODE_GYP_MODULE_NAME, Init);
}   // namespace IAST
