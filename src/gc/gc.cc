/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <iostream>
#include "gc.h"
#include "../iast.h"


namespace iast {
namespace gc {

void OnMarkSweepCompact(v8::Isolate *isolate, v8::GCType type, v8::GCCallbackFlags flags) {
    std::cout << "OnMarkSweepCompact" << std::endl;
    RehashAllTransactions();
}

void OnScavenge(v8::Isolate *isolate, v8::GCType type, v8::GCCallbackFlags flags) {
    std::cout << "OnScavenge" << std::endl;
    RehashAllTransactions();
}


}   // namespace gc
}   // namespace iast

