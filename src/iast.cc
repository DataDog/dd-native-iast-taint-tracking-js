/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/

#include <node.h>
#include <cstddef>
#include "iast.h"
#include "gc/gc.h"
#include "container/singleton.h"
#include "transaction_manager.h"
#include "api/taint_methods.h"
#include "tainted/transaction.h"
#include "api/concat.h"
#include "api/trim.h"
#include "api/slice.h"
#include "api/substring.h"
#include "api/replace.h"
#include "api/metrics.h"
#include "api/string_case.h"

using transactionManager = iast::container::Singleton<iast::TransactionManager<iast::tainted::Transaction,
      iast::tainted::transaction_key_t>>;

namespace iast {

void RehashAllTransactions(void) {
    transactionManager::GetInstance().RehashAll();
}

void RemoveTransaction(transaction_key_t id) {
    transactionManager::GetInstance().Remove(id);
}

Transaction* GetTransaction(transaction_key_t id) {
    return transactionManager::GetInstance().Get(id);
}

Transaction* NewTransaction(transaction_key_t id) {
    return transactionManager::GetInstance().New(id);
}

void SetMaxTransactions(size_t maxItems) {
    transactionManager::GetInstance().setMaxItems(maxItems);
}

void Init(v8::Local<v8::Object> exports) {
    api::TaintMethods::Init(exports);
    api::ConcatOperations::Init(exports);
    api::TrimOperations::Init(exports);
    api::SliceOperations::Init(exports);
    api::Substring::Init(exports);
    api::ReplaceOperations::Init(exports);
    api::StringCaseOperations::Init(exports);
    api::Metrics::Init(exports);
    exports->GetIsolate()->AddGCEpilogueCallback(iast::gc::OnScavenge, v8::GCType::kGCTypeScavenge);
    exports->GetIsolate()->AddGCEpilogueCallback(iast::gc::OnMarkSweepCompact, v8::GCType::kGCTypeMarkSweepCompact);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init);
}   // namespace iast
