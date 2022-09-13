#ifndef SRC_V8_GC_H_
#define SRC_V8_GC_H_

#include <v8.h>

#include "../tainted/tainted_object.h"

namespace iast {
namespace gc {
void OnMarkSweepCompact(v8::Isolate *isolate, v8::GCType type, v8::GCCallbackFlags flags);
void OnScavenge(v8::Isolate *isolate, v8::GCType type, v8::GCCallbackFlags flags);
void OnGarbageCollected(const v8::WeakCallbackInfo<tainted::TaintedObject> &info);
}  // namespace v8
}  // namespace iast
#endif /* ifndef SRC_V8_GC_H_ */
