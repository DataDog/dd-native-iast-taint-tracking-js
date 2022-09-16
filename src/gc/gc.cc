// Copyright 2022 Datadog, Inc.
#include "gc.h"
#include "../iast.h"


namespace iast {
namespace gc {

void OnMarkSweepCompact(v8::Isolate *isolate, v8::GCType type, v8::GCCallbackFlags flags) {
    RehashAllTransactions();
}

void OnScavenge(v8::Isolate *isolate, v8::GCType type, v8::GCCallbackFlags flags) {
    RehashAllTransactions();
}


}   // namespace gc
}   // namespace iast

