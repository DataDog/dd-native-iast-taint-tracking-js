// Copyright 2022 Datadog, Inc.
#ifndef SRC_TAINTED_RANGE_H_
#define SRC_TAINTED_RANGE_H_
#include <node.h>
#include <memory>

#include "input_info.h"

namespace iast {
namespace tainted {
class Range {
 public:
    explicit Range(int start, int end, InputInfo *inputInfo);
    Range(const Range& taintedRange);
    ~Range();
    v8::Local<v8::Object> toJSObject(v8::Isolate* isolate);
    int start;
    int end;
    InputInfo* inputInfo;
};
}    // namespace tainted
}    // namespace iast
#endif  // SRC_TAINTED_RANGE_H_
