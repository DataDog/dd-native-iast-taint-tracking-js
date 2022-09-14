// Copyright 2022 Datadog, Inc.
#ifndef SRC_GC_GC_H_
#define SRC_GC_GC_H_

#include <v8.h>

#include "../tainted/tainted_object.h"

namespace iast {
namespace gc {
void OnMarkSweepCompact(v8::Isolate *isolate, v8::GCType type, v8::GCCallbackFlags flags);
void OnScavenge(v8::Isolate *isolate, v8::GCType type, v8::GCCallbackFlags flags);
void OnGarbageCollected(const v8::WeakCallbackInfo<tainted::TaintedObject> &info);
}  // namespace gc
}  // namespace iast
#endif  // SRC_GC_GC_H_
