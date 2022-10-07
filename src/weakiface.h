/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/

#ifndef SRC_WEAKIFACE_H_
#define SRC_WEAKIFACE_H_

#include <cstdint>
#include <utility>

namespace iast {
using weak_key_t = uintptr_t;

template <typename T>
struct WeakObjIface {
    WeakObjIface<T>* _next = nullptr;
    weak_key_t _key = 0;

    bool IsEmpty() { return static_cast<T*>(this)->IsEmpty(); }
    template<class ...Args>
    weak_key_t Get(Args&& ...args) {
        return static_cast<T*>(this)->Get(std::forward<Args>(args)...);
    }
};
}  // namespace iast
#endif  // SRC_WEAKIFACE_H_
