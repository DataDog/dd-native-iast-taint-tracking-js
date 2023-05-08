/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include "propagation.h"

namespace iast {
namespace utils {
using tainted::Transaction;
using tainted::TaintedObject;

SharedRanges* getRangesInSlice(Transaction* transaction, TaintedObject* obj, int sliceStart, int sliceEnd) {
    SharedRanges* newRanges = nullptr;
    SharedRanges* oRanges = nullptr;

    if (!transaction || !obj || !(oRanges = obj->getRanges())) {
        return newRanges;
    }

    int resultLength = sliceEnd - sliceStart;
    for (auto it = oRanges->begin(); it != oRanges->end(); ++it) {
        auto oRange = *it;
        int start, end;

        if ((oRange->start < sliceStart) && (oRange->end <= sliceStart)) {
            // range out of bounds (left)
            continue;
        }

        if (oRange->start >= sliceEnd) {
            // out of bounds (right), no need to keep iterating
            break;
        }

        if ((oRange->start <= sliceStart) && (oRange->end > sliceEnd)) {
            // range greater than slice
            start = 0;
            end = oRange->end - sliceStart;
        } else if ((oRange->start >= sliceStart) && (oRange->end <= sliceEnd)) {
            // range contained
            start = oRange->start - sliceStart;
            end = oRange->end - sliceStart;
        } else if ((oRange->start < sliceStart) && (oRange->end <= sliceEnd)) {
            // parcial left
            start = 0;
            end = oRange->end - sliceStart;
        } else {
            // parcial right
            start = oRange->start - sliceStart;
            end = sliceEnd;
        }

        if (end > resultLength) {
            end = resultLength;
        }

        if (!newRanges) {
            newRanges = transaction->GetSharedVectorRange();
        }

        newRanges->PushBack(transaction->GetRange(start, end, oRange->inputInfo));
    }
    return newRanges;
}

}  //  namespace utils
}  //  namespace iast
