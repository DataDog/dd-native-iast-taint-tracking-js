#ifndef RANGESMANAGER_CC
#define RANGESMANAGER_CC

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
        //TransactionRanges::availableTransactionRanges->push(this);
    }

    Transaction::Transaction() {
        this->taintedMap = new WeakMap();
        this->createdSharedVectors = 0;
        this->availableSharedVectors = new std::queue<container::SharedVector<Range*>*>();
    };

    Transaction::~Transaction() {
      this->taintedMap->clean();
      this->cleanInputInfos();
    };

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

void SaveTaintedRanges(v8::Local<v8::Value> string, SharedRanges* taintedRanges, Transaction* transaction) {
    auto stringPointer = utils::GetLocalStringPointer(string);
    //auto isolate = v8::Isolate::GetCurrent();
    auto tainted = transaction->getAvailableTaintedObject();
    if (tainted != nullptr) {
        tainted->_key = stringPointer;
        tainted->setRanges(taintedRanges);
        //tainted->ranges = taintedRanges;
        tainted->Reset(string);
        //tainted->target.Reset(isolate, string);
        //tainted->target.SetWeak(tainted, HdivNative::OnGarbageCollected, v8::WeakCallbackType::kParameter);
        tainted->SetWeak(iast::gc::OnGarbageCollected, v8::WeakCallbackType::kParameter);
        transaction->AddTainted(stringPointer, tainted);
        //transactionRanges->taintedMap->insert(stringPointer, tainted);
    }
}


    InputInfo* Transaction::createNewInputInfo(v8::Local<v8::Value> parameterName, v8::Local<v8::Value> parameterValue, v8::Local<v8::Value> type)
    {
        InputInfo* newInputInfo = new InputInfo(parameterName, parameterValue, type);
        if (newInputInfo != nullptr) {
          this->inputInfoVector.push_back(newInputInfo);
        }

        return newInputInfo;
    }


    Range* GetTaintedRangeFromJsObject(Transaction* transaction, v8::Object* jsTaintedRange, v8::Isolate* isolate, v8::Local<v8::Context> context) {
        int start = utils::GetIntFromV8ObjectProperty(isolate, context, jsTaintedRange, "start");
        int end = utils::GetIntFromV8ObjectProperty(isolate, context, jsTaintedRange, "end");
        auto jsInputInfo = v8::Object::Cast(*(jsTaintedRange->Get(context, utils::NewV8String(isolate, "iinfo")).ToLocalChecked()));
        return transaction->getAvailableTaintedRange(start, end, GetInputInfoFromJsObject(jsInputInfo, isolate, context));
    }
}  // namespace tainted
}  // namespace iast

#endif
