#ifndef _IAST_H_
#define _IAST_H_

#include <cstddef>
#include <map>
#include <cstdint>
#include <iostream>


#include "iast_manager.h"

#include "tainted/transaction.h"

using iast::tainted::Transaction;

namespace iast {

void RehashAllTransactions(void);
void RemoveTransaction(iast_key_t id);
Transaction* GetTransaction(iast_key_t id);
Transaction* NewTransaction(iast_key_t id);

}  // namespace iast

#endif /* ifndef _IAST_H_ */

