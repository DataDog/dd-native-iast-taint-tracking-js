#ifndef SRC_IAST_TYPES_H_
#define SRC_IAST_TYPES_H_

#include "iastlimits.h"
#include "container/weakmap.h"
#include "container/shared_vector.h"
#include "container/queued_pool.h"
#include "container/pool.h"
#include "tainted/input_info.h"
#include "tainted/range.h"
#include "tainted/tainted_object.h"
#include "tainted/transaction.h"


using SharedRanges = iast::container::SharedVector<iast::tainted::Range*>;
using InputInfo = iast::tainted::InputInfo;
using RangePool = iast::container::Pool<iast::tainted::Range, iast::Limits::MAX_TAINTED_RANGES>;
using Transaction = iast::tainted::Transaction;
using TransactionPool = iast::container::QueuedPool<Transaction>;

#endif /* ifndef SRC_IAST_TYPES_H_ */
