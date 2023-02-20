/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
#ifndef SRC_API_METRICS_H
#define SRC_API_METRICS_H

#include <linux/limits.h>
#include <node.h>
namespace iast {
namespace api {

enum class TelemetryVerbosity {
    OFF = 0,
    MANDATORY,
    INFORMATION,
    DEBUG,
    MAX
};

class Metrics {
 public:
    static void Init(v8::Local<v8::Object> exports);

 private:
    Metrics();
    ~Metrics();
};
}   // namespace api
}   // namespace iast

#endif  // SRC_API_METRICS_H

