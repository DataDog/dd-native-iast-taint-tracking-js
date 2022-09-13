// Copyright 2022 Datadog, Inc.

#include <node.h>
#include "iast.h"
#include "gc/gc.h"
#include "tainted/string_methods.h"



namespace iast {

void Init(v8::Local<v8::Object> exports) {
    tainted::StringMethods::Init(exports);
    exports->GetIsolate()->AddGCEpilogueCallback(iast::gc::OnScavenge, v8::GCType::kGCTypeScavenge);
    exports->GetIsolate()->AddGCEpilogueCallback(iast::gc::OnMarkSweepCompact, v8::GCType::kGCTypeMarkSweepCompact);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init);
}   // namespace iast
