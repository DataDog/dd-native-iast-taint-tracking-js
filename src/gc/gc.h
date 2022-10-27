/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_GC_GC_H_
#define SRC_GC_GC_H_

#include <v8.h>

namespace iast {
namespace gc {
void OnMarkSweepCompact(v8::Isolate *isolate, v8::GCType type, v8::GCCallbackFlags flags);
void OnScavenge(v8::Isolate *isolate, v8::GCType type, v8::GCCallbackFlags flags);
}  // namespace gc
}  // namespace iast
#endif  // SRC_GC_GC_H_
