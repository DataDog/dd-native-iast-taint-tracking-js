#ifndef LIMITS_H
#define LIMITS_H

namespace iast {
struct Limits {
    const static int MAX_TAINTED_OBJECTS = 16384; // result of pow(2, 14);
    const static int MAX_TAINTED_RANGES = 100 * MAX_TAINTED_OBJECTS;
    const static int MAX_TAINTED_RANGE_VECTORS = MAX_TAINTED_OBJECTS;
};
}  // namespace iast


#endif /* ifndef LIMITS_H */
