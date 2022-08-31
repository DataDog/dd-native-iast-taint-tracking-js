// Copyright 2022 Datadog, Inc.

#include <node.h>

namespace iast {

    void Init(v8::Local<v8::Object> exports) {
    }

    NODE_MODULE(NODE_GYP_MODULE_NAME, Init);
}   // namespace iast
