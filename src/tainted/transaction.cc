/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#include <cstdint>
#include <string>
#include <map>
#include <list>
#include <vector>
#include <queue>
#include <memory>

#include "../gc/gc.h"
#include "../utils/jsobject_utils.h"
#include "transaction.h"
#include "../container/weakmap.h"

namespace iast {
namespace tainted {

void Transaction::Clean() noexcept {
    _taintedMap.clean();
    cleanInputInfos();
    _rangesPool.clear();
    cleanSharedVectors();
    _sharedRangesPool.clear();
    _taintedObjPool.clear();
}

Transaction::~Transaction() noexcept {
    Clean();
}

void Transaction::cleanInputInfos() noexcept {
    for (std::vector<InputInfo *>::iterator it = _usedInputInfo.begin();
         it != _usedInputInfo.end(); ++it) {
        if (*it) {
            delete *it;
        }
    }
    _usedInputInfo.resize(0);
}
void Transaction::cleanSharedVectors() {
    while (!_usedSharedRanges.empty()) {
        auto sr = _usedSharedRanges.front();
        _sharedRangesPool.push(sr);
        _usedSharedRanges.pop();
    }
}

InputInfo* Transaction::createNewInputInfo(v8::Local<v8::Value> parameterName,
        v8::Local<v8::Value> parameterValue,
        v8::Local<v8::Value> type) {
    InputInfo* newInputInfo = new InputInfo(parameterName, parameterValue, type);
    if (newInputInfo != nullptr) {
      _usedInputInfo.push_back(newInputInfo);
    }

    return newInputInfo;
}
}  // namespace tainted
}  // namespace iast
