// Copyright 2022 Datadog, Inc.

#ifndef SRC_IASTLIMITS_H_
#define SRC_IASTLIMITS_H_

namespace iast {
struct Limits {
    static const int MAX_RANGES = 100;
    static const int MAX_TAINTED_OBJECTS = 16384;  // result of pow(2, 14);
    static const int MAX_GLOBAL_TAINTED_RANGES = MAX_RANGES * MAX_TAINTED_OBJECTS;
    static const int MAX_TAINTED_RANGE_VECTORS = MAX_TAINTED_OBJECTS;
};
}  // namespace iast


#endif  // SRC_IASTLIMITS_H_
