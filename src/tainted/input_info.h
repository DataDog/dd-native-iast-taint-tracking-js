/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_TAINTED_INPUT_INFO_H_
#define SRC_TAINTED_INPUT_INFO_H_

#include <v8.h>

namespace iast {
namespace tainted {

class InputInfoV8Container {
 public:
    v8::Persistent<v8::Object> inputInfoV8;
    ~InputInfoV8Container() {
        this->inputInfoV8.Reset();
    }
};

struct InputInfo {
    InputInfo(v8::Local<v8::Value> parameterName,
            v8::Local<v8::Value> parameterValue, v8::Local<v8::Value> type);
    InputInfo(const InputInfo& inputInfo);
    ~InputInfo();

    InputInfo& operator=(const InputInfo& inputInfo);

    v8::Persistent<v8::Value> parameterValue;
    v8::Persistent<v8::Value> parameterName;
    v8::Persistent<v8::Value> type;
    InputInfoV8Container* inputInfoV8Container = nullptr;
};


InputInfo* GetInputInfoFromJsObject(v8::Object* jsInputInfo, v8::Isolate* isolate, v8::Local<v8::Context> context);

v8::Local<v8::Object> GetJsObjectFromInputInfo(v8::Isolate* isolate,
        v8::Local<v8::Context> context,
        InputInfo *inputInfo);
}   // namespace tainted
}   // namespace iast
#endif  // SRC_TAINTED_INPUT_INFO_H_
