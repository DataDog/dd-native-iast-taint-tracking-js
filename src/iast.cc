// Copyright 2022 Datadog, Inc.

#include <node.h>
#include "iast.h"
#include "gc/gc.h"
#include "container/singleton.h"
#include "tainted/string_methods.h"
#include "tainted/transaction.h"

using iastManager = iast::container::Singleton<iast::IastManager<iast::tainted::Transaction>>;

namespace iast {

void RehashAllTransactions(void) {
    iastManager::GetInstance().RehashAll();
}

void RemoveTransaction(iast_key_t id) {
    iastManager::GetInstance().Remove(id);
}

Transaction* GetTransaction(iast_key_t id) {
    return iastManager::GetInstance().Get(id);
}

Transaction* NewTransaction(iast_key_t id) {
    return iastManager::GetInstance().New(id);
}

void Init(v8::Local<v8::Object> exports) {
    tainted::StringMethods::Init(exports);
    exports->GetIsolate()->AddGCEpilogueCallback(iast::gc::OnScavenge, v8::GCType::kGCTypeScavenge);
    exports->GetIsolate()->AddGCEpilogueCallback(iast::gc::OnMarkSweepCompact, v8::GCType::kGCTypeMarkSweepCompact);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init);
}   // namespace iast
