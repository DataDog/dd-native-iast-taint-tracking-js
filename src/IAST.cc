// Copyright 2021 Datadog, Inc.

#include <node.h>

namespace IAST {

    void Init(v8::Local<v8::Object> exports) {
      /* equals::Equals::Init(exports); */
      /* tainted::StringMethods::Init(exports); */
      /* operations::PlusOperations::Init(exports); */
      /* operations::StringReplace::Init(exports); */
      /* operations::StringSubstring::Init(exports); */
      /* operations::StringSlice::Init(exports); */
      /* operations::ArrayJoin::Init(exports); */
      /* tainted::RangesManager::Init(exports); */
    }

    NODE_MODULE(NODE_GYP_MODULE_NAME, Init);
}   // namespace IAST
