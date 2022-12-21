/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_UTILS_PROPAGATION_H_
#define SRC_API_SLICE_H_

#include "../tainted/tainted_object.h"
#include "../tainted/transaction.h"

#define MAX(x,y) ((x > y) ? x : y)
#define MIN(x,y) ((x < y) ? x : y)

namespace iast {
namespace utils {
using tainted::Transaction;
using tainted::TaintedObject;

SharedRanges* getRangesInSlice(Transaction* transaction, TaintedObject* obj, int sliceStart, int sliceEnd);
}  // namespace utils
}  // namespace iast

#endif  // SRC_UTILS_PROPAGATION_H_
