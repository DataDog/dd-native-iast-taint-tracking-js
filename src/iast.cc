// Copyright 2022 Datadog, Inc.

#include <node.h>
#include "iast.h"
#include "gc/gc.h"
#include "container/singleton.h"
#include "transaction_manager.h"
#include "api/string_methods.h"
#include "tainted/transaction.h"
#include "api/concat.h"

using transactionManger = iast::container::Singleton<iast::TransactionManager<iast::tainted::Transaction>>;

namespace iast {

void RehashAllTransactions(void) {
    transactionManger::GetInstance().RehashAll();
}

void RemoveTransaction(iast_key_t id) {
    transactionManger::GetInstance().Remove(id);
}

Transaction* GetTransaction(iast_key_t id) {
    return transactionManger::GetInstance().Get(id);
}

Transaction* NewTransaction(iast_key_t id) {
    return transactionManger::GetInstance().New(id);
}

void Init(v8::Local<v8::Object> exports) {
    api::StringMethods::Init(exports);
    api::ConcatOperations::Init(exports);
    exports->GetIsolate()->AddGCEpilogueCallback(iast::gc::OnScavenge, v8::GCType::kGCTypeScavenge);
    exports->GetIsolate()->AddGCEpilogueCallback(iast::gc::OnMarkSweepCompact, v8::GCType::kGCTypeMarkSweepCompact);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init);
}   // namespace iast
