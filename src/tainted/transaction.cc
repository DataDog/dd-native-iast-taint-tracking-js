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

void Transaction::clean() {
    this->taintedMap->clean();
    this->cleanInputInfos();
    this->availableTaintedRanges.clear();
    this->cleanSharedVectors();
    this->taintedObjPool.clear();
}

Transaction::Transaction() {
    this->taintedMap = new WeakMap();
    this->createdSharedVectors = 0;
    this->availableSharedVectors = new std::queue<container::SharedVector<Range*>*>();
}

Transaction::~Transaction() {
  this->taintedMap->clean();
  this->cleanInputInfos();

  delete taintedMap;

  while (!this->availableSharedVectors->empty()) {
      auto sv = availableSharedVectors->front();
      availableSharedVectors->pop();
      delete sv;
  }

  delete this->availableSharedVectors;
}

void Transaction::cleanInputInfos() {
    for (std::vector<InputInfo *>::iterator it = this->inputInfoVector.begin();
         it != this->inputInfoVector.end(); ++it) {
        if (*it) {
            delete *it;
        }
    }
    this->inputInfoVector.resize(0);
}
void Transaction::cleanSharedVectors() {
    for (int i = 0; i < Limits::MAX_TAINTED_OBJECTS; i++) {
        auto taintedRangeVectorInUse = this->inUseSharedVectors[i];
        if (taintedRangeVectorInUse != nullptr) {
            taintedRangeVectorInUse->clear();
            this->inUseSharedVectors[i] = nullptr;
            this->availableSharedVectors->push(taintedRangeVectorInUse);
        }
    }
}

InputInfo* Transaction::createNewInputInfo(v8::Local<v8::Value> parameterName,
        v8::Local<v8::Value> parameterValue,
        v8::Local<v8::Value> type) {
    InputInfo* newInputInfo = new InputInfo(parameterName, parameterValue, type);
    if (newInputInfo != nullptr) {
      this->inputInfoVector.push_back(newInputInfo);
    }

    return newInputInfo;
}
}  // namespace tainted
}  // namespace iast
