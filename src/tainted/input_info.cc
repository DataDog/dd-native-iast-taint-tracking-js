#ifndef INPUTINFO_CC
#define INPUTINFO_CC

#include <v8.h>
#include <string>

#include "input_info.h"
#include "../utils/jsobject_utils.h"
#include "../iast_node.h"


using v8::String;
using v8::NewStringType;
using v8::Isolate;

namespace iast {
namespace tainted {
    InputInfo::InputInfo(v8::Local<v8::Value> parameterName,
                     v8::Local<v8::Value> parameterValue, v8::Local<v8::Value> type) {
        Isolate *isolate = v8::Isolate::GetCurrent();
        this->parameterName.Reset(isolate, parameterName);
        this->parameterValue.Reset(isolate, parameterValue);
        this->type.Reset(isolate, type);
    }

    InputInfo::InputInfo(const InputInfo& inputInfo) {
        auto isolate = v8::Isolate::GetCurrent();

        if (!inputInfo.parameterName.IsEmpty()) {
          this->parameterName.Reset(isolate, inputInfo.parameterName);
        } else {
          this->parameterName.Reset();
        }

        if (!inputInfo.parameterValue.IsEmpty()) {
          this->parameterValue.Reset(isolate, inputInfo.parameterValue);
        } else {
          this->parameterValue.Reset();
        }

        if (!inputInfo.type.IsEmpty()) {
          this->type.Reset(isolate, inputInfo.type);
        } else {
          this->type.Reset();
        }
    }

    InputInfo::~InputInfo() {
        if (!this->parameterName.IsEmpty()) {
            this->parameterName.Reset();
        }
        if (!this->parameterValue.IsEmpty()) {
            this->parameterValue.Reset();
        }
        if (!this->type.IsEmpty()) {
            this->type.Reset();
        }
        if (this->inputInfoV8Container != nullptr) {
            delete this->inputInfoV8Container;
        }
    }

    InputInfo& InputInfo::operator=(const InputInfo& inputInfo) {
        return *this;
    }

    InputInfo* GetInputInfoFromJsObject(v8::Object* jsInputInfo, Isolate* isolate, v8::Local<v8::Context> context) {
        auto parameterName = jsInputInfo->Get(context, utils::NewV8String(isolate, "parameterName")).ToLocalChecked();
        auto parameterValue = jsInputInfo->Get(context, utils::NewV8String(isolate, "parameterValue")).ToLocalChecked();
        auto typeValue = jsInputInfo->Get(context, utils::NewV8String(isolate, "type")).ToLocalChecked();
        return new InputInfo(parameterName, parameterValue, typeValue);
    }

    v8::Local<v8::Object> GetJsObjectFromInputInfo(Isolate* isolate, v8::Local<v8::Context> context, InputInfo *inputInfo) {
        if (inputInfo->inputInfoV8Container == nullptr) {
            auto iinfo = v8::Object::New(isolate);
            auto parameterName = v8::Local<v8::Value>::New(isolate, inputInfo->parameterName);
            auto parameterValue = v8::Local<v8::Value>::New(isolate, inputInfo->parameterValue);
            auto type = v8::Local<v8::Value>::New(isolate, inputInfo->type);
            iinfo->Set(context, utils::NewV8String(isolate, "parameterName"), parameterName).CHECK();
            iinfo->Set(context, utils::NewV8String(isolate, "parameterValue"), parameterValue).CHECK();
            iinfo->Set(context, utils::NewV8String(isolate, "type"), type).CHECK();
            inputInfo->inputInfoV8Container = new InputInfoV8Container();
            inputInfo->inputInfoV8Container->inputInfoV8.Reset(isolate, iinfo);
            return iinfo;
        } else {
            return v8::Local<v8::Object>::New(isolate, inputInfo->inputInfoV8Container->inputInfoV8);
        }
    }
} // namespace tainted
} // namespace iast
#endif
