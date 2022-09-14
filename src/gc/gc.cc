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

void OnGarbageCollected(const v8::WeakCallbackInfo<tainted::TaintedObject> &info) {
    auto tainted = (tainted::TaintedObject *) info.GetParameter();
    if (!tainted->IsEmpty()) {
        tainted->Reset();
    }
    auto transactionRanges = GetTransaction(tainted->getId());
    if (transactionRanges!= nullptr) {
        auto ranges = tainted->getRanges();
        // Just one ref wich belongs to inUse array so it can be cleared and returned to availabe queue.
        if (ranges->getRefs() == 1) {
            ranges->clear();
            transactionRanges->ReturnSharedVector(ranges);
        }
        transactionRanges->returnTaintedObject(tainted);
    }
    tainted->setRanges(nullptr);
}

}   // namespace gc
}   // namespace iast
