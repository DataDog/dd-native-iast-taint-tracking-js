/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_IAST_H_
#define SRC_IAST_H_

#include <cstddef>
#include <map>
#include <cstdint>
#include <iostream>


#include "transaction_manager.h"
#include "tainted/transaction.h"

using iast::tainted::Transaction;

namespace iast {

void RehashAllTransactions(void);
void RemoveTransaction(iast_key_t id);
Transaction* GetTransaction(iast_key_t id);
Transaction* NewTransaction(iast_key_t id);

}  // namespace iast

#endif  // SRC_IAST_H_

