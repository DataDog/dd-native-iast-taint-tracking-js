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
            int start;
            int end;
            InputInfo* inputInfo;
    };

    v8::Local<v8::Object> GetJsObjectFromRange(v8::Isolate* isolate, v8::Local<v8::Context> context, Range* taintedRange);
} // namespace tainted
} // namespace iast
#endif // SRC_TAINTED_RANGE_H_
