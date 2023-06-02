/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/

#ifndef SRC_IASTLIMITS_H_
#define SRC_IASTLIMITS_H_

#include <cstddef>
namespace iast {
struct Limits {
    static const size_t MAX_RANGES = 50;
    static const size_t MAX_TAINTED_OBJECTS = 4096;  // result of pow(2, 14);
    static const size_t MAX_GLOBAL_TAINTED_RANGES = MAX_RANGES * MAX_TAINTED_OBJECTS;
    static const size_t MAX_TAINTED_RANGE_VECTORS = MAX_TAINTED_OBJECTS;
};
}  // namespace iast


#endif  // SRC_IASTLIMITS_H_
